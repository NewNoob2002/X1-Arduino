#include <stdio.h>
#include "HAL_ESP.h"

HAL_ESP esp32;

extern "C" void app_main(void)
{
    esp32.Init_NVS();
    esp32.Init_StartWifi();

    while (1)
    {
        ESP_LOGI("main", "wifi is %d", esp32.GetWifiStatus());
        
        vTaskDelay(10000);
    }
}