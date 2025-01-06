// Copyright 2025 Ruben Berenguel

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include "../../output.hpp"
#include "./button.hpp"
#include "./common/pimoroni_common.hpp"
#include "./font8_data.hpp"
#include "./pico/stdlib.h"
#include "./pico_graphics.hpp"
#include "./rgbled.hpp"
#include "./setup.h"
#include "./splash.hpp"
#include "drivers/st7789/st7789.hpp"
#include "editor.h"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "pico/time.h"
#include "pico_display.hpp"

#define N_LINES_PER_SCREEN 5
std::list<int> prev_line_starts;

pimoroni::ST7789 st7789(pimoroni::PicoDisplay::WIDTH,
                        pimoroni::PicoDisplay::HEIGHT, pimoroni::ROTATE_0,
                        false, get_spi_pins(pimoroni::BG_SPI_FRONT));

pimoroni::PicoGraphics_PenP8 graphics(st7789.width, st7789.height, nullptr);

pimoroni::RGBLED led(pimoroni::PicoDisplay::LED_R, pimoroni::PicoDisplay::LED_G,
                     pimoroni::PicoDisplay::LED_B);

void text(pimoroni::PicoGraphics_PenP8* grph, const std::string_view& t,
          const int cursor_pos, const int32_t x, const int32_t y,
          const int32_t wrap, const uint8_t scale, EditorMode mode,
          int* prev_line_start_, int* next_line_start_) {
  // This comes from Pimoroni's PicoGraphics library, with some tweaks:
  // - Figure out cursor positioning.
  // - Draw the cursor depending on the editor mode.
  // - Provide a previous/next line for long-line pagination.
  // To do it, loops twice, one to count everything and another to
  // draw everything
  const uint8_t letter_spacing = 1;
  bool fixed_width = false;
  const bitmap::font_t* font = grph->bitmap_font;
  uint32_t char_offset = 0;
  uint32_t line_offset = 0;  // line (if wrapping) offset
  unicode_sorta::codepage_t codepage = unicode_sorta::PAGE_195;

  int32_t space_width =
      measure_character(font, ' ', scale, codepage, fixed_width);
  space_width += letter_spacing * scale;

  bool cursor_found = false;
  prev_line_starts.clear();
  int start_of_frame = 0;
  size_t i = 0;

  (*prev_line_start_) = -1;
  int prev_prev_line_start_ = -1;
  (*next_line_start_) = -1;

  while (i < t.length()) {
    // find length of current word
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
      word_width += measure_character(font, t[j], scale, codepage, fixed_width);
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
        prev_line_starts.push_back(prev_prev_line_start_);
        prev_prev_line_start_ = i;
      }

      if (!cursor_found) {
        start_of_frame = prev_line_starts.size() < N_LINES_PER_SCREEN ? 0 : *std::next(prev_line_starts.rbegin(), N_LINES_PER_SCREEN - 1);
        start_of_frame = start_of_frame < 0 ? 0 : start_of_frame;
      }
      if (cursor_found && (*next_line_start_) < 0) {
        (*next_line_start_) = i;
      }
    }
    auto rect_fun = [grph](int32_t x, int32_t y, int32_t w, int32_t h) {
      grph->rectangle(pimoroni::Rect(x, y, w, h));
    };
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
            measure_character(font, t[j], scale, codepage, fixed_width);
        char_offset += letter_spacing * scale;
      }
      codepage = unicode_sorta::PAGE_195;
    }

    // move character offset
    i = next_break += 1;
  }

  // Now we will draw all elements, shifting to show from the
  // subline that has the cursor.

  auto rect_fun = [grph](int32_t x, int32_t y, int32_t w, int32_t h) {
    grph->rectangle(pimoroni::Rect(x, y, w, h));
  };

  char_offset = 0;
  line_offset = 0;

  i = cursor_pos == -1 ? 0 : start_of_frame;
  bool cursor_drawn = false;
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
      word_width += measure_character(font, t[j], scale, codepage, fixed_width);
      word_width += letter_spacing * scale;
      codepage = unicode_sorta::PAGE_195;
    }
    if (char_offset != 0 && char_offset + word_width > (uint32_t)wrap) {
      char_offset = 0;
      line_offset += (font->height + 1) * scale;
    }
    // draw word
    for (size_t j = i; j < std::min(next_break + 1, t.length()); j++) {
      if (j == cursor_pos) {
        cursor_drawn = true;
        // draw cursor
        if (mode == EditorMode::kNormal) {
          auto width =
              measure_character(font, t[j], scale, codepage, fixed_width);
          rect_fun(x + char_offset - width - 1, y + line_offset + 8, width, 2);
        } else {
          if (cursor_pos != 0) {
            rect_fun(x + char_offset - 1, y + line_offset, 1, 8);
          } else {
            rect_fun(x + char_offset, y + line_offset, 1, 8);
          }
        }
      }
      if (t[j] == unicode_sorta::PAGE_194_START) {
        codepage = unicode_sorta::PAGE_194;
        continue;
      } else if (t[j] == unicode_sorta::PAGE_195_START) {
        continue;
      }
      if (t[j] == '\n') {
        line_offset += (font->height + 1) * scale;
        char_offset = 0;
      } else if (t[j] == ' ') {
        char_offset += space_width;
      } else {
        character(font, rect_fun, t[j], x + char_offset, y + line_offset, scale,
                  0, codepage);
        char_offset +=
            measure_character(font, t[j], scale, codepage, fixed_width);
        char_offset += letter_spacing * scale;
      }
      codepage = unicode_sorta::PAGE_195;
    }
    i = next_break += 1;
  }
  if (!cursor_drawn && cursor_pos != -1) {
    // draw cursor at end.
    if (mode == EditorMode::kNormal) {
      rect_fun(x + char_offset - 5, y + line_offset + 8, 5,
               2);  // Eyeballing the width here.
    } else {
      rect_fun(x + char_offset - 1, y + line_offset + 4, 1, 4);
    }
  }
}

