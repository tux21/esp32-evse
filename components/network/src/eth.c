#include "eth.h"

#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32

#include <driver/gpio.h>
#include <esp_eth.h>
#include <esp_eth_phy_lan87xx.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define ETH_MDC_GPIO    23
#define ETH_MDIO_GPIO   18
#define ETH_CLK_GPIO    0
#define ETH_CLK_EN_GPIO 16
#define ETH_PHY_ADDR    1

static const char* TAG = "eth";

static esp_netif_t* eth_netif = NULL;

static void event_handler(void* arg, esp_event_base_t base, int32_t id, void* data)
{
    if (base == ETH_EVENT) {
        if (id == ETHERNET_EVENT_CONNECTED) {
            ESP_LOGI(TAG, "Link up");
        } else if (id == ETHERNET_EVENT_DISCONNECTED) {
            ESP_LOGI(TAG, "Link down");
        }
    } else if (base == IP_EVENT && id == IP_EVENT_ETH_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)data;
        ESP_LOGI(TAG, "Got ip: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void eth_init(void)
{
    gpio_config_t clk_en = {
        .pin_bit_mask = BIT64(ETH_CLK_EN_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&clk_en));
    ESP_ERROR_CHECK(gpio_set_level(ETH_CLK_EN_GPIO, 1));
    vTaskDelay(pdMS_TO_TICKS(10));

    eth_esp32_emac_config_t esp32_cfg = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp32_cfg.smi_gpio.mdc_num = ETH_MDC_GPIO;
    esp32_cfg.smi_gpio.mdio_num = ETH_MDIO_GPIO;
    esp32_cfg.clock_config.rmii.clock_mode = EMAC_CLK_EXT_IN;
    esp32_cfg.clock_config.rmii.clock_gpio = ETH_CLK_GPIO;

    eth_mac_config_t mac_cfg = ETH_MAC_DEFAULT_CONFIG();
    mac_cfg.sw_reset_timeout_ms = 1000;
    esp_eth_mac_t* mac = esp_eth_mac_new_esp32(&esp32_cfg, &mac_cfg);

    eth_phy_config_t phy_cfg = ETH_PHY_DEFAULT_CONFIG();
    phy_cfg.phy_addr = ETH_PHY_ADDR;
    phy_cfg.reset_gpio_num = -1;
    esp_eth_phy_t* phy = esp_eth_phy_new_lan87xx(&phy_cfg);

    esp_eth_config_t eth_cfg = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&eth_cfg, &handle));

    esp_netif_config_t netif_cfg = ESP_NETIF_DEFAULT_ETH();
    eth_netif = esp_netif_new(&netif_cfg);
    ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(handle)));

    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_eth_start(handle));
    ESP_LOGI(TAG, "Ethernet started");
}

#else

void eth_init(void)
{
}

#endif
