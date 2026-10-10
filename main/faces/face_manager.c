#include "face_manager.h"
#include "schedule.h"
#include "settings.h"
#include "esp_log.h"
static int current = -1, last_day = -1, last_hour = -1;
static uint8_t schedule[24];
static bool select_face(int index) {
    if (index == current)
        return false;
    if (current >= 0 && faces[current]->deinit)
        faces[current]->deinit();
    current = index;
    if (faces[current]->init)
        faces[current]->init();
    ESP_LOGI("Face", "Current Face %02d - %s", current + 1, faces[current]->name);
    return true;
}
bool face_manager_update(const face_time_t *t) {
    int day = clock_day_number(t->local.tm_year + 1900, t->local.tm_mon + 1, t->local.tm_mday);
    bool hour_change = day != last_day || t->local.tm_hour != last_hour;
    if (day != last_day) {
        clock_schedule(t->local.tm_year + 1900, t->local.tm_mon + 1, t->local.tm_mday, schedule);
        last_day = day;
    }
    last_hour = t->local.tm_hour;
    return (current < 0 || (hour_change && settings.auto_face)) ? select_face(schedule[last_hour])
                                                                : false;
}
void face_manager_manual(int delta) {
    select_face((current + delta + FACE_COUNT) % FACE_COUNT);
}
void face_manager_auto(const face_time_t *t) {
    settings.auto_face = true;
    select_face(schedule[t->local.tm_hour]);
}
const clock_face_t *face_manager_current(void) {
    return faces[current];
}
int face_manager_index(void) {
    return current;
}
const char *face_manager_next(const face_time_t *t) {
    if (!settings.auto_face)
        return faces[current]->name;
    if (t->local.tm_hour < 23)
        return faces[schedule[t->local.tm_hour + 1]]->name;
    struct tm next = t->local;
    next.tm_mday++;
    next.tm_hour = 0;
    next.tm_min = 0;
    next.tm_sec = 0;
    mktime(&next);
    uint8_t s[24];
    clock_schedule(next.tm_year + 1900, next.tm_mon + 1, next.tm_mday, s);
    return faces[s[0]]->name;
}
