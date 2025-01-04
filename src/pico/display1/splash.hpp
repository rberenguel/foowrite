// Copyright 2025 Ruben Berenguel

#pragma once

#include "./pico_graphics.hpp"

int fastrand(int);

struct Point {
  int x;
  int y;
};
Point r45(const Point& pt);
Point ir45(const Point& pt);
void splash(pimoroni::PicoGraphics_PenP8 graphics);
int hrand(int n);
