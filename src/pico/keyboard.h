// Copyright 2025 Ruben Berenguel

#pragma once

#include "../setup.h"
#include "controller/uni_keyboard.h"

void* get_modifiers_from_hid(const uni_keyboard_t* kb, KeyModifiers* mod);
