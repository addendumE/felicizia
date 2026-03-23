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
	protocols[1].name="ws";
	protocols[1].callback = callback_protocol;
	protocols[1].user = this;
	protocols[1].rx_buffer_size = RX_BUFFER_BYTES;
	memset(&mount, 0, sizeof(mount));

	mount = {
	    .mountpoint = "/",                // URL base
	    .origin = "HTML",             // directory locale da servire
	    .def = "index.html",              // file di default
	    .protocol = NULL,                 // usa protocollo HTTP di default
	    .cgienv = NULL,
	    .extra_mimetypes = NULL,
	    .interpret = NULL,
	    .cgi_timeout = 0,
	    .cache_max_age = 0,
	    .auth_mask = 0,
	    .cache_reusable = 0,
	    .cache_revalidate = 0,
	    .cache_intermediaries = 0,
	    .origin_protocol = LWSMPRO_FILE,  // serve file dal filesystem
	    .mountpoint_len = 1,
	    .basic_auth_login_file = NULL
	};
	ESP_LOGI(TAG,"Websocket %p",this);

}

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
	info.mounts = &mount; // collega il mount
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
		lws_callback_on_writable_all_protocol(context, &protocols[1]);
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
