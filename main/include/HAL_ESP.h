#ifndef HAL_ESP_H_
#define HAL_ESP_H_

#include "define.h"

class HAL_ESP {
    public:
        HAL_ESP();
        ~HAL_ESP();
        void SayHello();
        void Init_NVS();
        //WIFI CONFIG
        void Init_StartWifi();
        void STOP_Wifi();
        //Get WIFI Status
        WIFI_STAT GetWifiStatus();

        void ConfigPins();
        //BAT I2c
        void ConfigI2c();
        error_t Write_BAT_Reg(uint16_t reg, const uint8_t *data, uint32_t size);
        void ConfigSPI();
    private:
        // Private members and methods
    SemaphoreHandle_t xVSPIMutex = NULL;
    SemaphoreHandle_t xDisplayMutex = NULL;
    
    //外设

    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t i2c_Bat_handle;//设备操作接口


    //状态
    wifi_config_t wifi_config;
    WIFI_STAT wifi_status = WIFI_NAN;
    BT_STAT bt_status = BT_NAN;

    };

#endif // !HAL_ESP_H_