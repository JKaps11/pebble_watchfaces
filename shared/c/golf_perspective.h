#pragma once

// The perspective math behind a golf hole drawn in one-point perspective.
//
// Kept free of the Pebble SDK, like everything in shared/c, so it can be tested
// on the host (ADR-0002/0003). The rendering that consumes it lives in
// shared/components/golf_course_layer.
//
// Everything works in *depth* rather than in display rows: 0 at the horizon,
// GOLF_DEPTH_MAX underfoot. That indirection is the whole point of the module —
// a watchface that gives the scene more vertical room sees further down the same
// hole rather than a differently-shaped one, so the two watchfaces sharing this
// code cannot drift apart geometrically.

#define GOLF_DEPTH_MAX 256

// The hole's centre line and half-width at the horizon and underfoot. A dogleg:
// the fairway drifts left as it comes toward the viewer as well as widening.
#define GOLF_FAR_CENTRE 122
#define GOLF_NEAR_CENTRE 86
#define GOLF_FAR_HALF 5
#define GOLF_NEAR_HALF 97

// How the scene is mapped onto the display: `horizon` is the row the sky meets
// the land, `ground` the first row below the scene. Both are supplied by the
// caller because the watchface, not this module, decides how much room the
// picture gets.
typedef struct {
  int horizon;
  int ground;
} GolfView;

// Depth of a display row, and the row a depth falls on. Inverses of each other
// up to integer truncation. A row above the horizon returns a negative depth
// rather than clamping, so a caller can tell "off the top of the scene" from
// "at the horizon".
int golf_depth_at_row(const GolfView *view, int row);
int golf_row_at_depth(const GolfView *view, int depth);

// The fairway's centre line and half-width at a depth.
int golf_centre_at_depth(int depth);
int golf_half_at_depth(int depth);

// The same two, addressed by display row, which is what a scanline renderer
// wants.
int golf_centre_at_row(const GolfView *view, int row);
int golf_half_at_row(const GolfView *view, int row);

// Mown stripes run in bands that get deeper as they approach the viewer, which
// is what sells the recession. Given a row, this reports which band it falls in;
// the renderer alternates two greens on the band's parity.
//
// `first_depth` is the height in rows of the band at the horizon, and `growth`
// how many rows each successive band gains.
int golf_stripe_band(const GolfView *view, int row, int first_depth, int growth);
