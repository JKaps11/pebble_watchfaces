#include <pebble.h>
#include "windows/main_window.h"

static void prv_init(void) {
  main_window_push();
}

static void prv_deinit(void) {
  main_window_destroy();
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
