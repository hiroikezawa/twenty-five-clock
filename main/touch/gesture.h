#pragma once
#include "touch_manager.h"
#include <stdbool.h>
void gesture_reset(void);
touch_event_t gesture_update(bool pressed, int x, int y, int64_t now);
