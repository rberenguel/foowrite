// Copyright 2025 Ruben Berenguel

// All ugly codebases should have a "common".
// I needed this here for some reason (I think
// to be able to use the differ in tests and in the pico).

#pragma once
#include <list>
#include <vector>
#include <string>

constexpr inline int max(int a, int b) {
  return a > b ? a : b;
}
constexpr inline int min(int a, int b) {
  return a < b ? a : b;
}

std::vector<int> StringDiffer(const std::vector<std::string>& outputting,
                              const std::vector<std::string>& displayed,
                              const int cursor_pos);
