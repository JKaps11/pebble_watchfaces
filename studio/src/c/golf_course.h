#pragma once

#include <pebble.h>

// The golf scenes, drawn from three vantages: the tee, the fairway, and the
// green. Each one is a whole hole, not a backdrop — dawn sky, treeline, turf
// mown in stripes that deepen toward the viewer, bunkers, trees down both sides.
//
// This is here rather than copied into each Variant for one reason: a Batch is
// only readable if the tiles differ where they claim to differ. Six hand-copied
// scenes drift, and a drifting background quietly turns a comparison of
// composition into a comparison of nothing. The same argument as studio_draw.h,
// with more at stake.
//
// Studio-only and disposable, like everything under studio/src/c.

// Where the sky meets the land. Fixed across every vantage, so that changing the
// vantage changes what you are looking at and not where you are standing.
#define GOLF_HORIZON 74

// The hole from the fairway, looking up at the green. Draws into the rows above
// `ground`, which is where the composition's foreground begins. Everything
// scales to that: a Variant that gives the scene more room sees further down the
// hole, which is a consequence of where its time sits and not a second thing
// being varied.
void golf_course_draw(GContext *ctx, int ground);

// The same hole after sundown: the sky drained to greens and black, the sun
// gone, the turf dark and the trees reading as shapes rather than as colour.
// Same geometry as golf_course_draw, so the two can be compared directly.
void golf_dusk_draw(GContext *ctx, int ground);

// The hole from the tee, which is golf_course_draw with the tee deck underfoot:
// a level mown platform across the near foreground, its markers set, a ball
// waiting on it. The deck's front edge is the lowest row the scene occupies.
void golf_tee_draw(GContext *ctx, int ground);

// The green, from a few paces off it: the putting surface filling the frame,
// mown finer and lighter than the fairway, with the cup and the pin close enough
// to read, a greenside bunker, and a ball on the surface.
void golf_green_draw(GContext *ctx, int ground);

// The fairway's centre line and half-width at a row, so a Variant can put its
// foreground edge or its panel against the hole rather than across it.
int golf_fairway_centre(int y, int ground);
int golf_fairway_half(int y, int ground);
