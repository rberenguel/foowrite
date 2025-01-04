// Copyright 2025 Ruben Berenguel

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <new>

#include <list>
#include <string>

#define VERIFY_MODE 1
#define DEBUG_MODE 0

#include "./editor.h"
#include "./editor_mode.h"
#include "./layout.h"
#include "./output.hpp"
#include "./setup.h"

std::string String(EditorMode mode) {
  switch (mode) {
    case EditorMode::kNormal:
      return "N";
    case EditorMode::kInsert:
      return "I";
    case EditorMode::kCommandLineMode:
      return "L";
    default:
      return "?";
  }
}

auto Editor::InsertOrReplaceLine(std::string new_line,
                                 std::list<std::string>::iterator row) {
  if (row == document_.end()) {
    document_.push_back(new_line);
    return --document_.end();
  } else {
    *row = new_line;
    return row;
  }
}

void Editor::UpdateCurrentLine(int direction) {
  auto at_end = row_ == document_.end();
  auto at_begin = row_ == document_.begin();
  auto skip = false;
  row_ = InsertOrReplaceLine(current_line_, row_);
  // The current row kind of points to the line end, going up one
  // is really going to "the current one". So, go one up if we
  // were at the end, update is not adding new lines.
  if (at_end && !at_begin) {
    --row_;
  }
  if (direction == -1 && row_ != document_.begin()) {
    --row_;
  }
  if (!at_end && direction == 1 && row_ != document_.end()) {
    // Guard with at_end, since the iterator has already changed
    ++row_;
    if (row_ == document_.end()) {
      // End is now a placeholder new line
      --row_;
      skip = true;
    }
  }
  std::list<char> updated_line(row_->begin(), row_->end());
  current_line_ = *row_;
  ncolumn_ = fmin(ncolumn_, current_line_.size());
}

void Editor::HandleEnter() {
  std::string remaining_line_str = current_line_.substr(0, ncolumn_);
  current_line_ = current_line_.substr(ncolumn_);
  row_ = InsertOrReplaceLine(remaining_line_str, row_);
  row_ = document_.insert(++row_, "---");
  ncolumn_ = 0;
}

std::string Editor::GetCurrentLine() {
  return current_line_;
}

int Editor::CountLines() {
  return document_.size();
}

void Editor::DispatchCommand(std::string command) {
  command_line_.clear();
  auto dummy = KeyModifiers{};
  for (const auto& c : command) {
    ProcessCommand(c, &dummy);
  }
}

