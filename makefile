ifeq ($(origin .RECIPEPREFIX), undefined)
    $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >
.ONESHELL: #

SRCS := $(shell find src -name '*.c' -o -name '*.cc')

clean:
> cd build.pico
> make clean

build.pico/foowrite-badger.uf2: $(SRCS)
> cd build.pico
> cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=BADGER -DUSE_QWERTY=off
> make -j4 || exit
> cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=BADGER -DUSE_QWERTY=on
> make -j4 || exit

build.pico/foowrite-gfx.uf2: $(SRCS)
> cd build.pico
> cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=GFX -DUSE_QWERTY=off
> make -j4 || exit
> cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=GFX -DUSE_QWERTY=on
> make -j4 || exit

build.pico/foowrite-display1.uf2: $(SRCS)
> cd build.pico
> cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=DISPLAY1 -DUSE_QWERTY=off
> make -j4 || exit
> cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=DISPLAY1 -DUSE_QWERTY=on
> make -j4 || exit

badger: build.pico/foowrite-badger.uf2
gfx: build.pico/foowrite-gfx.uf2
display1: build.pico/foowrite-display1.uf2

release: badger gfx display1