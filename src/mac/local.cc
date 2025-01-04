// Copyright 2025 Ruben Berenguel

#include <array>
#include <iostream>
#include <string>
#include "../editor.h"
#include "../editor_mode.h"
#include "../output.hpp"
#include "../setup.h"

void Output::ProcessEvent(EventType ev) {}
void Output::ProcessHandlers() {}
int Output::CurrentTimeInMillis() {
  return 1000;
}

void Output::Emit(const std::string& s, const int cursor_pos, EditorMode mode) {
}
void Editor::ProcessSaving() {}
void Output::CommandLine(const std::list<char>& c) {
  std::string as_str(c.begin(), c.end());
  std::cout << as_str << std::endl;
  std::cout.flush();
}
int Output::NextLine() {
  return -1;
}
int Output::PrevLine() {
  return -1;
}
void Output::CommandLine(const std::string& c) {}

void Output::Command(const OutputCommands& command) {
  switch (command) {
    default:
      break;
  }
}

// This is unused? How can I use it?
void Display(const std::string& line) {
  std::cout << line << std::endl;
}

void Output::Init(Editor* e) {
  editor = e;
}
