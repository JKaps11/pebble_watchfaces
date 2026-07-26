#include "rocket_arena.h"

#include "studio_draw.h"

// The Octane is authored on a 100 wide by 48 tall grid with the tyres resting on
// the bottom edge, and every dimension below is in that grid. Working in a fixed
// grid rather than in pixels is what lets the same car be 60 px wide in a corner
// and 140 px wide as the subject without two sets of numbers.
#define OCT_W 100
#define OCT_H 48

// Three numbers do most of the work of making this an Octane rather than a car.
// The deck falls from tail to nose, so the body is a wedge and not a slab. The
// tyres are nearly half the height of the whole car, which is the proportion the
// silhouette is actually recognised by. And the cabin is short and set well
// back, leaving a long bonnet ahead of it.
#define OCT_TAIL 16   // top of the bodywork at the tail
#define OCT_HOOD 22   // and at the front of the bonnet
#define OCT_SILL 36   // bottom of the bodywork
#define OCT_ROOF 8    // top of the cabin
#define OCT_AXLE 36   // wheel centres
#define OCT_TYRE 12   // wheel radius

static int prv_ux(GRect f, int u) {
  return f.origin.x + (u * f.size.w / OCT_W);
}

static int prv_uy(GRect f, int v) {
  return f.origin.y + (v * f.size.h / OCT_H);
}

static void prv_ubox(GContext *ctx, GRect f, int u0, int v0, int u1, int v1,
                     GColor colour) {
  int x0 = prv_ux(f, u0);
  int y0 = prv_uy(f, v0);
  int w = prv_ux(f, u1) - x0;
  int h = prv_uy(f, v1) - y0;
  if (w < 1) {
    w = 1;
  }
  if (h < 1) {
    h = 1;
  }
  graphics_context_set_fill_color(ctx, colour);
  graphics_fill_rect(ctx, GRect(x0, y0, w, h), 0, GCornerNone);
}

// The top of the bodywork all the way along: a steady fall from tail to bonnet,
// then a sharp drop over the last tenth for the nose.
static int prv_deck_top(int u) {
  if (u <= 88) {
    return OCT_TAIL + ((u * (OCT_HOOD - OCT_TAIL)) / 88);
  }
  return OCT_HOOD + ((u - 88) * 9 / 12);
}

// The cabin's upper edge: a steep rear pillar, a short roof, then a long raked
// windscreen that lands back on the deck. Returns -1 where there is no cabin,
// which is what makes the column loop below a single pass rather than three.
static int prv_cabin_top(int u) {
  if (u < 30 || u > 80) {
    return -1;
  }
  if (u <= 40) {
    return OCT_TAIL - ((u - 30) * (OCT_TAIL - OCT_ROOF) / 10);
  }
  if (u <= 62) {
    return OCT_ROOF;
  }
  return OCT_ROOF + ((u - 62) * 13 / 18);
}

static void prv_wheel(GContext *ctx, GRect f, int u) {
  int rx = OCT_TYRE * f.size.w / OCT_W;
  int ry = OCT_TYRE * f.size.h / OCT_H;
  GPoint centre = GPoint(prv_ux(f, u), prv_uy(f, OCT_AXLE));

  if (rx < 3) {
    rx = 3;
  }
  if (ry < 3) {
    ry = 3;
  }
  studio_fill_ellipse(ctx, centre, rx, ry, GColorBlack);
  studio_fill_ellipse(ctx, centre, rx * 6 / 10, ry * 6 / 10, GColorDarkGray);
  studio_fill_ellipse(ctx, centre, rx * 3 / 10, ry * 3 / 10, GColorLightGray);
}

