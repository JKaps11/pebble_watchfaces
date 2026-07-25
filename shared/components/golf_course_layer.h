#pragma once

#include <pebble.h>

// Reusable custom Layer that draws one golf hole, from a choice of vantages.
//
// The scene does not change with time, so unlike time_date_layer there is no
// update call — a caller creates it, positions it, and forgets about it. Its
// perspective comes from shared/c/golf_perspective, which is where the geometry
// is tested; this file is only the rendering.
//
// Internal state lives in the layer's own data block via layer_get_data() —
// callers only ever touch the Layer*.

// Where the sky meets the land, in display rows. Fixed rather than a parameter:
// a caller that moved it would be drawing a different scene, and the two
// watchfaces sharing this component are meant to be on the same course.
#define GOLF_COURSE_HORIZON 74

typedef enum {
  // The hole from the tee, with the tee deck underfoot: markers set, a ball
  // waiting, the green and the pin away on the skyline.
  GOLF_VANTAGE_TEE,
  // The green from a few paces off: the putting surface filling the frame,
  // mown finer and lighter than the fairway, the cup and pin close enough to
  // read, a greenside bunker and a ball on the surface.
  GOLF_VANTAGE_GREEN,
} GolfVantage;

// `ground` is the first display row below the scene — where the watchface's
// foreground begins. Everything scales to it, so a caller that gives the scene
// more room sees further down the same hole rather than a different one.
Layer *golf_course_layer_create(GRect frame, GolfVantage vantage, int ground);
void golf_course_layer_destroy(Layer *layer);
