// Copyright 2025 Ruben Berenguel

// Splash screen for the resolution of the GFX pack

#include <vector>

#include "./hardware/structs/rosc.h"
#include "./pico_graphics.hpp"
#include "./splash.hpp"

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

void splash(pimoroni::PicoGraphics_Pen1Bit graphics) {
  dwSeed = to_ms_since_boot(get_absolute_time()) | rosc_hw->randombit;
  fastrand(to_ms_since_boot(get_absolute_time()) |
           rosc_hw->randombit);  // Jiggle
  graphics.set_pen(0);
  graphics.clear();
  graphics.set_pen(15);
  std::vector<Point> points;

  int x1 = 15 + fastrand(20);
  int y1 = 5 + fastrand(10);
  int xS = fastrand(20);

  Point p1 = {x1, y1};
  Point p2 = {x1 + 50 - xS, y1 - 7};
  Point p3 = {p2.x + 35, p2.y + xS};

  points.emplace_back(p1);
  points.emplace_back(p2);
  points.emplace_back(p3);

  std::vector<Point> range;
  // Iterate through the points
  int i = 0;
  for (auto i = 0; i < points.size(); i++) {
    Point pt = points[i];
    graphics.pixel(pimoroni::Point(pt.x, pt.y));
    if (i == 0) {
      Point p = {pt.x - 64 + pt.y, 64};
      if (p.x < 0) {
        p.y = p.y + p.x;
        p.x = 1;
      }
      graphics.pixel(pimoroni::Point(pt.x, pt.y));
      range.emplace_back(p);
    }

    if (i > 0) {
      const Point ptm1 = points[i - 1];
      const Point r1 = r45(ptm1);
      const Point r2 = r45(pt);
      const Point p = ir45({r1.x, r2.y});
      range.emplace_back(p);
      graphics.line(pimoroni::Point(pt.x, pt.y),
                    pimoroni::Point(pt.x + 3, pt.y + 3));
    }
    range.push_back(pt);

    if (i == points.size() - 1) {
      Point p = {pt.x + 64 - pt.y, 64};
      if (p.x > 128) {
        p.y = p.y + (127 - p.x);
        p.x = 127;
      }
      graphics.pixel(pimoroni::Point(pt.x, pt.y));
      graphics.pixel(pimoroni::Point(pt.x, pt.y + 1));
      range.emplace_back(p);
    }
  }
  //  Draw lines connecting the points in 'range'
  for (auto i = 1; i < range.size(); i++) {
    const Point p = range[i - 1];
    const Point q = range[i];
    graphics.line(pimoroni::Point(p.x, p.y), pimoroni::Point(q.x, q.y));
    graphics.line(pimoroni::Point(p.x + 1, p.y), pimoroni::Point(q.x + 1, q.y));
  }

  for (const Point& pt : points) {
    const Point p1 = {pt.x - 6, pt.y + 18};
    const Point p2 = {p1.x - 6, p1.y + 9};
    const Point p2w = {p1.x + 4, p1.y + 3};
    const Point p3 = {p2.x - 2, p2.y + 6};
    const Point p3w = {p2.x + 3, p2.y + 2};

    graphics.line(pimoroni::Point(pt.x, pt.y), pimoroni::Point(p1.x, p1.y));
    graphics.line(pimoroni::Point(p1.x, p1.y), pimoroni::Point(p2.x, p2.y));
    graphics.line(pimoroni::Point(p1.x, p1.y), pimoroni::Point(p2w.x, p2w.y));
    graphics.line(pimoroni::Point(p2.x, p2.y), pimoroni::Point(p3w.x, p3w.y));
    graphics.line(pimoroni::Point(p2.x, p2.y), pimoroni::Point(p3.x, p3.y));
  }
  graphics.set_pen(15);
  graphics.set_font("serif");
  auto size = graphics.measure_text("foowrite", 0.6f);
  graphics.rectangle(pimoroni::Rect(22, 14, size + 1, 15));
  graphics.set_pen(0);
  graphics.set_thickness(1);
  graphics.text("foowrite", pimoroni::Point(23, 21), 128, 0.6f);
  graphics.pixel(pimoroni::Point(22, 14));
  graphics.pixel(pimoroni::Point(22, 28));
  graphics.pixel(pimoroni::Point(22 + size, 14));
  graphics.pixel(pimoroni::Point(22 + size, 28));

  graphics.set_pen(15);
  // With the current zoom, aliasing looks weird on i and o
  // Correct i dot
  graphics.pixel(pimoroni::Point(22 + 59, 15));
  // Correct o roundness
  graphics.pixel(pimoroni::Point(22 + 18, 21));
  graphics.pixel(pimoroni::Point(22 + 18 + 12, 21));
  graphics.set_pen(0);
  graphics.pixel(pimoroni::Point(22 + 14, 19));
  graphics.pixel(pimoroni::Point(22 + 14 + 12, 19));
  graphics.set_pen(15);  // To erase space for the quotes
  // Other potential quotes, both Douglas Adams.
  // "Writer's block is an illusion. Procrastination doubly so."
  // "I love deadlines. I love the whooshing noise they make as they go by."
  int threshold = hrand(10);
  graphics.set_font("bitmap8");
  graphics.set_pen(0);
  if (threshold < 6) {
    graphics.rectangle(pimoroni::Rect(0, 41, 128, 23));
    graphics.set_pen(15);
    graphics.text("The mountains are calling", pimoroni::Point(4, 45), 128,
                  0.5f);
    graphics.text("    and I must write", pimoroni::Point(7, 53), 128, 0.5f);
    return;
  }
  if (threshold < 8) {
    graphics.rectangle(pimoroni::Rect(0, 31, 128, 64 - 31));
    graphics.set_pen(15);
    graphics.text("Ever tried. Ever failed.", pimoroni::Point(4, 32), 128,
                  0.5f);
    graphics.text(" No matter.", pimoroni::Point(4, 40), 128, 0.5f);
    graphics.text("Try Again. Fail again.", pimoroni::Point(4, 48), 128, 0.5f);
    graphics.text(" Fail better.", pimoroni::Point(4, 56), 128, 0.5f);
    return;
  }
  if (threshold > 0) {
    graphics.rectangle(pimoroni::Rect(0, 48, 296, 16));
    graphics.set_pen(15);
    graphics.text("Not all those who wander", pimoroni::Point(0, 48), 128,
                  0.5f);
    graphics.text("                are lost", pimoroni::Point(0, 56), 128,
                  0.5f);
  }
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
