// Copyright 2025 Ruben Berenguel

#pragma once

#include "./keyboard.h"
#include "./pico/util/queue.h"

typedef struct {
  uint8_t keycode;
  KeyModifiers modifiers;
} queue_entry_t;

extern queue_t call_queue;
