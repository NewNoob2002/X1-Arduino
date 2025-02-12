#include "esp_netif.h"
#include "lwip/arch.h"
#include "lwip/inet.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "dirent.h"


#include "wifi_cfg.h"

const char *ssid = "my_test-wifi";
const char *password = "12345678";

const char *mountpoint = "/www";



const IPAddress local_ip(192, 168, 10, 1);     // 设置静态 IP 地址
const IPAddress gateway(192, 168, 10, 1);        // 设置网关地址
const IPAddress subnet(255, 255, 255, 0);       // 设置子网掩码

void wifi_function(void)
{
    WiFi.mode(WIFI_AP);

    WiFi.softAPConfig(local_ip, gateway, subnet);

    if(WiFi.softAP(ssid, password))
    {
        ESP_LOGI("wifi info", " wifi create success");
    }
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    
    if(LittleFS.begin(true, mountpoint, 10, "webfs"))
    {
        ESP_LOGI("littlefs info", "Fs Init Ok");
        ESP_LOGI("littlefs info", "Partition size: total: %d, used: %d", 
                LittleFS.totalBytes(), LittleFS.usedBytes());

    }

}

void server_start(void)
{
    static int times=0;
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = "";
        File file = LittleFS.open("/webui.html", "r");
        if (!file) {
            request->send(500, "text/plain", "Failed to open file");
            return;
            }
        while (file.available()) {
              html += (char)file.read();
            }
            // const String html_read = html;
            // request->send(200, "text/html", html); // 返回 HTML 内容
        file.close();
        ESP_LOGI("server","read time %d", times++);
        request->send(200, "text/html", html); // 返回 HTML 内容
      });

    server.on("/download_page", HTTP_GET, [](AsyncWebServerRequest* request){
        String response;
        DIR *dir = opendir(mountpoint);
        if (dir == NULL) {
            ESP_LOGE("File Listing", "Failed to open LittleFS directory");
            return;
        }
        struct dirent *entry;
        response += "<html><body><h1>Files in LittleFS</h1><ul>";

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) { // 只列出文件，不列出子目录
                // 为每个文件生成一个下载链接
                response += "<li>";
                response += "<a href=\"/download?file=";
                response += entry->d_name;
                response += "\">";
                response += entry->d_name;
                response +="</a></li>";
            }
        }
        closedir(dir);
        response += "</ul></body></html>";

        request->send(200, "text/html", response);

      });
    
      server.on("/download", HTTP_POST, [](AsyncWebServerRequest* request){
        
        String file_name = "/";
        
        // 从 URL 中获取文件名
        int position = request->url().indexOf("?file=");
        if (position != -1) {
            file_name += request->url().substring(position+6);
            Serial.println(file_name);
        }
        if (file_name == "/" || file_name.length() <= 6) {
            request->send(500, "text/plain", "get file_name error");
            return;
        }

        File file = LittleFS.open(file_name, "r");
        if (!file) {
            request->send(500, "text/plain", "Failed to open file");
            return;
            }
        
        // 获取文件大小
        // size_t fileSize = file.size();

        request->send(file, file_name, "text/html", true );
        file.close();
      });
}

void web_Task(void)
{
    // wifi_function();//wifi配置
    // server_start();
    // server.begin();
}

