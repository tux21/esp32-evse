set(network_priv_requires nvs_flash esp_netif esp_wifi esp_driver_gpio mdns)

if(CONFIG_IDF_TARGET_ESP32)
    list(APPEND network_priv_requires esp_eth espressif__lan87xx)
endif()

idf_component_register(SRC_DIRS "src"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES ${network_priv_requires})
