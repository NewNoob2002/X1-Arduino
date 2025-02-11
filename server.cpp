#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <esp_ota_ops.h>

// 定义 AP 模式参数
const char* AP_SSID = "ESP32-OTA-AP";
const char* AP_PASSWORD = "12345678";

AsyncWebServer server(80); // 创建异步服务器对象

// OTA 相关变量
bool isOTAStart = false;
size_t otaSize = 0;

// 处理固件上传的回调函数
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) { // 如果是第一个数据包
    if (!filename.endsWith(".bin")) { // 检查文件格式
      request->send(400, "text/plain", "Error: Only .bin files allowed");
      return;
    }

    // 初始化 OTA 升级
    Serial.println("OTA Start");
    isOTAStart = true;
    otaSize = 0;
    
    // 检查可用 OTA 分区空间
    if (Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.runAsync(true); // 异步模式（避免阻塞）
    } else {
      Update.printError(Serial);
      request->send(500, "text/plain", "OTA Init Failed");
      isOTAStart = false;
    }
  }

  if (isOTAStart) {
    // 写入数据到 OTA 分区
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
      request->send(500, "text/plain", "Write Failed");
      isOTAStart = false;
    }
    otaSize += len;
    Serial.printf("Progress: %d%%\n", (otaSize * 100) / (index + len + (final ? 0 : 1)));
  }

  if (final) { // 如果是最后一个数据包
    if (Update.end(true)) { // 结束并验证固件
      Serial.printf("OTA Success: %u bytes\n", otaSize);
      request->send(200, "text/plain", "OK. Device will reboot.");
      delay(1000);
      ESP.restart(); // 重启设备
    } else {
      Update.printError(Serial);
      request->send(500, "text/plain", "OTA End Failed");
    }
    isOTAStart = false;
  }
}

void setup() {
  Serial.begin(115200);
  
  // 初始化 NVS（OTA 依赖）
  if (esp_err_t ret = nvs_flash_init(); ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    nvs_flash_erase();
    nvs_flash_init();
  }

  // 启动 AP 模式
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  // 配置服务器路由
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // 返回简单的上传页面
    String html = "<form method='POST' action='/update' enctype='multipart/form-data'>"
                  "<input type='file' name='firmware' accept='.bin'>"
                  "<input type='submit' value='Upload'>"
                  "</form>";
    request->send(200, "text/html", html);
  });

  // 处理固件上传
  server.on("/update", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
      handleUpload(request, filename, index, data, len, final);
    }
  );

  server.begin(); // 启动服务器
}

void loop() {} // 空循环（异步服务器无需处理）
