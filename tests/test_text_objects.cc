#include <gtest/gtest.h>
#include "../src/editor.h"
#include "../src/editor_mode.h"
#include "./editor_stubs.hpp"
#include "../src/layout.h"

TEST(FooWrite, TextObjects_DD) {
  // This is as in PiWrite
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123\nU\x29");
  auto line = editor.GetCurrentLine();
  auto doc = editor.GetDocument();
  EXPECT_EQ(line, "123");
  EXPECT_EQ(doc, "123\n\n");
  SendString(&editor, "ddi42");
  line = editor.GetCurrentLine();
  //doc = editor.GetDocument();
  //EXPECT_EQ(doc, "\n\n"); This would not be consistent with other behaviour
  SendString(&editor, "D");
  doc = editor.GetDocument();
  EXPECT_EQ(doc, "42\n\n");
}

TEST(FooWrite, TextObjects_DD_2) {
  // This is as in PiWrite
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123\x29");
  SendString(&editor, "ddi456U");
  auto line = editor.GetCurrentLine();
  auto doc = editor.GetDocument();
  EXPECT_EQ(line, "456");
  EXPECT_EQ(doc, "456\n");
}

TEST(FooWrite, TextObjects_DDollar) {
  // This is as in PiWrite
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123L\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "123");
  SendString(&editor, "d");
  auto mods = KeyModifiers{};
  mods.shift = true;
  editor.ProcessKey(KEY_4, &mods, false);
  mods.shift = false;
  SendString(&editor, "iab");
  SendString(&editor, "D");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "ab1");  // Yes, vim does this
}

TEST(FooWrite, TextObjects_CDollar) {
  // Equiv to d$a
  // This is as in PiWrite
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor,
             "i123L\x29"
             "c");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "123");
  auto mods = KeyModifiers{};
  mods.shift = true;
  editor.ProcessKey(KEY_4, &mods, false);
  mods.shift = false;
  SendString(&editor, "1");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "11");  // Yes, vim does this
}


TEST(FooWrite, TextObjects_daw) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwi6\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 6789 00");
  SendString(&editor, "daw");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 00");
}

TEST(FooWrite, TextObjects_daw_at_end) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwi6\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 6789 00");
  SendString(&editor, "wdaw");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 6789");
}

TEST(FooWrite, TextObjects_diw) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwdiw");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456  00");
}

TEST(FooWrite, TextObjects_caw_mid) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwcaw999");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 99900");
}

TEST(FooWrite, TextObjects_caw_end) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwwcaw999");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 789999");
}

TEST(FooWrite, TextObjects_dawi_at_end) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwi6\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 6789 00");
  SendString(&editor, "wdawifoo");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 678foo9");
}

TEST(FooWrite, TextObjects_caw_at_end) {
  // This is currently failing by accessing something it shouldn't.
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwwcaw999");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 789999");
}

TEST(FooWrite, TextObjects_ciw) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29wwciw999");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 999 00");
}