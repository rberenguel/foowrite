// Copyright 2025 Ruben Berenguel

#include <list>
#include <string>
#include <vector>
#include "./common.h"

// This was part of the machinery to do partial updates on the Badger eink
// It never worked flawlessly, but when it worked well it was awesome.

std::vector<int> StringDiffer(const std::vector<std::string>& outputting,
                              const std::vector<std::string>& displayed,
                              const int cursor) {
  std::vector<int> returning(max(outputting.size(), displayed.size()));
  int current_char = 0;
  printf("sd: %d / %d\n", static_cast<int>(outputting.size()),
         static_cast<int>(displayed.size()));
  for (auto i = 0; i < min(outputting.size(), displayed.size()); i++) {
    // For each line, skip all common characters, counting them
    auto output = outputting[i];
    auto display = displayed[i];
    printf("sd[%d]: o: %s d: %s\n", i, output.c_str(), display.c_str());
    auto out_it = output.begin();
    auto disp_it = display.begin();
    returning[i] = -1;
    if (out_it != output.end() || disp_it != display.end()) {
      // If there is some output _or_
      // the display line is dirty we start counting
      returning[i] = 0;
    }
    while ((out_it != output.end() || disp_it != display.end()) &&
           *out_it == *disp_it) {
      ++(returning[i]);
      ++out_it;
      ++disp_it;
      ++current_char;
    }
  }
  return returning;
}