void rocket_octane_draw(GContext *ctx, GRect frame, GColor body, bool boosting) {
  int x0 = prv_ux(frame, 0);
  int x1 = prv_ux(frame, OCT_W);
  int span = x1 - x0;

  if (span < OCT_W / 4) {
    span = OCT_W / 4;
  }

  // The flame first, so the body covers where it leaves the car and the join
  // never has to be drawn.
  if (boosting) {
    studio_fill_triangle(ctx, GPoint(prv_ux(frame, -34), prv_uy(frame, 28)),
                         GPoint(prv_ux(frame, 4), prv_uy(frame, 22)),
                         GPoint(prv_ux(frame, 4), prv_uy(frame, 34)),
                         ROCKET_ORANGE);
    studio_fill_triangle(ctx, GPoint(prv_ux(frame, -17), prv_uy(frame, 28)),
                         GPoint(prv_ux(frame, 4), prv_uy(frame, 25)),
                         GPoint(prv_ux(frame, 4), prv_uy(frame, 31)),
                         GColorIcterine);
  }

  // Body, cabin and glass in one pass down the car, because all three are
  // described by a height per column and none of them is a rectangle.
  for (int x = x0; x < x1; x++) {
    int u = (x - x0) * OCT_W / span;
    int deck = prv_deck_top(u);
    int cabin = prv_cabin_top(u);
    int top = (cabin >= 0 && cabin < deck) ? cabin : deck;
    int y_top = prv_uy(frame, top);

    graphics_context_set_fill_color(ctx, body);
    graphics_fill_rect(ctx, GRect(x, y_top, 1, prv_uy(frame, OCT_SILL) - y_top),
                       0, GCornerNone);

    // A one-pixel highlight along the top edge. Low-poly car art lives or dies
    // on this: without it the wedge is a flat colour and reads as a slab.
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(x, y_top, 1, 1), 0, GCornerNone);

    // The glazing, in black rather than a dark blue. It is drawn over a hole in
    // the car, so whatever colour it takes has to differ from every ground the
    // car is ever placed on — and the first attempt used the same dark blue as
    // the arena floor, which made the greenhouse vanish rather than glaze.
    if (cabin >= 0 && u >= 32 && u <= 78 && cabin + 2 < deck - 1) {
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(
          ctx, GRect(x, prv_uy(frame, cabin + 2), 1,
                     prv_uy(frame, deck - 1) - prv_uy(frame, cabin + 2)),
          0, GCornerNone);
    }
  }

  // The wing. Octane's sits low and close over the rear deck on two stubby
  // struts — mounted high on tall struts it becomes a different car entirely,
  // which is exactly what the first attempt at this drawing produced.
  prv_ubox(ctx, frame, 5, 12, 9, OCT_TAIL, GColorBlack);
  prv_ubox(ctx, frame, 20, 12, 24, OCT_TAIL, GColorBlack);
  prv_ubox(ctx, frame, 0, 8, 30, 12, GColorBlack);

  prv_ubox(ctx, frame, 8, 33, 92, OCT_SILL, GColorBlack);   // underbody
  prv_ubox(ctx, frame, 84, 32, 100, 37, GColorBlack);       // front splitter
  prv_ubox(ctx, frame, 0, OCT_TAIL, 4, 33, GColorBlack);    // blunt tail
  prv_ubox(ctx, frame, 90, 26, 97, 30, GColorIcterine);     // headlight

  prv_wheel(ctx, frame, 24);
  prv_wheel(ctx, frame, 76);
}

// ---------------------------------------------------------------------------
// The Fennec, which unlike the Octane above is drawn rotated.
//
// Rotation is why this car is built from polygons rather than from rectangles
// and column loops. Pebble can rotate a bitmap but not a drawing, and there is
// no offscreen context to draw one into, so the shape has to be authored as
// points and turned before it is filled. It is a cheaper thing to do than it
// sounds: the whole car is nine convex polygons and two circles.
//
// Authored on the same 100-long grid, tail at u=0, wheels resting at v=45, and
// turned about the grid's middle.
#define FEN_LEN 100
#define FEN_CX 50
#define FEN_CY 24

// A box: flat roof, upright screen, vertical hatch, short overhangs. Every one
// of those survives being forty pixels long, which is the entire reason this
// Session swapped cars.
//
// Two proportions do the work, and both were got wrong before they were got
// right. The greenhouse against the tub: with the belt line halfway up, the
// glazing and the sill left eight units of paint and the car came out black with
// a blue nose. And the overhangs: a Fennec is cab-forward, so the roof is half
// the length of the whole car and there is almost nothing beyond the wheels —
// with long overhangs the same outline reads as a wedge, not a hatchback.
static const int8_t FEN_BODY[] = {
  4, 20, 14, 8, 64, 8, 78, 20, 92, 20, 98, 25, 98, 34, 4, 34,
};
static const int8_t FEN_GLASS[] = { 17, 10, 62, 10, 73, 19, 17, 19 };
static const int8_t FEN_PILLAR[] = { 38, 10, 44, 10, 44, 19, 38, 19 };
static const int8_t FEN_ROOF[] = { 14, 8, 64, 8, 64, 10, 14, 10 };
static const int8_t FEN_WING[] = { 12, 4, 32, 4, 32, 9, 12, 9 };
static const int8_t FEN_SILL[] = { 10, 32, 90, 32, 90, 34, 10, 34 };
static const int8_t FEN_SPLITTER[] = { 82, 32, 99, 32, 99, 36, 82, 36 };
static const int8_t FEN_LAMP[] = { 88, 22, 96, 22, 96, 26, 88, 26 };
static const int8_t FEN_FLAME[] = { 2, 22, 2, 32, -32, 27 };
static const int8_t FEN_CORE[] = { 2, 24, 2, 30, -15, 27 };

