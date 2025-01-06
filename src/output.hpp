// Copyright 2025 Ruben Berenguel

// This defines the API for an "output", that is, some sort of screen.
// The reference implementation is that of pico/gfx/gfx.cpp and for now
// is the only one feature-complete.

#pragma once

#include <list>
#include <string>
#include <vector>

#include "editor_mode.h"
#include "setup.h"

class Editor;  // Forward declaration to avoid a circular dependency

enum class OutputCommands { kFlush, kSplash, kCommandMode };

class Output {
 public:
  int CurrentTimeInMillis();
  int NextLine();
  int PrevLine();
  void Emit(const std::string& s, const int cursor_pos, EditorMode mode);
  void CommandLine(const std::list<char>& s);
  void CommandLine(const std::string& s);
  void Command(const OutputCommands& command);
  void Init(Editor*);
  void ProcessHandlers();
  void ProcessEvent(EventType ev);

 private:
  int prev_button_press_ = -1;
  int backlight_brightness = 40;
  int prev_line_start_ = -1;
  int next_line_start_ = -1;
  Editor* editor;
  // The operations would feel better if this was an array and not a list though
};
