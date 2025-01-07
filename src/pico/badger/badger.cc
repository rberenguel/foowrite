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

#define N_LINES_PER_SCREEN 5
std::list<int> prev_line_starts;

pimoroni::Badger2040 badger;

namespace BadgerGlobals {

struct ScreenUpdate {
  uint32_t x;
  uint32_t y;
  int width;
  int height;
};

std::string on_screen = "";  // Removing the cpplint rule for this
int on_screen_cursor = -1;
int last_key_in = -1;
int emits_with_no_clear = 0;
std::list<ScreenUpdate> screen_updates;

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
  int new_cursor_pos = -1;
  const bitmap::font_t* font = &font8;  // TODO(me) improve
  uint32_t char_offset = 0;
  uint32_t line_offset = 0;  // line (if wrapping) offset
  unicode_sorta::codepage_t codepage = unicode_sorta::PAGE_195;
  int32_t space_width =
      bitmap::measure_character(font, ' ', scale, codepage, fixed_width);
  space_width += letter_spacing * scale;

  BadgerGlobals::screen_updates.clear();
  prev_line_starts.clear();

  bool cursor_found = false;
  int start_of_frame = 0;
  size_t i = 0;

  (*prev_line_start_) = -1;
  prev_line_starts.push_back(-1);
  (*next_line_start_) = -1;

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
        (*prev_line_start_) = prev_line_starts.back();
        prev_line_starts.push_back(i);
      }

      if (!cursor_found) {
        start_of_frame =
            prev_line_starts.size() < N_LINES_PER_SCREEN
                ? 0
                : *std::next(prev_line_starts.rbegin(), N_LINES_PER_SCREEN - 1);
        start_of_frame = start_of_frame < 0 ? 0 : start_of_frame;
      }
      if (cursor_found && (*next_line_start_) < 0) {
        (*next_line_start_) = i;
      }
    }
    for (size_t j = i; j < std::min(next_break + 1, t.length()); j++) {
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
  printf("On screen before: \n --- \n %s \n ---\n", on_screen.c_str());
  auto previous_screen_length = on_screen.size();
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
    bool same = true;
    for (size_t j = i; j < next_break; j++) {
      if (on_screen.size() > j && on_screen[j] != to_display[j]) {
        same = false;
      }
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
      if (same) {
        printf(
            "Not introducing a cleaning block, the word was already broken "
            "before\n");
      } else {
        printf("Introducing a cleaning block at word break %d %d %d %d\n",
               x + char_offset, y + line_offset, 296 - (x + char_offset),
               font->height * scale);
        BadgerGlobals::screen_updates.push_back(
            {x + char_offset, y + line_offset, 296 - (x + char_offset),
             font->height * scale});
        badger->pen(15);
        badger->rectangle(x + char_offset, y + line_offset,
                          296 - (x + char_offset), font->height * scale);
      }

      char_offset = 0;
      line_offset += (font->height + 1) * scale;
      num_lines++;
    }
    if (num_lines >= 7) {
      break;
    }
    // draw word
    for (size_t j = i; j < std::min(next_break + 1, to_display.length()); j++) {
      const int cursor_width = 8;
      const int shift = cursor_width + 4;
      if (j == on_screen_cursor) {
        // TODO(me) Try to erase the previous cursor,
        // but this won't trigger a character redraw yet
        badger->pen(15);
        badger->rectangle(x + char_offset - shift, y + line_offset + 16,
                          cursor_width, 16);

        BadgerGlobals::screen_updates.push_back(
            {x + char_offset - shift, y + line_offset + 16, cursor_width, 16});
      }
      if (j == cursor_pos) {
        cursor_drawn = true;
        // draw cursor
        if (mode == EditorMode::kNormal) {
          auto width = word_width += bitmap::measure_character(
              font, to_display[j], scale, codepage, fixed_width);  // Not used?
          rect_fun(x + char_offset - shift, y + line_offset + 16, cursor_width,
                   2);
          BadgerGlobals::screen_updates.push_back({x + char_offset - shift,
                                                   y + line_offset + 16,
                                                   cursor_width, 16});
          new_cursor_pos = cursor_pos;
        } else {
          if (cursor_pos != 0) {
            // TODO(me)
          } else {
            // TODO(me)
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
        BadgerGlobals::screen_updates.push_back(
            {x + char_offset, y + line_offset, 296 - (x + char_offset),
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
            BadgerGlobals::screen_updates.push_back(
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
            // TODO(me)
          }
          if (to_display[j] != on_screen[j]) {
            BadgerGlobals::screen_updates.push_back(
                {x + char_offset, y + line_offset, char_width,
                 font->height * scale});
            badger->pen(15);
            badger->rectangle(x + char_offset, y + line_offset, char_width,
                              font->height * scale);
          }
        } else {
          BadgerGlobals::screen_updates.push_back({x + char_offset,
                                                   y + line_offset, char_width,
                                                   font->height * scale});
          badger->pen(15);
          badger->rectangle(x + char_offset, y + line_offset, char_width,
                            font->height * scale);
          // The case where the sent string is shorter is
          // not covered in this else
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
      // TODO(me)
    } else {
      // TODO(me)
    }
  }
  if (to_display.size() < previous_screen_length) {
    printf("Introducing two cleanings for a shorter string %d %d %d %d\n",
           x + char_offset, y + line_offset, 296 - (x + char_offset),
           128 - font->height * scale);
    BadgerGlobals::screen_updates.push_back({x + char_offset, y + line_offset,
                                             296 - (x + char_offset),
                                             128 - font->height * scale});
    BadgerGlobals::screen_updates.push_back(
        {0, y + line_offset + (font->height + 1) * scale, 296,
         128 - (line_offset + (font->height + 1) * scale)});
    badger->pen(15);
    badger->rectangle(0, y + line_offset + (font->height + 1) * scale, 296,
                      128 - (line_offset + (font->height + 1) * scale));
    badger->rectangle(
        x + char_offset, y + line_offset, 296 - (x + char_offset),
        128 - font->height * scale);  // TODO(me) this is repeated now
  }
  on_screen = t.substr(start_of_frame);  // This should be a copy
  on_screen_cursor = new_cursor_pos;
  printf("On screen now: \n --- \n %s \n ---\n", on_screen.c_str());
  // TODO(me) on_screen should actually start at frame
}

void AdjustYAndHeight(BadgerGlobals::ScreenUpdate* update) {
  // Calculate the remainder when y is divided by 8
  int remainder = update->y % 8;

  // Adjust y to be the nearest multiple of 8 less than or
  // equal to the current y
  update->y -= remainder;

  // Adjust height to compensate for the change in y
  update->height += remainder;

  // Ensure the height covers the original area by adding
  // any remaining difference
  // to make it a multiple of 8.
  int heightRemainder = update->height % 8;
  if (heightRemainder != 0) {
    update->height += 8 - heightRemainder;
  }
}

void MergeUpdates() {
  printf("Merging\n");
  if (BadgerGlobals::screen_updates.empty()) {
    return;
  }
  for (auto& update : BadgerGlobals::screen_updates) {
    AdjustYAndHeight(&update);
  }
  std::list<BadgerGlobals::ScreenUpdate> merged;
  bool is_first = true;
  BadgerGlobals::ScreenUpdate prev;
  for (const auto& curr : BadgerGlobals::screen_updates) {
    printf("(%d,%d - %d x %d) ", curr.x, curr.y, curr.width, curr.height);
    if (is_first) {
      prev = curr;
      is_first = false;
      continue;  // Kind of ugly, but I don't want to fiddle with iterators
    }
    if (prev.y == curr.y && (prev.x + prev.width >= curr.x)) {
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
  BadgerGlobals::screen_updates.clear();
  BadgerGlobals::screen_updates = std::move(merged);
}

}  // namespace BadgerGlobals

void Output::CommandLine(const std::list<char>& command) {
  std::string as_str(command.begin(), command.end());
  this->CommandLine(as_str);
}

void Output::CommandLine(const std::string& command) {
  // TODO(me) This is very slow, why? It only updates a rectangle
  // Maybe it is triggering emit from the editor?
  badger.update_speed(3);
  badger.thickness(2);
  badger.pen(15);
  badger.rectangle(0, 112, 296, 16);
  badger.pen(0);
  BadgerGlobals::text(&badger, command, -1, 0, 120, 296, 1,
                      EditorMode::kCommandLineMode, &prev_line_start_,
                      &next_line_start_);
  badger.partial_update(0, 112, 296, 16, true);
}

void Output::Emit(const std::string& current_line_str, const int cursor_pos,
                  EditorMode mode) {
  // Should:
  // - Keep track of what is displayed
  // - Compare the current line splits with the displayed line splits
  // - Anything equal at the beginning of the line stays
  // - Anything different up to the end of the line triggers a clean
  printf("Emit with mode: %d\n", mode);
  if (mode == EditorMode::kCommandLineMode) {
    // Avoid refreshing the whole screen with no reason to do it
    return;
  }
  if (current_line_str.empty() && mode == EditorMode::kInsert &&
      !(BadgerGlobals::on_screen.empty())) {
    // TODO(me) This might still not be enough
    // (i.e. comparing with empty screen is needed, but might not be enough)
    badger.pen(15);
    badger.clear();
    badger.update(true);
  }
  if (BadgerGlobals::emits_with_no_clear > 100) {
    badger.update(true);
    BadgerGlobals::emits_with_no_clear = 0;
  }
  ++(BadgerGlobals::emits_with_no_clear);
  badger.pen(0);
  badger.update_speed(3);
  badger.thickness(2);
  badger.font("bitmap8");
  auto now = CurrentTimeInMillis();
  if (now - BadgerGlobals::last_key_in < 150) {
    printf("Avoiding a fast write\n");
    BadgerGlobals::last_key_in = now;
    return;  // TODO(me) this should not return,
    // but call text first to make sure the screen is updated
    // with what it _should_ have
  }
  BadgerGlobals::last_key_in = now;

  BadgerGlobals::text(&badger, current_line_str, cursor_pos, 0, 0, 296, 2, mode,
                      &prev_line_start_, &next_line_start_);

  if (!badger.is_busy()) {
    BadgerGlobals::MergeUpdates();
    if (BadgerGlobals::screen_updates.size() >= 6) {
      badger.update(true);
      return;
    }

    for (auto it = BadgerGlobals::screen_updates.begin();
         it != BadgerGlobals::screen_updates.end(); ++it) {
      auto& update = *it;
      badger.partial_update(update.x, update.y, update.width, update.height,
                            true);
    }
  } else {
    printf("Badger busy\n");
  }
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
