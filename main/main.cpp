#include "HAL_ESP.h"

 HAL_ESP esp32;
 AsyncWebServer server(80);
 BluetoothSerial SerialBT;
 const esp_partition_t *running = esp_ota_get_running_partition();

static void wifi_task(void)
{
    switch (esp32.Get_Wifi_Status()) {
        case WIFI_RUNing:
            digitalWrite(GPIO_NUM_2, 1);
            // ESP_LOGI("wifi", "wifi is running");
            break;
        case WIFI_ERROR:
            ESP_LOGE("wifi_restart", "AP in error state");
            esp32.Wifi_ReStart();
            break;
        case WIFI_STOP:
            SerialBT.begin(Device_name, false, true);
            ESP_LOGI("wifi", "wifi is stop");
            break;
    }
}

extern "C" void app_main(void)
{
    esp32.Check_Image_Status();

    esp32.Init_NVS();
    esp32.ConfigPins();
    esp32.ConfigI2c();
    esp32.wifi_Init();
    esp32.Config_Server();
    WIFI_STAT wifi_status = esp32.Start_Wifi();
    if(wifi_status == WIFI_ERROR)
    {
        esp32.Wifi_ReStart();
    }
    while (1)
    {
        // ESP_LOGI("main", "wifi is %d", esp32.GetWifiStatus());
        // esp32.Start_Wifi_Task();
        wifi_task();
        vTaskDelay(1000);
        esp32.Start_Server_Task();
        vTaskDelay(1000);
        //已经验证
        // update_partition = esp_ota_get_next_update_partition(NULL);
        // ESP_LOGI("OTA", "OTA 更新分区: %s", update_partition->label);

        // esp32.get_task_status();
        
        // ESP_LOGI("gpio", "gpio_2 level is %d", digitalRead(GPIO_NUM_2));
        // digitalWrite(GPIO_NUM_2, 1);

        // vTaskDelay(1000);

        // digitalWrite(GPIO_NUM_2, 0);
    }
}