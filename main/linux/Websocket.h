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
#include <libwebsockets.h>
#include <esp_log.h>
using namespace std;


#define RX_BUFFER_BYTES 2048

class Websocket: public Thread, public Lock {
public:
	Websocket();
	virtual ~Websocket();
	void start(int port);
	virtual void onMessage(char *) = 0;
	virtual void onOTAenter() = 0;
	virtual void onOTAexit() = 0;
	virtual void onConfigRead(string &s) = 0;
	virtual bool onConfigWrite(string&s) = 0;
	void loop();
	void send(string);
private:

struct my_post_buffer {
    char data[16384];
    size_t len = 0;
	char uri[64];
};

	vector <string> txMessages;
	string rxBuffer;
	int port;

	unsigned char txData[LWS_SEND_BUFFER_PRE_PADDING + RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
	struct lws_context *context;
	static int callback_protocol( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
	static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
	static int callback_api( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len );
	struct lws_protocols protocols[4];
	struct lws_protocol_vhost_options pvo_opt;
	struct lws_protocol_vhost_options pvo;
	struct lws_context_creation_info info;
	typedef struct lws * ws_client_id;
	struct lws_http_mount mount_files;
	struct lws_http_mount mount_api;
	list <struct lws *> clients;
};

#endif /* MAIN_WEBSOCKET_H_ */
