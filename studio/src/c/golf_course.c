#include "golf_course.h"

#include "studio_draw.h"

#define ROUGH GColorDarkGreen
#define STRIPE_A GColorMayGreen
#define STRIPE_B GColorIslamicGreen
#define SAND GColorPastelYellow
#define LIP GColorWindsorTan
#define DISTANT GColorMidnightGreen

// The green is mown finer than the fairway, so it takes its own lighter pair.
#define PUTT_A GColorInchworm
#define PUTT_B GColorMayGreen

// The hole in normalised depth: 0 at the horizon, 256 underfoot. Working in
// depth rather than in rows is what lets the scene keep its shape whatever row
// the foreground starts at.
#define FAR_CENTRE 122
#define NEAR_CENTRE 86
#define FAR_HALF 5
#define NEAR_HALF 97

static int prv_depth(int y, int ground) {
  return (y - GOLF_HORIZON) * 256 / (ground - GOLF_HORIZON);
}

static int prv_row(int depth, int ground) {
  return GOLF_HORIZON + (depth * (ground - GOLF_HORIZON) / 256);
}

static int prv_centre(int depth) {
  return FAR_CENTRE - ((FAR_CENTRE - NEAR_CENTRE) * depth / 256);
}

static int prv_half(int depth) {
  return FAR_HALF + ((NEAR_HALF - FAR_HALF) * depth / 256);
}

int golf_fairway_centre(int y, int ground) {
  return prv_centre(prv_depth(y, ground));
}

int golf_fairway_half(int y, int ground) {
  return prv_half(prv_depth(y, ground));
}

// The palette is a light setting, not a set of six independent choices — every
// selector below answers the same question, so a Variant asks for dusk once and
// the whole hole agrees with itself.
static GColor prv_sky_band(int index, bool dusk) {
  if (dusk) {
    switch (index) {
      case 0: return GColorBlack;
      case 1: return GColorBlack;
      case 2: return GColorMidnightGreen;
      case 3: return GColorDarkGreen;
      case 4: return GColorIslamicGreen;
      case 5: return GColorMayGreen;
      default: return GColorInchworm;
    }
  }
  switch (index) {
    case 0: return GColorOxfordBlue;
    case 1: return GColorDukeBlue;
    case 2: return GColorLiberty;
    case 3: return GColorVividCerulean;
    case 4: return GColorPictonBlue;
    case 5: return GColorRajah;
    default: return GColorChromeYellow;
  }
}

static GColor prv_rough(bool dusk) { return dusk ? DISTANT : ROUGH; }
static GColor prv_sand(bool dusk) { return dusk ? GColorWindsorTan : SAND; }
static GColor prv_lip(bool dusk) { return dusk ? GColorBlack : LIP; }
static GColor prv_treeline(bool dusk) { return dusk ? GColorBlack : DISTANT; }

// Trees are one colour at every distance. They were graded darker as they came
// forward, which is true of a photograph and reads on a 200-pixel display as a
// row of black holes punched in the turf.
static GColor prv_tree_colour(bool dusk) { return dusk ? ROUGH : DISTANT; }

static GColor prv_stripe(int band, bool dusk) {
  if (dusk) {
    return (band % 2) ? STRIPE_B : GColorDarkGreen;
  }
  return (band % 2) ? STRIPE_A : STRIPE_B;
}

static int prv_isqrt(int value) {
  int root = 0;
  while ((root + 1) * (root + 1) <= value) {
    root++;
  }
  return root;
}

// An ellipse mown in bands that deepen downward — the same trick the fairway
// uses, confined to a shape, which is what a green actually looks like.
static void prv_striped_ellipse(GContext *ctx, GPoint centre, int radius_x,
                                int radius_y, GColor light, GColor dark) {
  int band = 0;
  int depth = 3;
  int rows = 0;

  for (int dy = -radius_y; dy <= radius_y; dy++) {
    int half = radius_x
               * prv_isqrt((radius_y * radius_y) - (dy * dy)) / radius_y;
    graphics_context_set_fill_color(ctx, (band % 2) ? light : dark);
    graphics_fill_rect(ctx, GRect(centre.x - half, centre.y + dy, half * 2, 1),
                       0, GCornerNone);
    if (++rows >= depth) {
      rows = 0;
      depth += 2;
      band++;
    }
  }
}

