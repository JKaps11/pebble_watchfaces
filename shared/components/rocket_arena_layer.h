#pragma once

#include <pebble.h>

// Reusable custom Layer that draws a stadium seen from the pitch, looking down
// it at the goal, with a battle-car in the foreground.
//
// Like golf_course_layer, the scene does not change with time, so there is no
// update call — a caller creates it, positions it, and forgets about it. Its
// transform comes from shared/c/rocket_geometry, which is where the maths is
// tested; this file is only the rendering.
//
// Internal state lives in the layer's own data block via layer_get_data() —
// callers only ever touch the Layer*.

// Where the floor meets the far wall, in display rows. Fixed rather than a
// parameter, for the same reason the golf horizon is: a caller that moved it
// would be drawing a different arena, and the watchfaces sharing this component
// are meant to be in the same stadium.
#define ROCKET_ARENA_HORIZON 152

typedef enum {
  // The car between the viewer and the goal, on the deck, driving away. The
  // goal is drawn small against a large car, which is the scene's only depth
  // cue and the reason it reads at all.
  ROCKET_VANTAGE_APPROACH,
  // The car off the deck with its nose to the sky, climbing at the goal, and
  // its shadow left behind on the floor.
  //
  // The car is drawn in profile here rather than from behind, and that is a
  // finding rather than an oversight: a car seen end-on looks identical whether
  // its nose points at the sky or at the goal, so an aerial has to be shown from
  // the side even when the rest of the scene faces down the pitch.
  ROCKET_VANTAGE_AERIAL,
} RocketVantage;

Layer *rocket_arena_layer_create(GRect frame, RocketVantage vantage);
void rocket_arena_layer_destroy(Layer *layer);
