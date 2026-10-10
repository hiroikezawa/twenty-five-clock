/* Build the identical rasterizer/faces/scheduler as a Windows DLL for QA. */
#include "face.h"
#include "schedule.h"
#include <string.h>
#include <math.h>
#include "gesture.h"
__declspec(dllexport) int face_count(void) {
    return FACE_COUNT;
}
__declspec(dllexport) const char *face_name(int i) {
    return faces[i]->name;
}
__declspec(dllexport) void test_gesture_reset(void) {
    gesture_reset();
}
__declspec(dllexport) int test_gesture(int down, int x, int y, int64_t now) {
    return gesture_update(down != 0, x, y, now).kind;
}
static uint16_t bg_pixels[466 * 466], frame_pixels[466 * 466], reference[466 * 466];
static uint16_t rotated_bg[466 * 466];
static float test_angle;
__declspec(dllexport) int test_twelve_hour_dynamic(int id) {
    face_time_t t = {
        .local =
            {.tm_year = 126, .tm_mon = 9, .tm_mday = 9, .tm_hour = 10, .tm_min = 9, .tm_wday = 5},
        .utc = 1791508140,
        .second = 38.5};
    canvas_t bg = {.pixels = bg_pixels}, a = {.pixels = frame_pixels}, b = {.pixels = reference};
    draw_clear(&bg, faces[id]->background);
    faces[id]->draw_static(&bg, &t);
    memcpy(frame_pixels, bg_pixels, sizeof(bg_pixels));
    memcpy(reference, bg_pixels, sizeof(bg_pixels));
    a.font_style = b.font_style = bg.font_style;
    faces[id]->draw_dynamic(&a, &t);
    t.local.tm_hour += 12;
    faces[id]->draw_dynamic(&b, &t);
    return memcmp(frame_pixels, reference, sizeof(reference)) == 0;
}
__declspec(dllexport) int test_dirty_restore(int id, int dx, int dy) {
    face_time_t t = {.local = {.tm_year = 126,
                               .tm_mon = 9,
                               .tm_mday = 7,
                               .tm_hour = 10,
                               .tm_min = 9,
                               .tm_sec = 0,
                               .tm_wday = 3},
                     .utc = 1791335340};
    canvas_t bg = {.pixels = bg_pixels, .dx = dx, .dy = dy};
    canvas_t frame = {.pixels = frame_pixels, .dx = dx, .dy = dy, .tracking = true};
    canvas_t ref = {.pixels = reference, .dx = dx, .dy = dy};
    draw_clear(&bg, faces[id]->background);
    faces[id]->draw_static(&bg, &t);
    if (test_angle != 0) {
        canvas_t target = {.pixels = rotated_bg};
        draw_rotate_cache(&target, &bg, test_angle);
        memcpy(bg_pixels, rotated_bg, sizeof(bg_pixels));
        frame.rotated = ref.rotated = true;
        frame.rc = ref.rc = cosf(test_angle);
        frame.rs = ref.rs = sinf(test_angle);
    }
    frame.font_style = ref.font_style = bg.font_style;
    memcpy(frame_pixels, bg_pixels, sizeof(frame_pixels));
    for (int i = 0; i < 120; i++) {
        draw_restore(&frame, &bg, frame.dirty);
        memset(frame.dirty, 0, sizeof(frame.dirty));
        memcpy(reference, bg_pixels, sizeof(reference));
        t.second = i * 0.5;
        faces[id]->draw_dynamic(&frame, &t);
        faces[id]->draw_dynamic(&ref, &t);
        if (memcmp(frame_pixels, reference, sizeof(reference)))
            return i + 1;
    }
    return 0;
}
__declspec(dllexport) int test_rotated_restore(int id, int dx, int dy, float angle) {
    test_angle = angle;
    int result = test_dirty_restore(id, dx, dy);
    test_angle = 0;
    return result;
}
__declspec(dllexport) int test_rotate_point(float angle, float x, float y) {
    canvas_t c = {.pixels = frame_pixels, .rotated = true, .rc = cosf(angle), .rs = sinf(angle)};
    memset(frame_pixels, 0, sizeof(frame_pixels));
    draw_disc(&c, x, y, 2, 0xffffff);
    int px = lroundf(233 + cosf(angle) * (x - 233) - sinf(angle) * (y - 233));
    int py = lroundf(233 + sinf(angle) * (x - 233) + cosf(angle) * (y - 233));
    return frame_pixels[py * 466 + px] != 0;
}
__declspec(dllexport) void test_schedule(int y, int m, int d, uint8_t *out) {
    clock_schedule(y, m, d, out);
}
__declspec(dllexport) void render_face(int id, int year, int month, int day, int hour, int minute,
                                       double second, uint16_t *pixels) {
    face_time_t t = {0};
    t.local.tm_year = year - 1900;
    t.local.tm_mon = month - 1;
    t.local.tm_mday = day;
    t.local.tm_hour = hour;
    t.local.tm_min = minute;
    t.local.tm_sec = (int)second;
    int days = clock_day_number(year, month, day);
    t.local.tm_wday = (days + 4) % 7;
    t.utc = (time_t)days * 86400 + hour * 3600 + minute * 60 + (int)second - 9 * 3600;
    t.second = second;
    canvas_t c = {.pixels = pixels};
    draw_clear(&c, faces[id]->background);
    faces[id]->draw_static(&c, &t);
    faces[id]->draw_dynamic(&c, &t);
}
