// Copyright 2025 Ruben Berenguel

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <array>

#include "./font8_data.hpp"
#include "./pico/stdlib.h"
#include "./pico_graphics.hpp"
#include "./pico_unicorn.hpp"
#include "./setup.h"

// For fun, I started implementing a version for the Pimoroni
// Unicorn, a 17x7 coloured led matrix. I never figured out
// a good way to display the text in a vim-like way, the only
// option I could think of was going the ed way
// (https://en.wikipedia.org/wiki/Ed_(software)), but my
// efforts went to the Badger version initially.

// This won't match the current definitions used for the gfx pack,
// so won't compile.

pimoroni::PicoUnicorn pico_unicorn;
pimoroni::PicoGraphics_PenP8 graphics = pimoroni::PicoGraphics_PenP8(
    pico_unicorn.WIDTH, pico_unicorn.HEIGHT, nullptr);
pimoroni::Pen BLACK;
pimoroni::Pen BLUE;
pimoroni::Pen YELLOW;
pimoroni::Pen RED;


void emit(std::string s, std::string log, const KeyModifiers* modifiers) {
  graphics.set_pen(BLACK);
  graphics.clear();
  if (s.length() > 1) {
    graphics.set_pen(RED);
  } else {
    graphics.set_pen(BLUE);
  }

  graphics.set_font(&font8);
  graphics.text(s, pimoroni::Point(0, 0), 0, 1.0);
  pico_unicorn.update(&graphics);
}

void display(std::string line) {
  int t = pico_unicorn.WIDTH;
  for (auto i = 0; i < line.length() + 4 * pico_unicorn.WIDTH;
       i++) {  // The font has 4 pixels of width
    graphics.set_pen(BLACK);
    graphics.clear();
    graphics.set_pen(YELLOW);
    graphics.set_font(&font8);
    graphics.text(line, pimoroni::Point(t, 0), 0, 1.0);
    pico_unicorn.update(&graphics);
    sleep_ms(100);
    t--;
  }
}

void output_init() {
  BLACK = graphics.create_pen(0, 0, 0);
  BLUE = graphics.create_pen(0, 0, 100);
  YELLOW = graphics.create_pen(100, 50, 0);
  RED = graphics.create_pen(100, 0, 0);
  pico_unicorn.init();
  graphics.set_pen(BLACK);
  graphics.clear();
}
