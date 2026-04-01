#include "Websocket.h"
#include "esp_log.h"
#define TAG "WS"

Websocket::Websocket():
	Thread("WS"),
	context(NULL)
{
	memset(&protocols, 0, sizeof(protocols));
	protocols[0].name="http";
	protocols[0].callback = callback_http;
	protocols[0].user = this;
	protocols[1].name="http-api";
	protocols[1].callback = callback_api;
	protocols[1].per_session_data_size = sizeof(Websocket::my_post_buffer),
	protocols[1].user = this;
	protocols[2].name="ws";
	protocols[2].callback = callback_protocol;
	protocols[2].user = this;
	protocols[2].rx_buffer_size = RX_BUFFER_BYTES;
	memset(&mount_files, 0, sizeof(mount_files));
	mount_files = {
	   	.mount_next = &mount_api,      // IMPORTANTISSIMO: chain dei mount
	    .mountpoint = "/",                // URL base
	    .origin = "HTML",             // directory locale da servire
	    .def = "indexJQ.html",              // file di default
		.protocol = "http",
	    .origin_protocol = LWSMPRO_FILE,  // serve file dal filesystem
	    .mountpoint_len = 1
	};

	
	memset(&mount_api, 0, sizeof(mount_api));
	mount_api = {
    	.mountpoint = "/api",
		.protocol = "http-api", // <-- protocollo che intercetterà /api/*
    	.origin_protocol = LWSMPRO_CALLBACK,
		.mountpoint_len = 4
	};
		ESP_LOGI(TAG,"Websocket %p",this);
};

Websocket::~Websocket() {
}

void Websocket::start(int _port)
{
	port = 8080;
	Thread::start();
}



void Websocket::loop()
{
	ESP_LOGI(TAG,"WEBSOCKET");
	lws_set_log_level(LLL_NOTICE | LLL_INFO | LLL_ERR, NULL);
	lws_set_log_level(LLL_ERR, NULL);
	memset( &info, 0, sizeof(info) );
	info.port = port;
	info.protocols = protocols;
	info.mounts = &mount_files; // collega il mount
	context = lws_create_context( &info );
	while(true)
	{
		lws_service( context, 10);
	}
}


void Websocket::send(string s)
{
	if (clients.size() > 0)
	{
		Lock::take();
		txMessages.push_back(s);
		Lock::give();
		lws_callback_on_writable_all_protocol(context, &protocols[2]);
	}
}


int Websocket::callback_protocol( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	static string txBuffer;
	int txLen;
	lws_write_protocol write_mode;
	bool is_start = false;
	Websocket * me = (Websocket *) lws_get_protocol(wsi)->user;
	me->take();
	bool go = txBuffer.size()==0 && me->txMessages.size()>0;
	me->give();

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
			if (go)
			{
				me->take();
				txBuffer = me->txMessages.back();
				me->txMessages.pop_back();
				is_start = true;
				me->give();
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
				bool cont;
				me->take();
				cont = txLen || me->txMessages.size();
				me->give();
				if (cont) {
					lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
				}
			}

			break;
		default:
			break;
	}
	return 0;
}

int Websocket::callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	return lws_callback_http_dummy(wsi, reason, user, in, len);
}




int Websocket::callback_api(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len)
{
    struct my_post_buffer *buf = (my_post_buffer *)user;
	Websocket * me = (Websocket *) lws_get_protocol(wsi)->user;

    switch (reason) {
    case LWS_CALLBACK_HTTP: {
        const char *uri = (const char *)in;
		strcpy(buf->uri,uri);
 		if (strcmp(buf->uri,"/readConf") == 0) {
        	// Buffer per risposta
        	unsigned char buffer[LWS_PRE + 1024];
        	unsigned char *p = &buffer[LWS_PRE];
        	unsigned char *start = p;
        	unsigned char *end = buffer + sizeof(buffer);
        	string body;
			me->onConfigRead(body);
        	ESP_LOGI(TAG,"get conf: size is: %d",body.size());
        	// Header HTTP
        	lws_add_http_header_status(wsi, 200, &p, end);
        	lws_add_http_header_content_length(wsi, body.size(), &p, end);
        	lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_TYPE,
                                        (unsigned char *)"text/plain",
                                        10, &p, end);
			lws_add_http_header_by_name(wsi,
        		(unsigned char *)"Content-Disposition:",
        		(unsigned char *)"attachment; filename=\"config.conf\"",
        		34, &p, end);


	        lws_finalize_http_header(wsi, &p, end);
	
    	    // Scrivi header
        	if (lws_write(wsi, start, p - start, LWS_WRITE_HTTP_HEADERS) < 0)
            	return -1;

        	// Scrivi body
        	if (lws_write(wsi, (unsigned char *)body.c_str(),
                      body.size(), LWS_WRITE_HTTP_FINAL) < 0)
            	return -1;
	        	ESP_LOGI(TAG,"get conf: done");
			return -1;
        }

        break;
    }

    case LWS_CALLBACK_HTTP_BODY:
        memcpy(buf->data + buf->len, in, len);
        buf->len += len;
       	return 0;
        break;

    case LWS_CALLBACK_HTTP_BODY_COMPLETION: {
        // body Post completo
		if (strcmp(buf->uri,"/writeConf") == 0)
		{
			string s = string((char*)buf->data,buf->len);
			me->onConfigWrite(s);
			lws_return_http_status(wsi, HTTP_STATUS_OK, NULL);
		}
        return -1;
    }

    default:
        break;
    }

    return 0;
}
