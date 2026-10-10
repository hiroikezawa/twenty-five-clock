#pragma once
#include <stdint.h>
/* Gregorian calendar, independent of timezone/library state. */
int32_t clock_day_number(int year, int month, int day);
uint32_t clock_date_seed(int year, int month, int day);
void clock_schedule(int year, int month, int day, uint8_t out[24]);
