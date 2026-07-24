#pragma once

#include <pebble.h>

// A Variant is one candidate watchface design. Exactly one Variant source file
// is compiled into any given Studio build — the wscript picks it from the
// STUDIO_VARIANT setting — and the entry point in main.c dispatches to whichever
// one that was through the descriptor below.
//
// Compiling only the selected Variant is deliberate: Variants are disposable, and
// a half-finished one sitting in the directory must not stop its siblings
// rendering.
typedef struct {
  const char *name;
  void (*load)(Window *window);    // required: build the layer tree
  void (*unload)(Window *window);  // required: tear it back down
  void (*tick)(struct tm *now);    // optional: redraw for a new time
} Variant;

// Defined by the selected Variant source file, via STUDIO_VARIANT below.
const Variant *variant_selected(void);

// Boilerplate for the bottom of a Variant source file.
#define STUDIO_VARIANT(variant_name, load_fn, unload_fn, tick_fn) \
  const Variant *variant_selected(void) {                         \
    static const Variant variant = {                              \
      .name = variant_name,                                       \
      .load = load_fn,                                            \
      .unload = unload_fn,                                        \
      .tick = tick_fn,                                            \
    };                                                            \
    return &variant;                                              \
  }
