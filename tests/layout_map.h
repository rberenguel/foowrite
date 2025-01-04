#pragma once

#include <stdint.h> // for uint8_t
#include <map>
#include <string>

const std::map<std::string, uint8_t> layout_map = {
    {"mod_lctrl", 0x01},
    {"mod_lshift", 0x02},
    {"mod_lalt", 0x04},
    {"mod_lmeta", 0x08},
    {"mod_rctrl", 0x10},
    {"mod_rshift", 0x20},
    {"mod_ralt", 0x40},
    {"mod_rmeta", 0x80},
    {"none", 0x00}, // no key pressed
    {"err_ovf", 0x01}, // keyboard error roll over
    {"a", 0x04}, // keyboard a and a
    {"b", 0x05}, // keyboard b and b
    {"c", 0x06}, // keyboard c and c
    {"d", 0x07}, // keyboard d and d
    {"e", 0x08}, // keyboard e and e
    {"f", 0x09}, // keyboard f and f
    {"g", 0x0a}, // keyboard g and g
    {"h", 0x0b}, // keyboard h and h
    {"i", 0x0c}, // keyboard i and i
    {"j", 0x0d}, // keyboard j and j
    {"k", 0x0e}, // keyboard k and k
    {"l", 0x0f}, // keyboard l and l
    {"m", 0x10}, // keyboard m and m
    {"n", 0x11}, // keyboard n and n
    {"o", 0x12}, // keyboard o and o
    {"p", 0x13}, // keyboard p and p
    {"q", 0x14}, // keyboard q and q
    {"r", 0x15}, // keyboard r and r
    {"s", 0x16}, // keyboard s and s
    {"t", 0x17}, // keyboard t and t
    {"u", 0x18}, // keyboard u and u
    {"v", 0x19}, // keyboard v and v
    {"w", 0x1a}, // keyboard w and w
    {"x", 0x1b}, // keyboard x and x
    {"y", 0x1c}, // keyboard y and y
    {"z", 0x1d}, // keyboard z and z
    {"1", 0x1e}, // keyboard 1 and !
    {"2", 0x1f}, // keyboard 2 and @
    {"3", 0x20}, // keyboard 3 and #
    {"4", 0x21}, // keyboard 4 and $
    {"5", 0x22}, // keyboard 5 and %
    {"6", 0x23}, // keyboard 6 and ^
    {"7", 0x24}, // keyboard 7 and &
    {"8", 0x25}, // keyboard 8 and *
    {"9", 0x26}, // keyboard 9 and (
    {"0", 0x27}, // keyboard 0 and )
    {"\n", 0x28}, // keyboard return (enter)
    {"\x29", 0x29}, // keyboard escape
    {"\b", 0x2a}, // keyboard delete (backspace)
    {"tab", 0x2b}, // keyboard tab
    {" ", 0x2c}, // keyboard spacebar
    {"minus", 0x2d}, // keyboard - and _
    {"equal", 0x2e}, // keyboard = and +
    {"leftbrace", 0x2f}, // keyboard [ and {
    {"rightbrace", 0x30}, // keyboard ] and }
    {"backslash", 0x31}, // keyboard \ and |
    {"hashtilde", 0x32}, // keyboard non-us # and ~
    {"semicolon", 0x33}, // keyboard ; and :
    {"apostrophe", 0x34}, // keyboard ' and "
    {"grave", 0x35}, // keyboard ` and ~
    {"comma", 0x36}, // keyboard , and <
    {"dot", 0x37}, // keyboard . and >
    {"slash", 0x38}, // keyboard / and ?
    {"capslock", 0x39}, // keyboard caps lock
    {"f1", 0x3a}, // keyboard f1
    {"f2", 0x3b}, // keyboard f2
    {"f3", 0x3c}, // keyboard f3
    {"f4", 0x3d}, // keyboard f4
    {"f5", 0x3e}, // keyboard f5
    {"f6", 0x3f}, // keyboard f6
    {"f7", 0x40}, // keyboard f7
    {"f8", 0x41}, // keyboard f8
    {"f9", 0x42}, // keyboard f9
    {"f10", 0x43}, // keyboard f10
    {"f11", 0x44}, // keyboard f11
    {"f12", 0x45}, // keyboard f12
    {"sysrq", 0x46}, // keyboard print screen
    {"scrolllock", 0x47}, // keyboard scroll lock
    {"pause", 0x48}, // keyboard pause
    {"insert", 0x49}, // keyboard insert
    {"home", 0x4a}, // keyboard home
    {"pageup", 0x4b}, // keyboard page up
    {"delete", 0x4c}, // keyboard delete forward
    {"end", 0x4d}, // keyboard end
    {"pagedown", 0x4e}, // keyboard page down
    {"R", 0x4f}, // keyboard right arrow
    {"L", 0x50}, // keyboard left arrow
    {"D", 0x51}, // keyboard down arrow
    {"U", 0x52}, // keyboard up arrow
    {"numlock", 0x53}, // keyboard num lock and clear
    {"kpslash", 0x54}, // keypad /
    {"kpasterisk", 0x55}, // keypad *
    {"kpminus", 0x56}, // keypad -
    {"kpplus", 0x57}, // keypad +
    {"kpenter", 0x58}, // keypad enter
};