void Editor::ProcessCommand(char c, KeyModifiers* modifiers) {
  auto emits = true;
  std::string command_line_str(command_line_.begin(), command_line_.end());
  switch (c) {
    case '\0':
      break;
    case ':':
      mode_ = EditorMode::kCommandLineMode;
      command_line_.emplace_back(':');
      output->CommandLine(command_line_);

      break;
    case 'c':
      if (command_line_str == "c") {
        // TODO(me): Change full line
      } else {
        command_line_.emplace_back('c');
      }
      break;
    case 'b':
      command_line_.clear();
      {
        ncolumn_ = fmax(--ncolumn_, 0);
        ncolumn_ = fmax(--ncolumn_, 0);
        auto advance = true;
        if (current_line_[ncolumn_] == ' ') {
          advance = false;
        }
        while (ncolumn_ >= 0 && current_line_[ncolumn_] != ' ') {
          --ncolumn_;
        }
        if (advance) {
          if (ncolumn_ <= current_line_.size() - 1) {
            ++ncolumn_;
            if (ncolumn_ <= current_line_.size() - 1) {
              ++ncolumn_;
            }
          }
        }
      }
      break;
    case 'w':
      // daw diw caw ciw
      if (command_line_str == "da") {
        command_line_.clear();
        auto start = ncolumn_;
        auto end = ncolumn_;
        while (start > 0 && current_line_[start] != ' ') {
          --start;
        }
        while (end <= current_line_.size() - 1 && current_line_[end] != ' ') {
          ++end;
        }
        current_line_.erase(
            start,
            end - start);  // Erase takes a length as second argument
        ncolumn_ = fmin(
            ncolumn_,
            current_line_.size());  // Handle deletion at end (test: caw_at_end)
        break;
      }
      if (command_line_str == "di") {
        command_line_.clear();
        auto start = ncolumn_;
        auto end = ncolumn_;
        while (start > 0 && current_line_[start] != ' ') {
          --start;
        }
        if (start < current_line_.size() - 1) {
          ++start;
        }
        while (end <= current_line_.size() - 1 && current_line_[end] != ' ') {
          ++end;
        }
        current_line_.erase(
            start,
            end - start);  // Erase takes a length as second argument
        break;
      }
      if (command_line_str == "ca") {
        if (ncolumn_ == current_line_.size()) {
          DispatchCommand("dawa");
        } else {
          DispatchCommand("dawi");
        }

        break;
      }
      if (command_line_str == "ci") {
        if (ncolumn_ == current_line_.size()) {
          DispatchCommand("diwa");
        } else {
          DispatchCommand("diwi");
        }
        break;
      }
      command_line_.clear();
      while (ncolumn_ <= current_line_.size() - 1 &&
             current_line_[ncolumn_] != ' ') {
        ++ncolumn_;
      }
      if (ncolumn_ <= current_line_.size() - 1) {
        ++ncolumn_;
        if (ncolumn_ <= current_line_.size() - 1) {
          ++ncolumn_;
        }
      } else {
        ncolumn_ = current_line_.size();
      }

      break;
    case 'd':
      if (command_line_str == "d") {
        current_line_.clear();
        ncolumn_ = 0;
        command_line_ = {};
      } else {
        command_line_.emplace_back('d');
      }
      break;
    case '$':
      if (command_line_str == "d") {
        ncolumn_ = fmax(--ncolumn_, 0);
        current_line_.erase(ncolumn_, current_line_.size() - ncolumn_);
        command_line_.clear();
      }
      if (command_line_str == "c") {
        DispatchCommand("d$a");
      }
      if (command_line_str == "") {
        ncolumn_ = current_line_.size();
      }
      output->Emit(current_line_, ncolumn_, mode_);
      break;
    case '^':
      if (command_line_str == "") {
        auto start = 0;
        while (start < current_line_.size() && current_line_[start] == ' ') {
          ++start;
        }
        ncolumn_ = fmin(current_line_.size(), start + 1);
      }
      output->Emit(current_line_, ncolumn_, mode_);
      break;
    case '0':
      if (command_line_str == "") {
        ncolumn_ = fmin(1, current_line_.size());
      }
      output->Emit(current_line_, ncolumn_, mode_);
      break;
    case 'a':
      if (modifiers->ctrl) {
        ncolumn_ = 1;
      } else {
        if (command_line_str == "d" || command_line_str == "c") {
          command_line_.emplace_back('a');
        } else {
          mode_ = EditorMode::kInsert;
        }
      }
      output->Emit(current_line_, ncolumn_, mode_);
      break;
    case 'i':
      if (modifiers->meta) {
        if (row_ != document_.begin()) {
          --row_;
        } else {
          row_ = document_.begin();
        }
      } else {
        if (command_line_str == "d" || command_line_str == "c") {
          command_line_.emplace_back('i');
        } else {
          ncolumn_ = fmax(--ncolumn_, 0);
          mode_ = EditorMode::kInsert;
        }
      }
      break;
    case 'n':
      if (modifiers->meta) {
        ncolumn_ = fmax(0, --ncolumn_);
      }
      break;
    case 'e':
      if (modifiers->meta) {
        break;
      }
      if (modifiers->ctrl) {
        ncolumn_ = current_line_.size();
        break;
      }
      break;
    case 'o':
      if (modifiers->meta) {
        ncolumn_ = fmin(0, ++ncolumn_);
      }
      break;
  }
  if (emits) {
    output->Emit(current_line_, ncolumn_, mode_);
  }
  return;
}

