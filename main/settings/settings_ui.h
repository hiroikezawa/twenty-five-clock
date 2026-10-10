#pragma once
#include "draw.h"
#include "touch_manager.h"
#include "face.h"
bool settings_ui_active(void);
bool settings_ui_event(touch_event_t e);
bool settings_ui_tick(void);
void settings_ui_draw(canvas_t *c, const face_time_t *t);
