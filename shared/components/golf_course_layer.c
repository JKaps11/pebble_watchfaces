#include "golf_course_layer.h"
#include "golf_perspective.h"

// Turf and sky. Named for what they are on a golf hole rather than for their
// colour, because the palette was tuned by eye against the emulator and the
// names are the only record of what each one is doing.
#define ROUGH GColorDarkGreen
#define FAIRWAY_LIGHT GColorMayGreen
#define FAIRWAY_DARK GColorIslamicGreen
#define PUTTING_LIGHT GColorInchworm
#define PUTTING_DARK GColorMayGreen
#define SAND GColorPastelYellow
#define SAND_LIP GColorWindsorTan
#define TREE GColorMidnightGreen

#define SKY_BANDS 7
#define SKY_BAND_HEIGHT 11
#define STRIPE_FIRST_DEPTH 4
#define STRIPE_GROWTH 2

typedef struct {
  GolfVantage vantage;
  int ground;
} GolfCourseLayerData;

static int prv_isqrt(int value) {
  int root = 0;
  while ((root + 1) * (root + 1) <= value) {
    root++;
  }
  return root;
}

// Pebble's graphics API has no filled ellipse and a scene needs one constantly:
// a putting green, a bunker, the sun on a horizon, the crown of a tree. Drawn as
// scanlines, so it costs one fill per row and nothing else.
static void prv_fill_ellipse(GContext *ctx, GPoint centre, int radius_x,
                             int radius_y, GColor colour) {
  if (radius_x < 1 || radius_y < 1) {
    return;
  }
  graphics_context_set_fill_color(ctx, colour);
  for (int dy = -radius_y; dy <= radius_y; dy++) {
    int half = radius_x * prv_isqrt((radius_y * radius_y) - (dy * dy))
               / radius_y;
    graphics_fill_rect(ctx, GRect(centre.x - half, centre.y + dy, half * 2, 1),
                       0, GCornerNone);
  }
}