std::string Editor::GetDocument() {
  std::string full_doc;

  for (const auto& line : document_) {
    full_doc += line + "\n";
  }
  return full_doc;
}

void Editor::ProcessHandlers() {
  output->ProcessHandlers();
}

void Editor::ProcessEvent(EventType ev) {
  switch (ev) {
    case EV_BT_OFF:
      output->ProcessEvent(EV_BT_OFF);
      break;
    case EV_BT_ON:
      output->ProcessEvent(EV_BT_ON);
      break;
    case EV_SAVE:
      ProcessSaving();
      break;
  }
  return;
}

void Editor::ProcessKey(const uint8_t key, KeyModifiers* modifiers,
                        bool batched) {
  if (key == 0) {
    output->Emit(current_line_, ncolumn_, mode_);
    return;
  }
  char c = get_char_from_key(key, modifiers);
  printf("%c\n", c);
  int now = output->CurrentTimeInMillis();
  if ((now - previously_shifted < 150) && key == previous_key) {
    return;
  }
  if (modifiers->shift) {
    previously_shifted = now;
    previous_key = key;
  } else {
    previously_shifted = -1;
    previous_key = -1;
  }
  // Modeless keys, which are basically arrows, or likely "device commands"
  switch (key) {
    case KEY_LEFT:
      ncolumn_ = fmax(--ncolumn_, 0);
      if (!batched) {
        output->Emit(current_line_, ncolumn_, mode_);
      }

      return;
      break;
    case KEY_RIGHT:
      ncolumn_ = fmin(++ncolumn_, current_line_.size());  // TODO(me) test this
      if (!batched) {
        output->Emit(current_line_, ncolumn_, mode_);
      }
      return;
      break;
    case KEY_UP:
      if (output->PrevLine() >= 0) {
        ncolumn_ = output->PrevLine();
        output->Emit(current_line_, ncolumn_, mode_);
        return;
      } else {
        if (row_ == document_.begin()) {
          ncolumn_ = 0;
          UpdateCurrentLine(-1);
          output->Emit(current_line_, ncolumn_, mode_);
          return;
        }
      }
      UpdateCurrentLine(-1);
      if (!batched) {
        output->Emit(current_line_, ncolumn_, mode_);
      }
      return;
      break;
    case KEY_DOWN:
      if (output->NextLine() >= 0) {
        ncolumn_ = output->NextLine();
        output->Emit(current_line_, ncolumn_, mode_);
        return;
      }
      UpdateCurrentLine(+1);
      if (!batched) {
        output->Emit(current_line_, ncolumn_, mode_);
      }
      return;
      break;
    default:
      break;
  }
  if (mode_ == EditorMode::kNormal) {
    switch (key) {
      case KEY_SPACE:
        break;
    }
    ProcessCommand(c, modifiers);
    return;
  }
  if (mode_ == EditorMode::kInsert) {
    // Only two full modes, normal and "other",
    // which of course is insert. Technically
    // there is also command line mode, but it
    // is a part of normal, in a way.
    auto begin_line = current_line_.begin();
    switch (key) {
      case KEY_ENTER:
        // Enter:
        // - [ ] [ ] At end of the line, add current line as a joined string to
        // the document (at current row position)
        //     and create an empty current line
        // - [ ] [ ] In the middle of the line, add current line until cursor as
        // a joined string at current row position,
        //     and create a current line with the remaining contents after the
        //     cursor (i.e. remove before cursor as a string)
        // Note that in the end both are the same.
        HandleEnter();
        output->Emit(current_line_, ncolumn_, mode_);
        break;
      case KEY_ESC:
        HandleEsc();
        break;
      case KEY_CAPSLOCK:
        HandleEsc();
        break;
      case KEY_BACKSPACE:
        //  The cursor points to where a character is going to be inserted. We
        //  can only delete if there was something on the list
        if (current_line_.size() < ncolumn_) {
          break;
        }
        if (current_line_.size() == 0) {
          break;
        }
        ncolumn_ = fmax(0, --ncolumn_);

        std::advance(begin_line,
                     ncolumn_);  // Move the iterator to the desired position
        current_line_.erase(begin_line);
        if (!batched) {
          output->Emit(current_line_, ncolumn_, mode_);
        }
        break;
      case KEY_SPACE:
        c = ' ';  // Let this fall back at default
      default:
        // DEFAULT INSERTION
        current_line_.insert(ncolumn_, 1,
                             c);  // Size of the insertion (for char)
        ncolumn_++;
        if (!batched) {
          output->Emit(current_line_, ncolumn_, mode_);
        }
    }
    return;
  }
  if (mode_ == EditorMode::kCommandLineMode) {
    switch (key) {
      case KEY_BACKSPACE:
        command_line_.pop_back();
        output->CommandLine(command_line_);
        break;
      case KEY_ENTER:
        auto command = std::string(command_line_.begin(), command_line_.end());
        command_line_.clear();
        if (command == ":q") {
          mode_ = EditorMode::kNormal;
          output->Command(OutputCommands::kSplash);
          return;
        }
        if (command == ":wc") {
          int wc = 0;
          int cc = 0;
          WordCount(&wc, &cc);
          constexpr size_t kBufferSize = 64;
          char buffer[kBufferSize];
          snprintf(buffer, kBufferSize, "w: %d c: %d l: %d", wc, cc,
                   CountLines());
          output->CommandLine(buffer);
        }
#if VERIFY_MODE
        // Commands to test max size in memory and have
        // example texts
        //
        // With GFX:
        // Chars in doc: 108500
        // Chars in line: 61440
        // With badger:
        // Chars in doc: 106500
        // Chars in line: 11391 -> Chars in line: 11365
        // With string, in line: 61440
        if (command == ":test_line") {
          int accum = 0;
          try {
            while (true) {
              current_line_.push_back('a');
              ++accum;
              printf("Line: %d\n", accum);
            }
          } catch (const std::bad_alloc& e) {
            current_line_.clear();
            printf("Out of memory: %d\n", accum);
          } catch (...) {
            current_line_.clear();
            printf("Out of memory? %d\n", accum);
          }
        }
        if (command == ":lorem") {
          current_line_.assign(
              "The treeship Yggdrasill was a marvel of bio-engineering, a "
              "living vessel grown from the heartwood of a world tree. Its "
              "hull was a tapestry of interwoven bark and leaves, its decks a "
              "network of branches and vines. Sunlight filtered through the "
              "canopy, casting dappled shadows on the mossy floor. The air was "
              "filled with the scent of pine needles and the gentle rustling "
              "of leaves. Yggdrasill was a ship that breathed and grew, a "
              "testament to the harmonious union of technology and nature.");
          UpdateCurrentLine(-1);
          // Fake key up to update the document to contain the current line…
          // if not empty
          output->Emit(current_line_, ncolumn_, mode_);
        }
        if (command == ":test_doc") {
          int accum = 0;
          const std::string hundred =
              "0123456789012345678901234567890123456789012345678901234567890123"
              "456789012345678901234567890123456789";
          while (true) {
            document_.emplace_back(hundred + hundred + hundred + hundred +
                                   hundred);
            ++accum;
            printf("Doc: %d\n", accum * 500);
          }
        }
#endif  // VERIFY_MODE
        if (command == ":w") {
          if (current_line_.size()) {
            UpdateCurrentLine(-1);
            // Fake key up to update the document to contain the current line…
            // if not empty
          } else {
#if DEBUG_MODE
            printf("Skipped empty\n");
#endif  // DEBUG_MODE
          }
          should_save = true;
        }
        if (command == ":e") {
          FILE* rFile = fopen("/file.txt", "r");
          current_line_.clear();
          document_.clear();
          if (rFile) {
            fseek(rFile, 0, SEEK_END);
            uint32_t f1Size = ftell(rFile);
            rewind(rFile);

            char* buffer = reinterpret_cast<char*>(
                malloc(sizeof(char) * (f1Size + 1)));  // +1 for null terminator
            fread(buffer, 1, f1Size, rFile);
            buffer[f1Size] = '\0';  // Null-terminate the buffer

            // Process buffer into lines
            char* lineStart = buffer;
            for (uint32_t i = 0; i < f1Size; ++i) {
              if (buffer[i] == '\n' || i == f1Size - 1) {
                buffer[i] = '\0';  // Temporarily null-terminate the line
                document_.emplace_back(lineStart);
                lineStart = buffer + i + 1;
              }
            }

            free(buffer);
            fclose(rFile);
            row_ = document_.begin();
            current_line_ = *row_;
            ncolumn_ = 0;
            output->Emit(current_line_, ncolumn_, mode_);
            output->CommandLine("loaded");
          } else {
            output->CommandLine("error");
            // TODO(me) RGB to red
          }
        }
        if (command == ":ps") {
          printf("\n");
          printf("--------- Start ---------\n");
          printf("\n");
          for (const auto& line : document_) {
            printf("%s\n", line.c_str());
          }
          printf("--------- End ---------\n");
          output->CommandLine("sent");
        }
        mode_ = EditorMode::kNormal;
        return;
    }
    switch (c) {
      case '\0':
        break;
      default:
        command_line_.emplace_back(c);
#if DEBUG_MODE
        printf("Default case in handling keys for commands");
#endif  // DEBUG_MODE
        output->CommandLine(command_line_);
        return;
    }
  }
}

