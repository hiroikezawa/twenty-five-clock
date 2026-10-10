#include "settings_ui.h"
#include "settings.h"
#include "ntp_manager.h"
#include "rtc_manager.h"
#include "esp_log.h"
#include <stdio.h>
static int page;
static bool setup_seen;
bool settings_ui_active(void) {
    return page != 0;
}
bool settings_ui_tick(void) {
    if (ntp_status().setup && !setup_seen) {
        setup_seen = true;
        if (!clock_time_valid()) {
            page = 3;
            return true;
        }
    }
    return false;
}
bool settings_ui_event(touch_event_t e) {
    if (e.kind == TOUCH_NONE)
        return false;
    if (e.kind == TOUCH_LONG) {
        page = page ? 0 : 1;
        return true;
    }
    if (!page && !clock_time_valid() && e.kind == TOUCH_TAP) {
        ntp_request_setup();
        page = 3;
        return true;
    }
    if (!page)
        return false;
    if (e.kind == TOUCH_LEFT || e.kind == TOUCH_RIGHT) {
        page = page == 1 ? 2 : 1;
        return true;
    }
    if (e.kind != TOUCH_TAP && e.kind != TOUCH_DOUBLE)
        return false;
    if (e.y > 370 || (page == 3 && e.y > 320)) {
        page = 0;
        return true;
    }
    if (page == 2) {
        if (e.y >= 320 && e.y <= 366) {
            ntp_request_setup();
            page = 3;
        }
        return true;
    }
    if (page != 1)
        return true;
    if (e.y >= 108 && e.y < 228) {
        unsigned char *values[] = {&settings.day, &settings.evening, &settings.night};
        unsigned char *v = values[(e.y - 108) / 40];
        int next = *v + (e.x < 233 ? -5 : 5);
        *v = next < 1 ? 1 : next > 100 ? 100 : next;
    } else if (e.y >= 228 && e.y < 268)
        settings.sweep = !settings.sweep;
    else if (e.y >= 268 && e.y < 308)
        settings.auto_face = !settings.auto_face;
    else if (e.y >= 308 && e.y < 365) {
        page = 2;
        return true;
    }
    esp_err_t err = settings_save();
    if (err != ESP_OK)
        ESP_LOGE("Settings", "NVS save failed: %s", esp_err_to_name(err));
    return true;
}
void settings_ui_draw(canvas_t *c, const face_time_t *t) {
    char text[64];
    draw_text(c, 233, 79,
              page == 3   ? "WI-FI SETUP"
              : page == 2 ? "CLOCK STATUS"
                          : "SETTINGS",
              2, 0xf0e7d6);
    if (page == 1) {
        snprintf(text, sizeof(text), "- DAY       %3d +", settings.day);
        draw_text(c, 233, 128, text, 2, 0xc8d9e9);
        snprintf(text, sizeof(text), "- EVENING   %3d +", settings.evening);
        draw_text(c, 233, 168, text, 2, 0xc8d9e9);
        snprintf(text, sizeof(text), "- NIGHT     %3d +", settings.night);
        draw_text(c, 233, 208, text, 2, 0xc8d9e9);
        draw_text(c, 233, 248, settings.sweep ? "SWEEP SECONDS  ON" : "SWEEP SECONDS OFF", 2,
                  0xc8d9e9);
        draw_text(c, 233, 288, settings.auto_face ? "AUTO CHANGE    ON" : "AUTO CHANGE   OFF", 2,
                  0xc8d9e9);
        draw_text(c, 233, 338, "WI-FI / TIME STATUS", 2, 0x78bdb7);
    } else if (page == 2) {
        network_status_t net = ntp_status();
        draw_text(c, 233, 122, ntp_status_message(net), 2, 0xc8d9e9);
        draw_text(c, 233, 161, "LAST NTP SYNC (JST)", 1, 0x78bdb7);
        if (net.last_sync) {
            struct tm tm;
            localtime_r(&net.last_sync, &tm);
            strftime(text, sizeof(text), "%Y-%m-%d %H:%M", &tm);
        } else
            snprintf(text, sizeof(text), "NOT YET SYNCED");
        draw_text(c, 233, 185, text, 2, 0xc8d9e9);
        draw_text(c, 233, 227, "CURRENT RTC TIME (JST)", 1, 0x78bdb7);
        time_t rtc;
        if (rtc_read(&rtc) == ESP_OK) {
            struct tm tm;
            localtime_r(&rtc, &tm);
            strftime(text, sizeof(text), "%Y-%m-%d", &tm);
            draw_text(c, 233, 252, text, 2, 0xc8d9e9);
            strftime(text, sizeof(text), "%H:%M:%S", &tm);
        } else
            snprintf(text, sizeof(text), "RTC INVALID");
        draw_text(c, 233, 278, text, 2, 0xc8d9e9);
        draw_text(c, 233, 342, "CONFIGURE WI-FI", 2, 0xebbc77);
    } else {
        network_status_t net = ntp_status();
        draw_text(c, 233, 119, "CONNECT PHONE WI-FI TO", 1, 0xc8d9e9);
        draw_text(c, 233, 145, net.setup ? net.ap : "STARTING...", 3, 0xffffff);
        draw_text(c, 233, 183, "PASSWORD / 12 CHARS", 2, 0x78bdb7);
        /* 42-pixel-high monospaced glyphs, six characters per row.
           Display grouping only: the WPA2 password remains 12 continuous chars. */
        char first[7], second[7];
        snprintf(first, sizeof(first), "%.6s", net.password);
        snprintf(second, sizeof(second), "%.6s", net.password + 6);
        unsigned font = c->font_style;
        c->font_style = 3;
        draw_text(c, 233, 232, first, 6, 0xffffff);
        draw_text(c, 233, 287, second, 6, 0xffffff);
        c->font_style = font;
        draw_text(c, 233, 325, "TOP THEN BOTTOM / NO SPACES", 1, 0xc8d9e9);
        draw_text(c, 233, 352, "192.168.4.1", 3, 0xffffff);
    }
    draw_text(c, 233, 390, "CLOSE", 2, 0x899eaa);
}