// The same shape mown in bands that deepen downward — the fairway's trick,
// confined to an ellipse, which is what a putting green actually looks like.
static void prv_fill_striped_ellipse(GContext *ctx, GPoint centre, int radius_x,
                                     int radius_y, GColor light, GColor dark) {
  int band = 0;
  int depth = 3;
  int rows = 0;

  if (radius_x < 1 || radius_y < 1) {
    return;
  }

  for (int dy = -radius_y; dy <= radius_y; dy++) {
    int half = radius_x * prv_isqrt((radius_y * radius_y) - (dy * dy))
               / radius_y;
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

// Filled by fanning strokes from the apex across the opposite edge. Only the
// pennant needs it, so it stays here rather than becoming a general primitive.
static void prv_fill_triangle(GContext *ctx, GPoint apex, GPoint from, GPoint to,
                              GColor colour) {
  const int steps = 24;
  graphics_context_set_stroke_color(ctx, colour);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i <= steps; i++) {
    graphics_draw_line(ctx, apex,
                       GPoint(from.x + ((to.x - from.x) * i / steps),
                              from.y + ((to.y - from.y) * i / steps)));
  }
}

static GColor prv_sky_band(int index) {
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

static void prv_draw_sky(GContext *ctx, int width) {
  for (int band = 0; band < SKY_BANDS; band++) {
    int top = band * SKY_BAND_HEIGHT;
    int height = (top + SKY_BAND_HEIGHT + 1 > GOLF_COURSE_HORIZON)
                     ? (GOLF_COURSE_HORIZON - top)
                     : SKY_BAND_HEIGHT + 1;
    graphics_context_set_fill_color(ctx, prv_sky_band(band));
    graphics_fill_rect(ctx, GRect(0, top, width, height), 0, GCornerNone);
  }

  prv_fill_ellipse(ctx, GPoint(width * 3 / 4, 72), 19, 19, GColorYellow);

  // The far treeline, so the horizon is an edge and not a colour change. The
  // varying height is deliberately arithmetic rather than random: the scene has
  // to render identically every time for the screenshot tests to mean anything.
  //
  // The index rather than the coordinate drives that arithmetic because the
  // first tree starts off the left edge at a negative x, and C's % keeps the
  // sign of its left operand — so the obvious version gives that one a radius
  // of zero and divides by it.
  for (int i = 0; (i * 17) - 6 < width + 10; i++) {
    prv_fill_ellipse(ctx, GPoint((i * 17) - 6, GOLF_COURSE_HORIZON + 2), 11,
                     9 + ((i * 7) % 11), TREE);
  }
}

// A bunker, flat to the ground with a lip on its near side. Placed in depth and
// offset sideways from the fairway edge, so it stays with the hole.
static void prv_draw_bunker(GContext *ctx, const GolfView *view, int depth,
                            int side, int offset, int radius) {
  int y = golf_row_at_depth(view, depth);
  int x = golf_centre_at_depth(depth)
          + (side * (golf_half_at_depth(depth) + offset));
  int flat = radius * 2 / 5;
  prv_fill_ellipse(ctx, GPoint(x, y + 1), radius, flat + 1, SAND_LIP);
  prv_fill_ellipse(ctx, GPoint(x, y), radius, flat, SAND);
}

// One colour at every distance. Grading them darker as they come forward is
// true of a photograph and reads on a 200-pixel display as a row of black holes
// punched in the turf.
static void prv_draw_tree(GContext *ctx, const GolfView *view, int depth,
                          int side) {
  int y = golf_row_at_depth(view, depth);
  int size = 7 + (depth * 20 / GOLF_DEPTH_MAX);
  int x = golf_centre_at_depth(depth)
          + (side * (golf_half_at_depth(depth) + size + 7));

  graphics_context_set_fill_color(ctx, SAND_LIP);
  graphics_fill_rect(ctx, GRect(x - 2, y - size, 4, size), 0, GCornerNone);
  prv_fill_ellipse(ctx, GPoint(x, y - size), size, size * 5 / 4, TREE);
}

// `flag` is not derived from `height`: from the green the pole is three times as
// tall but the pennant only half again as wide, and scaling the two together
// gives a banner rather than a flag.
static void prv_draw_pin(GContext *ctx, GPoint foot, int height, int flag) {
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, foot, GPoint(foot.x, foot.y - height));
  prv_fill_triangle(ctx, GPoint(foot.x + flag, foot.y - height + 6),
                    GPoint(foot.x + 1, foot.y - height),
                    GPoint(foot.x + 1, foot.y - height + 12), GColorWhite);
}

// The hole seen from up the fairway: the shared basis of the tee vantage.
static void prv_draw_hole(GContext *ctx, const GolfView *view, int width) {
  prv_draw_sky(ctx, width);

  graphics_context_set_fill_color(ctx, ROUGH);
  graphics_fill_rect(ctx, GRect(0, view->horizon, width,
                                view->ground - view->horizon),
                     0, GCornerNone);

  for (int row = view->horizon; row < view->ground; row++) {
    int band = golf_stripe_band(view, row, STRIPE_FIRST_DEPTH, STRIPE_GROWTH);
    int half = golf_half_at_row(view, row);
    graphics_context_set_fill_color(ctx,
                                    (band % 2) ? FAIRWAY_LIGHT : FAIRWAY_DARK);
    graphics_fill_rect(ctx,
                       GRect(golf_centre_at_row(view, row) - half, row,
                             half * 2, 1),
                       0, GCornerNone);
  }

  prv_draw_bunker(ctx, view, 26, 1, 6, 13);    // greenside, right
  prv_draw_bunker(ctx, view, 116, -1, 8, 21);  // fairway, left
  prv_draw_bunker(ctx, view, 186, 1, 10, 18);  // fairway, right

  prv_fill_ellipse(ctx, GPoint(GOLF_FAR_CENTRE, GOLF_COURSE_HORIZON + 6), 19, 7,
                   PUTTING_DARK);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx,
                     GRect(GOLF_FAR_CENTRE - 1, GOLF_COURSE_HORIZON + 3, 3, 2),
                     0, GCornerNone);

  // Trees before the pin, so the flag is never crossed by one.
  for (int depth = 46; depth < GOLF_DEPTH_MAX; depth += 62) {
    prv_draw_tree(ctx, view, depth, -1);
    prv_draw_tree(ctx, view, depth, 1);
  }

  prv_draw_pin(ctx, GPoint(GOLF_FAR_CENTRE, GOLF_COURSE_HORIZON + 3), 29, 24);
}

