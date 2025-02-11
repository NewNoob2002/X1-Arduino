#include "HAL_ESP.h"
#include "MP2762A.h"

#define MASTER_FREQUENCY CONFIG_I2C_MASTER_FREQUENCY

static void button_event_cb(void *arg, void *data)
{
  iot_button_print_event((button_handle_t)arg);
}

HAL_ESP::HAL_ESP()
{
  xVSPIMutex = xSemaphoreCreateMutex();
  xDisplayMutex = xSemaphoreCreateMutex();
}

HAL_ESP::~HAL_ESP()
{
  vSemaphoreDelete(xVSPIMutex);
  vSemaphoreDelete(xDisplayMutex);
}

void HAL_ESP::SayHello()
{
  ESP_LOGI("HAL_ESP", "Hello World");
}

void HAL_ESP::Init_NVS()
{
  ESP_LOGI("nvs", "nvs_flash_init");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    if (partition != NULL)
    {
      err = esp_partition_erase_range(partition, 0, partition->size);
      if (!err)
      {
        err = nvs_flash_init();
      }
      else
      {
        ESP_LOGI("nvs", "Failed to format the broken NVS partition!");
      }
    }
    else
    {
      ESP_LOGI("nvs", "Could not find NVS partition");
    }
  }
  if (err)
  {
    ESP_LOGI("nvs", "Failed to initialize NVS! Error: %u", err);
  }
}

void HAL_ESP::Init_StartWifi()
{
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
  
  if (wifi_status == WIFI_NAN || wifi_status == WIFI_STOP)
  {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *netif = esp_netif_new(&netif_config);
    assert(netif);
    ESP_ERROR_CHECK(esp_netif_attach_wifi_ap(netif));
    ESP_ERROR_CHECK(esp_wifi_set_default_wifi_ap_handlers());
    // esp_netif_create_default_wifi_ap();

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    wifi_config = {
        .ap = {
            .ssid = wifi_name,
            .password = wifi_password,
            .ssid_len = uint8_t(strlen(wifi_name)),
            .channel = WIFI_CHANNEL,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .max_connection = WIFI_MAX_CONN,
            .beacon_interval = 100}
          };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    
    esp_err_t ret = esp_wifi_start();
    if (ret == ESP_OK)
    {
      ESP_LOGI("wifi_start", "wifi name: %s, wifi password: %s", wifi_name, wifi_password);
      wifi_status = WIFI_RUNing;
    }
  }
  else if (wifi_status == WIFI_RUNing)
  {
    ESP_LOGI("wifi_start", "wifi is already running");
    return;
  }
  else
  {
    ESP_LOGI("wifi_start", "wifi status error");
    wifi_status = WIFI_ERROR;
    return;
  }
}

void HAL_ESP::STOP_Wifi()
{
  switch (wifi_status)
  {
  case WIFI_NAN:
    break;
  case WIFI_STOP:
    break;
  case WIFI_RUNing:
    esp_wifi_stop();
    wifi_status = WIFI_STOP;
    ESP_LOGI("wifi_stop", "wifi stop");
    break;

  default:
    wifi_status = WIFI_ERROR;
    break;
  }
}

WIFI_STAT HAL_ESP::GetWifiStatus()
{
  
  return wifi_status;
}

void HAL_ESP::ConfigPins()
{
  // for radio config mode
  //  pinMode(Radio_Config_Pin, OUTPUT);
  //  digitalWrite(Radio_Config_Pin, LOW);

  // pinMode(GNSS_PWREN_Pin, OUTPUT);
  // for enable BAT
  // pinMode(MCU_BAT_POWER_Pin, OUTPUT);
  // digitalWrite(MCU_BAT_POWER_Pin, HIGH);
  // 按键设置

  gpio_config_t led_status =
      {
          .pin_bit_mask = GPIO_NUM_2,
          .mode = GPIO_MODE_OUTPUT,
          .pull_up_en = GPIO_PULLUP_DISABLE,
          .pull_down_en = GPIO_PULLDOWN_DISABLE,
          .intr_type = GPIO_INTR_DISABLE,

      };

  gpio_config(&led_status);

  gpio_set_level(GPIO_NUM_2, 1);

  button_config_t btn_cfg = {
      .long_press_time = 1000,
      .short_press_time = 180,
  };
  button_gpio_config_t gpio_cfg = {
      .gpio_num = GPIO_NUM_0,
      .active_level = 0,
      .enable_power_save = true,
  };
  button_handle_t btn;
  esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn);
  assert(ret == ESP_OK);

  ret = iot_button_register_cb(btn, BUTTON_PRESS_DOWN, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_PRESS_UP, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT_DONE, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_START, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_HOLD, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_UP, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_PRESS_END, NULL, button_event_cb, NULL);
  // 状态灯
  //  pinMode(POWER_LED_Pin, OUTPUT);
  //  pinMode(CHARGER_LED_Pin, OUTPUT);
  //  pinMode(NET_LED_Pin, OUTPUT);
  //  pinMode(GNSS_LED_Pin, OUTPUT);
  //  pinMode(DATA_LED_Pin, OUTPUT);
}

void HAL_ESP::ConfigI2c()
{
  const i2c_master_bus_config_t bus_cfg = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = BAT_I2C_SDA_Pin,
      .scl_io_num = BAT_I2C_SCL_Pin,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
  };

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

  const i2c_device_config_t bat_device = {
      .dev_addr_length = I2C_ADDR_BIT_LEN_7,
      .device_address = 0x5C,
      .scl_speed_hz = CONFIG_I2C_MASTER_FREQUENCY,
      .scl_wait_us = 2000,
      .flags = {
          .disable_ack_check = false,
      }};

  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &bat_device, &i2c_Bat_handle));
}

error_t HAL_ESP::Write_BAT_Reg(uint16_t reg, const uint8_t *data, uint32_t size)
{

  i2c_master_transmit(i2c_Bat_handle, data, size, 1000);
  return error_t();
}

void HAL_ESP::ConfigSPI()
{
}
