// Copyright 2025 Ruben Berenguel

#include "../layout.h"

#include <ctype.h>   // for toupper()
#include <stdint.h>  // For uint8_t
#include <stdio.h>

#include "../setup.h"

char get_char_from_key(const uint8_t key, KeyModifiers* modifiers) {
  static const char keymap[] = {
      '\0', '\0', '\0', '\0', 'a', 'b', 'c', 'd',  'e',  'f',  'g',
      'h',  'i',  'j',  'k',  'l', 'm', 'n', 'o',  'p',  'q',  'r',
      's',  't',  'u',  'v',  'w', 'x', 'y', 'z',  '1',  '2',  '3',
      '4',  '5',  '6',  '7',  '8', '9', '0', '\0', '\0', '\0', '\0',
      ' ',  '-',  '=',  '[',  ']', '#', ';', '\'', ',',  '.',  '/'};
  static const char shift_keymap[] = {
      '\0', '\0', '\0', '\0', 'A', 'B', 'C', 'D',  'E',  'F',  'G',
      'H',  'I',  'J',  'K',  'L', 'M', 'N', 'O',  'P',  'Q',  'R',
      'S',  'T',  'U',  'V',  'W', 'X', 'Y', 'Z',  '!',  '@',  '#',
      '$',  '%',  '^',  '&',  '*', '(', ')', '\0', '\0', '\0', '\0',
      ' ',  '_',  '+',  '{',  '}', '#', ':', '"',  '<',  '>',  '?'};

  // Check if the keycode is valid
  if (key >= sizeof(keymap)) {
    return '?';  // Return '?' for unknown key codes
  }
  char c = keymap[key];
  if (modifiers->shift) {
    c = shift_keymap[key];
  }
  // Handle shift for letters
  /*if (modifiers & 0x02) {  // Bit 1 indicates Shift is pressed
      c = toupper(c);
  }*/

  return c;
}