int Output::NextLine() {
  return next_line_start_;
}
int Output::PrevLine() {
  return prev_line_start_;
}

pimoroni::Pen WHITE = graphics.create_pen(255, 255, 255);
pimoroni::Pen DARK = graphics.create_pen(10, 10, 10);
pimoroni::Pen ORANGE = graphics.create_pen(200, 200, 0);

bool dark_mode = false;

void Output::CommandLine(const std::list<char>& command) {
  std::string as_str(command.begin(), command.end());
  this->CommandLine(as_str);
}
void Output::CommandLine(const std::string& command) {
  graphics.set_pen(dark_mode ? DARK : WHITE);
  graphics.rectangle(pimoroni::Rect(0, 120, pimoroni::PicoDisplay::WIDTH, 8));
  graphics.set_pen(ORANGE);
  text(&graphics, command, -1, 0, 120, pimoroni::PicoDisplay::WIDTH, 2,
       EditorMode::kCommandLineMode, &prev_line_start_, &next_line_start_);
  st7789.update(&graphics);
}

void Output::Emit(const std::string& current_line_str, const int cursor_pos,
                  EditorMode mode) {
  if (mode == EditorMode::kNormal) {
    led.set_rgb(5, 10, 10);  // A friendly green
  }
  if (mode == EditorMode::kInsert) {
    led.set_rgb(10, 5, 0);  // A subtle brown
  }
  graphics.set_font("bitmap8");
  graphics.set_pen(dark_mode ? DARK : WHITE);
  graphics.clear();
  graphics.set_pen(dark_mode ? WHITE : DARK);
  text(&graphics, current_line_str, cursor_pos, 1, 1,
       pimoroni::PicoDisplay::WIDTH, 2, mode, &prev_line_start_,
       &next_line_start_);
  st7789.update(&graphics);
}

void Output::ProcessHandlers() {
  pimoroni::Button button_x(pimoroni::PicoDisplay::X);
  pimoroni::Button button_b(pimoroni::PicoDisplay::B);
  if (button_x.raw()) {
    auto now = CurrentTimeInMillis();
    if (now - prev_button_press_ > 150) {
      splash(graphics);
      st7789.update(&graphics);
      prev_button_press_ = now;
    }
  }
  if (button_b.raw()) {
    auto now = CurrentTimeInMillis();
    if (now - prev_button_press_ > 150) {
      dark_mode = !dark_mode;
      prev_button_press_ = now;
      editor->Refresh();
    }
  }
}

void Output::ProcessEvent(EventType ev) {
  switch (ev) {
    case EV_BT_OFF:
      led.set_rgb(0, 0, 30);
      break;
    case EV_BT_ON:
      led.set_rgb(0, 30, 0);
      break;
  }
}

int Output::CurrentTimeInMillis() {
  return static_cast<int>(to_ms_since_boot(get_absolute_time()));
}

void Output::Init(Editor* e) {
  editor = e;
  st7789.set_backlight(100);
  led.set_rgb(0, 0, 30);
  graphics.set_pen(dark_mode ? DARK : WHITE);
  graphics.clear();
  splash(graphics);
  st7789.update(&graphics);
}

void Output::Command(const OutputCommands& command) {
  switch (command) {
    case OutputCommands::kSplash:
      break;
    case OutputCommands::kCommandMode:
      break;
    default:
      break;
  }
}
