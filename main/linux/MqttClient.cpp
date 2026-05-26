#include "MqttClient.h"
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <regex>

static const char *TAG = "MQTT_CLIENT_LINUX";

MqttClient::MqttClient() : Thread("MqttClient"), mosq(nullptr), port(1883), keepalive(60), connected(false) {
    mosquitto_lib_init();
}

MqttClient::~MqttClient() {
    stop();
    if (mosq) {
        mosquitto_destroy(mosq);
        mosq = nullptr;
    }
    mosquitto_lib_cleanup();
}

bool MqttClient::start(const std::string& uri) {
    ESP_LOGI(TAG, "Starting MQTT client with URI: %s", uri.c_str());

    std::regex uri_regex("(mqtt|tcp)://([a-zA-Z0-9\.-]+)(:([0-9]+))?");
    std::smatch matches;

    if (std::regex_match(uri, matches, uri_regex)) {
        host = matches[2].str();
        if (matches[4].matched) {
            port = std::stoi(matches[4].str());
        }
    } else {
        ESP_LOGE(TAG, "Invalid MQTT URI format: %s", uri.c_str());
        return false;
    }

    ESP_LOGI(TAG, "Connecting to host: %s, port: %d", host.c_str(), port);

    mosq = mosquitto_new(NULL, true, this);
    if (!mosq) {
        ESP_LOGE(TAG, "Failed to create mosquitto instance");
        return false;
    }

    mosquitto_connect_callback_set(mosq, MqttClient::on_connect_callback);
    mosquitto_disconnect_callback_set(mosq, MqttClient::on_disconnect_callback);
    mosquitto_publish_callback_set(mosq, MqttClient::on_publish_callback);
    mosquitto_message_callback_set(mosq, MqttClient::on_message_callback);
    mosquitto_subscribe_callback_set(mosq, MqttClient::on_subscribe_callback);
    mosquitto_unsubscribe_callback_set(mosq, MqttClient::on_unsubscribe_callback);

    int ret = mosquitto_connect(mosq, host.c_str(), port, keepalive);
    if (ret != MOSQ_ERR_SUCCESS) {
        ESP_LOGE(TAG, "mosquitto_connect failed: %s", mosquitto_strerror(ret));
        return false;
    }

    // Start the mosquitto loop in a separate thread
    Thread::start();
    return true;
}

bool MqttClient::stop() {
    ESP_LOGI(TAG, "Stopping MQTT client");
    if (mosq) {
        mosquitto_disconnect(mosq);
        Thread::stop(); // Stop the loop thread
        return true;
    }
    return false;
}

bool MqttClient::publish(const std::string& topic, const std::string& payload, int qos, int retain) {
    if (!mosq || !connected) {
        ESP_LOGE(TAG, "Not connected to MQTT broker, cannot publish");
        return false;
    }
    int ret = mosquitto_publish(mosq, nullptr, topic.c_str(), payload.length(), payload.c_str(), qos, retain);
    if (ret != MOSQ_ERR_SUCCESS) {
        ESP_LOGE(TAG, "mosquitto_publish failed: %s", mosquitto_strerror(ret));
        return false;
    }
    return true;
}

bool MqttClient::subscribe(const std::string& topic, int qos) {
    if (!mosq || !connected) {
        ESP_LOGE(TAG, "Not connected to MQTT broker, cannot subscribe");
        return false;
    }
    int ret = mosquitto_subscribe(mosq, nullptr, topic.c_str(), qos);
    if (ret != MOSQ_ERR_SUCCESS) {
        ESP_LOGE(TAG, "mosquitto_subscribe failed: %s", mosquitto_strerror(ret));
        return false;
    }
    return true;
}

bool MqttClient::unsubscribe(const std::string& topic) {
    if (!mosq || !connected) {
        ESP_LOGE(TAG, "Not connected to MQTT broker, cannot unsubscribe");
        return false;
    }
    int ret = mosquitto_unsubscribe(mosq, nullptr, topic.c_str());
    if (ret != MOSQ_ERR_SUCCESS) {
        ESP_LOGE(TAG, "mosquitto_unsubscribe failed: %s", mosquitto_strerror(ret));
        return false;
    }
    return true;
}

void MqttClient::setOnMessageCallback(MessageCallback cb) {
    onMessageCb = cb;
}

void MqttClient::setOnConnectCallback(ConnectCallback cb) {
    onConnectCb = cb;
}

void MqttClient::setOnDisconnectCallback(DisconnectCallback cb) {
    onDisconnectCb = cb;
}

void MqttClient::loop() {
    if (mosq) {
        // mosquitto_loop_forever handles reconnects internally
        int ret = mosquitto_loop_forever(mosq, -1, 1);
        ESP_LOGI(TAG, "mosquitto_loop_forever exited with code: %d", ret);
    }
}

// Static libmosquitto callbacks
void MqttClient::on_connect_callback(struct mosquitto *mosq, void *obj, int rc) {
    MqttClient *client = static_cast<MqttClient*>(obj);
    if (rc == 0) {
        ESP_LOGI(TAG, "Connected to MQTT broker successfully");
        client->connected = true;
        if (client->onConnectCb) {
            client->onConnectCb();
        }
    } else {
        ESP_LOGE(TAG, "Connection failed: %s", mosquitto_connack_string(rc));
        client->connected = false;
    }
}

void MqttClient::on_disconnect_callback(struct mosquitto *mosq, void *obj, int rc) {
    MqttClient *client = static_cast<MqttClient*>(obj);
    ESP_LOGI(TAG, "Disconnected from MQTT broker (RC: %d)", rc);
    client->connected = false;
    if (client->onDisconnectCb) {
        client->onDisconnectCb();
    }
}

void MqttClient::on_publish_callback(struct mosquitto *mosq, void *obj, int mid) {
    // MqttClient *client = static_cast<MqttClient*>(obj);
    //ESP_LOGD(TAG, "Message with mid %d has been published.", mid);
}

void MqttClient::on_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    MqttClient *client = static_cast<MqttClient*>(obj);
    if (message->payloadlen) {
        std::string topic(message->topic);
        if (client->onMessageCb) {
            client->onMessageCb(topic, (const char*)message->payload, message->payloadlen,0,0);
        }
    } else {
        ESP_LOGI(TAG, "Received message (null payload): Topic: %s", message->topic);
    }
}

void MqttClient::on_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
    // MqttClient *client = static_cast<MqttClient*>(obj);
    ESP_LOGD(TAG, "Subscribed (mid: %d)", mid);
    for (int i = 0; i < qos_count; i++) {
        ESP_LOGD(TAG, "  QoS granted: %d", granted_qos[i]);
    }
}

void MqttClient::on_unsubscribe_callback(struct mosquitto *mosq, void *obj, int mid) {
    // MqttClient *client = static_cast<MqttClient*>(obj);
    ESP_LOGD(TAG, "Unsubscribed (mid: %d)", mid);
}
