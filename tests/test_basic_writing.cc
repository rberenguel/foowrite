#include <gtest/gtest.h>
#include "../src/editor.h"
#include "../src/editor_mode.h"
#include "./editor_stubs.hpp"
#include "../src/layout.h"

TEST(FooWrite, BasicWriting_Hello) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ihelloU");
  auto result = editor.GetCurrentLine();
  auto count = editor.CountLines();
  EXPECT_EQ(result, "hello");
  EXPECT_EQ(count, 1);
}

TEST(FooWrite, BasicWriting_Hello2) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ihello\n");
  auto line = editor.GetCurrentLine();
  auto doc = editor.GetDocument();
  auto count = editor.CountLines();
  EXPECT_EQ(line, "");
  EXPECT_EQ(doc, "hello\n---\n");
  EXPECT_EQ(count, 2);
}

TEST(FooWrite, BasicWriting_Hello3) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ihello\nworld");
  auto line = editor.GetCurrentLine();
  auto doc = editor.GetDocument();
  auto count = editor.CountLines();
  EXPECT_EQ(line, "world");
  EXPECT_EQ(doc, "hello\n---\n");
  EXPECT_EQ(count, 2);
}

TEST(FooWrite, BasicWriting_NewLineAtEnd) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ifoo\nbar\n");
  auto result = editor.GetDocument();
  auto count = editor.CountLines();
  EXPECT_EQ(count, 3);
  EXPECT_EQ(result, "foo\nbar\n---\n");
}

TEST(FooWrite, BasicWriting_NewLineAtMid) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ibarL\nsUpq");
  auto line = editor.GetCurrentLine();
  auto doc = editor.GetDocument();
  auto count = editor.CountLines();
  EXPECT_EQ(count, 2);
  EXPECT_EQ(line, "bpqa");
  EXPECT_EQ(doc, "ba\nsr\n");
}

TEST(FooWrite, BasicWriting_NewLineMultiple) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ia\n\nb\ncUUdU");
  auto doc = editor.GetDocument();
  auto line = editor.GetCurrentLine();
  auto count = editor.CountLines();
  EXPECT_EQ(count, 4);
  EXPECT_EQ(line, "a");
  EXPECT_EQ(doc, "a\nd\nb\nc\n");
}


TEST(FooWrite, BasicWriting_Backspace) {
  auto editor = Editor::GetInstance();
  editor.ResetState();
  auto mods = KeyModifiers{};
  SendString(&editor, "ithis\b");
  auto result = editor.GetCurrentLine();
  EXPECT_EQ(result, "thi");
  editor.ProcessKey(KEY_BACKSPACE, &mods, false);
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "th");
  editor.ProcessKey(KEY_BACKSPACE, &mods, false);
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "t");
  editor.ProcessKey(KEY_BACKSPACE, &mods, false);
  result = editor.GetCurrentLine();
  EXPECT_EQ(result, "");
}