// A convex polygon by scanline. Filling by fanning strokes from a vertex — the
// trick studio_fill_triangle uses — opens visible gaps once a shape is as large
// as a car, so this walks rows and finds the span instead.
static void prv_fill_poly(GContext *ctx, const GPoint *pts, int n,
                          GColor colour) {
  int min_y = pts[0].y;
  int max_y = pts[0].y;

  for (int i = 1; i < n; i++) {
    if (pts[i].y < min_y) {
      min_y = pts[i].y;
    }
    if (pts[i].y > max_y) {
      max_y = pts[i].y;
    }
  }

  graphics_context_set_fill_color(ctx, colour);
  for (int y = min_y; y <= max_y; y++) {
    int left = 32767;
    int right = -32767;

    for (int i = 0; i < n; i++) {
      GPoint a = pts[i];
      GPoint b = pts[(i + 1) % n];
      int x;

      if (a.y == b.y) {
        continue;
      }
      if (y < ((a.y < b.y) ? a.y : b.y) || y > ((a.y < b.y) ? b.y : a.y)) {
        continue;
      }
      x = a.x + (((b.x - a.x) * (y - a.y)) / (b.y - a.y));
      if (x < left) {
        left = x;
      }
      if (x > right) {
        right = x;
      }
    }

    if (right >= left) {
      graphics_fill_rect(ctx, GRect(left, y, right - left + 1, 1), 0,
                         GCornerNone);
    }
  }
}

// Grid to display: shift to the grid's own origin, scale, turn. Both cars go
// through here — the profile one about (FEN_CX, FEN_CY), the rear view about its
// own middle — so there is one piece of trigonometry in this file and not two.
// `vscale` is a percentage applied to the vertical only, before the turn. It is
// how pitch is done: foreshortening a rear view is a squash, not a rotation, and
// squashing before rotating keeps a banked car's squash along its own axis
// rather than the display's.
static GPoint prv_grid_point_v(GPoint centre, int scale, int vscale, int32_t cs,
                               int32_t sn, int u, int v, int cx, int cy) {
  int du = (u - cx) * scale / FEN_LEN;
  int dv = (v - cy) * scale * vscale / (FEN_LEN * 100);
  return GPoint(centre.x + (int16_t)(((du * cs) - (dv * sn)) / TRIG_MAX_RATIO),
                centre.y + (int16_t)(((du * sn) + (dv * cs)) / TRIG_MAX_RATIO));
}

static GPoint prv_grid_point(GPoint centre, int scale, int32_t cs, int32_t sn,
                             int u, int v, int cx, int cy) {
  return prv_grid_point_v(centre, scale, 100, cs, sn, u, v, cx, cy);
}

static void prv_grid_poly_v(GContext *ctx, GPoint centre, int scale, int vscale,
                            int32_t cs, int32_t sn, const int8_t *grid, int n,
                            int cx, int cy, GColor colour) {
  GPoint pts[8];

  for (int i = 0; i < n; i++) {
    pts[i] = prv_grid_point_v(centre, scale, vscale, cs, sn, grid[i * 2],
                              grid[(i * 2) + 1], cx, cy);
  }
  prv_fill_poly(ctx, pts, n, colour);
}

static void prv_grid_poly(GContext *ctx, GPoint centre, int scale, int32_t cs,
                          int32_t sn, const int8_t *grid, int n, int cx, int cy,
                          GColor colour) {
  prv_grid_poly_v(ctx, centre, scale, 100, cs, sn, grid, n, cx, cy, colour);
}

static GPoint prv_fen_point(GPoint centre, int length, int32_t cs, int32_t sn,
                            int u, int v) {
  return prv_grid_point(centre, length, cs, sn, u, v, FEN_CX, FEN_CY);
}

static void prv_fen_poly(GContext *ctx, GPoint centre, int length, int32_t cs,
                         int32_t sn, const int8_t *grid, int n, GColor colour) {
  prv_grid_poly(ctx, centre, length, cs, sn, grid, n, FEN_CX, FEN_CY, colour);
}

static void prv_fen_wheel(GContext *ctx, GPoint centre, int length, int32_t cs,
                          int32_t sn, int u, bool detailed) {
  GPoint at = prv_fen_point(centre, length, cs, sn, u, 35);
  int r = 9 * length / FEN_LEN;

  if (r < 2) {
    r = 2;
  }
  studio_fill_ellipse(ctx, at, r, r, GColorBlack);
  studio_fill_ellipse(ctx, at, r * 4 / 10, r * 4 / 10,
                      detailed ? GColorLightGray : GColorDarkGray);
}

