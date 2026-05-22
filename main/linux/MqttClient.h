#pragma once

#include <string>
#include <functional>
#include <mosquitto.h> // For libmosquitto
#include "esp_log.h"   // For logging macros like ESP_LOGI
#include "Thread.h"    // To run mosquitto_loop_forever in a separate thread
#include "Lock.h"      // For thread safety, if needed

class MqttClient : public Thread, public Lock { // Inherit from Thread to run the loop, and Lock for thread safety
public:
    // Tipi per le callback
    using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;
    using ConnectCallback = std::function<void()>;
    using DisconnectCallback = std::function<void()>;

    MqttClient();
    virtual ~MqttClient();

    bool start(const std::string& uri);
    bool stop();
    bool publish(const std::string& topic, const std::string& payload, int qos = 0, int retain = 0);
    bool subscribe(const std::string& topic, int qos = 0);
    bool unsubscribe(const std::string& topic);

    // Metodi per registrare le callback C++
    void setOnMessageCallback(MessageCallback cb);
    void setOnConnectCallback(ConnectCallback cb);
    void setOnDisconnectCallback(DisconnectCallback cb);

private:
    struct mosquitto *mosq;
    std::string host;
    int port;
    int keepalive;
    
    MessageCallback onMessageCb;
    ConnectCallback onConnectCb;
    DisconnectCallback onDisconnectCb;
    bool connected;

    // libmosquitto callbacks
    static void on_connect_callback(struct mosquitto *mosq, void *obj, int rc);
    static void on_disconnect_callback(struct mosquitto *mosq, void *obj, int rc);
    static void on_publish_callback(struct mosquitto *mosq, void *obj, int mid);
    static void on_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
    static void on_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
    static void on_unsubscribe_callback(struct mosquitto *mosq, void *obj, int mid);

    // Thread loop implementation
    void loop() override;
};