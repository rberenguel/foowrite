#include <gtest/gtest.h>
#include "../src/editor.h"
#include "../src/editor_mode.h"
#include "./editor_stubs.hpp"
#include "../src/layout.h"


TEST(FooWrite, Movements_NewLineBackspaceAndDownArrow) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "ia\n\nb\ncD");
  auto doc = editor.GetDocument();
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(doc, "a\n\nb\nc\n");
  // a
  //
  // b
  // c
  EXPECT_EQ(line, "c");
  line = editor.GetCurrentLine();
  SendString(&editor, "UfU");
  doc = editor.GetDocument();
  line = editor.GetCurrentLine();
  EXPECT_EQ(doc, "a\n\nbf\nc\n");
  EXPECT_EQ(line, "");
  SendString(&editor, "gU");
  doc = editor.GetDocument();
  line = editor.GetCurrentLine();
  EXPECT_EQ(doc, "a\ng\nbf\nc\n");
  EXPECT_EQ(line, "a");
  SendString(&editor, "\b\bz");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "z");
  SendString(&editor, "UeDe");
  line = editor.GetCurrentLine();
  auto count = editor.CountLines();
  EXPECT_EQ(count, 4);
  EXPECT_EQ(line, "ge");
  doc = editor.GetDocument();
  // e hasn't been committed to the document yet
  EXPECT_EQ(doc, "ez\ng\nbf\nc\n");
}

TEST(FooWrite, Movements_InsertAtEnd) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ifoo");
  editor.ProcessKey(KEY_ESC, &mods, false);
  // Ctrl-a in normal mode
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  auto result = editor.GetCurrentLine();
  EXPECT_EQ(result, "foo");
  SendString(&editor, "ifoo");
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "foofoo");
  editor.ProcessKey(KEY_ESC, &mods, false);
  // Ctrl-e in normal mode
  mods.ctrl = true;
  editor.ProcessKey(KEY_E, &mods, false);
  SendString(&editor, "ibar");
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "foofobaro");
}

TEST(FooWrite, Movements_NormalA) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ab");
  auto result = editor.GetCurrentLine();
  EXPECT_EQ(result, "b");
  SendString(&editor,
             "\x29"
             "ac");
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "bc");
  editor.ProcessKey(KEY_ESC, &mods, false);
  // Ctrl-a in normal mode
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "aq");
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "bqc");
  // Ctrl-a in normal mode, with I
  editor.ProcessKey(KEY_ESC, &mods, false);
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "ifg");
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "fgbqc");
}

TEST(FooWrite, Movements_UpAlignment) {
  // This is as in PiWrite, esc moves insertion one to the left
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "i1\n2U3");
  auto result = editor.GetCurrentLine();
  EXPECT_EQ(result, "13");
  SendString(&editor,
             "\n4U\x29"
             "i5D");
  result = editor.GetCurrentLine();
  auto doc = editor.GetDocument();
  EXPECT_EQ(doc, "513\n4\n2\n");
  EXPECT_EQ(result, "4");
}

TEST(FooWrite, Movements_w) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "i0\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 789 00");
  SendString(&editor, "wwi6\x29");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 6789 00");
  SendString(&editor, "wwwwwwwwi1");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0123 456 6789 010");
}

TEST(FooWrite, Movements_b) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = true;
  editor.ProcessKey(KEY_E, &mods, false);
  mods.ctrl = false;
  SendString(&editor, "ix\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "123 456 789 0x0");
  SendString(&editor, "bia\x29");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "123 456 789 a0x0");
  SendString(&editor, "bbic\x29");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "123 456 c789 a0x0");
}

TEST(FooWrite, Movements_dollar) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i123 456 789 00\x29");
  auto mods = KeyModifiers{};
  mods.ctrl = true;
  editor.ProcessKey(KEY_A, &mods, false);
  mods.ctrl = false;
  mods.shift = true;
  editor.ProcessKey(KEY_4, &mods, false);
  SendString(&editor, "ix\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "123 456 789 0x0");
}

TEST(FooWrite, Movements_hat) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i     123 456\x29");
  auto mods = KeyModifiers{};
  mods.shift = true;
  editor.ProcessKey(KEY_6, &mods, false);
  SendString(&editor, "i0\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "     0123 456");
  editor.ProcessKey(KEY_4, &mods, false);
  editor.ProcessKey(KEY_6, &mods, false); 
  SendString(&editor, "ax\x29");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "     0x123 456"); 
}

TEST(FooWrite, Movements_zero) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  SendString(&editor, "i     123 456\x29");
  SendString(&editor, "0");
  SendString(&editor, "i0\x29");
  auto line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0     123 456");
  auto mods = KeyModifiers{};
  mods.shift = true;
  editor.ProcessKey(KEY_4, &mods, false);
  SendString(&editor, "0");
  SendString(&editor, "ax\x29");
  line = editor.GetCurrentLine();
  EXPECT_EQ(line, "0x     123 456"); 
}