// One car, drawn twice over: `detailed` is the solid one at the head of a spin,
// and the plain form is the copies behind it.
//
// The copies are the same nine polygons rather than a flat silhouette of the
// body. A silhouette was the obvious economy and it does not work — without the
// glazing, the wheels and the wing, forty pixels of Fennec is a parallelogram,
// and a trail of parallelograms reads as debris rather than as a car that was
// just there.
static void prv_fennec(GContext *ctx, GPoint centre, int length, int32_t angle,
                       GColor body, bool detailed, bool boosting) {
  int32_t cs = cos_lookup(angle);
  int32_t sn = sin_lookup(angle);

  if (boosting) {
    prv_fen_poly(ctx, centre, length, cs, sn, FEN_FLAME, 3, ROCKET_ORANGE);
    prv_fen_poly(ctx, centre, length, cs, sn, FEN_CORE, 3, GColorIcterine);
  }

  prv_fen_poly(ctx, centre, length, cs, sn, FEN_BODY, 8, body);
  prv_fen_poly(ctx, centre, length, cs, sn, FEN_SILL, 4, GColorBlack);
  prv_fen_poly(ctx, centre, length, cs, sn, FEN_GLASS, 4, GColorBlack);
  // A B-pillar, without which the glazing is one long black slot and the car
  // reads as a limousine rather than a hatchback.
  prv_fen_poly(ctx, centre, length, cs, sn, FEN_PILLAR, 4, body);
  prv_fen_poly(ctx, centre, length, cs, sn, FEN_ROOF, 4,
               detailed ? GColorWhite : GColorLightGray);
  prv_fen_poly(ctx, centre, length, cs, sn, FEN_WING, 4, GColorBlack);
  prv_fen_poly(ctx, centre, length, cs, sn, FEN_SPLITTER, 4, GColorBlack);
  if (detailed) {
    prv_fen_poly(ctx, centre, length, cs, sn, FEN_LAMP, 4, GColorIcterine);
  }

  prv_fen_wheel(ctx, centre, length, cs, sn, 24, detailed);
  prv_fen_wheel(ctx, centre, length, cs, sn, 76, detailed);
}

void rocket_fennec_draw(GContext *ctx, GPoint centre, int length, int32_t angle,
                        GColor body, bool boosting) {
  prv_fennec(ctx, centre, length, angle, body, true, boosting);
}

void rocket_fennec_ghost(GContext *ctx, GPoint centre, int length, int32_t angle,
                         GColor colour) {
  prv_fennec(ctx, centre, length, angle, colour, false, false);
}

// ---------------------------------------------------------------------------
// The same car from behind, on its own 100-wide grid. Looking down the pitch
// rather than across it is a different drawing and not a rotation of this one:
// what the viewer gets is the hatch, the wing seen end-on, the tail lights and
// two boost plumes coming at the camera.
#define REAR_CX 50
#define REAR_CY 40

// Sizes here were all cut back once the car had been looked at large. Tail
// lights the width of a door, boost cores half the width of their own plume and
// a bumper running into both tyres turned the back of the car into three bands
// of colour — every one of those is a detail that has to stay a detail.
static const int8_t REAR_WING[] = { 12, 6, 88, 6, 88, 13, 12, 13 };
static const int8_t REAR_STRUT_A[] = { 24, 13, 32, 13, 32, 21, 24, 21 };
static const int8_t REAR_STRUT_B[] = { 68, 13, 76, 13, 76, 21, 68, 21 };
static const int8_t REAR_CABIN[] = { 26, 18, 74, 18, 82, 36, 18, 36 };
static const int8_t REAR_WINDOW[] = { 32, 22, 68, 22, 73, 32, 27, 32 };
static const int8_t REAR_TUB[] = { 14, 36, 86, 36, 90, 58, 10, 58 };
static const int8_t REAR_BUMPER[] = { 14, 52, 86, 52, 86, 59, 14, 59 };
static const int8_t REAR_LAMP_A[] = { 17, 39, 33, 39, 33, 44, 17, 44 };
static const int8_t REAR_LAMP_B[] = { 67, 39, 83, 39, 83, 44, 67, 44 };
static const int8_t REAR_TYRE_A[] = { 2, 45, 15, 45, 15, 66, 2, 66 };
static const int8_t REAR_TYRE_B[] = { 85, 45, 98, 45, 98, 66, 85, 66 };
static const int8_t REAR_HUB_A[] = { 4, 52, 13, 52, 13, 60, 4, 60 };
static const int8_t REAR_HUB_B[] = { 87, 52, 96, 52, 96, 60, 87, 60 };
// The plumes are not here: their reach depends on pitch, so they are built at
// draw time in rocket_fennec_rear.

