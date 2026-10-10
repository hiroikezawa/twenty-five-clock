#include "render_benchmark.h"
#include "sdkconfig.h"
#ifdef CONFIG_CLOCK_SELF_TEST
#include "display_manager.h"
#include "face.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/time.h>
void render_benchmark(void) {
    ESP_LOGI("Benchmark", "25 faces / 120 animated frames each; no NVS or RTC changes");
    display_brightness(85);
    display_rotation(0.645771823f); /* 37 degrees: non-cardinal rotation */
    for (int id = 0; id < FACE_COUNT; id++) {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        face_time_t t = {.utc = tv.tv_sec};
        localtime_r(&tv.tv_sec, &t.local);
        t.local.tm_hour = 10;
        t.local.tm_min = 9; /* Separated hands, not overlapping at midnight. */
        canvas_t *c = display_static_begin(0, 0, faces[id]->background);
        faces[id]->draw_static(c, &t);
        display_static_end();
        c = display_frame_begin();
        faces[id]->draw_dynamic(c, &t);
        display_frame_end();
        int64_t sum = 0, max = 0, draw_sum = 0, flush_sum = 0, start = esp_timer_get_time();
        unsigned late = 0, pixels = 0;
        for (int i = 0; i < 120; i++) {
            int64_t begin = esp_timer_get_time();
            t.second = i * 0.5; /* Cover a full minute and every angle. */
            c = display_frame_begin();
            faces[id]->draw_dynamic(c, &t);
            int64_t drawn = esp_timer_get_time();
            display_frame_end();
            draw_sum += drawn - begin;
            flush_sum += esp_timer_get_time() - drawn;
            int64_t elapsed = esp_timer_get_time() - begin;
            sum += elapsed;
            if (elapsed > max)
                max = elapsed;
            if (elapsed > 16667)
                late++;
            pixels += display_last_pixels();
            vTaskDelay(1);
        }
        ESP_LOGI("Benchmark",
                 "%02d %s: avg=%lld us max=%lld us late=%u/120 mean_pixels=%u capacity=%.1f fps "
                 "internal_heap=%u draw=%lld us flush=%lld us",
                 id + 1, faces[id]->name, (long long)(sum / 120), (long long)max, late,
                 pixels / 120, 120000000.0 / (esp_timer_get_time() - start),
                 (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                 (long long)(draw_sum / 120), (long long)(flush_sum / 120));
    }
    ESP_LOGI("Benchmark", "complete; proceeding to normal RTC clock and Wi-Fi provisioning");
}
#endif
