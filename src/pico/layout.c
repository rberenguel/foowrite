// Copyright 2025 Ruben Berenguel

#include "../layout.h"

#include <ctype.h>   // for toupper()
#include <stdint.h>  // For uint8_t
#include <stdio.h>

#include "setup.h"

char get_char_from_key(const uint8_t key, KeyModifiers* modifiers) {
  // I have not tested the qwerty implementation, I just translated the
  // Colemak one (which I have tested quite a bit).
#if USE_QWERTY
  // Qwerty
  static const char keymap[] = {
      '\0', '\0', '\0', '\0', 'a',  'b',  'c',  'd',  'e',  'f', 'g', 'h',
      'i',  'j',  'k',  'l',  'm',  'n',  'o',  'p',  'q',  'r', 's', 't',
      'u',  'v',  'w',  'x',  'y',  'z',  '1',  '2',  '3',  '4', '5', '6',
      '7',  '8',  '9',  '0',  '\0', '\0', '\0', '\0', '\0', '-', '=', '[',
      ']',  '|',  '#',  ';',  '\'', '`',  ',',  '.',  '/'};
  static const char shift_keymap[] = {
      '\0', '\0', '\0', '\0', 'A',  'B',  'C',  'D',  'E',  'F', 'G', 'H',
      'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',  'Q',  'R', 'S', 'T',
      'U',  'V',  'W',  'X',  'Y',  'Z',  '!',  '@',  '#',  '$', '%', '^',
      '&',  '*',  '(',  ')',  '\0', '\0', '\0', '\0', '\0', '_', '+', '{',
      '}',  '\\', '#',  ':',  '"',  '~',  '<',  '>',  '?'};
#else
  // Colemak
  static const char keymap[] = {
      '\0', '\0', '\0', '\0', 'a',  'b',  'c',  's',  'f',  't', 'd', 'h',
      'u',  'n',  'e',  'i',  'm',  'k',  'y',  ';',  'q',  'p', 'r', 'g',
      'l',  'v',  'w',  'x',  'j',  'z',  '1',  '2',  '3',  '4', '5', '6',
      '7',  '8',  '9',  '0',  '\0', '\0', '\0', '\0', '\0', '-', '=', '[',
      ']',  '|',  '#',  'o',  '\'', '`',  ',',  '.',  '/'};
  static const char shift_keymap[] = {
      '\0', '\0', '\0', '\0', 'A',  'B',  'C',  'S',  'F',  'T', 'D', 'H',
      'U',  'N',  'E',  'I',  'M',  'K',  'Y',  ':',  'Q',  'P', 'R', 'G',
      'L',  'V',  'W',  'X',  'J',  'Z',  '!',  '@',  '#',  '$', '%', '^',
      '&',  '*',  '(',  ')',  '\0', '\0', '\0', '\0', '\0', '_', '+', '{',
      '}',  '\\', '#',  'O',  '"',  '~',  '<',  '>',  '?'};
#endif  // USE_QWERTY
  // Check if the keycode is valid
  if (key >= sizeof(keymap)) {
    return '?';  // Return '?' for unknown key codes
  }

  char c = keymap[key];
  if (modifiers->shift) {
    c = shift_keymap[key];
  }

  return c;
}