// The floor of the car, seen only once the nose comes up. Its depth is the pitch
// itself, so this is one polygon built at draw time rather than a constant.
static void prv_rear_underside(GContext *ctx, GPoint centre, int width,
                               int vscale, int32_t cs, int32_t sn, int pitch,
                               GColor body, bool outline) {
  int depth = 60 + (pitch * 12 / 100);
  // A sliver, not a slab. With the nose to the sky and the viewer behind it, the
  // rear of the car is nearly face-on and its floor is mostly hidden behind
  // itself — only the front lip of it clears the bumper.
  int8_t floor_pan[] = { 14, 58, 86, 58, 76, (int8_t)depth, 24, (int8_t)depth };

  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, floor_pan, 4, REAR_CX,
                  REAR_CY, outline ? GColorBlack : GColorDarkGray);
}

void rocket_fennec_rear(GContext *ctx, GPoint centre, int width, int32_t roll,
                        int pitch, GColor body, GColor plume, bool boosting,
                        bool outline) {
  int32_t cs = cos_lookup(roll);
  int32_t sn = sin_lookup(roll);
  // Barely any squash, and this took two goes to get right. Foreshortening the
  // rear view hard was the obvious reading of "pitched up" and it is the wrong
  // one — a flattened rear is what a car skimming the floor looks like, not one
  // pointing at the sky. Nose-up seen from behind is close to face-on; what
  // actually carries the pitch is the roll and the length of the boost.
  int vscale = 100 - (pitch * 10 / 100);
  GColor dark = GColorBlack;
  GColor skin = outline ? GColorBlack : body;

  // In outline form the whole car is drawn once oversize in the body colour and
  // then again in black on top, which leaves a keyline and costs one extra pass
  // rather than a second set of polygons.
  if (outline) {
    prv_grid_poly_v(ctx, centre, width + 6, vscale, cs, sn, REAR_WING, 4, REAR_CX,
                  REAR_CY, body);
    prv_grid_poly_v(ctx, centre, width + 6, vscale, cs, sn, REAR_CABIN, 4, REAR_CX,
                  REAR_CY, body);
    prv_grid_poly_v(ctx, centre, width + 6, vscale, cs, sn, REAR_TUB, 4, REAR_CX, REAR_CY,
                  body);
    prv_grid_poly_v(ctx, centre, width + 6, vscale, cs, sn, REAR_TYRE_A, 4, REAR_CX,
                  REAR_CY, body);
    prv_grid_poly_v(ctx, centre, width + 6, vscale, cs, sn, REAR_TYRE_B, 4, REAR_CX,
                  REAR_CY, body);
  }

  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_TYRE_A, 4, REAR_CX, REAR_CY,
                dark);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_TYRE_B, 4, REAR_CX, REAR_CY,
                dark);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_STRUT_A, 4, REAR_CX, REAR_CY,
                dark);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_STRUT_B, 4, REAR_CX, REAR_CY,
                dark);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_CABIN, 4, REAR_CX, REAR_CY,
                skin);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_WINDOW, 4, REAR_CX, REAR_CY,
                dark);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_TUB, 4, REAR_CX, REAR_CY, skin);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_BUMPER, 4, REAR_CX, REAR_CY,
                dark);
  // Hubs, so the tyres do not merge with the bumper into one black band.
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_HUB_A, 4, REAR_CX, REAR_CY,
                outline ? GColorBlack : GColorDarkGray);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_HUB_B, 4, REAR_CX, REAR_CY,
                outline ? GColorBlack : GColorDarkGray);
  prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_WING, 4, REAR_CX, REAR_CY,
                dark);

  if (!outline) {
    prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_LAMP_A, 4, REAR_CX, REAR_CY,
                  plume);
    prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, REAR_LAMP_B, 4, REAR_CX, REAR_CY,
                  plume);
  }

  if (pitch > 0) {
    prv_rear_underside(ctx, centre, width, vscale, cs, sn, pitch, body, outline);
  }

  // The plumes come toward the viewer, so they go on last and over everything.
  // They also grow with pitch: a car holding a climb is holding boost, and from
  // behind that plume is the longest thing in the frame.
  if (boosting) {
    int reach = 78 + (pitch * 42 / 100);
    int core = 66 + (pitch * 30 / 100);
    int8_t flame_a[] = { 34, 54, 46, 54, 40, (int8_t)reach };
    int8_t flame_b[] = { 54, 54, 66, 54, 60, (int8_t)reach };
    int8_t core_a[] = { 38, 54, 42, 54, 40, (int8_t)core };
    int8_t core_b[] = { 58, 54, 62, 54, 60, (int8_t)core };

    prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, flame_a, 3, REAR_CX,
                    REAR_CY, plume);
    prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, flame_b, 3, REAR_CX,
                    REAR_CY, plume);
    prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, core_a, 3, REAR_CX,
                    REAR_CY, GColorWhite);
    prv_grid_poly_v(ctx, centre, width, vscale, cs, sn, core_b, 3, REAR_CX,
                    REAR_CY, GColorWhite);
  }
}

