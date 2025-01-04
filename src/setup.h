// Copyright 2025 Ruben Berenguel

// Common stuff between C and C++ parts

#ifndef SRC_SETUP_H_
#define SRC_SETUP_H_

#include <stdbool.h>
#include <stdint.h>  // For uint8_t

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  bool shift;
  bool ctrl;
  bool meta;  // Also known as "Super" or "Windows" key
  bool alt;
} KeyModifiers;

// C-compatible  declarations

typedef enum {
  EV_BT_ON,
  EV_BT_OFF,
  EV_SAVE
} EventType;

void ProcessChar(uint8_t c, KeyModifiers* mod, bool batched);
void ProcessHandlers();
void ProcessEvent(EventType ev);
void ProcessSaving();
void EditorOutputInit();

#ifdef __cplusplus
}  // extern "C"

#endif

#endif  // SRC_SETUP_H_
