/*
 * Websocket.cpp
 *
 *  Created on: 1 ott 2021
 *      Author: maurizio
 */

#include "Websocket.h"
#include "esp_log.h"
#define TAG "WS"

Websocket::Websocket():
#ifdef LINUX_PLATFORM
	Thread("WS"),
	context(NULL)
#else
	Thread("WS")
#endif

{
#ifdef LINUX_PLATFORM
	memset(&protocols, 0, sizeof(protocols));
	protocols[0].name="http-only";
	protocols[0].callback = callback_http;
	protocols[0].user = this;
	protocols[1].name="default";
	protocols[1].callback = callback_protocol;
	protocols[1].user = this;
	protocols[1].rx_buffer_size = RX_BUFFER_BYTES;
#else
	server = NULL;
    memset(&ws_uri, 0, sizeof(httpd_uri_t));
    memset(&index_uri, 0, sizeof(httpd_uri_t));
    memset(&ota_uri, 0, sizeof(httpd_uri_t));

	ws_uri.uri = "/";
	ws_uri.method     = HTTP_GET;
	ws_uri.handler    = callback_protocol;
	ws_uri.user_ctx   = this;
	ws_uri.is_websocket = true;
	ws_uri.supported_subprotocol = "ws";

	index_uri.uri       = "/*";
	index_uri.method    = HTTP_GET;
	index_uri.handler   = callback_http;
	index_uri.user_ctx  = this;
	index_uri.is_websocket = false;

	ota_uri.uri       = "/update";
	ota_uri.method    = HTTP_POST;
	ota_uri.handler   = callback_http_upload;
	ota_uri.user_ctx  = this;
	ota_uri.is_websocket = false;
#endif
}

Websocket::~Websocket() {
}

void Websocket::start(int port)
{
#ifdef LINUX_PLATFORM
	pvo_opt = {
	    NULL, NULL, "default", "1"
	};
	pvo = {
	    NULL, &pvo_opt, "default", ""
	};

	memset( &info, 0, sizeof(info) );

	info.port = port;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.pvo = &pvo;

	context = lws_create_context( &info );
	Thread::start();
#else
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.uri_match_fn = httpd_uri_match_wildcard;  // Abilita wildcard
	config.server_port = port;
	// Start the httpd server
	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) == ESP_OK) {
	    // Registering the ws handler
	    ESP_LOGI(TAG, "Registering URI handlers");
	    httpd_register_uri_handler(server, &ota_uri);
		httpd_register_uri_handler(server, &ws_uri);
	    httpd_register_uri_handler(server, &index_uri);

	}
#endif
}



void Websocket::loop()
{
#ifdef LINUX_PLATFORM
	while(true)
	{
		lws_service( context, 0);
	}
#endif
}

//void Websocket::onMessage(std::function <void(string)> f)
//{
//	onMessageNotify = f;
//}

void Websocket::send(string s)
{
	Lock::take();
	if (clients.size() > 0 && txMessages.size()<10)
	{
		txMessages.push_back(s);
	#ifdef LINUX_PLATFORM
		lws_callback_on_writable_all_protocol(context, &protocols[1]);
	#else
		trigger_aync_send();
	#endif
	}
	Lock::give();
}


