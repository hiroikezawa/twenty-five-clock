#pragma once
#include "face.h"
bool face_manager_update(const face_time_t *t);
void face_manager_manual(int delta);
void face_manager_auto(const face_time_t *t);
const clock_face_t *face_manager_current(void);
const char *face_manager_next(const face_time_t *t);
int face_manager_index(void);
