#include "../src/editor.h"
#include "../src/editor_mode.h"

#include "../src/layout.h"
#include "./layout_map.h"
#include "./editor_stubs.hpp"

void SendString(Editor* editor, std::string string) {
  auto mods = KeyModifiers{};
  for (const auto& it : string) {
    std::string letter(1, it);
    const auto key_code = layout_map.at(letter);
    editor->ProcessKey(key_code, &mods, false);
  }
}


void Output::Emit(const std::string& s, const int cursor_pos, EditorMode mode) {
}
void Editor::ProcessSaving() {}
void Output::CommandLine(const std::list<char>& c) {}
void Output::CommandLine(const std::string& c) {}
void Output::Command(const OutputCommands& command) {}
int Output::NextLine() {

  return -1;
}
int Output::PrevLine() {

  return -1;
}
void Output::ProcessEvent(EventType ev){};
void Output::ProcessHandlers(){};
void Output::Init(Editor* e) {}
int Output::CurrentTimeInMillis() {
  return 1000;
}
