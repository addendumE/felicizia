/*
 * Websocket.cpp
 *
 *  Created on: 1 ott 2021
 *      Author: maurizio
 */

#include "Websocket.h"
#include "esp_log.h"
#define TAG "WS"

Websocket::Websocket()
{
	server = NULL;
    memset(&ws_uri, 0, sizeof(httpd_uri_t));
    memset(&index_uri, 0, sizeof(httpd_uri_t));
    memset(&ota_uri, 0, sizeof(httpd_uri_t));
    memset(&readConf_uri, 0, sizeof(httpd_uri_t));
    memset(&writeConf_uri, 0, sizeof(httpd_uri_t));

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

	ota_uri.uri       = "/api/update";
	ota_uri.method    = HTTP_POST;
	ota_uri.handler   = callback_http_upload;
	ota_uri.user_ctx  = this;
	ota_uri.is_websocket = false;

	writeConf_uri.uri       = "/api/writeConf";
	writeConf_uri.method    = HTTP_POST;
	writeConf_uri.handler   = callback_http_writeConf;
	writeConf_uri.user_ctx  = this;
	writeConf_uri.is_websocket = false;

	readConf_uri.uri       = "/api/readConf";
	readConf_uri.method    = HTTP_GET;
	readConf_uri.handler   = callback_http_readConf;
	readConf_uri.user_ctx  = this;
	readConf_uri.is_websocket = false;
}

Websocket::~Websocket() {
}

void Websocket::start(int port)
{
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
		httpd_register_uri_handler(server, &readConf_uri);
		httpd_register_uri_handler(server, &writeConf_uri);
	    httpd_register_uri_handler(server, &index_uri);



	}
}


void Websocket::send(const std::string &s)
{
	Lock::take();
	for (auto it = clients.begin(); it != clients.end(); ) {
    	if (ws_send_chunked_sync(*it,(const uint8_t *)s.c_str(), s.size(),1024) != ESP_OK) {
        	ESP_LOGI(TAG, "client removed %d", *it);
        	it = clients.erase(it); // erase ritorna il prossimo iteratore valido
    	} else {
        	++it; // solo avanzare se non cancelliamo
    	}
	}
	Lock::give();	
}




void Websocket::ws_send_chunk_cb(void *arg)
{
    ws_chunk_ctx_t *ctx = (ws_chunk_ctx_t *)arg;
	ESP_LOGI(TAG, "ws_send_chunk_cb enter");
    httpd_ws_frame_t frame;
    memset(&frame, 0, sizeof(frame));

    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = (uint8_t *)(ctx->data + ctx->offset);
    frame.len = ctx->to_send;
    httpd_ws_send_frame_async(ctx->server, ctx->sockfd, &frame);
    xSemaphoreGive(ctx->done);   // ✅ segnala che questo chunk è stato inviato
	ESP_LOGI(TAG, "ws_send_chunk_cb exit");
}



esp_err_t Websocket::ws_send_chunked_sync(
                               int sockfd,
                               const uint8_t *data,
                               size_t len,
                               size_t chunk_size)
{
    ws_chunk_ctx_t ctx = {
        .server = server,
        .sockfd = sockfd,
        .data   = data,
        .offset = 0,
        .done   = xSemaphoreCreateBinary()
    };

    if (!ctx.done)
        return ESP_ERR_NO_MEM;

    esp_err_t ret = ESP_OK;
   	ESP_LOGI(TAG, "ws_send_chunked_sync enter");
    while (ctx.offset < len) {

        ctx.to_send = len - ctx.offset;
        if (ctx.to_send > chunk_size)
            ctx.to_send = chunk_size;

        // ✅ invia questo chunk dentro thread del webserver
	   	ESP_LOGI(TAG, "ws_send_chunked_sync enter 1");
        ret = httpd_queue_work(server, ws_send_chunk_cb, &ctx);
        if (ret != ESP_OK) break;
		ESP_LOGI(TAG, "ws_send_chunked_sync enter 2");
		vTaskDelay(1);
        // ✅ aspetta che il webserver abbia completato l’invio
        if (xSemaphoreTake(ctx.done, portMAX_DELAY) != pdTRUE) {
            ret = ESP_ERR_TIMEOUT;
            break;
        }
	ESP_LOGI(TAG, "ws_send_chunked_sync enter 3");
        ctx.offset += ctx.to_send;
    }
   	ESP_LOGI(TAG, "ws_send_chunked_sync exit");
    vSemaphoreDelete(ctx.done);
    return ret;
}



#define WS_CHUNK_SIZE 16*1024  

// Funzione di invio frammentato
esp_err_t Websocket::ws_send_large_buffer(httpd_req_t *req, const uint8_t *data, size_t len)
{
    size_t sent = 0;
   	ESP_LOGI(TAG, "send to %p ",req);
    while (sent < len) {
        size_t chunk = (len - sent) > WS_CHUNK_SIZE ? WS_CHUNK_SIZE : (len - sent);

        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(ws_pkt));
        ws_pkt.type    = HTTPD_WS_TYPE_TEXT;
        ws_pkt.payload = (uint8_t *)(data + sent);
        ws_pkt.len     = chunk;
        ws_pkt.final   = ((sent + chunk) >= len);
        esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Errore invio frame");
            return ret;
        }
        sent += chunk;
    }
    return ESP_OK;
}

#include <algorithm>

esp_err_t Websocket::callback_protocol(httpd_req_t *req)
{
	Websocket * me = (Websocket *) req->user_ctx;

	if (req->method == HTTP_GET) {
		me->take();
		int fd = httpd_req_to_sockfd(req);
		auto result = me->clients.insert(fd);
       	ESP_LOGI(TAG, "client added %d -> %d",fd,result.second);
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

esp_err_t Websocket::callback_http(httpd_req_t *req)
{
    struct fs_file file;
	const char *uri = req->uri;
    // Se URI è "/", servi index.html
    if (strcmp(uri, "/") == 0) {
        uri = "/index.html";
    }
    int err = fs_open(&file,uri);
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

esp_err_t Websocket::callback_http_readConf(httpd_req_t *req)
{
	Websocket &me = *(Websocket*) req->user_ctx;
	string tmp;
	me.onConfigRead(tmp);
	httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, (char*)tmp.c_str(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t Websocket::callback_http_writeConf(httpd_req_t *req)
{
	Websocket &me = *(Websocket*) req->user_ctx;
	int total_len = req->content_len;
 	std::string body;
    body.resize(total_len);  // ✅ alloca buffer interno
    int received = 0;
    while (received < total_len) {
        int ret = httpd_req_recv(
            req,
            body.data() + received,           // 👈 scrittura diretta
            total_len - received
        );

        if (ret <= 0) {
            return ESP_FAIL;
        }
        received += ret;
    }
	me.onConfigWrite(body);
    httpd_resp_sendstr(req, "conf written");
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
