/*
 * Websocket.h
 *
 *  Created on: 1 ott 2021
 *      Author: maurizio
 */

#ifndef MAIN_WEBSOCKET_H_
#define MAIN_WEBSOCKET_H_

#include <string>
#include <list>
#include <vector>
#include <functional>
#include "Thread.h"
#include "Lock.h"
#ifdef LINUX_PLATFORM
#include <libwebsockets.h>
#else
#include "esp_system.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
//#include "esp_eth.h"
#include <esp_http_server.h>
#include "lwip/apps/fs.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_image_format.h"

#endif
using namespace std;


#define RX_BUFFER_BYTES 2048

class Websocket: public Thread, public Lock {
public:
	Websocket();
	virtual ~Websocket();
	void start(int port);
	virtual void onMessage(const string&) = 0;
	virtual void onOTAenter() = 0;
	virtual void onOTAexit() = 0;
	void loop();
	void send(string);
private:
	vector <string> txMessages;
	string rxBuffer;

	//function <void(string)> onMessageNotify;


#ifdef LINUX_PLATFORM
	unsigned char txData[LWS_SEND_BUFFER_PRE_PADDING + RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
	struct lws_context *context;
	static int callback_protocol( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
	static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
	struct lws_protocols protocols[3];
	struct lws_protocol_vhost_options pvo_opt;
	struct lws_protocol_vhost_options pvo;
	struct lws_context_creation_info info;
	typedef struct lws * ws_client_id;
#else
	typedef int ws_client_id;
	struct async_resp_arg {
		Websocket * ws;
		ws_client_id client;
	};
	void trigger_aync_send();
	static void ws_async_send(void * arg);
	static esp_err_t callback_http(httpd_req_t *req);
	static esp_err_t callback_http_upload(httpd_req_t *req);
	static esp_err_t callback_protocol(httpd_req_t *req);

	esp_err_t start_ota();
	httpd_handle_t server;
	httpd_uri_t ws_uri;
	httpd_uri_t index_uri;
	httpd_uri_t ota_uri;
	esp_ota_handle_t update_handle;
	const esp_partition_t *update_partition;
#endif
	list <ws_client_id> clients;
};

#endif /* MAIN_WEBSOCKET_H_ */
