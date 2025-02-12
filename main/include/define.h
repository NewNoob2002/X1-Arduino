#ifndef DEFINES_H_
#define DEFINES_H_

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

#include "esp_event.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/sys.h"
#include "lwip/inet.h"
#include <ESPAsyncWebServer.h>
#include "Update.h"
#include "LittleFS.h"

#include "BluetoothSerial.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#include "dirent.h"
#include "esp_vfs.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include "iot_button.h"
#include "button_gpio.h"


#define Hardware_version "V1.0.0"
#define Software_version "V1.0.0"
#define WIFI_SSID      "mywifi"
#define WIFI_PASS      "12345678"
#define WIFI_CHANNEL        1
#define WIFI_MAX_CONN       2

#define wifi_name "my_test-wifi"
#define wifi_password "12345678"

//蓝牙
#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define SPP_SERVER_NAME "SPP_SERVER"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
#define SPP_SHOW_MODE SPP_SHOW_SPEED    /*Choose show mode: show data or speed*/

static const char local_device_name[] = "esp32_bt";
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;


#define LCD 1
/*gpio 引脚定义*/

#define FN_KEY                  GPIO_NUM_9
/*状态灯*/
#define POWER_LED_Pin           7
#define CHARGER_LED_Pin         8
#define NET_LED_Pin             10
#define GNSS_LED_Pin            11
#define DATA_LED_Pin            12

#define ESP32_BOOT_Pin          13
/*电台引脚*/
#define Radio_Config_Pin        1
#define Radio_RX_Pin            3
#define Radio_TX_Pin            4
#define Radio_PWR_Pin           14
/*gps模块串口引脚*/
#define GNSS_PWREN_Pin          5
#define MCU_98x_RX_Pin          15
#define MCU_98x_TX_Pin          16
/*BMS芯片输出引脚*/
#define MP2762_ACOK_Pin         17

/*NAND flash 引脚*/
#define SDIO_SCK_Pin            18
#define SDIO_DI_Pin             19
#define SDIO_DO_Pin             22

/*电池引脚*/
#define Charge_Status_Pin       2
#define MCU_BAT_POWER_Pin       6
#define BAT_I2C_SDA_Pin         GPIO_NUM_20
#define BAT_I2C_SCL_Pin         GPIO_NUM_21

#if LCD
#define LCD_SCK_Pin             12
#define LCD_SDI_Pin             13
#define LCD_RS_Pin              11
#define LCD_CS_Pin              10
#define LCD_BLK_Pin             14
#endif // DEBUG


typedef enum {
  WIFI_RUNing=0,
  WIFI_STOP,
  WIFI_ERROR
}WIFI_STAT;

typedef enum {
  BT_RUNing=0,
  BT_STOP,
  BT_ERROR
}BT_STAT;

typedef enum {
  Server_RUNing=0,
  Server_STOP,
}Server_STAT;

typedef struct {
  WIFI_STAT _state;
  wifi_config_t _config;
  uint8_t retry_count;
} AP_Config_t;
// typedef struct {
//   i2c_device_config_t bat_device;
//   uint8_t bat_i2c_addr;
//   uint8_t write_time_ms;
// }i2c_bat_config_t;

extern AsyncWebServer server;
extern BluetoothSerial BT;

#endif
