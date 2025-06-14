#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_eth.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define ETH_PHY_ADDR 1
#define ETH_MDC_GPIO 23
#define ETH_MDIO_GPIO 18
#define ETH_RST_GPIO -1
#define BROKER_URI   "mqtt://192.168.1.10" // IP do PC com Mosquitto

static const char *TAG = "MQTT_ETH";
static esp_mqtt_client_handle_t mqtt_client = NULL;

static void mqtt_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT conectado");
            esp_mqtt_client_subscribe(mqtt_client, "esp32/comando", 0);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "Comando recebido: %.*s", event->data_len, event->data);
            break;
        default:
            break;
    }
}

static void publish_task(void *pvParameters) {
    while (1) {
        const char *msg = "{\"id\":\"esteira1\",\"valor\":42}";
        esp_mqtt_client_publish(mqtt_client, "esp32/dados", msg, 0, 1, 0);
        ESP_LOGI(TAG, "Mensagem enviada: %s", msg);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void got_ip_event(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    ESP_LOGI(TAG, "IP recebido: " IPSTR, IP2STR(&event->ip_info.ip));

    // Iniciar cliente MQTT após obter IP
    esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = BROKER_URI,
};

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    // Iniciar tarefa de publicação
    xTaskCreate(publish_task, "publish_task", 4096, NULL, 5, NULL);
}

void ethernet_init(void) {
    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&netif_cfg);

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    emac_config.smi_mdc_gpio_num = ETH_MDC_GPIO;
    emac_config.smi_mdio_gpio_num = ETH_MDIO_GPIO;
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = ETH_PHY_ADDR;
    phy_config.reset_gpio_num = ETH_RST_GPIO;
    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;

    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);


    // Pulsar GPIO16
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << 16),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    gpio_set_level(GPIO_NUM_16, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Registra evento de IP antes de iniciar a Ethernet
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event, NULL));

    // Ethernet init
    ethernet_init();
}