#ifdef LINUX_PLATFORM
int Websocket::callback_protocol( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	static string txBuffer="";
	int txLen;
	lws_write_protocol write_mode;
	bool is_start=false;
	Websocket * me = (Websocket *) lws_get_protocol(wsi)->user;
	switch( reason ) {
			case LWS_CALLBACK_CLOSED:
				ESP_LOGI(TAG,"CLOSED %p",wsi);
				me->take();
				me->clients.remove(wsi);
				me->give();
				break;
			case LWS_CALLBACK_ESTABLISHED:
				ESP_LOGI(TAG,"ESTABLSHED %p",wsi);
				me->take();
				me->clients.push_back(wsi);
				me->give();
				break;
		case LWS_CALLBACK_RECEIVE:
			me->rxBuffer += string((char*)in,len);
			if (lws_is_final_fragment(wsi))
			{
				me->onMessage(me->rxBuffer);
				me->rxBuffer="";
			}
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			if (txBuffer.size()==0 && me->txMessages.size()>0)
			{
				txBuffer = me->txMessages.back();
				me->txMessages.pop_back();
				is_start = true;
			}
			txLen = txBuffer.size();
			if (txLen >0)
			{
				if (txLen > RX_BUFFER_BYTES) {
					txLen = RX_BUFFER_BYTES;
				}
				memcpy( &me->txData[LWS_SEND_BUFFER_PRE_PADDING], txBuffer.c_str(),txLen  );

				write_mode = (lws_write_protocol)lws_write_ws_flags(LWS_WRITE_TEXT,is_start,txBuffer.size()<=RX_BUFFER_BYTES);
				lws_write( wsi, &me->txData[LWS_SEND_BUFFER_PRE_PADDING], txLen, write_mode );
				txBuffer = txBuffer.substr(txLen);
				txLen = txBuffer.size();
				if (txLen) {
					lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
				}
			}
			break;
		default:
			break;
	}
	return 0;
}
#else
void Websocket::trigger_aync_send()
{
	for (auto c:clients)
	{
	    struct async_resp_arg *resp_arg = (struct async_resp_arg *) malloc(sizeof(struct async_resp_arg));
		 resp_arg->ws = this;
		 resp_arg->client = c;
		 httpd_queue_work(server, ws_async_send, resp_arg);
	}
}

void Websocket::ws_async_send(void * arg)
{
	static string txBuffer;
	static bool fragmented;
    struct async_resp_arg *resp_arg = (struct async_resp_arg *) arg;
    Websocket * me = resp_arg->ws;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    me->take();
	if (txBuffer.size()==0 && resp_arg->ws->txMessages.size()>0)
	{
	    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
		txBuffer = resp_arg->ws->txMessages.back();
		resp_arg->ws->txMessages.pop_back();
		if (txBuffer.size() > RX_BUFFER_BYTES) {
			fragmented = true;
		}
		else
		{
			fragmented = false;
		}
	}
	else
	{
	    ws_pkt.type = HTTPD_WS_TYPE_CONTINUE;
	}

	int txLen = txBuffer.size();
	if (txLen >0)
	{
		if (txLen > RX_BUFFER_BYTES) {
			txLen = RX_BUFFER_BYTES;
		}

	    ws_pkt.payload = (uint8_t*)txBuffer.c_str(),
	    ws_pkt.len = txLen;
	    ws_pkt.final = txBuffer.size()<=RX_BUFFER_BYTES;
	    ws_pkt.fragmented = fragmented;

	    esp_err_t ret = httpd_ws_send_frame_async(me->server, resp_arg->client, &ws_pkt);
	    if (ret != ESP_OK)
	    {
		    ESP_LOGI(TAG,"client %d disconnected",resp_arg->client);
		    me->clients.remove(resp_arg->client);
	    }
		txBuffer = txBuffer.substr(txLen);
		if (txBuffer.size()) {
			me->trigger_aync_send();
		}
		free(resp_arg);

	}
	me->give();
}

esp_err_t Websocket::callback_protocol(httpd_req_t *req)
{
	Websocket * me = (Websocket *) req->user_ctx;

	if (req->method == HTTP_GET) {
		int fd = httpd_req_to_sockfd(req);
		me->take();
		ESP_LOGI(TAG, "New connection on %d",fd);
		me->clients.push_back(fd);
		me->give();
	    return ESP_OK;
	}

	httpd_ws_frame_t ws_pkt;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	/* Set max_len = 0 to get the frame len */
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
	    return ret;
	}
	ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
	if (ws_pkt.len) {
		uint8_t *buf = NULL;
		/* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
	    buf = (uint8_t *) calloc(1, ws_pkt.len + 1);
	    if (buf == NULL) {
	    	ESP_LOGE(TAG, "Failed to calloc memory for buf");
	        return ESP_ERR_NO_MEM;
	    }
	    ws_pkt.payload = buf;
	    /* Set max_len = ws_pkt.len to get the frame payload */
	    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
	  //  if (me->onMessageNotify) {
	    //	me->onMessageNotify(string((char*)ws_pkt.payload, (size_t)ws_pkt.len));
	    //}
	    
	    ESP_LOGI(TAG, "Got packet with message: %s [%d] frag:%d  final:%d", ws_pkt.payload,
	    		ws_pkt.len,ws_pkt.fragmented,ws_pkt.final);
		me->onMessage(string((char*)ws_pkt.payload, (size_t)ws_pkt.len));
	    free(buf);
	}
	
	//if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
	//	free(buf);
	 //   return trigger_async_send(req->handle, req);
	//}

	//ret = httpd_ws_send_frame(req, &ws_pkt);
	//if (ret != ESP_OK) {
	//	ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
	//}
	return ret;
}
#endif