void rocket_octane_top(GContext *ctx, GPoint centre, int length, GColor body,
                       bool facing_down) {
  int half_l = length / 2;
  int half_w = (length * 3 / 5) / 2;
  int width = half_w * 2;
  // Everything that tells the car which way it points is measured from the tail,
  // so the whole shape flips on this one sign.
  int tail = facing_down ? (centre.y - half_l) : (centre.y + half_l);
  int nose_ward = facing_down ? 1 : -1;

  // Wheels first and outboard, so the body edge cuts them the way an arch does.
  graphics_context_set_fill_color(ctx, GColorBlack);
  for (int side = -1; side <= 1; side += 2) {
    int x = centre.x + (side * (half_w + 1)) - 1;
    graphics_fill_rect(ctx, GRect(x, centre.y - half_l + 2, 3, length / 4), 0,
                       GCornerNone);
    graphics_fill_rect(ctx, GRect(x, centre.y + half_l - 2 - (length / 4), 3,
                                  length / 4), 0, GCornerNone);
  }

  graphics_context_set_fill_color(ctx, body);
  graphics_fill_rect(ctx, GRect(centre.x - half_w, centre.y - half_l, width,
                                length), 3, GCornersAll);

  graphics_context_set_fill_color(ctx, GColorOxfordBlue);
  graphics_fill_rect(ctx, GRect(centre.x - half_w + 2,
                                centre.y - (length / 6) + (nose_ward * 2),
                                width - 4, length / 3), 0, GCornerNone);

  // The spoiler overhangs the tail, which is the only thing distinguishing the
  // two ends of a box at this size.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(centre.x - half_w - 2,
                                facing_down ? tail : (tail - 2), width + 4, 2),
                     0, GCornerNone);
}

void rocket_ball_draw(GContext *ctx, GPoint centre, int radius) {
  studio_fill_ellipse(ctx, centre, radius, radius, GColorBlack);
  studio_fill_ellipse(ctx, centre, radius - 1, radius - 1, GColorWhite);

  if (radius < 7) {
    return;
  }

  int patch = radius / 4;
  if (patch < 2) {
    patch = 2;
  }
  studio_fill_ellipse(ctx, centre, radius / 3, radius / 3, GColorBlack);
  for (int i = 0; i < 5; i++) {
    GPoint at = studio_point_at(centre, TRIG_MAX_ANGLE * i / 5,
                                radius * 2 / 3);
    studio_fill_ellipse(ctx, at, patch, patch, GColorBlack);
  }
}

void rocket_boost_pad(GContext *ctx, GPoint centre, int radius) {
  studio_fill_triangle(ctx, GPoint(centre.x, centre.y - radius),
                       GPoint(centre.x - radius, centre.y),
                       GPoint(centre.x + radius, centre.y), ROCKET_BOOST);
  studio_fill_triangle(ctx, GPoint(centre.x, centre.y + radius),
                       GPoint(centre.x - radius, centre.y),
                       GPoint(centre.x + radius, centre.y), ROCKET_BOOST);
}

