#pragma once

#include <string>
#include <functional>
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"

class MqttClient {
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
    esp_mqtt_client_handle_t client;
    MessageCallback onMessageCb;
    ConnectCallback onConnectCb;
    DisconnectCallback onDisconnectCb;
    bool connected;
    // Gestore degli eventi statico richiesto da esp_event
    static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data);
    void handleEvent(esp_mqtt_event_handle_t event);
};
