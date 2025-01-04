// Copyright 2025 Ruben Berenguel

// The header for the editor. I think it has to be .h for some
// linker reason.

#pragma once

#include <list>
#include <string>

#include "./output.hpp"
#include "./setup.h"

class Editor {
 public:
  static Editor& GetInstance() {
    static Editor instance;
    return instance;
  }
  // Used for testing basically
  std::string GetCurrentLine();
  std::string GetDocument();
  int CountLines();
  // Used for testing basically
  void ResetState() {
    current_line_.clear();
    ncolumn_ = 0;
    row_ = document_.end();
    mode_ = EditorMode::kNormal;
  }
  void ProcessKey(const uint8_t key, KeyModifiers* modifiers, bool batched);
  void ProcessHandlers();
  void ProcessEvent(EventType ev);
  void Init();
  void ProcessSaving();

 private:
  std::list<std::string>::iterator row_;
  int ncolumn_ = 0;
  int nrow_ = 0;
  int previously_shifted = -1;  // Handle bouncing
  int previous_key = -1;
  Output* output;
  Editor() : mode_(EditorMode::kNormal) {}
  void HandleEsc();
  auto InsertOrReplaceLine(std::string new_line,
                           std::list<std::string>::iterator row);
  void UpdateCurrentLine(int);
  void WordCount(int* wc, int* cc);
  void HandleEnter();
  void ProcessCommand(char c, KeyModifiers* modifiers);
  void DispatchCommand(std::string command);
  std::string current_line_;
  std::list<char> command_line_;
  std::list<std::string> document_;
  EditorMode mode_;
  bool should_save = false;
};
