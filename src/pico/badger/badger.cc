// Copyright 2025 Ruben Berenguel

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <string>

#include "../../common.h"
#include "../../output.hpp"
#include "./badger2040.hpp"
#include "./common/pimoroni_common.hpp"
#include "./pico/stdlib.h"
#include "./setup.h"
#include "./splash.hpp"

// This is a WIP with the Badger2040 screen from Pimoroni, using
// partial updates for each character. I eventually gave up and
// went with LCD. Keeping the screen in sync properly was too much
// of a hassle.

// This won't match the current definitions used for the gfx pack,
// so won't compile.

#define N_LINES 5

pimoroni::Badger2040 badger;

void Output::CommandLine(const std::list<char>& command,
                         const std::string& log) {
  std::string as_str(command.begin(), command.end());
  this->CommandLine(as_str, log);
}

void Output::CommandLine(const std::string& command, const std::string& log) {
  badger.update_speed(3);
  badger.font("sans");
  badger.thickness(2);
  badger.pen(15);
  badger.rectangle(0, 112, 296, 16);
  badger.pen(0);
  badger.text(command, 0, 120, 0.5);
  badger.partial_update(0, 112, 296, 16, true);
}

void Output::Emit(const std::string& current_line_str, const int cursor_pos) {
  // Should:
  // - Keep track of what is displayed
  // - Compare the current line splits with the displayed line splits
  // - Anything equal at the beginning of the line stays
  // - Anything different up to the end of the line triggers a clean
  if (emits_with_no_clear > 100) {
    badger.update(true);
  }
  uint8_t updateFlags = 0;
  badger.update_speed(3);
  badger.thickness(2);
  badger.font("sans");
  std::vector<std::string> chunks;
  int chunk_size = 22;  // eyeballing the font size
  if (current_line_str.size() == 0) {
    badger.pen(15);
    badger.clear();
    badger.update();
    displayed_lines_.clear();
  }
  /*auto it = current_line.begin();

  while (it != current_line.end()) {
    std::list<char> chunk;
    int count = 0;

    while (it != current_line.end() && count < chunk_size) {
      chunk.push_back(*it);
      ++it;
      ++count;
    }

    chunks.push_back(chunk);
  }*/
  auto starting_index =
      cursor_pos /
      (chunk_size *
       N_LINES);  // This assumes the cursor is always in the string, beware
  printf("pos: %d / (%d) - %s\n", cursor_pos, starting_index,
         current_line_str.c_str());
  auto count = 0;
  for (size_t i = starting_index * (chunk_size * N_LINES);
       i < current_line_str.size(); i += chunk_size) {
    chunks.push_back(current_line_str.substr(i, chunk_size));
    ++count;  // number of chunks
    if (count == N_LINES) {
      break;
    }
  }

  if (displayed_lines_.empty()) {
    // Populate the list
    for (int i = 0; i < fmin(N_LINES, chunks.size()); i++) {
      // Make sure this is always populated!
      printf("Cleaned displayed lines for #chunks %d\n", chunks.size());
      displayed_lines_.emplace_back("");
    }
  }

  auto diffed = StringDiffer(chunks, displayed_lines_, cursor_pos);
  printf(("\ndiffed size: " + std::to_string(diffed.size()) + "\n").c_str());
  for (int i = 0; i < diffed.size(); i++) {
    if (i >= chunks.size()) {
      if (displayed_lines_.size() >= i) {
        badger.pen(15);
        badger.clear();
        badger.update(/*blocking=*/true);
        displayed_lines_.clear();
        printf("Full clear\n");
        break;
      }
    }
    auto diff_start = std::next(displayed_lines_[i].begin(), diffed[i]);
    auto diff2_start = std::next(chunks[i].begin(), diffed[i]);
    std::string same(displayed_lines_[i].begin(), diff_start);
    if (diff_start != displayed_lines_[i].begin()) {
      --diff_start;
    }
    std::string dl(displayed_lines_[i].begin(), displayed_lines_[i].end());
    std::string diff(diff_start, displayed_lines_[i].end());
    std::string diff2(diff2_start, chunks[i].end());
    printf(("\ndstart: %s (%d/%d) same: %s diff: %s diff2: %s disp: %s\n"),
           std::to_string(diffed[i]).c_str(), chunks.size(),
           displayed_lines_.size(), same.c_str(), diff.c_str(), diff2.c_str(),
           dl.c_str());
    auto skip_size = badger.measure_text(same, 0.8);
    auto clear_size = badger.measure_text(diff, 0.8);
    auto clear_size2 = badger.measure_text(diff2, 0.8);
    printf(("\n1: skip: " + std::to_string(skip_size) +
            " clear: " + std::to_string(clear_size) + "\n")
               .c_str());
    clear_size = min(max(clear_size, clear_size2) + 24, 296);
    skip_size = max(skip_size - 24, 0);
    printf(("\n2: skip: " + std::to_string(skip_size) +
            " clear: " + std::to_string(clear_size) + "\n")
               .c_str());
    badger.pen(15);
    badger.rectangle(skip_size, 24 * i, clear_size, 24);
    printf("FFS: %d %d\n", i, displayed_lines_.size());
    if (i >= displayed_lines_.size()) {
      displayed_lines_.push_back("");
    }
    displayed_lines_[i].assign(chunks[i].begin(), chunks[i].end());
    std::string as_string(displayed_lines_[i].begin(),
                          displayed_lines_[i].end());
    badger.pen(0);
    badger.text(as_string, 0, 8 + i * 24, 0.80f);
    printf(("\n loop level: " + std::to_string(i) + " y: " +
            std::to_string(i * 24) + ":" + std::to_string(24 + i * 24) + "\n")
               .c_str());
    if (diffed[i] >= 22) {  // Size of screen
      printf("Skipping partial update for this line");
      continue;
    } else {
      if (!badger.is_busy()) {
        badger.partial_update(skip_size, i * 24, clear_size, 24 + i * 24, true);
      } else {
        printf("Badger busy\n");
      }
    }
  }
  ++emits_with_no_clear;
}

void flush() {
  badger.update_speed(0);
  badger.clear();
  for (auto i = 0; i < 296; i++) {
    for (auto j = 0; j < 128; j++) {
      if (fastrand(2) > 0) {
        badger.pen(0);
      } else {
        badger.pen(15);
      }
      badger.pixel(i, j);
    }
  }
  badger.update();
}

void Output::Init(Editor* e) {
  editor = e;
  for (auto i = 0; i < N_LINES; i++) {
    displayed_lines_.push_back("");
  }
  printf("\n\n=======\nbadger2040 starting up\n\n");

  badger.init();
  badger.update_speed(0);
  badger.pen(15);
  badger.clear();
  badger.update();
}

void Output::Command(const OutputCommands& command) {
  switch (command) {
    case OutputCommands::kSplash:
      splash(badger);
      break;
    default:
      break;
  }
}