void rocket_goal_draw(GContext *ctx, GRect frame, GColor team) {
  int right = frame.origin.x + frame.size.w;
  int foot = frame.origin.y + frame.size.h;
  const int bar = 5;
  const int shoulder = 14;

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, frame, 0, GCornerNone);

  // The net, drawn as a mesh rather than a texture: at this size a crosshatch on
  // black is the only thing that reads as depth.
  graphics_context_set_stroke_color(ctx, ROCKET_NET);
  graphics_context_set_stroke_width(ctx, 1);
  for (int x = frame.origin.x + 6; x < right - 4; x += 8) {
    graphics_draw_line(ctx, GPoint(x, frame.origin.y + 4), GPoint(x, foot - 2));
  }
  for (int y = frame.origin.y + 6; y < foot - 2; y += 8) {
    graphics_draw_line(ctx, GPoint(frame.origin.x + 4, y), GPoint(right - 4, y));
  }

  // The mouth: three bars and two chamfered shoulders, which is the shape that
  // separates a Rocket League goal from a football one.
  graphics_context_set_fill_color(ctx, team);
  graphics_fill_rect(ctx, GRect(frame.origin.x, frame.origin.y, bar,
                                frame.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(right - bar, frame.origin.y, bar, frame.size.h),
                     0, GCornerNone);
  graphics_fill_rect(ctx, GRect(frame.origin.x, frame.origin.y, frame.size.w,
                                bar), 0, GCornerNone);

  studio_fill_triangle(ctx, GPoint(frame.origin.x, frame.origin.y + shoulder),
                       GPoint(frame.origin.x, frame.origin.y),
                       GPoint(frame.origin.x + shoulder, frame.origin.y), team);
  studio_fill_triangle(ctx, GPoint(right - 1, frame.origin.y + shoulder),
                       GPoint(right - 1, frame.origin.y),
                       GPoint(right - 1 - shoulder, frame.origin.y), team);

  // A white keyline inside the team colour, because the goal in the game is lit
  // and a flat bar of orange is not.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(frame.origin.x + bar, frame.origin.y + bar,
                                frame.size.w - (bar * 2),
                                frame.size.h - bar + 1));
}

void rocket_pitch_draw(GContext *ctx, GRect frame) {
  int right = frame.origin.x + frame.size.w;
  int foot = frame.origin.y + frame.size.h;
  int mid_y = frame.origin.y + (frame.size.h / 2);
  int mid_x = frame.origin.x + (frame.size.w / 2);
  const int mouth = 34;

  graphics_context_set_fill_color(ctx, ROCKET_TURF);
  graphics_fill_rect(ctx, frame, 16, GCornersAll);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_round_rect(ctx, GRect(frame.origin.x + 4, frame.origin.y + 4,
                                      frame.size.w - 8, frame.size.h - 8), 13);
  graphics_draw_line(ctx, GPoint(frame.origin.x + 4, mid_y),
                     GPoint(right - 4, mid_y));
  graphics_draw_circle(ctx, GPoint(mid_x, mid_y), 24);

  // Both goal mouths, cut into the boundary rather than sitting inside it.
  graphics_context_set_fill_color(ctx, ROCKET_BLUE);
  graphics_fill_rect(ctx, GRect(mid_x - mouth, frame.origin.y + 2, mouth * 2,
                                10), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, ROCKET_ORANGE);
  graphics_fill_rect(ctx, GRect(mid_x - mouth, foot - 12, mouth * 2, 10), 0,
                     GCornerNone);

  // The four corner pads and the two on the side walls — the arrangement a
  // player reads the pitch by.
  rocket_boost_pad(ctx, GPoint(frame.origin.x + 24, frame.origin.y + 36), 7);
  rocket_boost_pad(ctx, GPoint(right - 24, frame.origin.y + 36), 7);
  rocket_boost_pad(ctx, GPoint(frame.origin.x + 24, foot - 36), 7);
  rocket_boost_pad(ctx, GPoint(right - 24, foot - 36), 7);
  rocket_boost_pad(ctx, GPoint(frame.origin.x + 14, mid_y), 6);
  rocket_boost_pad(ctx, GPoint(right - 14, mid_y), 6);
}

void rocket_floor_draw(GContext *ctx, int horizon, int bottom) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, 200, horizon), 0, GCornerNone);

  // The lit wall. Alternating team segments, which is what the far side of a
  // stadium actually looks like from the floor.
  for (int x = 0; x < 200; x += 25) {
    graphics_context_set_fill_color(ctx, ((x / 25) % 2) ? ROCKET_BLUE_DEEP
                                                        : ROCKET_BLUE);
    graphics_fill_rect(ctx, GRect(x, horizon - 7, 24, 6), 0, GCornerNone);
  }

  graphics_context_set_fill_color(ctx, ROCKET_FLOOR);
  graphics_fill_rect(ctx, GRect(0, horizon, 200, bottom - horizon), 0,
                     GCornerNone);

  // Floor lines converging on the centre of the far wall. Drawn well past both
  // edges so the fan does not visibly stop.
  graphics_context_set_stroke_color(ctx, GColorLiberty);
  graphics_context_set_stroke_width(ctx, 2);
  for (int x = -320; x <= 520; x += 84) {
    graphics_draw_line(ctx, GPoint(100, horizon), GPoint(x, bottom));
  }

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, horizon + 1), GPoint(200, horizon + 1));
}

