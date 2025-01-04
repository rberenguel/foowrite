// Copyright 2025 Ruben Berenguel

#pragma once

#include "./badger2040.hpp"

int fastrand(int);

struct Point {
  int x;
  int y;
};
Point r45(const Point& pt);
Point ir45(const Point& pt);
void splash(pimoroni::Badger2040 badger);
int hrand(int n);
