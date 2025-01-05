cmake_minimum_required(VERSION 3.13)

set(CMAKE_OSX_SYSROOT /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX15.2.sdk)

set(PROJECT_NAME "foowrite")
set(TARGET_NAME "-")
set(LAYOUT_NAME "")

option(BUILD_FOR_PICO "Build for the pico w" ON)

set(DISPLAY "GFX" CACHE STRING "Select display type")
set_property(CACHE DISPLAY PROPERTY STRINGS 
  "NONE" "UNICORN" "BADGER" "GFX" "DISPLAY1")

option(USE_QWERTY "Use the qwerty layout" OFF)

# Example: cmake .. -DBUILD_FOR_PICO=on -DDISPLAY=DISPLAY1 -DUSE_QWERTY=off

if(USE_QWERTY)
    add_definitions(-DQWERTY_MODE)
    set(LAYOUT_NAME "-qwerty")
endif()


# PICO_BOARD should be set to 'pico_w', or not set at all.
if(DEFINED PICO_BOARD AND NOT ${PICO_BOARD} STREQUAL "pico_w")
    message(FATAL_ERROR "PICO_BOARD must be set to 'pico_w' or not set at all")
else()
    set(PICO_BOARD "pico_w")
endif()

if(BUILD_FOR_PICO)
    set(PICO_CXX_ENABLE_EXCEPTIONS 1)
    set(BLUEPAD32_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/bluepad32)
    # To use BTstack from Pico SDK do
    #set(BTSTACK_ROOT ${PICO_SDK_PATH}/lib/btstack)
    # To use it from Bluepad32 (up-to-date, with custom patches for controllers):
    set(BTSTACK_ROOT ${BLUEPAD32_ROOT}/external/btstack)
    set(PICO_BTSTACK_PATH ${BTSTACK_ROOT})
    # initialize the SDK based on PICO_SDK_PATH
    # note: this must happen before project()
    set(PICO_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pico-sdk)
    set(PIMORONI_PICO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pimoroni-pico)
    include(pico_sdk_import.cmake)
    include(pimoroni_pico_sdk_import.cmake)
endif()


project(foowrite C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(BUILD_FOR_PICO)
    # initialize the Raspberry Pi Pico SDK
    pico_sdk_init()
    add_subdirectory (pico-filesystem pfs)
    if(DISPLAY STREQUAL "GFX")
        message("Building for Pico GFX")
        set(TARGET_NAME "-gfx")
        include(libraries/pico_graphics/pico_graphics)
        include(libraries/gfx_pack/gfx_pack)
        add_executable(foowrite
            src/main.c
            src/pico/pico_bt.c
            src/pico/keyboard.c
            src/pico/gfx/gfx.cc
            src/pico/gfx/splash.cc
            src/editor.cc
            src/pico/saving.cc
            src/pico/layout.c
        )
        target_link_libraries(foowrite PUBLIC
            flash_filesystem
            pico_stdlib
            pico_time
            pico_multicore
            pico_cyw43_arch_none
            pico_btstack_classic
            pico_btstack_cyw43
            bluepad32
            hardware_pio
            hardware_dma
            pico_graphics
            gfx_pack
        )
    endif()
    if(DISPLAY STREQUAL "DISPLAY1")
        message("Building for Display Pack 1 (1 inch)")
        set(TARGET_NAME "-display1")
        include(libraries/pico_graphics/pico_graphics)
        include(libraries/pico_display/pico_display)
        include(drivers/st7789/st7789)
        include(drivers/rgbled/rgbled)
        include(drivers/button/button)
        add_executable(foowrite
            src/main.c
            src/pico/pico_bt.c
            src/pico/keyboard.c
            src/pico/display1/display1.cc
            src/pico/display1/splash.cc
            src/editor.cc
            src/pico/saving.cc
            src/pico/layout.c
        )
        target_link_libraries(foowrite PUBLIC
            hardware_pio
            flash_filesystem
            pico_stdlib
            pico_time
            hardware_spi
            hardware_pwm
            pico_multicore
            pico_cyw43_arch_none
            pico_btstack_classic
            pico_btstack_cyw43
            bluepad32
            hardware_dma
            pico_display
            pico_graphics
            rgbled
            button
            st7789
        )
    endif()
    if(DISPLAY STREQUAL "UNICORN")
        message("Building for Pico Unicorn")
        set(TARGET_NAME "-unicorn")
        include(libraries/pico_graphics/pico_graphics)
        include(libraries/pico_unicorn/pico_unicorn)
        add_executable(foowrite
            src/main.c
            src/pico/pico_bt.c
            src/pico/keyboard.c
            src/pico/unicorn/unicorn.cc
            src/editor.cc
            src/pico/layout.c
        )
        target_link_libraries(foowrite PUBLIC
            flash_filesystem
            pico_stdlib
            pico_multicore
            pico_cyw43_arch_none
            pico_btstack_classic
            pico_btstack_cyw43
            bluepad32
            hardware_pio
            hardware_dma
            #pico_graphics
            pico_unicorn
        )
    endif()
    if(DISPLAY STREQUAL "BADGER")
        message("Building for Badger")
        set(TARGET_NAME "-badger")
        include(libraries/badger2040/badger2040)
        #include(libraries/pico_graphics/pico_graphics)
        add_executable(foowrite
            src/main.c
            src/screendiffer.cc
            src/pico/pico_bt.c
            src/pico/keyboard.c
            src/pico/badger/splash.cc
            src/pico/badger/badger.cc
            src/editor.cc
            src/pico/saving.cc
            src/pico/layout.c
        )
        target_link_libraries(foowrite PUBLIC
            flash_filesystem
            badger2040
            pico_graphics
            hardware_spi
            pico_multicore
            pimoroni_i2c
            pico_stdlib
            pico_cyw43_arch_none
            pico_btstack_classic
            pico_btstack_cyw43
            bluepad32
        )
    endif()

    include(common/pimoroni_i2c)
    include(common/pimoroni_bus)

    target_include_directories(foowrite PRIVATE
        src
        ${BLUEPAD32_ROOT}/src/components/bluepad32/include)

    # Needed for btstack_config.h / sdkconfig.h
    # so that libbluepad32 can include them
    include_directories(foowrite src)

    # Needed when using BTstack from our branch
    include_directories(${BTSTACK_ROOT}/3rd-party/bluedroid/encoder/include)
    include_directories(${BTSTACK_ROOT}/3rd-party/bluedroid/decoder/include)

    add_subdirectory(${BLUEPAD32_ROOT}/src/components/bluepad32 libbluepad32)
    pico_enable_stdio_usb(foowrite 1)
    pico_enable_stdio_uart(foowrite 0)

    # create map/bin/hex/uf2 file in addition to ELF.
    pico_add_extra_outputs(foowrite)


    set_target_properties(foowrite PROPERTIES OUTPUT_NAME "${PROJECT_NAME}${TARGET_NAME}${LAYOUT_NAME}")


else()
    # On Mac we do can run tests instead
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    add_executable(${PROJECT_NAME}
        src/screendiffer.cc
        src/mac/local.cc
        src/main.cc
        src/editor.cc
        src/mac/layout.c
    )

    enable_testing()

    aux_source_directory(tests TEST_SOURCES)
    add_executable(
        tests
        src/screendiffer.cc
        ${TEST_SOURCES}
        src/editor.cc
        src/mac/layout.c
    )
    target_link_libraries(
        tests
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(tests)
endif()