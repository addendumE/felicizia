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
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

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
	virtual void onConfigRead(string &s) = 0;
	virtual bool onConfigWrite(string&s) = 0;
	void send(const string&);
private:
	QueueHandle_t xQueueTx;
	void loop();
	struct ws_tx_job_t {
    	httpd_handle_t httpd;
    	int client_fd;
    	httpd_ws_frame_t frm;
	};

	list <int> clients;


	static esp_err_t callback_http(httpd_req_t *req);
	static esp_err_t callback_http_upload(httpd_req_t *req);
	static esp_err_t callback_http_readConf(httpd_req_t *req);
	static esp_err_t callback_http_writeConf(httpd_req_t *req);
	static esp_err_t callback_protocol(httpd_req_t *req);
	static void ws_tx_work_cb(void *arg);

	bool ws_enqueue_fragmented_text(const std::string& msg);

	esp_err_t start_ota();
	httpd_handle_t server;
	httpd_uri_t ws_uri;
	httpd_uri_t index_uri;
	httpd_uri_t ota_uri;
	httpd_uri_t readConf_uri;
	httpd_uri_t writeConf_uri;
	esp_ota_handle_t update_handle;
	const esp_partition_t *update_partition;
};

#endif /* MAIN_WEBSOCKET_H_ */
