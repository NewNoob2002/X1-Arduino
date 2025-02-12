#ifndef HAL_ESP_H_
#define HAL_ESP_H_

#include "define.h"


class HAL_ESP {
    public:
        HAL_ESP();
        ~HAL_ESP();
        void Init_NVS();
        //WIFI CONFIG
        void wifi_Init();
        WIFI_STAT Start_Wifi();
        void STOP_Wifi();
        void Wifi_ReStart();
        //WebSerber Config
        void Config_Server();
        void Start_Server_Task();
        void Stop_Server_Task();
        //Get WIFI Status
        WIFI_STAT Get_Wifi_Status();
        uint8_t Get_Wifi_Retry_Count();
        //BT Status
        void BT_Begin(const char* device_name);
        void BT_Write(const uint8_t *buffer, size_t size);
        void ConfigPins();
        //BAT I2c
        void ConfigI2c();
        error_t Write_BAT_Reg(uint16_t reg, const uint8_t *data, uint32_t size);
        void ConfigSPI();
    private:
        // Private members and methods
    SemaphoreHandle_t xVSPIMutex = NULL;
    SemaphoreHandle_t xDisplayMutex = NULL;
    
    nvs_handle_t my_nvs_handle;
    //外设
    
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t i2c_Bat_handle;//设备操作接口


    //状态
    AP_Config_t _wifi_config = {
        ._state = WIFI_STOP,
        ._config = { 
          .ap = {
            .ssid = wifi_name,
            .password = wifi_password,
            .ssid_len = uint8_t(strlen(wifi_name)),
            .channel = WIFI_CHANNEL,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = WIFI_MAX_CONN,
            .beacon_interval = 100},
        },
        .retry_count = 0,
    };
    BT_STAT bt_status = BT_STOP;
    Server_STAT Server_status = Server_STOP;
    /*WIFI配置 */
    const esp_netif_ip_info_t netif_soft_ap_ip = {
        .ip = {.addr = ESP_IP4TOADDR(192, 168, 10, 12)},
        .netmask = {.addr = ESP_IP4TOADDR(255, 255, 255, 0)},
        .gw = {.addr = ESP_IP4TOADDR(192, 168, 10, 12)}
  
    };

    const esp_netif_inherent_config_t _esp_netif_inherent_ap_config = {
        .flags = (esp_netif_flags_t)(ESP_NETIF_IPV4_ONLY_FLAGS(ESP_NETIF_DHCP_SERVER) | ESP_NETIF_FLAG_AUTOUP),
        .mac = {0},
        .ip_info = &netif_soft_ap_ip,
        .get_ip_event = 0,
        .lost_ip_event = 0,
        .if_key = "WIFI_AP_DEF",
        .if_desc = "ap",
        .route_prio = 10,
        .bridge_info = NULL};
    
    const esp_netif_config_t netif_config = {
            .base = &_esp_netif_inherent_ap_config,
            .driver = NULL,
            .stack = ESP_NETIF_NETSTACK_DEFAULT_WIFI_STA,
        };


};
    
extern HAL_ESP esp32;
#endif // !HAL_ESP_H_