void rocket_downfield_draw(GContext *ctx, int horizon, GColor floor, GColor line,
                           GColor wall) {
  const int far = horizon - 46;
  const int flank = 36;
  GPoint left[] = { { 0, horizon - 100 }, { flank, far }, { flank, horizon },
                    { 0, horizon } };
  GPoint right[] = { { 200, horizon - 100 }, { 200 - flank, far },
                     { 200 - flank, horizon }, { 200, horizon } };

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, 200, far), 0, GCornerNone);

  // The far wall first, then the two side walls over it. Drawn in that order
  // because the side walls have to occlude the far one to read as nearer.
  graphics_context_set_fill_color(ctx, wall);
  graphics_fill_rect(ctx, GRect(0, far, 200, horizon - far), 0, GCornerNone);
  prv_fill_poly(ctx, left, 4, wall);
  prv_fill_poly(ctx, right, 4, wall);

  // A lit edge along the top of each wall. This is what gives the scene its
  // depth: two lines running away to the same point say more about where the
  // viewer is standing than any amount of shading.
  graphics_context_set_stroke_color(ctx, ROCKET_BLUE);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, GPoint(0, horizon - 100), GPoint(flank, far));
  graphics_draw_line(ctx, GPoint(200, horizon - 100), GPoint(200 - flank, far));
  graphics_draw_line(ctx, GPoint(flank, far), GPoint(200 - flank, far));

  graphics_context_set_fill_color(ctx, floor);
  graphics_fill_rect(ctx, GRect(0, horizon, 200, 228 - horizon), 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, line);
  graphics_context_set_stroke_width(ctx, 2);
  for (int x = -360; x <= 560; x += 66) {
    graphics_draw_line(ctx, GPoint(100, horizon), GPoint(x, 228));
  }

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, horizon), GPoint(200, horizon));
}

void rocket_stands_draw(GContext *ctx, int horizon) {
  const int far = horizon - 46;

  // Three terraces stepping back, each a row of blocks with gaps. Individual
  // spectators are not worth drawing at this size; the texture of a crowd is,
  // and a row of two-pixel blocks is exactly that texture.
  for (int tier = 0; tier < 3; tier++) {
    int y = far - 10 - (tier * 11);
    int inset = 10 + (tier * 12);

    if (y < 2) {
      break;
    }
    graphics_context_set_fill_color(ctx, GColorOxfordBlue);
    graphics_fill_rect(ctx, GRect(inset, y, 200 - (inset * 2), 9), 0,
                       GCornerNone);
    for (int x = inset + 2; x < 200 - inset - 2; x += 5) {
      graphics_context_set_fill_color(ctx, ((x + tier) % 3) ? GColorLiberty
                                                            : GColorBlueMoon);
      graphics_fill_rect(ctx, GRect(x, y + 2, 3, 5), 0, GCornerNone);
    }
  }

  // Two floodlight rigs on the skyline, which is what says stadium rather than
  // sports hall.
  for (int side = 0; side < 2; side++) {
    int x = side ? 160 : 34;
    int y = far - 46;

    if (y < 4) {
      continue;
    }
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, GRect(x + 2, y, 2, 14), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorIcterine);
    graphics_fill_rect(ctx, GRect(x - 5, y - 6, 16, 7), 0, GCornerNone);
  }
}

void rocket_ceiling_draw(GContext *ctx, int soffit) {
  graphics_context_set_fill_color(ctx, ROCKET_FLOOR);
  graphics_fill_rect(ctx, GRect(0, 0, 200, soffit), 0, GCornerNone);

  // Panel joints running away from the viewer, so the ceiling has a direction.
  graphics_context_set_stroke_color(ctx, GColorLiberty);
  graphics_context_set_stroke_width(ctx, 2);
  for (int x = -320; x <= 520; x += 84) {
    graphics_draw_line(ctx, GPoint(100, soffit), GPoint(x, 0));
  }

  // Lit in the same blue as the floor's far wall rather than in the other
  // team's orange, which was the first idea and came back muddy — Emery has no
  // dark orange that survives being a thin strip.
  for (int x = 0; x < 200; x += 25) {
    graphics_context_set_fill_color(ctx, ((x / 25) % 2) ? ROCKET_BLUE_DEEP
                                                        : ROCKET_BLUE);
    graphics_fill_rect(ctx, GRect(x, soffit - 7, 24, 6), 0, GCornerNone);
  }

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, soffit), GPoint(200, soffit));
}