void Editor::Refresh(){
  // Helper to get the output to request a refresh
  output->Emit(current_line_, ncolumn_, mode_);
}

void Editor::Init() {
  output = new Output();  // This dies with the editor, so :shrug:
  row_ = document_.begin();
  // begin = end when empty, but I think this is easier to understand
  output->Init(this);
}

void Editor::WordCount(int* wc, int* cc) {
  (*wc) = 0;
  (*cc) = 0;
  for (const std::string& line : document_) {
    int word_count = 0;
    bool in_word = false;

    for (char c : line) {
      (*cc) = (*cc) + 1;
      if (std::isspace(c)) {
        if (in_word) {
          word_count++;
        }
        in_word = false;
      } else {
        in_word = true;
      }
    }

    if (in_word) {
      word_count++;
    }

    (*wc) += word_count;
  }
}

void Editor::HandleEsc() {
  if (mode_ == EditorMode::kInsert) {
    mode_ = EditorMode::kNormal;
    output->Command(OutputCommands::kCommandMode);
    output->Emit(current_line_, ncolumn_, mode_);
  }
}

// Expose the C-compatible functions
extern "C" void ProcessChar(uint8_t k, KeyModifiers* mod, bool batched) {
  Editor::GetInstance().ProcessKey(k, mod, batched);
}

extern "C" void ProcessHandlers() {
  Editor::GetInstance().ProcessHandlers();
}

extern "C" void ProcessEvent(EventType ev) {
  Editor::GetInstance().ProcessEvent(ev);
}

extern "C" void EditorOutputInit() {
  Editor::GetInstance().Init();
}

// Arrows:
// up right down left
// modifiers=0, pressed keys=[0x52](0x20004c0c) id=0 modifiers=0, pressed
// keys=[], battery=0 modifiers=0, pressed keys=[0x4f](0x20004c0c) id=0
// modifiers=0, pressed keys=[], battery=0 modifiers=0, pressed
// keys=[0x51](0x20004c0c) id=0 modifiers=0, pressed keys=[], battery=0
// modifiers=0, pressed keys=[0x50](0x20004c0c) id=0 modifiers=0, pressed
// keys=[], battery=0
