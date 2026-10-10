#include "clock_manager.h"
#include "face_manager.h"
#include "display_manager.h"
#include "settings.h"
#include "settings_ui.h"
#include "touch_manager.h"
#include "ntp_manager.h"
#include "rtc_manager.h"
#include "gravity.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/time.h>
#include <inttypes.h>
static void offset(int slot, int *x, int *y) {
    slot %= 24;
    if (slot < 0)
        slot += 24;
    if (slot < 6) {
        *x = slot - 3;
        *y = -3;
    } else if (slot < 12) {
        *x = 3;
        *y = slot - 9;
    } else if (slot < 18) {
        *x = 15 - slot;
        *y = 3;
    } else {
        *x = -3;
        *y = 21 - slot;
    }
}
void clock_manager_run(void) {
    int old_face = -1, old_slot = -1, brightness = -1, dx = 0, dy = 0;
    time_t old_minute = -1;
    int old_date = -1;
    bool old_valid = false;
    time_t old_second = 0;
    int64_t next_frame = 0, next_log = 0;
    int64_t next_rotation = 0;
    bool refresh = true;
    unsigned frames = 0, overruns = 0;
    int64_t worst = 0;
    for (;;) {
        int64_t now = esp_timer_get_time();
        bool turned = false;
        if (now >= next_rotation && !touch_is_pressed()) {
            turned = display_rotation(gravity_angle());
            next_rotation = now + 250000;
        }
        struct timeval tv;
        gettimeofday(&tv, NULL);
        face_time_t t = {.utc = tv.tv_sec, .second = tv.tv_sec % 60 + tv.tv_usec / 1000000.0};
        localtime_r(&tv.tv_sec, &t.local);
        refresh |= face_manager_update(&t);
        touch_event_t e = touch_poll(now);
        bool was_ui = settings_ui_active(), auto_before = settings.auto_face;
        if (settings_ui_event(e))
            refresh = true;
        else if (!was_ui) {
            if (e.kind == TOUCH_TAP || e.kind == TOUCH_LEFT) {
                face_manager_manual(1);
                refresh = true;
            } else if (e.kind == TOUCH_RIGHT) {
                face_manager_manual(-1);
                refresh = true;
            } else if (e.kind == TOUCH_DOUBLE) {
                face_manager_auto(&t);
                ESP_ERROR_CHECK(settings_save());
                refresh = true;
            }
        }
        if (!auto_before && settings.auto_face) {
            face_manager_auto(&t);
            refresh = true;
        }
        refresh |= settings_ui_tick();
        bool ui = settings_ui_active();
        const clock_face_t *face = face_manager_current();
        int id = face_manager_index();
        /* A provisional midnight is not a real night: keep setup readable. */
        int b = clock_time_valid() ? settings_brightness(t.local.tm_hour) : settings.day;
        if (ui && b < 65)
            b = 65; /* Keep credentials readable, including during night hours. */
        if (b != brightness) {
            brightness = b;
            display_brightness(b);
            ESP_LOGI("Clock", "Brightness %d%%", b);
        }
        int slot = (tv.tv_sec / 120 + id * 5) % 24;
        bool valid = clock_time_valid();
        int date_key = t.local.tm_year * 366 + t.local.tm_yday;
        bool stat = refresh || (!valid && old_second != tv.tv_sec) || (face->minute_cache && old_minute != tv.tv_sec / 60) ||
                    old_date != date_key || old_face != id || old_slot != slot ||
                    old_valid != valid;
        bool sweep = face->sweep && settings.sweep && !ui;
        if (!sweep)
            t.second = t.local.tm_sec;
        if (stat || turned || (sweep && now >= next_frame) || old_second != tv.tv_sec) {
            int64_t begin = esp_timer_get_time();
            if (stat || ui) {
                if (slot != old_slot || id != old_face) {
                    int nx, ny;
                    offset(slot, &nx, &ny);
                    if (id != old_face && old_face >= 0 && nx == dx && ny == dy)
                        offset(slot + 1, &nx, &ny);
                    dx = nx;
                    dy = ny;
                }
                canvas_t *c = display_static_begin(dx, dy, ui ? 0x08101b : face->background);
                if (ui)
                    settings_ui_draw(c, &t);
                else if (valid) {
                    face->draw_static(c, &t);
                } else {
                    draw_clear(c,0x08101b);
                    draw_text(c,233,165,"TIME NOT SET",3,0xffd085);
                    network_status_t network=ntp_status();
                    draw_text(c,233,219,ntp_status_message(network),2,0xffffff);
                    draw_text(c,233,271,"TAP TO SET HOME WI-FI",2,0xa6c7df);
                    draw_text(c,233,310,network.phase==NET_WIFI_FAILED || network.phase==NET_NTP_FAILED
                        ? "RETRYING AUTOMATICALLY" : "WAITING FOR CORRECT TIME",1,0xa6c7df);
                }
                display_static_end();
            }
            canvas_t *c = display_frame_begin();
            if (!ui && valid)
                face->draw_dynamic(c, &t);
            display_frame_end();
            int64_t elapsed = esp_timer_get_time() - begin;
            if (elapsed > worst)
                worst = elapsed;
            if (sweep && elapsed > 16667)
                overruns++;
            frames++;
            old_face = id;
            old_minute = tv.tv_sec / 60;
            old_date = date_key;
            old_valid = valid;
            old_slot = slot;
            old_second = tv.tv_sec;
            refresh = false;
            if (sweep) {
                next_frame += 16667;
                if (next_frame < now)
                    next_frame = now + 16667;
            } else
                next_frame = now;
        }
        if (now >= next_log) {
            char current[32];
            strftime(current, sizeof(current), "%Y-%m-%d %H:%M:%S JST", &t.local);
            ESP_LOGI(
                "Clock",
                "Current time %s | RTC %s | NTP %lld | Current Face %s | Next Face %s | Brightness "
                "%d | Free heap %u internal %u | frames %u / late %u / max %lld us / pixels %u",
                current, rtc_valid() ? "valid" : "invalid", (long long)ntp_status().last_sync,
                face->name, face_manager_next(&t), brightness, (unsigned)esp_get_free_heap_size(),
                (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), frames, overruns,
                (long long)worst, display_last_pixels());
            next_log = now + 60000000;
            frames = overruns = 0;
            worst = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
