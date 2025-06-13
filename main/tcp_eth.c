#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_eth.h"
#include "driver/gpio.h"

#define HOST_IP_ADDR    "192.168.1.10"
#define PORT            5000

#define ETH_PHY_ADDR    1
#define ETH_MDC_GPIO    23
#define ETH_MDIO_GPIO   18
#define ETH_RST_GPIO    -1

static const char *TAG = "eth_client";
static esp_netif_t *eth_netif = NULL;

static void got_ip_event(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;
    ESP_LOGI(TAG, "IP acquired: " IPSTR, IP2STR(&ip_info->ip));
}

void ethernet_init(void)
{
    ESP_LOGI(TAG, "Initializing Ethernet...");

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_cfg);

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_esp32_emac_config_t emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    emac_config.smi_mdc_gpio_num = ETH_MDC_GPIO;
    emac_config.smi_mdio_gpio_num = ETH_MDIO_GPIO;

    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&emac_config, &mac_config);

    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = ETH_PHY_ADDR;
    phy_config.reset_gpio_num = ETH_RST_GPIO;

    esp_eth_phy_t *phy = esp_eth_phy_new_lan87xx(&phy_config);

    esp_eth_config_t eth_config = ETH_DEFAULT_CONFIG(mac, phy);

    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config, &eth_handle));
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));
    ESP_ERROR_CHECK(esp_eth_start(eth_handle));
}

void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    const char *payload = "esteira1";

    while (1) {
        struct sockaddr_in dest_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
        };
        inet_pton(AF_INET, HOST_IP_ADDR, &dest_addr.sin_addr.s_addr);

        int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sock < 0) {
            ESP_LOGE(TAG, "Socket creation failed: errno %d", errno);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "Connecting to %s:%d...", HOST_IP_ADDR, PORT);
        if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) {
            ESP_LOGE(TAG, "Connection failed: errno %d", errno);
            close(sock);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
            continue;
        }

        send(sock, payload, strlen(payload), 0);
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len > 0) {
            rx_buffer[len] = 0;
            ESP_LOGI(TAG, "Received: %s", rx_buffer);
        }

        shutdown(sock, SHUT_RDWR);
        close(sock);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Configure GPIO16 as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << 16),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Set GPIO16 to high level
    gpio_set_level(GPIO_NUM_16, 1);

    // Wait 1 second
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Initialize Ethernet after the pulse
    ethernet_init();

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event, NULL));

    xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
}