static void prv_draw_tee(GContext *ctx, const GolfView *view, int width) {
  const int deck = view->ground - 34;

  prv_draw_hole(ctx, view, width);

  // The tee deck: level, mown short, and lighter than anything beyond it,
  // because it is the one piece of ground the wearer is standing on. Its front
  // edge is where the watchface's foreground starts, which is what lets a
  // straight split read as the front of a tee rather than as a caption bar.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, deck - 3, width, 3), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, PUTTING_LIGHT);
  graphics_fill_rect(ctx, GRect(0, deck, width, view->ground - deck), 0,
                     GCornerNone);
  graphics_context_set_fill_color(ctx, PUTTING_DARK);
  graphics_fill_rect(ctx, GRect(0, deck, width, 5), 0, GCornerNone);

  for (int side = -1; side <= 1; side += 2) {
    int x = (width / 2) + (side * 68);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x - 8, deck + 14, 16, 4), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(x - 7, deck + 7, 14, 8), 2, GCornersAll);
  }

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect((width / 2) - 1, deck + 17, 2, 8), 0,
                     GCornerNone);
  prv_fill_ellipse(ctx, GPoint(width / 2, deck + 16), 5, 5, GColorWhite);
}

static void prv_draw_green(GContext *ctx, const GolfView *view, int width) {
  const int span = view->ground - view->horizon;
  const int cup_y = view->horizon + (span * 50 / 100);

  prv_draw_sky(ctx, width);

  graphics_context_set_fill_color(ctx, ROUGH);
  graphics_fill_rect(ctx, GRect(0, view->horizon, width, span), 0, GCornerNone);

  // The collar, then the surface: wide enough to run out of the frame on both
  // sides, because a green you can see the edges of is a lawn.
  prv_fill_ellipse(ctx, GPoint(width / 2, view->horizon + (span * 72 / 100)),
                   152, span * 54 / 100, FAIRWAY_DARK);
  prv_fill_striped_ellipse(ctx,
                           GPoint(width / 2, view->horizon + (span * 74 / 100)),
                           130, span * 46 / 100, PUTTING_LIGHT, PUTTING_DARK);

  // The greenside bunker, cut into the collar at the front left.
  prv_fill_ellipse(ctx, GPoint(16, view->horizon + (span * 52 / 100) + 2), 42,
                   (span * 13 / 100) + 2, SAND_LIP);
  prv_fill_ellipse(ctx, GPoint(16, view->horizon + (span * 52 / 100)), 42,
                   span * 13 / 100, SAND);

  // A ball a putt away, with the shadow that makes it sit on the grass rather
  // than float over it.
  prv_fill_ellipse(ctx, GPoint(64, view->horizon + (span * 68 / 100) + 4), 6, 2,
                   FAIRWAY_DARK);
  prv_fill_ellipse(ctx, GPoint(64, view->horizon + (span * 68 / 100)), 5, 5,
                   GColorWhite);

  prv_fill_ellipse(ctx, GPoint(122, cup_y + 2), 9, 4, FAIRWAY_DARK);
  prv_fill_ellipse(ctx, GPoint(122, cup_y), 8, 3, GColorBlack);
  prv_draw_pin(ctx, GPoint(122, cup_y), 92, 34);
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GolfCourseLayerData *data = (GolfCourseLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);
  GolfView view = {
    .horizon = GOLF_COURSE_HORIZON,
    .ground = data->ground,
  };

  if (data->vantage == GOLF_VANTAGE_GREEN) {
    prv_draw_green(ctx, &view, bounds.size.w);
  } else {
    prv_draw_tee(ctx, &view, bounds.size.w);
  }
}

Layer *golf_course_layer_create(GRect frame, GolfVantage vantage, int ground) {
  Layer *layer = layer_create_with_data(frame, sizeof(GolfCourseLayerData));
  GolfCourseLayerData *data = (GolfCourseLayerData *)layer_get_data(layer);
  data->vantage = vantage;
  data->ground = ground;
  layer_set_update_proc(layer, prv_update_proc);
  return layer;
}

void golf_course_layer_destroy(Layer *layer) {
  layer_destroy(layer);
}