static void prv_sky(GContext *ctx, bool dusk) {
  for (int band = 0; band < 7; band++) {
    int top = band * 11;
    int height = (top + 12 > GOLF_HORIZON) ? (GOLF_HORIZON - top) : 12;
    graphics_context_set_fill_color(ctx, prv_sky_band(band, dusk));
    graphics_fill_rect(ctx, GRect(0, top, 200, height), 0, GCornerNone);
  }

  if (!dusk) {
    studio_fill_ellipse(ctx, GPoint(150, 72), 19, 19, GColorYellow);
  }

  // The far treeline, so the horizon is an edge and not a colour change.
  for (int x = -6; x < 210; x += 17) {
    studio_fill_ellipse(ctx, GPoint(x, GOLF_HORIZON + 2), 11,
                        9 + ((x * 7) % 11), prv_treeline(dusk));
  }
}

// A bunker, drawn flat to the ground with a lip on its near side. Placed in
// depth and offset sideways from the fairway edge, so it stays with the hole.
static void prv_bunker(GContext *ctx, int depth, int side, int offset,
                       int radius, int ground, bool dusk) {
  int y = prv_row(depth, ground);
  int x = prv_centre(depth) + (side * (prv_half(depth) + offset));
  int flat = radius * 2 / 5;
  studio_fill_ellipse(ctx, GPoint(x, y + 1), radius, flat + 1, prv_lip(dusk));
  studio_fill_ellipse(ctx, GPoint(x, y), radius, flat, prv_sand(dusk));
}

static void prv_tree(GContext *ctx, int depth, int side, int ground,
                     bool dusk) {
  int y = prv_row(depth, ground);
  int size = 7 + (depth * 20 / 256);
  int x = prv_centre(depth) + (side * (prv_half(depth) + size + 7));
  GColor colour = prv_tree_colour(dusk);

  graphics_context_set_fill_color(ctx, prv_lip(dusk));
  graphics_fill_rect(ctx, GRect(x - 2, y - size, 4, size), 0, GCornerNone);
  studio_fill_ellipse(ctx, GPoint(x, y - size), size, size * 5 / 4, colour);
}

// The pin, drawn last so nothing crosses it. `height` is set by the vantage and
// `flag` is not derived from it: from the green the pole is three times as tall
// but the pennant is only half again as wide, and scaling the two together gives
// a banner rather than a flag.
static void prv_pin(GContext *ctx, GPoint foot, int height, int flag) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, foot, GPoint(foot.x, foot.y - height));
  studio_fill_triangle(ctx, GPoint(foot.x + flag, foot.y - height + 6),
                       GPoint(foot.x + 1, foot.y - height),
                       GPoint(foot.x + 1, foot.y - height + 12), GColorWhite);
}

static void prv_course(GContext *ctx, int ground, bool dusk) {
  prv_sky(ctx, dusk);

  graphics_context_set_fill_color(ctx, prv_rough(dusk));
  graphics_fill_rect(ctx, GRect(0, GOLF_HORIZON, 200, ground - GOLF_HORIZON), 0,
                     GCornerNone);

  // Mown stripes, each band deeper than the last so the hole recedes.
  int y = GOLF_HORIZON;
  int depth = 4;
  for (int band = 0; y < ground; band++) {
    graphics_context_set_fill_color(ctx, prv_stripe(band, dusk));
    for (int row = y; row < y + depth && row < ground; row++) {
      int at = prv_depth(row, ground);
      int half = prv_half(at);
      graphics_fill_rect(ctx, GRect(prv_centre(at) - half, row, half * 2, 1), 0,
                         GCornerNone);
    }
    y += depth;
    depth += 2;
  }

  prv_bunker(ctx, 26, 1, 6, 13, ground, dusk);    // greenside, right
  prv_bunker(ctx, 116, -1, 8, 21, ground, dusk);  // fairway, left
  prv_bunker(ctx, 186, 1, 10, 18, ground, dusk);  // fairway, right

  // The green, and the pin standing against the sky.
  studio_fill_ellipse(ctx, GPoint(FAR_CENTRE, GOLF_HORIZON + 6), 19, 7,
                      dusk ? STRIPE_B : PUTT_B);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(FAR_CENTRE - 1, GOLF_HORIZON + 3, 3, 2), 0,
                     GCornerNone);

  // Trees down both sides, nearer ones larger. Drawn before the pin so the
  // flag stays clear of them.
  for (int at = 46; at < 256; at += 62) {
    prv_tree(ctx, at, -1, ground, dusk);
    prv_tree(ctx, at, 1, ground, dusk);
  }

  prv_pin(ctx, GPoint(FAR_CENTRE, GOLF_HORIZON + 3), 29, 24);
}

