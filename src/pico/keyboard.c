// Copyright 2025 Ruben Berenguel

#include "../setup.h"
#include "controller/uni_keyboard.h"

void get_modifiers_from_hid(const uni_keyboard_t* kb, KeyModifiers* modifiers) {
  modifiers->shift = false;
  modifiers->ctrl = false;
  modifiers->meta = false;
  if (kb->modifiers & 0x8 || kb->modifiers & 0x80) {
    modifiers->meta = true;
  }
  if (kb->modifiers & 0x2 || kb->modifiers & 0x20) {
    modifiers->shift = true;
  }
  if (kb->modifiers & 0x1 || kb->modifiers & 0x10) {
    modifiers->ctrl = true;
  }
  return;
}
