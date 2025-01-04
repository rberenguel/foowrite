// Copyright 2025 Ruben Berenguel

#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "./layout.h"
#include "./setup.h"

struct termios orig_termios;

#define CTRL_KEY(k) ((k) & 0x1f)

// A big part of this version (one that can partially be used on a
// computer) is taken from antirez's kilo editor in C, based
// on the excellent walkthrough by snaptoken in
// https://viewsourcecode.org/snaptoken/kilo

void die(const char* s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}
void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  std::cout << std::hex << static_cast<int>(c) << "\r\n";
  if (c == '\x1b') {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return '\x1b';
    return '\x1b';
  } else {
    return c;
  }
}

int main() {
  enableRawMode();
  EditorOutputInit();
  while (1) {
    char k = editorReadKey();
    if (k == CTRL_KEY('c'))
      break;
    auto code = static_cast<int>(k) - 93;
    KeyModifiers modifiers{};
    if (k == '\x1b') {
      ProcessChar(KEY_ESC, &modifiers, /*batch=*/false);
      continue;
    }
    if (k == '\x20') {
      ProcessChar(KEY_SPACE, &modifiers, /*batch=*/false);
      continue;
    }
    if (iscntrl(k) && k != 127) {
      modifiers.ctrl = true;
      ProcessChar((k - 93) & 0x1f, &modifiers, /*batch=*/false);
    } else {
      ProcessChar(k - 93, &modifiers, /*batch=*/false);
    }
  }
  return 0;
}
