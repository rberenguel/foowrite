// Copyright 2025 Ruben Berenguel

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include "../../output.hpp"
#include "./common/pimoroni_common.hpp"
#include "./font8_data.hpp"
#include "./gfx_pack.hpp"
#include "./pico/stdlib.h"
#include "./pico_graphics.hpp"
#include "./rgbled.hpp"
#include "./setup.h"
#include "./splash.hpp"
#include "pico/time.h"

pimoroni::ST7567 st7567(128, 64, pimoroni::GfxPack::gfx_pack_pins);
pimoroni::PicoGraphics_Pen1Bit graphics(st7567.width, st7567.height, nullptr);
pimoroni::RGBLED backlight_rgb(pimoroni::GfxPack::BL_R, pimoroni::GfxPack::BL_G,
                               pimoroni::GfxPack::BL_B,
                               pimoroni::Polarity::ACTIVE_HIGH);

void text(pimoroni::PicoGraphics_Pen1Bit* grph, const std::string_view& t,
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
        prev_prev_line_start_ = i;
      }

      if (!cursor_found) {
        start_of_frame = i;
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

void Output::CommandLine(const std::list<char>& command) {
  backlight_rgb.set_rgb(0, 50, 0);
  std::string as_str(command.begin(), command.end());
  this->CommandLine(as_str);
}
void Output::CommandLine(const std::string& command) {
  backlight_rgb.set_rgb(0, 50, 0);
  graphics.set_pen(0);
  graphics.rectangle(pimoroni::Rect(0, 56, 128, 8));
  graphics.set_pen(15);
  text(&graphics, command, -1, 0, 56, 128, 1, EditorMode::kCommandLineMode,
       &prev_line_start_, &next_line_start_);
  st7567.update(&graphics);
}

void Output::Emit(const std::string& current_line_str, const int cursor_pos,
                  EditorMode mode) {
  if (mode == EditorMode::kNormal) {
    backlight_rgb.set_rgb(5, 50, 10);  // A friendly green
  }
  if (mode == EditorMode::kInsert) {
    backlight_rgb.set_rgb(70, 40, 0);  // A subtle brown
  }
  graphics.set_pen(0);
  graphics.clear();
  graphics.set_pen(15);
  text(&graphics, current_line_str, cursor_pos, 0, 0, 128, 1, mode,
       &prev_line_start_, &next_line_start_);
  st7567.update(&graphics);
}

void Output::ProcessHandlers() {
  pimoroni::Button button_e(pimoroni::GfxPack::E);
  pimoroni::Button button_b(pimoroni::GfxPack::B);

  if (button_b.raw()) {
    backlight_brightness = (backlight_brightness + 16) % 256;
    st7567.set_backlight(backlight_brightness);
  }
  if (button_e.raw()) {
    auto now = CurrentTimeInMillis();
    if (now - prev_button_press_ > 150) {
      splash(graphics);
      st7567.update(&graphics);
      prev_button_press_ = now;
    }
  }
}

void Output::ProcessEvent(EventType ev) {
  switch (ev) {
    case EV_BT_OFF:
      backlight_rgb.set_rgb(5, 5, 100);
      break;
    case EV_BT_ON:
      backlight_rgb.set_rgb(5, 100, 5);
      break;
  }
}

int Output::CurrentTimeInMillis() {
  return static_cast<int>(to_ms_since_boot(get_absolute_time()));
}

void Output::Init(Editor* e) {
  editor = e;
  st7567.set_backlight(40);
  backlight_rgb.set_rgb(5, 5, 100);
  graphics.set_pen(0);
  graphics.set_font("bitmap8");
  graphics.clear();
  splash(graphics);
  st7567.update(&graphics);
  graphics.set_pen(15);
}

void Output::Command(const OutputCommands& command) {
  switch (command) {
    case OutputCommands::kSplash:
      backlight_rgb.set_rgb(0, 50, 50);
      splash(graphics);
      st7567.update(&graphics);
      break;
    case OutputCommands::kCommandMode:
      backlight_rgb.set_rgb(50, 50, 0);
      break;
    default:
      break;
  }
}
