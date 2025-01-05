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
#include "pico/time.h"

// This is a WIP with the Badger2040 screen from Pimoroni, using
// partial updates for each character. I eventually gave up and
// went with LCD. Keeping the screen in sync properly was too much
// of a hassle.

// This won't match the current definitions used for the gfx pack,
// so won't compile.

#define N_LINES 5

pimoroni::Badger2040 badger;

std::string on_screen = "";

int last_key_in = -1;

struct ScreenUpdate {
  long unsigned int x;
  long unsigned int y;
  long unsigned int width;
  long unsigned int height;
};

std::list<ScreenUpdate> screen_updates;

void AdjustYAndHeight(ScreenUpdate* update) {
  // Calculate the remainder when y is divided by 8
  int remainder = update->y % 8;

  // Adjust y to be the nearest multiple of 8 less than or equal to the current y
  update->y -= remainder;

  // Adjust height to compensate for the change in y
  update->height += remainder;

  // Ensure the height covers the original area by adding any remaining difference
  // to make it a multiple of 8.
  int heightRemainder = update->height % 8;
  if (heightRemainder != 0) {
    update->height += 8 - heightRemainder;
  }
}

void MergeUpdates() {
  printf("Merging\n");
  if (screen_updates.empty()) {
    return;
  }
  for (auto& update : screen_updates) {
    AdjustYAndHeight(&update);
  }
  std::list<ScreenUpdate> merged;
  bool is_first = true;
  ScreenUpdate prev;
  for (const auto& curr : screen_updates) {
    printf("(%d,%d - %d x %d) ", curr.x, curr.y, curr.width, curr.height);
    if (is_first) {
      prev = curr;
      is_first = false;
      continue;  // Kind of ugly, but I don't want to fiddle with iterators
    }
    if (prev.y == curr.y && (prev.x + prev.width == curr.x)) {
      prev.width += curr.width;
    } else {
      merged.push_back(prev);
      prev = curr;
    }
  }
  merged.push_back(prev);
  printf("\n");
  for (auto& m : merged) {
    printf("Merged: (%d,%d - %d x %d)\n", m.x, m.y, m.width, m.height);
  }
  screen_updates.clear();
  screen_updates = std::move(merged);
}

