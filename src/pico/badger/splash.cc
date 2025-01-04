// Copyright 2025 Ruben Berenguel

// Splash screen with mountains for the resolution of
// the Badger.

#include "./splash.hpp"
#include "./badger2040.hpp"
#include "./hardware/structs/rosc.h"

int dwSeed = 42;

int fastrand(int x) {
  dwSeed = dwSeed * 0x343fd + 0x269ec3;
  return (static_cast<int>(((dwSeed >> 16) & 0x7fff) * x >> 15));
}

Point r45(const Point& pt) {
  float rotatedX = (pt.x - pt.y) / 1.4142;
  float rotatedY = (pt.x + pt.y) / 1.4142;
  return {static_cast<int>(rotatedX), static_cast<int>(rotatedY)};
}

Point ir45(const Point& pt) {
  float originalX = (pt.x + pt.y) / 1.4142;
  float originalY = (pt.y - pt.x) / 1.4142;
  return {static_cast<int>(originalX), static_cast<int>(originalY)};
}

void splash(pimoroni::Badger2040 badger) {
  badger.update_speed(0);
  badger.thickness(2);
  dwSeed = to_ms_since_boot(get_absolute_time()) | rosc_hw->randombit;
  fastrand(to_ms_since_boot(get_absolute_time()) |
           rosc_hw->randombit);  // Jiggle
  badger.pen(15);
  badger.clear();
  badger.pen(0);
  std::vector<Point> points;

  int x1 = 60 + fastrand(40);
  int y1 = 20 + fastrand(20);
  int xS = fastrand(40);

  Point p1 = {x1, y1};
  Point p2 = {x1 + 100 - xS, y1 - 15};
  Point p3 = {p2.x + 70, p2.y + xS};
  // Point p1 = {88, 32};
  // Point p2 = {172, 17};
  // Point p3 = {222, 37};
  points.emplace_back(p1);
  points.emplace_back(p2);
  points.emplace_back(p3);

  // 'console.log' equivalent - likely using 'std::cout' or your logging
  // function

  std::vector<Point> range;
  // Iterate through the points
  int i = 0;
  for (auto i = 0; i < points.size(); i++) {
    Point pt = points[i];
    badger.pixel(pt.x, pt.y);
    if (i == 0) {
      Point p = {pt.x - 128 + pt.y, 128};
      if (p.x < 0) {
        p.y = p.y + p.x;
        p.x = 1;
      }
      badger.pixel(p.x, p.y);
      range.emplace_back(p);
    }

    if (i > 0) {
      const Point ptm1 = points[i - 1];
      const Point r1 = r45(ptm1);
      const Point r2 = r45(pt);
      const Point p = ir45({r1.x, r2.y});
      range.emplace_back(p);
      badger.line(p.x, p.y, p.x + 3, p.y + 3);
    }
    range.push_back(pt);

    if (i == points.size() - 1) {
      Point p = {pt.x + 128 - pt.y, 128};
      if (p.x > 296) {
        p.y = p.y + (295 - p.x);
        p.x = 295;
      }
      badger.pixel(p.x, p.y);
      badger.pixel(p.x, p.y + 1);
      range.emplace_back(p);
    }
  }
  // return;
  //  Draw lines connecting the points in 'range'
  for (auto i = 1; i < range.size(); i++) {
    const Point p = range[i - 1];
    const Point q = range[i];
    badger.line(p.x, p.y, q.x, q.y);
    badger.line(p.x + 1, p.y, q.x + 1, q.y);
  }

  for (const Point& pt : points) {
    const Point p1 = {pt.x - 12, pt.y + 36};
    const Point p2 = {p1.x - 12, p1.y + 18};
    const Point p2w = {p1.x + 9, p1.y + 6};
    const Point p3 = {p2.x - 4, p2.y + 12};
    const Point p3w = {p2.x + 5, p2.y + 5};

    badger.line(pt.x, pt.y, p1.x, p1.y);
    badger.line(p1.x, p1.y, p2.x, p2.y);
    badger.line(p1.x, p1.y, p2w.x, p2w.y);
    badger.line(p2.x, p2.y, p3w.x, p3w.y);
    badger.line(p2.x, p2.y, p3.x, p3.y);
  }
  badger.update_speed(0);
  badger.pen(15);
  badger.font("serif");
  auto size = badger.measure_text("foowrite", 0.8f);
  badger.rectangle(85, 60, 70 + static_cast<int>(0.4 * size), 24);
  badger.pen(0);
  badger.thickness(1);
  badger.text("foowrite", 85, 68, 0.8f);
  badger.font("cursive");
  // "Writer's block is an illusion. Procrastination doubly so." Based on Adams.
  // "I love deadlines. I love the whooshing noise they make as they go by."
  int threshold = hrand(10);
  printf(("Random threshold: " + std::to_string(threshold) + "\n").c_str());
  badger.pen(15);
  if (threshold > 0) {
    badger.rectangle(0, 99, 296, 48);
    badger.pen(0);
    badger.text("The mountains are calling", 40, 110, 0.65f);
    badger.text("and I must write", 80, 121, 0.65f);
  }
  /*if (threshold >= 0 && threshold <= 9) {
    badger.rectangle(0, 94, 296, 48);
    badger.pen(0);
    badger.text("Ever tried. Ever failed. No matter.", 5, 102, 0.65f);
    badger.text("Try Again. Fail again. Fail better.", 5, 115, 0.65f);
  }
  if (threshold == 10) {
    badger.pen(15);
    badger.rectangle(0, 104, 296, 24);
    badger.pen(0);
    badger.text("Not all those who wander are lost", 5, 115, 0.65f);
  }*/
  badger.update();
}

int hrand(int n) {
  volatile uint32_t acc = 0;
  for (int i = 0; i < n; i++) {
    volatile uint32_t* rnd_reg =
        reinterpret_cast<uint32_t*>(ROSC_BASE + ROSC_RANDOMBIT_OFFSET);
    acc += *rnd_reg;
  }
  return static_cast<int>(acc);
}
