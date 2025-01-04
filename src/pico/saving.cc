// Copyright 2025 Ruben Berenguel

#include <hardware/sync.h>
#include <stdio.h>
#include <stdlib.h>
#include "./editor.h"

void Editor::ProcessSaving() {
  // Why? The processing loop for the bluetooth stack is hot in core 0,
  // and interrupts. Writing to flash needs to be wrapped without interrupts…
  // so needs to happen in core 0. This is the handler that will save for us,
  // on "next" event.
  if (should_save) {
    FILE* wFile =
        fopen("/file.txt", "w");  // "w" erases file if it already exists
    if (wFile) {
      for (const auto& line : document_) {
        if (line.size()) {
          uint32_t ints = save_and_disable_interrupts();
#if DEBUG_MODE
          printf("Writing (%zu) %s\n", line.size(), line.c_str());
#endif  // DEBUG_MODE
          int chars_written = fprintf(wFile, "%s", line.c_str());
          restore_interrupts(ints);
#if DEBUG_MODE
          printf("Written: %d\n", chars_written);
#endif  // DEBUG_MODE
          if (ferror(wFile)) {
#if DEBUG_MODE
            printf("Error writing to file: %d\n", errno);
#endif  // DEBUG_MODE
            output->CommandLine("error");
          }
        } else {
          printf("Skipped empty\n");
        }
        fprintf(wFile, "\n");  // Add newline after each line
      }
#if DEBUG_MODE
      printf("W\n");
#endif  // DEBUG_MODE
      uint32_t ints = save_and_disable_interrupts();
      fflush(wFile);
      restore_interrupts(ints);
#if DEBUG_MODE
      printf("W+F\n");
#endif  // DEBUG_MODE
      ints = save_and_disable_interrupts();
      fclose(wFile);
      restore_interrupts(ints);
#if DEBUG_MODE
      printf("W+C\n");
#endif  // DEBUG_MODE
      output->CommandLine("saved");
    }
    should_save = false;
  }
}
