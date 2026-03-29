/*******************************************************************************
MIT License

Copyright (c) 2023 LEON-LINKS-room

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#include "web_server.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "gps_deal.h"

static const char *TAG = "web_server";

static const char index_html[] = 
"<!DOCTYPE html>"
"<html lang=\"zh-CN\">"
"	<head>"
"		<meta charset=\"UTF-8\">"
"		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, viewport-fit=cover, user-scalable=no\">"
"		<title>GPS Panel</title>"
"		<style>"
"			* {"
"				margin: 0;"
"				padding: 0;"
"				box-sizing: border-box;"
"				-webkit-tap-highlight-color: transparent;"
"			}"
"			"
"			body {"
"				background: linear-gradient(145deg, #10182c 0%, #1b2b42 100%);"
"				font-family: system-ui, -apple-system, 'Segoe UI', Roboto, Helvetica, sans-serif;"
"				min-height: 100vh;"
"				display: flex;"
"				align-items: center;"
"				justify-content: center;"
"				padding: 20px;"
"			}"
"			"
"			.card {"
"				background: rgba(22, 34, 48, 0.75);"
"				backdrop-filter: blur(20px);"
"				border-radius: 56px;"
"				box-shadow: 0 25px 45px rgba(0, 0, 0, 0.3), 0 0 0 1px rgba(255, 255, 255, 0.08);"
"				width: 100%;"
"				max-width: 460px;"
"				padding: 28px 24px 36px;"
"			}"
"			"
"			.title {"
"				text-align: center;"
"				margin-bottom: 28px;"
"			}"
"			"
"			.title h1 {"
"				font-size: 1.75rem;"
"				font-weight: 600;"
"				background: linear-gradient(135deg, #f0f9ff, #b9e2ff);"
"				background-clip: text;"
"				-webkit-background-clip: text;"
"				color: transparent;"
"			}"
"			"
"			.title p {"
"				font-size: 0.8rem;"
"				color: #9bb4d0;"
"				margin-top: 6px;"
"			}"
"			"
"			.info-grid {"
"				display: flex;"
"				flex-direction: column;"
"				gap: 20px;"
"				margin-bottom: 32px;"
"			}"
"			"
"			.info-item {"
"				background: rgba(10, 20, 30, 0.6);"
"				border-radius: 32px;"
"				padding: 16px 20px;"
"				display: flex;"
"				align-items: center;"
"				justify-content: space-between;"
"				flex-wrap: wrap;"
"				border: 0.5px solid rgba(255, 255, 255, 0.2);"
"			}"
"			"
"			.info-label {"
"				font-size: 0.9rem;"
"				font-weight: 500;"
"				text-transform: uppercase;"
"				color: #9bbcdd;"
"				display: flex;"
"				align-items: center;"
"				gap: 8px;"
"			}"
"			"
"			.info-label span {"
"				font-size: 1.25rem;"
"			}"
"			"
"			.info-value {"
"				font-size: 1.35rem;"
"				font-weight: 600;"
"				font-family: monospace;"
"				color: #f1f9ff;"
"				background: rgba(0, 0, 0, 0.3);"
"				padding: 6px 14px;"
"				border-radius: 40px;"
"				word-break: break-word;"
"				text-align: right;"
"				max-width: 65%;"
"			}"
"			"
"			.status-message {"
"				background: rgba(0, 0, 0, 0.5);"
"				border-radius: 28px;"
"				padding: 12px 18px;"
"				font-size: 0.85rem;"
"				text-align: center;"
"				color: #ffda9e;"
"				border-left: 3px solid #ffb347;"
"				margin-top: 12px;"
"			}"
"			"
"			.footer-note {"
"				font-size: 0.7rem;"
"				text-align: center;"
"				color: #6c8db0;"
"				margin-top: 24px;"
"				padding-top: 18px;"
"				border-top: 0.5px solid rgba(255, 255, 255, 0.15);"
"			}"
"			"
"			@media (max-width: 460px) {"
"				.card {"
"					padding: 20px 18px 28px;"
"				}"
"				"
"				.info-value {"
"					font-size: 1.1rem;"
"					max-width: 70%;"
"					padding: 4px 10px;"
"				}"
"				"
"				.info-label {"
"					font-size: 0.8rem;"
"				}"
"			}"
"			"
"			.loading-dot:after {"
"				content: ' ...';"
"				animation: blink 1.2s infinite;"
"			}"
"			"
"			@keyframes blink {"
"				0%, 100% {"
"					opacity: 0.3;"
"				}"
"				50% {"
"					opacity: 1;"
"				}"
"			}"
"		</style>"
"	</head>"
"	<body>"
"		<div class=\"card\">"
"			<div class=\"title\">"
"				<h1> ⛰️ </h1>"
"				<p> To the mountains </p>"
"			</div>"
"			"
"			<div class=\"info-grid\">"
"				<div class=\"info-item\">"
"					<div class=\"info-label\">"
"						<span>🕒</span> 时间"
"					</div>"
"					<div class=\"info-value\" id=\"utcTime\">--:--:--</div>"
"				</div>"
"				"
"				<div class=\"info-item\">"
"					<div class=\"info-label\">"
"						<span>📅</span> 日期"
"					</div>"
"					<div class=\"info-value\" id=\"utcDate\">----.--.--</div>"
"				</div>"
"				"
"				<div class=\"info-item\">"
"					<div class=\"info-label\">"
"						<span>🌐</span> 纬度"
"					</div>"
"					<div class=\"info-value\" id=\"latitude\">--</div>"
"				</div>"
"				"
"				<div class=\"info-item\">"
"					<div class=\"info-label\">"
"						<span>🌏</span> 经度"
"					</div>"
"					<div class=\"info-value\" id=\"longitude\">--</div>"
"				</div>"
"				"
"				<div class=\"info-item\">"
"					<div class=\"info-label\">"
"						<span>🏔</span> 海拔"
"					</div>"
"					<div class=\"info-value\" id=\"altitude\">---</div>"
"				</div>"
"				<div class=\"info-item\">"
"					<div class=\"info-label\">"
"						<span>💨</span> 时速"
"					</div>"
"					<div class=\"info-value\" id=\"nowspeed\">---</div>"
"				</div>"
"			</div>"
"			"
"			<div id=\"statusArea\" class=\"status-message\" style=\"display:none;\"></div>"
"			"
"			<div class=\"footer-note\">"
"				Power By Leon-Links"
"			</div>"
"		</div>"
"		"
"		<script>"
"			function fetchGPS() {"
"				fetch('/api/gps')"
"					.then(response => response.json())"
"					.then(data => {"
"						document.getElementById('utcTime').innerText = data.utc_time || '--:--:--';"
"						document.getElementById('utcDate').innerText = data.utc_date || '----.--.--';"
"						document.getElementById('latitude').innerHTML = data.latitude_display || '--';"
"						document.getElementById('longitude').innerHTML = data.longitude_display || '--';"
"						"
"						let alt = data.altitude !== undefined ? data.altitude + ' ' + data.altitude_unit : '---';"
"						document.getElementById('altitude').innerHTML = alt;"
"						let spd = data.nowspeed !== undefined ? data.nowspeed + ' ' + data.nowspeed_unit : '---';"
"						document.getElementById('nowspeed').innerHTML = spd;"
"						let statusDiv = document.getElementById('statusArea');"
"						if (!data.valid) {"
"							statusDiv.style.display = 'block';"
"							statusDiv.innerText = '📡 未定位，等待卫星信号....';"
"						} else {"
"							statusDiv.style.display = 'none';"
"						}"
"					})"
"					.catch(err => {"
"						console.error(err);"
"						document.getElementById('statusArea').style.display = 'block';"
"						document.getElementById('statusArea').innerText = '📡 未定位，等待卫星信号....';"
"					});"
"			}"
"			"
"			fetchGPS();"
"			setInterval(fetchGPS, 1000);"
"		</script>"
"	</body>"
"</html>";

static void build_json_response(char *buffer, size_t buffer_size) {
    int date_flag = 0;
    char valid[16]={0};
    char altitude[16]={0};
    char time_buf[16]={0};
    int hour,min,sec;
    char date_buf[16]={0};
    int year,mon,day;
    char lat_buf[16]={0};
    int la_du,la_fen,la_miao;
    char lng_buf[16]={0};
    int ln_du,ln_fen,ln_miao;
    char alt_unit[16]={0};
    int valid_i = 0;
    float altitude_f = 0;
    int altitude_i = 0;
    char spd_buf[16]={0};
    float nowspeed_f = 0;
    int nowspeed_i = 0;

    gps_info_get("valid",valid);
    valid_i = atoi(valid);
    gps_info_get("altitude",altitude);
    altitude_f = atof(altitude);
    altitude_i = (int)altitude_f;

    gps_info_get("time_buf",time_buf);
    sscanf(time_buf,"%2d%2d%2d",&hour,&min,&sec);

    if((hour+8)>=24){
        date_flag = 1;
        hour = hour+8-24;
    }
    else{
        date_flag = 0;
        hour = hour+8;
    }
    sprintf(time_buf,"%d:%d:%d",hour,min,sec);
    gps_info_get("date_buf",date_buf);
    sscanf(date_buf,"%2d%2d%2d",&day,&mon,&year);

    if(date_flag){

        int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        bool is_leap_year = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
        if (is_leap_year){
            days_in_month[1] = 29;
        }
        day++;
        if(day > days_in_month[mon - 1]){
            day = 1;
            mon++;
            if (mon > 12) {
                mon = 1;
                year++;
            }
        }
    }
    sprintf(date_buf,"20%d.%d.%d",year,mon,day);
    gps_info_get("lat_buf",lat_buf);
    sscanf(lat_buf,"%2d%2d.%4d",&la_du,&la_fen,&la_miao);
    la_miao = la_miao*60/10000;
    sprintf(lat_buf,"%d'%d'%d",la_du,la_fen,la_miao);
    gps_info_get("lng_buf",lng_buf);
    sscanf(lng_buf,"%3d%2d.%4d",&ln_du,&ln_fen,&ln_miao);
    ln_miao = ln_miao*60/10000;
    sprintf(lng_buf,"%d'%d'%d",ln_du,ln_fen,ln_miao);
    gps_info_get("alt_unit",alt_unit);
    gps_info_get("speed",spd_buf);
    nowspeed_f = atof(spd_buf);
    nowspeed_i = (int)nowspeed_f;

    if(valid_i != 0){
        snprintf(buffer, buffer_size,
            "{\"valid\":%s,\"utc_time\":\"%s\",\"utc_date\":\"%s\","
            "\"latitude_display\":\"%s N\",\"longitude_display\":\"%s E\","
            "\"altitude\":%d,\"altitude_unit\":\"%s\","
            "\"nowspeed\":%d,\"nowspeed_unit\":\"Km/h\"}",
            "true", time_buf, date_buf, lat_buf, lng_buf,
            altitude_i, alt_unit, nowspeed_i);
    }
    else{
        snprintf(buffer, buffer_size,
            "{\"valid\":%s,\"utc_time\":\"--:--:--\",\"utc_date\":\"----.--.--\","
            "\"latitude_display\":\"--\",\"longitude_display\":\"--\","
            "\"altitude\":\"---\",\"altitude_unit\":\"\","
            "\"nowspeed\":\"---\",\"nowspeed_unit\":\"\"}",
            "false");
    }
}

static esp_err_t root_get_handler(httpd_req_t *req) {

    httpd_resp_set_type(req, "text/html; charset=utf-8");

    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");
    httpd_resp_set_hdr(req, "Pragma", "no-cache");
    httpd_resp_set_hdr(req, "Expires", "0");

    httpd_resp_send(req, index_html, strlen(index_html));
    
    return ESP_OK;
}

static esp_err_t api_gps_get_handler(httpd_req_t *req) {
    char json_response[512];
    build_json_response(json_response, sizeof(json_response));
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, strlen(json_response));
    return ESP_OK;
}

static httpd_handle_t start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t api_uri = {
            .uri       = "/api/gps",
            .method    = HTTP_GET,
            .handler   = api_gps_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &api_uri);

        ESP_LOGI(TAG, "HTTP server started");
        return server;
    }
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return NULL;
}

void start_web_server(void) {
    start_webserver();
}

#define AP_SSID      "Leon-Links-GPS"
#define AP_PASS      "12345678"
#define AP_CHANNEL   1
#define AP_MAX_CONN  2

void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data){
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START){
        ESP_LOGI(TAG, "Wi-Fi AP started. SSID: %s, password: %s", AP_SSID, AP_PASS);
    }
}

void wifi_init_softap(void){
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = AP_SSID,
            .password = AP_PASS,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .max_connection = AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
}