void golf_course_draw(GContext *ctx, int ground) {
  prv_course(ctx, ground, false);
}

void golf_dusk_draw(GContext *ctx, int ground) {
  prv_course(ctx, ground, true);
}

void golf_tee_draw(GContext *ctx, int ground) {
  const int deck = ground - 27;

  golf_course_draw(ctx, ground);

  // The tee deck: level, mown short, and lighter than anything beyond it,
  // because it is the one piece of ground the viewer is standing on.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, deck - 3, 200, 3), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, PUTT_A);
  graphics_fill_rect(ctx, GRect(0, deck, 200, ground - deck), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, PUTT_B);
  graphics_fill_rect(ctx, GRect(0, deck, 200, 4), 0, GCornerNone);

  // Two markers and a ball teed between them.
  for (int side = -1; side <= 1; side += 2) {
    int x = 100 + (side * 68);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x - 6, deck + 9, 12, 4), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(x - 5, deck + 4, 10, 6), 1, GCornersAll);
  }

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(99, deck + 12, 2, 7), 0, GCornerNone);
  studio_fill_ellipse(ctx, GPoint(100, deck + 11), 4, 4, GColorWhite);
}

void golf_green_draw(GContext *ctx, int ground) {
  const int span = ground - GOLF_HORIZON;
  const int cup_y = GOLF_HORIZON + (span * 50 / 100);

  prv_sky(ctx, false);

  graphics_context_set_fill_color(ctx, ROUGH);
  graphics_fill_rect(ctx, GRect(0, GOLF_HORIZON, 200, span), 0, GCornerNone);

  // The collar, then the surface itself: wide enough to run out of the frame on
  // both sides, because a green you can see the edges of is a lawn.
  studio_fill_ellipse(ctx, GPoint(100, GOLF_HORIZON + (span * 72 / 100)), 152,
                      span * 54 / 100, STRIPE_B);
  prv_striped_ellipse(ctx, GPoint(100, GOLF_HORIZON + (span * 74 / 100)), 130,
                      span * 46 / 100, PUTT_A, PUTT_B);

  // The greenside bunker, cut into the collar at the front left.
  studio_fill_ellipse(ctx, GPoint(16, GOLF_HORIZON + (span * 52 / 100) + 2), 42,
                      (span * 13 / 100) + 2, LIP);
  studio_fill_ellipse(ctx, GPoint(16, GOLF_HORIZON + (span * 52 / 100)), 42,
                      span * 13 / 100, SAND);

  // A ball on the surface, a putt away, with the shadow that makes it sit on
  // the grass rather than float over it.
  studio_fill_ellipse(ctx, GPoint(64, GOLF_HORIZON + (span * 68 / 100) + 4), 6,
                      2, STRIPE_B);
  studio_fill_ellipse(ctx, GPoint(64, GOLF_HORIZON + (span * 68 / 100)), 5, 5,
                      GColorWhite);

  // The cup, and a pin tall enough that the pennant clears the treeline at
  // either of the two grounds this scene is drawn at.
  studio_fill_ellipse(ctx, GPoint(122, cup_y + 2), 9, 4, STRIPE_B);
  studio_fill_ellipse(ctx, GPoint(122, cup_y), 8, 3, GColorBlack);
  prv_pin(ctx, GPoint(122, cup_y), 92, 34);
}
