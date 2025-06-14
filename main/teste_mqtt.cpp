#include "mqtt_client.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "MQTT";

esp_mqtt_client_handle_t client;

static void send_sensor_data() {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "temperatura", 24.5);
    cJSON_AddNumberToObject(root, "umidade", 56);
    char *msg = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(client, "esp32/dados", msg, 0, 0, 0);
    free(msg);
    cJSON_Delete(root);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado ao broker MQTT");
            esp_mqtt_client_subscribe(client, "esp32/comando", 0);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Comando recebido: %.*s", event->data_len, event->data);
            break;
        default:
            break;
    }
}

void app_main() {
    // Configure rede Wi-Fi ou Ethernet antes disso

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://192.168.1.100", // IP do seu PC
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    while (1) {
        send_sensor_data();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
