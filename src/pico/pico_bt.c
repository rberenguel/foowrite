// Copyright 2025 Ruben Berenguel

// Based on the bluepad32 example file

#include <pico/cyw43_arch.h>
#include <pico/time.h>
#include <stddef.h>
#include <string.h>
#include <uni.h>

#include "./keyboard.h"
#include "./queue.h"
#include "./sdkconfig.h"
#include "./setup.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

extern queue_t call_queue;

//
// Platform Overrides
//
static void pico_init(int argc, const char** argv) {
  ARG_UNUSED(argc);
  ARG_UNUSED(argv);

#if 0
#endif
}

static void pico_on_init_complete(void) {
  // Safe to call "unsafe" functions since they are called from BT thread

  // Start scanning
  uni_bt_enable_new_connections_unsafe(true);

  // Based on runtime condition, you can delete or list the stored BT keys.
  uni_bt_del_keys_unsafe();

  // Turn off LED once init is done.
  // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

  //    uni_bt_service_set_enabled(true);

  uni_property_dump_all();
}

static uni_error_t pico_on_device_discovered(bd_addr_t addr, const char* name,
                                             uint16_t cod, uint8_t rssi) {
  // You can filter discovered devices here. Return any value different from
  // UNI_ERROR_SUCCESS;
  // @param addr: the Bluetooth address
  // @param name: could be NULL, could be zero-length, or might contain the
  // name.
  // @param cod: Class of Device. See "uni_bt_defines.h" for possible values.
  // @param rssi: Received Signal Strength Indicator (RSSI) measured in dBms.
  // The higher (255) the better.

  // As an example, if you want to filter out keyboards, do:
  /*if (((cod & UNI_BT_COD_MINOR_MASK) & UNI_BT_COD_MINOR_KEYBOARD) ==
  UNI_BT_COD_MINOR_KEYBOARD) { return UNI_ERROR_IGNORE_DEVICE;
  }*/

  return UNI_ERROR_SUCCESS;
}

static void pico_on_device_connected(uni_hid_device_t* d) {
  ProcessEvent(EV_BT_ON);
}

static void pico_on_device_disconnected(uni_hid_device_t* d) {
  ProcessEvent(EV_BT_ON);
}

static uni_error_t pico_on_device_ready(uni_hid_device_t* d) {
  // You can reject the connection by returning an error.
  return UNI_ERROR_SUCCESS;
}

static void pico_on_controller_data(uni_hid_device_t* d,
                                    uni_controller_t* ctl) {
  static uint8_t leds = 0;
  static uint8_t enabled = true;
  static uni_controller_t prev = {0};
  uni_gamepad_t* gp;

  // Used to prevent spamming the log, but should be removed in production.
  //    if (memcmp(&prev, ctl, sizeof(*ctl)) == 0) {
  //        return;
  //    }
  prev = *ctl;
  // Print device Id before dumping gamepad.
  logi("(%p) id=%d ", d, uni_hid_device_get_idx_for_instance(d));
  uni_controller_dump(ctl);

  switch (ctl->klass) {
    case UNI_CONTROLLER_CLASS_KEYBOARD:
      uni_keyboard_dump(&ctl->keyboard);
      KeyModifiers modifiers = {0};
      // TODO(me) This should check for modifiers only?
      uint8_t key_count = 0;
      for (size_t i = 0; i < UNI_KEYBOARD_PRESSED_KEYS_MAX; i++) {
        if ((&ctl->keyboard)->pressed_keys[i] != 0) {
          key_count++;
        }
      }
      if (key_count > 1) {
        break;
      }
      // Avoid bouncing when typing fast
      uint8_t keycode = (&ctl->keyboard)->pressed_keys[0];
      get_modifiers_from_hid(&ctl->keyboard, &modifiers);
      ProcessEvent(EV_SAVE);
      queue_entry_t entry = {keycode, modifiers};
      queue_add_blocking(&call_queue, &entry);
      break;
    default:
      loge("Unsupported controller class: %d\n", ctl->klass);
      break;
  }
}

static const uni_property_t* pico_get_property(uni_property_idx_t idx) {
  ARG_UNUSED(idx);
  return NULL;
}

static void pico_on_oob_event(uni_platform_oob_event_t event, void* data) {
  // Could handle event == UNI_PLATFORM_OOB_BLUETOOTH_ENABLED
  // but we don't need this and the less strings, the more memory available.
}

//
// Entry Point
//
struct uni_platform* get_pico(void) {
  static struct uni_platform plat = {
      .name = "Pico",
      .init = pico_init,
      .on_init_complete = pico_on_init_complete,
      .on_device_discovered = pico_on_device_discovered,
      .on_device_connected = pico_on_device_connected,
      .on_device_disconnected = pico_on_device_disconnected,
      .on_device_ready = pico_on_device_ready,
      .on_oob_event = pico_on_oob_event,
      .on_controller_data = pico_on_controller_data,
      .get_property = pico_get_property,
  };

  return &plat;
}
