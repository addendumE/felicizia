#include "MqttClient.h"

static const char* TAG = "MqttClient";

MqttClient::MqttClient() : client(nullptr),connected(false) {
}

MqttClient::~MqttClient() {
    stop();
}

bool MqttClient::start(const std::string& uri) {
    if (client) {
        ESP_LOGW(TAG, "Il client MQTT è già in esecuzione.");
        return false;
    }

    esp_mqtt_client_config_t mqtt_cfg = {};
    // Nelle versioni di ESP-IDF 5.x, la configurazione si trova sotto 'broker'
    mqtt_cfg.broker.address.uri = uri.c_str();

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) {
        ESP_LOGE(TAG, "Errore durante l'inizializzazione del client MQTT.");
        return false;
    }

    // Registrazione all'event loop
    esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, this);
    
    if (esp_mqtt_client_start(client) != ESP_OK) {
        ESP_LOGE(TAG, "Impossibile avviare il client MQTT.");
        return false;
    }

    return true;
}

bool MqttClient::stop() {
    if (client) {
        esp_mqtt_client_stop(client);
        esp_mqtt_client_destroy(client);
        client = nullptr;
    }
    return true;
}

bool MqttClient::publish(const std::string& topic, const std::string& payload, int qos, int retain) {
    if (!client) return false;
    if (!connected) return false;
    ESP_LOGI(TAG,"publish %s %s",topic.c_str(),payload.c_str());
    int msg_id = esp_mqtt_client_publish(client, topic.c_str(), payload.c_str(), payload.length(), qos, retain);
    return msg_id != -1;
}

bool MqttClient::subscribe(const std::string& topic, int qos) {
    if (!client) return false;
    ESP_LOGI(TAG,"subscribe %s",topic.c_str());
    int msg_id = esp_mqtt_client_subscribe(client, topic.c_str(), qos);
    return msg_id != -1;
}

bool MqttClient::unsubscribe(const std::string& topic) {
    if (!client) return false;
    int msg_id = esp_mqtt_client_unsubscribe(client, topic.c_str());
    return msg_id != -1;
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

void MqttClient::mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    MqttClient* instance = static_cast<MqttClient*>(handler_args);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    if (instance) {
        instance->handleEvent(event);
    }
}

void MqttClient::handleEvent(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connesso!");
            connected = true;
            if (onConnectCb) onConnectCb();
            break;
        case MQTT_EVENT_DISCONNECTED:
            connected = false;
            ESP_LOGI(TAG, "MQTT disconnesso!");
            if (onDisconnectCb) onDisconnectCb();
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: {
            if (event->current_data_offset == 0)
            {
                topic = std::string(event->topic, event->topic_len);
            }
            // I parametri topic e data all'interno dell'evento non sono 'null-terminated' (terminati con '\0')
            
            if (onMessageCb) onMessageCb(topic, event->data, event->data_len,event->current_data_offset,event->total_data_len);
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            // Ignora gli altri eventi (come ACK vari)
            break;
    }
}