void text(pimoroni::Badger2040* badger, const std::string_view& t,
          const int cursor_pos, const int32_t x, const int32_t y,
          const int32_t wrap, const uint8_t scale, EditorMode mode,
          int* prev_line_start_, int* next_line_start_) {
  // This comes from Pimoroni's PicoGraphics library, with some tweaks:
  // - Figure out cursor positioning.
  // - Draw the cursor depending on the editor mode.
  // - Provide a previous/next lie for long-line pagination.
  // To do it, loops twice, one to count everything and another to
  // draw everything
  std::string tc = "";
  tc = t;
  printf("To write: \n --- \n %s \n ---\n", tc.c_str());
  const uint8_t letter_spacing = 1;
  bool fixed_width = true;
  int num_lines = 0;
  const bitmap::font_t* font = &font8;  // TODO(me) improve
  uint32_t char_offset = 0;
  uint32_t line_offset = 0;  // line (if wrapping) offset
  unicode_sorta::codepage_t codepage = unicode_sorta::PAGE_195;
  int32_t space_width =
      bitmap::measure_character(font, ' ', scale, codepage, fixed_width);
  space_width += letter_spacing * scale;

  screen_updates.clear();

  bool cursor_found = false;
  int start_of_frame = 0;
  size_t i = 0;

  (*prev_line_start_) = -1;
  int prev_prev_line_start_ = -1;
  (*next_line_start_) = -1;

  /*if(cursor_pos == t.size()){
    // TODO(me) Why is this needed for the badger but not the others?
    cursor_found = true;
  }*/

  while (i < t.length()) {
    size_t next_space = t.find(' ', i + 1);
    if (next_space == std::string::npos) {
      next_space = t.length();
    }

    size_t next_linebreak = t.find('\n', i + 1);

    if (next_linebreak == std::string::npos) {
      next_linebreak = t.length();
    }

    size_t next_break = std::min(next_space, next_linebreak);

    uint16_t word_width = 0;
    for (size_t j = i; j < next_break; j++) {
      if (t[j] == unicode_sorta::PAGE_194_START) {
        codepage = unicode_sorta::PAGE_194;
        continue;
      } else if (t[j] == unicode_sorta::PAGE_195_START) {
        continue;
      }
      word_width +=
          bitmap::measure_character(font, t[j], scale, codepage, fixed_width);
      word_width += letter_spacing * scale;
      codepage = unicode_sorta::PAGE_195;
    }

    // if this word would exceed the wrap limit then
    // move to the next line
    if (char_offset != 0 && char_offset + word_width > (uint32_t)wrap) {
      char_offset = 0;
      line_offset += (font->height + 1) * scale;
      if (!cursor_found) {
        (*prev_line_start_) = prev_prev_line_start_;
        prev_prev_line_start_ = i;
      }

      if (!cursor_found) {
        printf("Changing start of frame\n");
        start_of_frame = i;
      }
      if (cursor_found && (*next_line_start_) < 0) {
        (*next_line_start_) = i;
      }
    }
    for (size_t j = i; j < std::min(next_break + 1, t.length()); j++) {
      //printf("j: %d c_p: %d\n", j, cursor_pos);
      if (j == cursor_pos) {
        cursor_found = true;
      }
      if (t[j] == unicode_sorta::PAGE_194_START) {
        codepage = unicode_sorta::PAGE_194;
        continue;
      } else if (t[j] == unicode_sorta::PAGE_195_START) {
        continue;
      }
      if (t[j] == '\n') {  // Linebreak
        line_offset += (font->height + 1) * scale;
        char_offset = 0;
      } else if (t[j] == ' ') {  // Space
        char_offset += space_width;
      } else {
        char_offset +=
            bitmap::measure_character(font, t[j], scale, codepage, fixed_width);
        char_offset += letter_spacing * scale;
      }
      codepage = unicode_sorta::PAGE_195;
    }

    // move character offset
    i = next_break += 1;
  }

  // Now we will draw all elements, shifting to show from the
  // subline that has the cursor.

  auto rect_fun = [badger](int32_t x, int32_t y, int32_t w, int32_t h) {
    badger->rectangle(x, y, w, h);
  };

  char_offset = 0;
  line_offset = 0;

  i = cursor_pos == -1 ? 0 : start_of_frame;
  auto to_display = tc.substr(i);
  i = 0;  //  Resetting so all comparisons make sense
  printf("To display: \n --- \n %s \n ---\n", to_display.c_str());
  bool cursor_drawn = false;
  while (i < to_display.length()) {
    size_t next_space = to_display.find(' ', i + 1);
    if (next_space == std::string::npos) {
      next_space = to_display.length();
    }

    size_t next_linebreak = to_display.find('\n', i + 1);

    if (next_linebreak == std::string::npos) {
      next_linebreak = to_display.length();
    }

    size_t next_break = std::min(next_space, next_linebreak);

    uint16_t word_width = 0;
    for (size_t j = i; j < next_break; j++) {
      if (to_display[j] == unicode_sorta::PAGE_194_START) {
        codepage = unicode_sorta::PAGE_194;
        continue;
      } else if (to_display[j] == unicode_sorta::PAGE_195_START) {
        continue;
      }
      word_width += bitmap::measure_character(font, to_display[j], scale,
                                              codepage, fixed_width);
      word_width += letter_spacing * scale;
      codepage = unicode_sorta::PAGE_195;
    }
    if (char_offset != 0 && char_offset + word_width > (uint32_t)wrap) {
      printf("Introducing a cleaning block at word break %d %d %d %d\n",
             x + char_offset, y + line_offset, 296 - (x + char_offset),
             font->height * scale);
      screen_updates.push_back({x + char_offset, y + line_offset,
                                296 - (x + char_offset), font->height * scale});
      badger->pen(15);
      badger->rectangle(x + char_offset, y + line_offset,
                        296 - (x + char_offset), font->height * scale);
      char_offset = 0;
      line_offset += (font->height + 1) * scale;
      num_lines++;
    }
    if (num_lines >= 7) {
      break;
    }
    // draw word
    for (size_t j = i; j < std::min(next_break + 1, to_display.length()); j++) {
      if (j == cursor_pos) {
        cursor_drawn = true;
        // draw cursor
        if (mode == EditorMode::kNormal) {
          auto width = word_width += bitmap::measure_character(
              font, to_display[j], scale, codepage, fixed_width);
          //rect_fun(x + char_offset - width - 1, y + line_offset + 8, width, 2);
        } else {
          if (cursor_pos != 0) {
            //rect_fun(x + char_offset - 1, y + line_offset, 1, 8);
          } else {
            //rect_fun(x + char_offset, y + line_offset, 1, 8);
          }
        }
      }
      if (to_display[j] == unicode_sorta::PAGE_194_START) {
        codepage = unicode_sorta::PAGE_194;
        continue;
      } else if (to_display[j] == unicode_sorta::PAGE_195_START) {
        continue;
      }
      if (to_display[j] == '\n') {
        printf("Introducing a cleaning block at new line %d %d %d %d\n",
               x + char_offset, y + line_offset, 296 - (x + char_offset),
               font->height * scale);
        screen_updates.push_back({x + char_offset, y + line_offset,
                                  296 - (x + char_offset),
                                  font->height * scale});
        badger->pen(15);
        badger->rectangle(x + char_offset, y + line_offset,
                          296 - (x + char_offset), font->height * scale);
        line_offset += (font->height + 1) * scale;
        char_offset = 0;
        // Will likely need to also add rectangles on new lines at end
      } else if (to_display[j] == ' ') {
        if (on_screen.size() > j) {
          if (on_screen[j] != ' ') {
            screen_updates.push_back(
                {x + char_offset, y + line_offset, space_width,
                 font->height *
                     scale});  // TODO(me) This is hacky, to get merging to work
            badger->pen(15);
            badger->rectangle(x + char_offset, y + line_offset, space_width,
                              font->height * scale);
          }
        }

        char_offset += space_width;
      } else {
        // Drawing character, potentially
        auto char_width = bitmap::measure_character(font, to_display[j], scale,
                                                    codepage, fixed_width) +
                          letter_spacing * scale;
        if (on_screen.size() > j) {
          if (to_display[j] == on_screen[j]) {
            //printf("No partial update, same character '%c'\n", t[j]);
          }
          if (to_display[j] != on_screen[j]) {
            //printf("Partial update, different character '%c' != '%c'\n", on_screen[j], t[j]);
            screen_updates.push_back({x + char_offset, y + line_offset,
                                      char_width, font->height * scale});
            badger->pen(15);
            badger->rectangle(x + char_offset, y + line_offset, char_width,
                              font->height * scale);
          }
        } else {
          //printf("Partial update, adding character '%c'\n", t[j]);
          screen_updates.push_back({x + char_offset, y + line_offset,
                                    char_width, font->height * scale});
          badger->pen(15);
          badger->rectangle(x + char_offset, y + line_offset, char_width,
                            font->height * scale);
          // The case where the sent string is shorter is not covered in this else
        }
        badger->pen(0);
        character(font, rect_fun, to_display[j], x + char_offset,
                  y + line_offset, scale, 0, codepage);
        char_offset += char_width;
      }
      codepage = unicode_sorta::PAGE_195;
    }
    i = next_break += 1;
  }
  if (!cursor_drawn && cursor_pos != -1) {
    // draw cursor at end.
    if (mode == EditorMode::kNormal) {
      //rect_fun(x + char_offset - 5, y + line_offset + 8, 5,
      //         2);  // Eyeballing the width here.
    } else {
      //rect_fun(x + char_offset - 1, y + line_offset + 4, 1, 4);
    }
  }
  if (to_display.size() < on_screen.size()) {
    screen_updates.push_back({x + char_offset, y + line_offset,
                              296 - (x + char_offset),
                              128 - font->height * scale});
    badger->pen(15);
    badger->rectangle(
        x + char_offset, y + line_offset, 296 - (x + char_offset),
        128 - font->height * scale);  // TODO(me) this is repeated now
  }
  on_screen = t.substr(start_of_frame);  // This should be a copy
  printf("On screen: \n --- \n %s \n ---\n", on_screen.c_str());
  // TODO(me) on_screen should actually start at frame
}

