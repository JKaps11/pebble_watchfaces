#pragma once

#include <stdbool.h>

// The 2D transform and polygon maths behind a battle-car drawn at an arbitrary
// attitude.
//
// Kept free of the Pebble SDK, like everything in shared/c, so it can be tested
// on the host (ADR-0002/0003). The rendering that consumes it lives in
// shared/components/rocket_arena_layer.
//
// A car is authored once on a fixed grid — ROCKET_GRID units nose to tail — and
// every pose it is ever drawn at is that same grid put through the transform
// below. That indirection is the point of the module: the two watchfaces sharing
// this code draw the same car, one of them level and one of them climbing, and
// there is no second set of coordinates that could drift from the first.

// The grid a car is authored on, nose to tail.
#define ROCKET_GRID 100

// The scale the rotation numerators are expressed over. Chosen to match the
// Pebble SDK's TRIG_MAX_RATIO so a caller can hand `cos_lookup(angle)` and
// `sin_lookup(angle)` straight in, without this module knowing what an angle is.
#define ROCKET_TRIG_MAX 0xffff

typedef struct {
  int x;
  int y;
} RocketPoint;

// How a car's grid maps onto the display.
//
// `vscale` is a percentage applied to the vertical only, and it is applied
// before the rotation rather than after — foreshortening a car is a squash along
// its own axis, so a banked car has to be squashed in its own frame or the
// squash arrives at the wrong angle.
typedef struct {
  int origin_x;  // where the car's grid centre lands on the display
  int origin_y;
  int scale;     // display pixels across one whole ROCKET_GRID
  int vscale;    // vertical foreshortening, percent; 100 is none
  int cos;       // rotation, over ROCKET_TRIG_MAX
  int sin;
} RocketPose;

// A pose at `x`,`y`, `scale` px long, level and unforeshortened.
RocketPose rocket_pose_level(int x, int y, int scale);

// Where grid point (`u`, `v`) lands, given that the grid's own centre of
// rotation is (`cx`, `cy`).
RocketPoint rocket_project(const RocketPose *pose, int cx, int cy, int u, int v);

// The horizontal span a convex polygon covers on display row `row`, inclusive
// at both ends. False when the row falls outside the polygon entirely, in which
// case `left` and `right` are untouched.
//
// This is the whole of a scanline fill that does not need a graphics context,
// and it is here rather than in the component because getting it wrong shows up
// as a car with holes in it — which is exactly the kind of thing worth having a
// host test for.
bool rocket_polygon_span(const RocketPoint *points, int count, int row,
                         int *left, int *right);
