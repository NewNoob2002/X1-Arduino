#include "HAL_ESP.h"
#include "MP2762A.h"

#define MASTER_FREQUENCY CONFIG_I2C_MASTER_FREQUENCY

// button callback
static void button_event_cb(void *arg, void *data)
{
  ESP_LOGI("double_click", "double_click");
  esp32.Wifi_ReStart();
  digitalWrite(GPIO_NUM_2, 1);
}

static void button_single_event_cb(void *arg, void *data)
{
  esp32.STOP_Wifi();
  digitalWrite(GPIO_NUM_2, 0);
  ESP_LOGI("single_click", "single_click");
}

static void button_long_event_cb(void *arg, void *data)
{

  ESP_LOGI("single_click", "single_click");
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

void HAL_ESP::get_task_status()
{
  char InfoBuffer[512] = {0};
  memset(InfoBuffer, 0, 512); // 信息缓冲区清零
  vTaskList((char *)&InfoBuffer);
  printf("任务名  任务状态  优先级  剩余栈  任务序号  cpu核\r\n");
  printf("\r\n%s\r\n", InfoBuffer);

#if 1
  char CPU_RunInfo[400] = {0}; // 保存任务运行时间信息
  memset(CPU_RunInfo, 0, 400); // 信息缓冲区清零
  vTaskGetRunTimeStats((char *)&CPU_RunInfo);
  printf("任务名       运行计数         使用率\r\n");
  printf("%s", CPU_RunInfo);
  printf("---------------------------------------------\r\n\n");
#endif

  // 打印剩余ram和堆容量
  printf("IDLE: ****free internal ram %d  all heap size: %d Bytes****\n", heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_8BIT));
  printf("IDLE: ****free SPIRAM size: %d Bytes****\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  vTaskDelay((10000) / portTICK_PERIOD_MS);
}

void HAL_ESP::Check_Image_Status()
{
  static bool IS_First_Run = true;

  if (IS_First_Run == true){
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK){
      if (ota_state == ESP_OTA_IMG_PENDING_VERIFY){
        // run diagnostic function ...
        bool diagnostic_is_ok = check_peripherals();
        if (diagnostic_is_ok){
          ESP_LOGI("Update", "Diagnostics completed successfully! Continuing execution ...");
          esp_ota_mark_app_valid_cancel_rollback();
        }
        else{
          ESP_LOGE("Update", "Diagnostics failed! Start rollback to the previous version ...");
          esp_ota_mark_app_invalid_rollback_and_reboot();
        }
      }
      else if (ota_state == ESP_OTA_IMG_VALID){
        ESP_LOGI("Update", "OTA image is valid! ");
        IS_First_Run = false;
        return;
      }
    }
  }

}

bool HAL_ESP::check_peripherals()
{
  bool ret = false;
  // 检查电池I2c
  // check 蓝牙
  // check 串口
  // check wifi
  switch (esp32.Get_Wifi_Status())
  {
  case WIFI_RUNing:
    ret = true;
    break;
  case WIFI_STOP:
    ret = false;
    break;
  default:
    break;
  }
  // Check your peripherals here
  return true;
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
    }
  }
  // 文件系统初始化
  if (LittleFS.begin(true, "/www", 10, "webfs"))
  {
    ESP_LOGI("littlefs info", "Fs Init Ok");
    ESP_LOGI("littlefs info", "Partition size: total: %d, used: %d",
             LittleFS.totalBytes(), LittleFS.usedBytes());
  }
}

void HAL_ESP::wifi_Init()
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
  // esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
  //                                     &event_handler, NULL, NULL);
}

WIFI_STAT HAL_ESP::Start_Wifi()
{
  if (_wifi_config._state == WIFI_RUNing)
    return WIFI_RUNing;

  uint8_t pass_len = sizeof(_wifi_config._config.ap.password);

  if (pass_len > 0 && pass_len < 9)
  {
    ESP_LOGE("wifi", "Password too short");
    _wifi_config._state = WIFI_ERROR;
    return WIFI_ERROR;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &_wifi_config._config));

  esp_err_t ret = esp_wifi_start();
  if (ret != ESP_OK)
  {
    return WIFI_ERROR;
    ESP_LOGI("wifi_start_error", "wifi_start_error");
  }
  _wifi_config._state = WIFI_RUNing;
  return _wifi_config._state;
}

void HAL_ESP::STOP_Wifi()
{
  if (_wifi_config._state == WIFI_STOP)
    return;

  esp_err_t ret = esp_wifi_stop();
  if (ret != ESP_OK)
  {
    ESP_LOGE("wifi_stop", "AP stop failed: %s", esp_err_to_name(ret));
    _wifi_config._state = WIFI_ERROR;
  }
  else
  {
    _wifi_config._state = WIFI_STOP;
  }
}

void HAL_ESP::Wifi_ReStart()
{
  if (_wifi_config._state == WIFI_ERROR)
    return;

  ESP_LOGI("wifi", "wifi restart");
  STOP_Wifi();
  vTaskDelay(2000);

  for (int i = 0; i < 3; i++)
  {
    if (Start_Wifi() == WIFI_RUNing)
    {
      ESP_LOGI("wifi", "wifi restart success");
      break;
    }
    vTaskDelay(5000);
  }
  return;
}