#ifdef LINUX_PLATFORM
int Websocket::callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	const char * mime;
	const char * file = (const char *) in;

	switch( reason )
	{
		case LWS_CALLBACK_HTTP:
			if (strcmp(file,"/")== 0) file = "/index.html";
			mime = lws_get_mimetype(file,NULL);
			lws_serve_http_file( wsi, &file[1], mime, NULL, 0 );
			break;
		default:
			break;
	}

	return 0;
}
#else
esp_err_t Websocket::callback_http(httpd_req_t *req)
{
    struct fs_file file;
	const char *uri = req->uri;
    // Se URI è "/", servi index.html
    if (strcmp(uri, "/") == 0) {
        uri = "/index.html";
    }
	ESP_LOGI(TAG,"opening %s",uri);
    int err = fs_open(&file,uri);
	ESP_LOGI(TAG,"open result %d",err);
    if(!err) {
		 // Determina il Content-Type dal file
        const char *content_type = "text/html";
        if (strstr(uri, ".css")) {
            content_type = "text/css";
        } else if (strstr(uri, ".js")) {
            content_type = "application/javascript";
        } else if (strstr(uri, ".png")) {
            content_type = "image/png";
        } else if (strstr(uri, ".jpg") || strstr(uri, ".jpeg")) {
            content_type = "image/jpeg";
        } else if (strstr(uri, ".ico")) {
            content_type = "image/x-icon";
        }
        
        httpd_resp_set_type(req, content_type);
        httpd_resp_send(req, file.data, file.len);
    }
	else 
	{
		httpd_resp_send_404(req);
	}
    return ESP_OK;
}

#define TMP_SIZE 8192
esp_err_t Websocket::callback_http_upload(httpd_req_t *req)
{
	 Websocket &me = *(Websocket*) req->user_ctx;
	 int total_len = req->content_len;
	 /*if (total_len >= SCRATCH_BUFSIZE) {
	    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "content too long");
	    return ESP_FAIL;
	 }*/
	 me.start_ota();
	 char *tmp = (char*)malloc(TMP_SIZE);
	 while (total_len > 0)
	 {
		 int received = httpd_req_recv(req, tmp, (total_len > TMP_SIZE) ? TMP_SIZE:total_len);
		 esp_err_t err = esp_ota_write(me.update_handle, (const void *)tmp, received);
		 if (err != ESP_OK)
		  {
			  ESP_LOGE(TAG,"esp_ota_write %d",err);
			  httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "esp_ota_write");
			  return ESP_FAIL;
		  }
		  total_len -= received;
	 }
	 free (tmp);

	 esp_err_t err = esp_ota_end(me.update_handle);
	 if (err != ESP_OK) {
		 if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
			 ESP_LOGE(TAG, "Image validation failed, image is corrupted");
	     }
	     ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
		  httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "esp_ota_end");
	     return ESP_FAIL;
	 }
	 err = esp_ota_set_boot_partition(me.update_partition);
	 if (err != ESP_OK) {
	     ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
		  httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "esp_ota_set_boot_partition");
	     return ESP_FAIL;
	 }
    httpd_resp_sendstr(req, "Fw upgrade success");
    me.onOTAexit();
    return ESP_OK;
}


esp_err_t Websocket::start_ota(void)
{
    esp_err_t err;
    ESP_LOGI(TAG, "Starting OTA");
    onOTAenter();
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();
    if(configured==NULL || running == NULL)
    {
        ESP_LOGE(TAG,"OTA data not found");
        return ESP_FAIL;
    }

    if (configured != running)
    {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08lx, but running from offset 0x%08lx",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08lx)",
             running->type, running->subtype, running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%lx",
             update_partition->subtype, update_partition->address);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_ota_begin failed ");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    return ESP_OK;
}
#endif
