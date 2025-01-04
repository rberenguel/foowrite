# foowrite

A distraction-free writing device you can take with you anywhere. With vim keybindings.

---

Click to see a video of it in action. It was cold.

[![](https://raw.githubusercontent.com/rberenguel/foowrite/refs/heads/main/media/foowrite-video-img.jpg)](https://www.youtube.com/watch?v=FT0cgbyiSng&mode=theatre)

- [foowrite](#foowrite)
  - [Building this](#building-this)
  - [Code shape](#code-shape)
  - [Flash the device](#flash-the-device)
  - [Usage](#usage)
  - [Why? Short story](#why-short-story)
  - [Roadmap](#roadmap)
  - [Dangers](#dangers)
  - [Credits](#credits)
  - [Contributing](#contributing)


## Building this

I haven't set up this from scratch in several months, so… I might be missing something. In principle:

- Clone this
- `git submodule update --init --recursive`

and then the usual

```bash
mkdir build.pico
cd build.pico
cmake .. -DBUILD_FOR_PICO=on
```

or

```bash
mkdir build.mac
cd build.mac
cmake .. -DBUILD_FOR_PICO=off
```

if you want to run the tests (`./tests` after the above). Additional flags can be used for particular Pico flavours, but currently only GFX compiles. See `CMakeLists.txt` for the options.

Requires: Pico SDK, Pimoroni libraries, [bluepad32](https://github.com/ricardoquesada/bluepad32), and [pico-filesystem](https://github.com/Memotech-Bill/pico-filesystem). See [Pimoroni](https://github.com/pimoroni/pimoroni-pico) and [Pico](https://github.com/pimoroni/pimoroni-pico/blob/main/setting-up-the-pico-sdk.md) for setup details, if anything is missing it is likely explained there, after all it is how I set it up too.

Reminder for myself in case I screw something:

```
 2152d0b7aba0b7cf5f54146e56b21c35dbb87024 bluepad32 (4.1.0)
 9821333c968d6a0d41b28ecff7f92973f798d0b9 pico-filesystem (heads/main)
 6a7db34ff63345a7badec79ebea3aaef1712f374 pico-sdk (1.5.1)
 bb51ce5ad0f6a880b4a16fdb7dd0729a27a057d3 pimoroni-pico (v1.22.2-1bitpng-99-gbb51ce5a)
```

Adding this should be a matter of `git submodule add https://github.com/path-to-repo`.

## Code shape

The editor core is in [`editor.cc`](https://github.com/rberenguel/foowrite/blob/main/src/editor.cc), which requires an implementation of the definitions in [`output.hpp`](https://github.com/rberenguel/foowrite/blob/main/src/output.hpp). For the GFX, that is in [`pico/gfx/gfx.cc`](https://github.com/rberenguel/foowrite/blob/main/src/pico/gfx/gfx.cc). The bluetooth stack runs on a loop on core 0, and the editor only reacts to key events on core 1. This might not be needed for the GFX, but was required for the Badger2040. The main loop is in C, and the hooks to the editor are provided as `extern C` functions calling the editor singleton. Saving to flash is a bit convoluted, since it needs to happen on core 0 (or more like, it can't be interrupted).

To avoid adding interrupts or more event handlers, every time a Bluetooth keypress is checked a method on the screen output is called to check any buttons (if provided). This makes using the device buttons (the GFX has buttons A-E) a bit strange: you hold one of them _and_ press a keyboard key, but avoids interrupts.

Adding a new screen to a Pico base is "as easy" as replicating what is available for the GFX, it reduces to figuring out how to emit text to screen, and handling some events.

Adding another system that is Bluepad32 compatible _should_ be doable but I don't want to figure that one out.

The editor itself is just a very large state machine keeping track of mode and what keys are in the command string. It's somewhat ugly, I want to refactor it a bit but I don't want to risk making the code longer.

The internal data for the editor is a `std::string` for the current line, and a `std::list<std::string>` for the document. All operations are then done on these. So far it has seemed fast enough and relatively easy, so there is no reason to add a rope or anything fancy. After all, the Pico can't handle a very long document anyway (around 100k characters are left of memory available).

## Flash the device

You will need the `uf2` file built above (or get one from [here](https://github.com/rberenguel/foowrite/releases/)) and a Pico W (TODO: test with a Pico 2 W, I have one for that) with a GFX Pack screen from Pimoroni. If you are feeling adventurous, just write your own screen driver (like [`gfx.cc`](https://github.com/rberenguel/foowrite/blob/main/src/pico/gfx/gfx.cc)). Put the device in DFU mode, upload the `uf2` file and the pico should boot with the splash screen and with a blue shade hinting at "I want to connect to something via bluetooth".

## Usage

The device will pair to any keyboard-like thing trying to pair. This may not be ideal in all situations, but I find it a feature, not a bug, so I won't change this. Once the pico starts the screen LED will turn blue, it is waiting to pair. Put a keyboard in pairing mode (it will pair to _any_ input device, so don't start playing with your unpaired PlayStation controller at the same time) and the LED will change to green. Ready!

To start writing, just press `i` or `a` to go from normal to insert mode, and write normally. Enter creates a new line, and long lines are wrapped and eventually paginated via arrow keys (no, no vim movement keys).

Default keyboard layout when building is Colemak, you can get Qwerty (not tested) by adding `-DUSE_QWERTY=on` to cmake.

The device has around 100k characters available to type in memory, and around 128kb for saving the file. Don't write your novel fully here!

Some basic vim-like normal mode commands are available:

- `daw`/`diw`: delete around/inside word
- `caw`/`ciw`: change around/inside word
- `dd`: delete current line
- `cc`: change current line
- `d$`: delete from cursor to end of line
- `^`/`0`/`$`: line movements
- `w`/`b`: word movements

Ex commands:

- `:w`: Save document
- `:e`: Open document
- `:wc`: Show word, character and line count in the command line space
- `:ps`: Send the current document through the serial port
- `:lorem` (only when setting `VERIFY_MODE` 1 in [`editor.cc`](https://github.com/rberenguel/foowrite/blob/main/src/editor.cc)): set one long line in the document
- `:test_doc` (only when setting `VERIFY_MODE` 1 in [`editor.cc`](https://github.com/rberenguel/foowrite/blob/main/src/editor.cc)): Keep adding 100 character lines until panic, shows number of lines in serial
- `:test_line` (only when setting `VERIFY_MODE` 1 in [`editor.cc`](https://github.com/rberenguel/foowrite/blob/main/src/editor.cc)): Keep adding characters to the current line until panic, shows number of characters in serial

To save, go to normal mode with `ESC`, type `:w` and press enter. For now there is only one file (I may change this in the future). To open it, `:e` and enter. To get the file "out", plug the pico to a computer, connect to it for example via `minicom` (`minicom -b 115200 -o -D /dev/tty.usbmodem11301` but the port in your case will be different), pair a keyboard and in normal mode type `:ps`. The file will be output to the serial console. I'm trying to find smoother ways to do this.

Brightness can be adjusted by pressing the `B` button, and while held, any keyboard key (for reasons). Different splash screens can be seen by pressing the `E` button and any keyboard key.

## Why? Short story

I have always been interested in having a distraction free writing device, but I always found issues with existing ones:

- Pricy.
- Ugly.
- Pricy and ugly.
- Horrible keyboard.
- Etc.

At some point around 2021/2022 I thought _I'll just write my own_ (with blackjack, etc). At the time
I tried using a M5Stack Core2, but couldn't figure out how to get Bluetooth host working on it and gave up.
I gave it another shot early 2023, with a Pico W but again had issues with getting host working, my low
level programming (and patience) are low. Then, I wrote [PiWrite](https://github.com/rberenguel/PiWrite).

Later, in 2024 I found [bluepad32](https://github.com/ricardoquesada/bluepad32)
which offers exactly what I want: Bluetooth host working on a Pico W.

I got hold of a [Pimoroni Badger2040](https://shop.pimoroni.com/products/badger-2040-w?variant=40514062188627)
and a [Pimoroni Unicorn](https://shop.pimoroni.com/products/pico-unicorn-pack?variant=32369501306963) 
and started trying to hook together all the pieces, with the main goal of having an eink, portable, text editor.

For the editor part, I just got inspired by what I wrote for [PiWrite](https://github.com/rberenguel/PiWrite) and
used a vi-like line editor. The problem was the eink part: full refresh was too slow, partial refresh looked
dirty, wasn't extremely fast, and keeping the text up to date while typing was extremely annoying, code-wise.

I lost a lot of time trying to get that to work, but the frustration made me stop almost completely (as a side effect
I wrote two Chrome extensions, [Goita](https://github.com/rberenguel/goita) and [Salta](https://github.com/rberenguel/salta), 
I wasn't just brooding). For [Christmas](https://www.youtube.com/watch?v=NvqhMdnpsyE) [Laia](https://substack.com/@threadhappens?)
got me a [Pimoroni GFX Pack](https://shop.pimoroni.com/products/pico-gfx-pack?variant=40414469062739) though. LCD! Fast refresh rate!

What I couldn't do in 3 months I did in 3 days, fixing several bugs, getting save to work and adding long-line pagination. It might be
buggy, but seems usable.

## Roadmap

- Some more vim commands (go to line and search, likely, `dt` and `ct`, less likely).
- Try on a Pico 2 W.
- Implementation for the small [Pico Display Pack](https://shop.pimoroni.com/products/pico-display-pack?variant=32368664215635) or the larger [Pico Display Pack 2.0](https://shop.pimoroni.com/products/pico-display-pack-2-0?variant=39374122582099), to have 2 reference implementations.

## Dangers

I haven't fully tested the device, typing and saving a lot yet. I plan on doing it, and assume I might lose something I write. If you use it, assume the same.

## Credits

- [bluepad32](https://github.com/ricardoquesada/bluepad32) for the stack that makes it actually work
- [pico-filesystem](https://github.com/Memotech-Bill/pico-filesystem) to allow saving easily
- [Pimoroni](https://github.com/pimoroni/pimoroni-pico) for the hardware options to choose (I'm not sponsored, just a normal fan)
- Splash screen is inspired on my algorithmic art sketch, [70s Patch](https://github.com/rberenguel/sketches?tab=readme-ov-file#202303---70s-patch-switzerland-p5js)

## Contributing

As in many of my recent projects, I _might_ accept contributions, but consider this a project for myself. So, it will be shaped as I like it. BUT! This is still _open source_. Feel free to fork it, tweak it to your liking and _make it yours_.
