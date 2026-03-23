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
#include "esp_system.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include <esp_http_server.h>
#include "lwip/apps/fs.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_image_format.h"

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
	list <ws_client_id> clients;
};

#endif /* MAIN_WEBSOCKET_H_ */