void Output::CommandLine(const std::list<char>& command) {
  std::string as_str(command.begin(), command.end());
  this->CommandLine(as_str);
}

void Output::CommandLine(const std::string& command) {
  badger.update_speed(3);
  badger.thickness(2);
  badger.pen(15);
  badger.rectangle(0, 112, 296, 16);
  badger.pen(0);
  text(&badger, command, -1, 0, 120, 296, 1, EditorMode::kCommandLineMode,
       &prev_line_start_, &next_line_start_);
  badger.partial_update(0, 112, 296, 16, true);
}

void Output::Emit(const std::string& current_line_str, const int cursor_pos,
                  EditorMode mode) {
  // Should:
  // - Keep track of what is displayed
  // - Compare the current line splits with the displayed line splits
  // - Anything equal at the beginning of the line stays
  // - Anything different up to the end of the line triggers a clean
  if (emits_with_no_clear > 100) {
    badger.update(true);
  }
  //uint8_t updateFlags = 0;
  badger.pen(0);
  badger.update_speed(3);
  badger.thickness(1);
  badger.font("bitmap8");
  auto now = CurrentTimeInMillis();
  if (now - last_key_in < 150) {
    printf("Avoiding a fast write\n");
    last_key_in = now;
    return;
  }
  last_key_in = now;
  //if(fabs(on_screen.size() - current_line_str.size()) > 10){
  //  full_update = true;
  //}
  text(&badger, current_line_str, cursor_pos, 0, 0, 296, 2, mode,
       &prev_line_start_, &next_line_start_);

  if (!badger.is_busy()) {
    MergeUpdates();
    if (screen_updates.size() >= 6) {
      badger.update(true);
      return;
    }

    for (auto& update : screen_updates) {
      //printf("Merged update: (%d,%d - %d x %d)\n", update.x, update.y, update.width, update.height);
      //badger.partial_update(skip_size, i * 24, clear_size, 24 + i * 24, true);
      badger.partial_update(update.x, update.y, update.width, update.height,
                            true);
    }
    //badger.update();
  } else {
    printf("Badger busy\n");
  }

  /*
  std::vector<std::string> chunks;
  int chunk_size = 22;  // eyeballing the font size
  if (current_line_str.size() == 0) {
    badger.pen(15);
    badger.clear();
    badger.update();
    displayed_lines_.clear();
  }
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
        badger.update(true);
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
  */
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

int Output::NextLine() {
  return next_line_start_;
}
int Output::PrevLine() {
  return prev_line_start_;
}

void Output::ProcessHandlers() {
  badger.update_button_states();
  printf("%b\n", badger.button_states());
  if (badger.pressed(badger.A)) {
    auto now = CurrentTimeInMillis();
    if (now - prev_button_press_ > 150) {
      printf("Hard refresh");
      badger.update(true);
    }
  }
}

void Output::ProcessEvent(EventType ev) {
  switch (ev) {
    case EV_BT_OFF:
      break;
    case EV_BT_ON:
      break;
  }
}

int Output::CurrentTimeInMillis() {
  return static_cast<int>(to_ms_since_boot(get_absolute_time()));
}