void HAL_ESP::Config_Server()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String html = "";
              File file = LittleFS.open("/root.html", "r");
            if (!file)
              {
                request->send(500, "text/plain", "Failed to open file");
                return;
              }
              while (file.available()) {
                html += (char)file.read();
              }
              // const String html_read = html;
              // request->send(200, "text/html", html); // 返回 HTML 内容
          file.close();
          request->send(200, "text/html", html); });
  server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              String html = "";
              File file = LittleFS.open("/web.html", "r");
            if (!file)
              {
                request->send(500, "text/plain", "Failed to open file");
                return;
              }
              while (file.available()) {
                html += (char)file.read();
              }
              // const String html_read = html;
              // request->send(200, "text/html", html); // 返回 HTML 内容
          file.close();
          request->send(200, "text/html", html); });
  // 更新页面
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              if(Update.hasError())
              {
                request->send(500, "text/plain", "Update failed");
              }
              else
              {
                request->send(200, "text/plain", "Update success");
                vTaskDelay(3000);
                esp_restart();
              } }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
            {
              // const esp_partition_t *running = esp_ota_get_running_partition();
              esp_ota_handle_t update_handle = 0;
              bool update_started = false;
              const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
              ESP_LOGI("OTA", "OTA 更新分区: %s", update_partition->label);
              if (!index)
              {
                ESP_LOGI("OTA", "开始 OTA 更新，文件名: %s", filename.c_str());
                assert(update_partition != NULL);
                ESP_LOGI("OTA", "Writing to partition subtype %d at offset 0x%"PRIx32,
                  update_partition->subtype, update_partition->address);

                esp_err_t ret = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                if (ret != ESP_OK)
                {
                  ESP_LOGE("OTA", "OTA 初始化失败: 0x%x", ret);
                  request->send(500, "text/plain", "OTA 初始化失败");
                  esp_ota_abort(update_handle);
                  return;
                }
                update_started = true;
              }

              if (!update_started)
                return;

              if (len > 0)
              {
                esp_err_t write_err = esp_ota_write(update_handle, data, len);
                if (write_err != ESP_OK)
                {
                  ESP_LOGE("OTA", "数据块写入失败: 0x%x", write_err);
                  esp_ota_abort(update_handle);
                  request->send(500, "text/plain", "数据写入失败");
                  update_started = false;
                  return;
                }
                ESP_LOGI("OTA", "数据块写入成功: %d", len);
              }

              // 完成升级
              if (final)
              {
                esp_err_t end_err = esp_ota_end(update_handle);
                if(end_err != ESP_OK)
                {
                  ESP_LOGE("OTA", "固件验证失败: 0x%x", end_err);
                  request->send(500, "text/plain", "固件校验失败");
                  return;
                }
                  ESP_LOGI("OTA", "固件验证成功");
                  request -> send(200, "text/plain", "固件验证成功, 设备重启中...");
                  esp_err_t set_boot_err = esp_ota_set_boot_partition(update_partition);
                    if (set_boot_err != ESP_OK)
                    {
                    ESP_LOGE("OTA", "启动分区设置失败: 0x%x", set_boot_err);
                    request->send(500, "text/plain", "启动配置失败");
                    }
                  esp_restart();
                }
                update_started = false;
              });
}

void HAL_ESP::Start_Server_Task()
{
  switch (Server_status)
  {
  case Server_STOP:
    if (_wifi_config._state == WIFI_RUNing)
    {
      server.begin();
      ESP_LOGI("server", "server start");
      Server_status = Server_RUNing;
    }
    else if (_wifi_config._state == WIFI_ERROR || _wifi_config._state == WIFI_STOP)
    {
      ESP_LOGI("server", "wifi is in error|stop state, can't start server");
    }
    break;
  case Server_RUNing:
    if (_wifi_config._state == WIFI_RUNing)
    {
      digitalWrite(GPIO_NUM_2, 0);
      // ESP_LOGI("server", "server is  running");
      return;
    }
    else if (_wifi_config._state == WIFI_ERROR || _wifi_config._state == WIFI_STOP)
    {
      ESP_LOGI("server", "wifi is in error|stop state, so stop server");
      Stop_Server_Task();
      Server_status = Server_STOP;
      return;
    }
  }
}

void HAL_ESP::Stop_Server_Task()
{
  server.end();
  ESP_LOGI("server", "server stop");
  return;
}

WIFI_STAT HAL_ESP::Get_Wifi_Status()
{

  return _wifi_config._state;
}

uint8_t HAL_ESP::Get_Wifi_Retry_Count()
{
  return _wifi_config.retry_count;
}

void HAL_ESP::BT_Begin(const char *device_name)
{
  BT.begin(device_name);
}

void HAL_ESP::BT_Write(const uint8_t *buffer, size_t size)
{
  BT.write(buffer, size);
}

// void HAL_ESP::BT_Write(const char *data, uint32_t size)
// {
//   BT.write(data);
// }

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

  pinMode(GPIO_NUM_2, OUTPUT);

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

  ret |= iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, NULL, button_single_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, NULL, button_event_cb, NULL);
  ret |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_HOLD, NULL, button_long_event_cb, NULL);
  // //状态灯
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
