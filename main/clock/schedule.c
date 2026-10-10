#include "schedule.h"
#include "face_count.h"
#include <stdio.h>
int32_t clock_day_number(int y, int m, int d) {
    y -= m <= 2;
    int era = (y >= 0 ? y : y - 399) / 400;
    unsigned yo = y - era * 400;
    unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    return era * 146097 + (int)(yo * 365 + yo / 4 - yo / 100 + doy) - 719468;
}
uint32_t clock_date_seed(int y, int m, int d) {
    char date[16];
    snprintf(date, sizeof(date), "%04d-%02d-%02d", y, m, d);
    uint32_t h = 2166136261u;
    for (char *p = date; *p; p++) {
        h ^= (uint8_t)*p;
        h *= 16777619u;
    }
    return h;
}
static uint32_t next(uint32_t *s) {
    *s += 0x9e3779b9u;
    uint32_t z = *s;
    z = (z ^ (z >> 16)) * 0x85ebca6bu;
    z = (z ^ (z >> 13)) * 0xc2b2ae35u;
    return z ^ (z >> 16);
}
static unsigned bounded(uint32_t *s, unsigned n) {
    uint32_t v, limit = (uint32_t)(-n) % n;
    do {
        v = next(s);
    } while (v < limit);
    return v % n;
}
static void swap(uint8_t *a, uint8_t *b) {
    uint8_t t = *a;
    *a = *b;
    *b = t;
}
/* Today omits a rotating face; the final face is tomorrow's omitted face.
   Midnight cannot repeat, and every 25-day window uses each face 24 times. */
void clock_schedule(int y, int m, int d, uint8_t out[24]) {
    _Static_assert(FACE_COUNT == 25, "25-face daily allocation");
    uint32_t seed = clock_date_seed(y, m, d);
    int day = clock_day_number(y, m, d);
    int omitted = (day % FACE_COUNT + FACE_COUNT) % FACE_COUNT;
    int terminal = (omitted + 1) % FACE_COUNT;
    int n = 0;
    for (int i = 0; i < FACE_COUNT; i++)
        if (i != omitted)
            out[n++] = i;
    for (int i = 23; i > 0; i--)
        swap(&out[i], &out[bounded(&seed, i + 1)]);
    for (int i = 0; i < 24; i++)
        if (out[i] == terminal) {
            swap(&out[i], &out[23]);
            break;
        }
}
