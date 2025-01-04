// Copyright 2025 Ruben Berenguel

// Based on the bluepad32 example file

#include <btstack_run_loop.h>
#include <pfs.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <uni.h>
#include <dirent.h>
#include <ffs_pico.h>
#include <pico/binary_info.h>
#include "pico/multicore.h"
#include "pico/queue.h"
#include "pico/util/queue.h"
#include "./sdkconfig.h"
#include "./setup.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

// Defined in pico_bt.c
struct uni_platform* get_pico(void);

// Set up Flash filesystem size
#define ROOT_SIZE 0x20000     // flash LFS size, last 0.125mb of flash
#define ROOT_OFFSET 0x1E0000  // offset from start of flash

// BT keypress queue depth
# define BT_EVENT_QUEUE 100

queue_t call_queue;

void core1_entry() {
  while (1) {
    bool batched = false;
    queue_entry_t entry;
    // If the queue depth is higher than 1, we are batching characters.
    // These handlers will only trigger on BT data, so require
    // pressing some key.
    ProcessHandlers();
    if (queue_get_level(&call_queue) > 1) {
      batched = true;
      while (queue_get_level(&call_queue) > 1) {
        queue_remove_blocking(&call_queue, &entry);
        ProcessChar(entry.keycode, &entry.modifiers, /*batch=*/true);
      }
    }
    queue_remove_blocking(&call_queue, &entry);
    if (entry.keycode == 0 && !batched) {
      continue;
    } else {
      if (batched) {
        // Send the character regardless to get a screen update for free.
        ProcessChar(entry.keycode, &entry.modifiers, /*batch=*/false);
        batched = false;
      } else {
        // In this case the character needs to be sent normally.
        // But do not reset batched.
        ProcessChar(entry.keycode, &entry.modifiers, /*batch=*/false);
      }
    }
  }
}

int main() {
  struct pfs_pfs* pfs;
  struct lfs_config cfg;
  ffs_pico_createcfg(&cfg, ROOT_OFFSET, ROOT_SIZE);
  pfs = pfs_ffs_create(&cfg);
  int mount = pfs_mount(pfs, "/");
  stdio_init_all();
  EditorOutputInit();
  queue_init(&call_queue, sizeof(queue_entry_t),
             BT_EVENT_QUEUE);
  // initialize CYW43 driver architecture
  // (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
  if (cyw43_arch_init()) {
    return -1;
  }
  multicore_launch_core1(core1_entry);
  // Turn-on LED. Turn it off once init is done.
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  // Must be called before uni_init()
  uni_platform_set_custom(get_pico());
  // Initialize BP32
  uni_init(0, NULL);
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
  btstack_run_loop_execute();
  return 0;
}
