#include "ntp_manager.h"
#include "settings.h"
#include "rtc_manager.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_http_server.h"
#include "esp_random.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define GOT_IP 1
#define SYNCED 2
#define SETUP 4
#define SAVED 8
#define FAILED 16
static EventGroupHandle_t events;
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static network_status_t status;
static httpd_handle_t server;
static wifi_ap_record_t nearby[24];
static uint16_t nearby_count;
static void scan_nearby(void) {
    nearby_count=0;
    wifi_scan_config_t config={.show_hidden=true};
    if(esp_wifi_scan_start(&config,true)!=ESP_OK) return;
    uint16_t count=24;
    if(esp_wifi_scan_get_ap_records(&count,nearby)==ESP_OK) nearby_count=count;
    ESP_LOGI("WiFi","Found %u nearby 2.4 GHz access points",nearby_count);
}
static void html_escape(char *out, const uint8_t *ssid) {
    /* At most 32 bytes * 6 characters plus terminator. */
    char *p=out;
    for(int i=0;i<32 && ssid[i];i++) {
        const char *entity=ssid[i]=='&' ? "&amp;" : ssid[i]=='<' ? "&lt;" :
            ssid[i]=='>' ? "&gt;" : ssid[i]=='\"' ? "&quot;" : NULL;
        if(entity) {size_t n=strlen(entity);memcpy(p,entity,n);p+=n;}
        else *p++=ssid[i];
    }
    *p=0;
}
static void phase(network_phase_t value) {
    portENTER_CRITICAL(&mux);
    status.phase=value;
    portEXIT_CRITICAL(&mux);
}
const char *ntp_status_message(network_status_t s) {
    switch(s.phase) {
    case NET_SETUP: return "WI-FI SETUP REQUIRED";
    case NET_CONNECTING: return "CONNECTING TO WI-FI";
    case NET_NTP: return "GETTING NETWORK TIME";
    case NET_WIFI_FAILED:
        switch(s.reason) {
        case 201: return "HOME WI-FI NOT FOUND";
        case 15: case 202: case 204: return "CHECK WI-FI PASSWORD";
        case 210: case 211: return "WI-FI SECURITY MISMATCH";
        default: return "WI-FI CONNECTION FAILED";
        }
    case NET_NTP_FAILED: return "TIME SERVER NO REPLY";
    case NET_SYNCED: return "TIME SYNCHRONIZED";
    default: return "STARTING WI-FI";
    }
}
network_status_t ntp_status(void) {
    portENTER_CRITICAL(&mux);
    network_status_t s = status;
    portEXIT_CRITICAL(&mux);
    return s;
}
void ntp_request_setup(void) {
    if (events)
        xEventGroupSetBits(events, SETUP);
}
static void event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI("WiFi", "Router connected; IP obtained");
        portENTER_CRITICAL(&mux);
        status.connected = true;
        portEXIT_CRITICAL(&mux);
        xEventGroupSetBits(events, GOT_IP);
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disconnected=data;
        portENTER_CRITICAL(&mux);
        status.connected = false;
        bool active=status.phase==NET_CONNECTING || status.phase==NET_NTP;
        if(active) status.reason=disconnected->reason;
        portEXIT_CRITICAL(&mux);
        xEventGroupClearBits(events, GOT_IP);
        if(active) {
            ESP_LOGW("WiFi", "Connection failed: reason=%u", disconnected->reason);
            xEventGroupSetBits(events, FAILED);
        }
    }
}
static void synced(struct timeval *tv) {
    xEventGroupSetBits(events, SYNCED);
}
static esp_err_t index_page(httpd_req_t *req) {
    static const char html[] =
        "<!doctype html><meta charset='utf-8'><meta name='viewport' content='width=device-width'><title>Clock "
        "Wi-Fi</title>"
        "<style>body{background:#10131b;color:#eee;font:18px system-ui;max-width:420px;margin:50px "
        "auto;padding:24px}input,button{box-sizing:border-box;width:100%;padding:14px;margin:10px "
        "0}</style>"
        "<h1>時計のWi-Fi設定</h1><p>接続先の<b>自宅ルーター／スマホの2.4GHz Wi-Fi</b>を選んでください。"
        "Clock-175ではありません。5GHz専用Wi-Fiには接続できません。</p>"
        "<form method='post' action='/save'><label>自宅のSSID<input name='ssid' list='nearby' maxlength='32' "
        "required></label><datalist id='nearby'>";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    esp_err_t e=httpd_resp_sendstr_chunk(req,html);
    if(e!=ESP_OK) return e;
    for(unsigned i=0;i<nearby_count;i++) {
        if(!nearby[i].ssid[0]) continue;
        bool duplicate=false;
        for(unsigned j=0;j<i;j++) if(!memcmp(nearby[i].ssid,nearby[j].ssid,32)) duplicate=true;
        if(duplicate) continue;
        char escaped[193],option[230];
        html_escape(escaped,nearby[i].ssid);
        snprintf(option,sizeof(option),"<option value=\"%s\"></option>",escaped);
        if((e=httpd_resp_sendstr_chunk(req,option))!=ESP_OK) return e;
    }
    e=httpd_resp_sendstr_chunk(req,
        "</datalist><label>自宅Wi-Fiのパスワード<input name='password' type='password' maxlength='63' "
        "autocomplete='new-password'></label><p>時計に表示された設定用パスワードとは別です。</p>"
        "<button>保存して接続する</button></form><p>SSID入力欄を押すと、時計が検出した2.4GHzの候補を選べます。"
        "候補にない場合はルーターの2.4GHzを有効にしてください。非公開SSIDは直接入力できます。</p>");
    return e==ESP_OK ? httpd_resp_send_chunk(req,NULL,0) : e;
}
static int hex(char c) {
    return c >= '0' && c <= '9'   ? c - '0'
           : c >= 'A' && c <= 'F' ? c - 'A' + 10
           : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                  : -1;
}
static bool decode(char *dst, size_t size, const char *src) {
    size_t n = 0;
    while (*src) {
        unsigned char c = *src++;
        if (c == '+')
            c = ' ';
        else if (c == '%') {
            if (!src[0] || !src[1] || hex(src[0]) < 0 || hex(src[1]) < 0)
                return false;
            c = hex(src[0]) * 16 + hex(src[1]);
            src += 2;
        }
        if (!c || n + 1 >= size)
            return false;
        dst[n++] = c;
    }
    dst[n] = 0;
    return true;
}
static esp_err_t save_page(httpd_req_t *req) {
    char body[512], es[128], ep[256], ssid[33], password[65];
    if (req->content_len <= 0 || req->content_len >= sizeof(body))
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid form");
    int n = 0;
    while (n < req->content_len) {
        int r = httpd_req_recv(req, body + n, req->content_len - n);
        if (r <= 0)
            return ESP_FAIL;
        n += r;
    }
    body[n] = 0;
    if (httpd_query_key_value(body, "ssid", es, sizeof(es)) != ESP_OK ||
        httpd_query_key_value(body, "password", ep, sizeof(ep)) != ESP_OK ||
        !decode(ssid, sizeof(ssid), es) || !decode(password, sizeof(password), ep) ||
        settings_save_wifi(ssid, password) != ESP_OK)
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                                   "SSID 1-32 bytes, password empty or 8-63 bytes required");
    memset(password, 0, sizeof(password));
    memset(body, 0, sizeof(body));
    esp_err_t e = httpd_resp_sendstr(
        req, "Saved. Clock will restart. You may reconnect to your normal Wi-Fi.");
    xEventGroupSetBits(events, SAVED);
    return e;
}
static void start_setup(void) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
    wifi_config_t cfg = {0};
    char pass[17];
    snprintf(pass, sizeof(pass), "%08lX%04lX", (unsigned long)esp_random(),
             (unsigned long)(esp_random() & 0xffff));
    snprintf((char *)cfg.ap.ssid, sizeof(cfg.ap.ssid), "Clock-175-%02X%02X", mac[4], mac[5]);
    strcpy((char *)cfg.ap.password, pass);
    cfg.ap.ssid_len = strlen((char *)cfg.ap.ssid);
    cfg.ap.channel = 1;
    cfg.ap.max_connection = 2;
    cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());
    scan_nearby();
    portENTER_CRITICAL(&mux);
    status.setup = true;
    status.phase = NET_SETUP;
    strcpy(status.ap, (char *)cfg.ap.ssid);
    strcpy(status.password, pass);
    portEXIT_CRITICAL(&mux);
    httpd_config_t http = HTTPD_DEFAULT_CONFIG();
    http.lru_purge_enable = true;
    http.stack_size = 6144;
    ESP_ERROR_CHECK(httpd_start(&server, &http));
    httpd_uri_t get = {.uri = "/", .method = HTTP_GET, .handler = index_page},
                post = {.uri = "/save", .method = HTTP_POST, .handler = save_page};
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &get));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &post));
    ESP_LOGI("WiFi", "Provisioning active; password is on the display; http://192.168.4.1");
}
static void network_task(void *unused) {
    char ssid[33] = {0}, password[65] = {0};
    bool configured = settings_wifi(ssid, password);
    ESP_LOGI("WiFi", "Saved router SSID: %s", configured ? ssid : "(not configured)");
    nvs_handle_t nv;
    if (nvs_open("clock", NVS_READONLY, &nv) == ESP_OK) {
        int64_t last = 0;
        nvs_get_i64(nv, "last_ntp", &last);
        nvs_close(nv);
        portENTER_CRITICAL(&mux);
        status.last_sync = last;
        portEXIT_CRITICAL(&mux);
    }
    for (;;) {
        if (!configured || (xEventGroupGetBits(events) & SETUP)) {
            start_setup();
            xEventGroupWaitBits(events, SAVED, pdFALSE, pdFALSE, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(1200));
            esp_restart();
        }
        wifi_config_t cfg = {0};
        ESP_LOGI("NTP", "Automatic time synchronization attempt (boot/interval)");
        /* wifi_sta_config_t.ssid is exactly 32 bytes (no extra terminator). */
        memcpy(cfg.sta.ssid, ssid, strlen(ssid));
        strcpy((char *)cfg.sta.password, password);
        cfg.sta.pmf_cfg.capable = true;
        cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        cfg.sta.failure_retry_cnt = 2;
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
        xEventGroupClearBits(events, GOT_IP | SYNCED | FAILED);
        portENTER_CRITICAL(&mux);
        status.reason=0;
        status.phase=NET_CONNECTING;
        portEXIT_CRITICAL(&mux);
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());
        EventBits_t bits =
            xEventGroupWaitBits(events, GOT_IP | SETUP | FAILED, pdFALSE, pdFALSE, pdMS_TO_TICKS(30000));
        bool ok = false;
        bool had_ip = (bits & GOT_IP) != 0;
        if ((bits & GOT_IP) && !(bits & SETUP)) {
            phase(NET_NTP);
            esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
            esp_sntp_setservername(0, "ntp.nict.jp");
            esp_sntp_setservername(1, "pool.ntp.org");
            esp_sntp_set_time_sync_notification_cb(synced);
            esp_sntp_init();
            bits =
                xEventGroupWaitBits(events, SYNCED | SETUP | FAILED, pdFALSE, pdFALSE, pdMS_TO_TICKS(45000));
            if (bits & SYNCED) {
                time_t now = time(NULL);
                struct tm t;
                localtime_r(&now, &t);
                ok = t.tm_year >= 124 && t.tm_year <= 199;
                if (ok) {
                    esp_err_t e = rtc_write(now);
                    ESP_LOGI("NTP", "JST synchronized; RTC update %s", esp_err_to_name(e));
                    portENTER_CRITICAL(&mux);
                    status.last_sync = now;
                    portEXIT_CRITICAL(&mux);
                    if (nvs_open("clock", NVS_READWRITE, &nv) == ESP_OK) {
                        nvs_set_i64(nv, "last_ntp", now);
                        nvs_commit(nv);
                        nvs_close(nv);
                    }
                }
            }
            esp_sntp_stop();
        }
        phase(ok ? NET_SYNCED : had_ip ? NET_NTP_FAILED : NET_WIFI_FAILED);
        ESP_ERROR_CHECK(esp_wifi_stop());
        portENTER_CRITICAL(&mux);
        status.connected = false;
        portEXIT_CRITICAL(&mux);
        /* IDF's 32-bit pdMS_TO_TICKS can overflow for multi-hour millisecond
           inputs at 1000 Hz. Convert seconds using a 64-bit intermediate. */
        uint64_t seconds =
            ok ? CONFIG_CLOCK_NTP_HOURS * 3600u
               : (esp_timer_get_time() < 300000000LL || !clock_time_valid() ? 30u : 900u);
        if (!ok)
            ESP_LOGW("NTP", "offline/timeout; keep RTC time, retry in %llu seconds", seconds);
        TickType_t delay = (TickType_t)(seconds * configTICK_RATE_HZ);
        xEventGroupWaitBits(events, SETUP, pdFALSE, pdFALSE, delay);
    }
}
void ntp_manager_start(void) {
    events = xEventGroupCreate();
    assert(events);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta = esp_netif_create_default_wifi_sta();
    esp_netif_t *ap = esp_netif_create_default_wifi_ap();
    if (!sta || !ap)
        abort();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_country_code("JP", true));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event, NULL));
    if (xTaskCreate(network_task, "clock_network", 6144, NULL, 3, NULL) != pdPASS)
        abort();
}
