#include "rocket_arena_layer.h"

#include "rocket_geometry.h"

// The team colours, and the surfaces they sit on.
//
// Two of these are not the colour the game uses, and both times the reason is
// the display. Boost is yellow rather than the game's chrome yellow, and the
// arena lighting is blue at both ends rather than blue at one and orange at the
// other: Emery washes the warm end of the palette out badly enough that an
// orange thinner than a goalpost comes back as a muddy brown.
#define TEAM_BLUE GColorVividCerulean
#define TEAM_BLUE_DEEP GColorDukeBlue
#define TEAM_ORANGE GColorOrange
#define BOOST GColorYellow
#define FLOOR GColorOxfordBlue
#define FLOOR_LINE GColorLiberty
#define NET GColorDarkGray

#define GOAL_FRAME GRect(58, 104, 84, 48)
#define BALL_CENTRE GPoint(100, 128)
#define BALL_RADIUS 11

// Both cars are authored on the same 100-unit grid rocket_geometry works in.
// The profile car is 48 units tall with its tyres on the bottom edge; the rear
// view is 78, which is why each carries its own centre of rotation.
#define PROFILE_CX 50
#define PROFILE_CY 24
#define REAR_CX 50
#define REAR_CY 40

typedef struct {
  RocketVantage vantage;
} RocketArenaLayerData;

// --- the profile car -------------------------------------------------------
//
// A Fennec: flat roof, upright screen, vertical hatch, short overhangs. Every
// one of those survives being forty pixels long, which a wedge does not.
static const int8_t PROFILE_BODY[] = {
  4, 20, 14, 8, 64, 8, 78, 20, 92, 20, 98, 25, 98, 34, 4, 34,
};
static const int8_t PROFILE_GLASS[] = { 17, 10, 62, 10, 73, 19, 17, 19 };
static const int8_t PROFILE_PILLAR[] = { 38, 10, 44, 10, 44, 19, 38, 19 };
static const int8_t PROFILE_ROOF[] = { 14, 8, 64, 8, 64, 10, 14, 10 };
static const int8_t PROFILE_WING[] = { 12, 4, 32, 4, 32, 9, 12, 9 };
static const int8_t PROFILE_SILL[] = { 10, 32, 90, 32, 90, 34, 10, 34 };
static const int8_t PROFILE_SPLITTER[] = { 82, 32, 99, 32, 99, 36, 82, 36 };
static const int8_t PROFILE_LAMP[] = { 88, 22, 96, 22, 96, 26, 88, 26 };
static const int8_t PROFILE_FLAME[] = { 2, 22, 2, 32, -32, 27 };
static const int8_t PROFILE_CORE[] = { 2, 24, 2, 30, -15, 27 };

// --- the same car from behind ----------------------------------------------
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
static const int8_t REAR_FLAME_A[] = { 34, 54, 46, 54, 40, 82 };
static const int8_t REAR_FLAME_B[] = { 54, 54, 66, 54, 60, 82 };
static const int8_t REAR_CORE_A[] = { 38, 54, 42, 54, 40, 68 };
static const int8_t REAR_CORE_B[] = { 58, 54, 62, 54, 60, 68 };

static void prv_fill_ellipse(GContext *ctx, GPoint centre, int radius_x,
                             int radius_y, GColor colour) {
  graphics_context_set_fill_color(ctx, colour);
  for (int dy = -radius_y; dy <= radius_y; dy++) {
    int root = 0;
    int square = (radius_y * radius_y) - (dy * dy);
    int half;

    while ((root + 1) * (root + 1) <= square) {
      root++;
    }
    half = radius_x * root / radius_y;
    graphics_fill_rect(ctx, GRect(centre.x - half, centre.y + dy, half * 2, 1),
                       0, GCornerNone);
  }
}

// A convex polygon by scanline. Filling by fanning strokes from one vertex —
// the trick that suffices for a pennant — opens visible gaps once a shape is as
// large as a car, so this walks rows and asks rocket_geometry for the span.
static void prv_fill_polygon(GContext *ctx, const RocketPoint *points, int count,
                             GColor colour) {
  int top = points[0].y;
  int bottom = points[0].y;

  for (int i = 1; i < count; i++) {
    if (points[i].y < top) {
      top = points[i].y;
    }
    if (points[i].y > bottom) {
      bottom = points[i].y;
    }
  }

  graphics_context_set_fill_color(ctx, colour);
  for (int row = top; row <= bottom; row++) {
    int left;
    int right;

    if (rocket_polygon_span(points, count, row, &left, &right)) {
      graphics_fill_rect(ctx, GRect(left, row, right - left + 1, 1), 0,
                         GCornerNone);
    }
  }
}

static void prv_fill_grid_shape(GContext *ctx, const RocketPose *pose, int cx,
                                int cy, const int8_t *grid, int count,
                                GColor colour) {
  RocketPoint points[8];

  for (int i = 0; i < count; i++) {
    points[i] = rocket_project(pose, cx, cy, grid[i * 2], grid[(i * 2) + 1]);
  }
  prv_fill_polygon(ctx, points, count, colour);
}

static void prv_profile_wheel(GContext *ctx, const RocketPose *pose, int u) {
  RocketPoint at = rocket_project(pose, PROFILE_CX, PROFILE_CY, u, 35);
  int radius = 9 * pose->scale / ROCKET_GRID;

  if (radius < 2) {
    radius = 2;
  }
  prv_fill_ellipse(ctx, GPoint(at.x, at.y), radius, radius, GColorBlack);
  prv_fill_ellipse(ctx, GPoint(at.x, at.y), radius * 4 / 10, radius * 4 / 10,
                   GColorLightGray);
}

// The car in profile, `length` px nose to tail, turned `angle` from level.
// Negative turns the nose up.
static void prv_draw_profile(GContext *ctx, GPoint centre, int length,
                             int32_t angle) {
  RocketPose pose = rocket_pose_level(centre.x, centre.y, length);

  pose.cos = cos_lookup(angle);
  pose.sin = sin_lookup(angle);

  // The flame first, so the body covers where it leaves the car and the join
  // never has to be drawn.
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_FLAME, 3,
                      TEAM_ORANGE);
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_CORE, 3,
                      GColorIcterine);

  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_BODY, 8,
                      TEAM_BLUE);
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_SILL, 4,
                      GColorBlack);
  // The glazing is black rather than a dark blue. It is a hole in the car, so
  // whatever colour it takes has to differ from every ground the car is placed
  // on — in the same dark blue as the arena floor the greenhouse vanishes.
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_GLASS, 4,
                      GColorBlack);
  // A B-pillar, without which the glazing is one long slot and the car reads as
  // a limousine rather than a hatchback.
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_PILLAR, 4,
                      TEAM_BLUE);
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_ROOF, 4,
                      GColorWhite);
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_WING, 4,
                      GColorBlack);
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_SPLITTER, 4,
                      GColorBlack);
  prv_fill_grid_shape(ctx, &pose, PROFILE_CX, PROFILE_CY, PROFILE_LAMP, 4,
                      GColorIcterine);

  prv_profile_wheel(ctx, &pose, 24);
  prv_profile_wheel(ctx, &pose, 76);
}

// The car from behind, `width` px across, banked by `roll`.
static void prv_draw_rear(GContext *ctx, GPoint centre, int width, int32_t roll) {
  RocketPose pose = rocket_pose_level(centre.x, centre.y, width);

  pose.cos = cos_lookup(roll);
  pose.sin = sin_lookup(roll);

  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_TYRE_A, 4, GColorBlack);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_TYRE_B, 4, GColorBlack);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_STRUT_A, 4,
                      GColorBlack);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_STRUT_B, 4,
                      GColorBlack);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_CABIN, 4, TEAM_BLUE);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_WINDOW, 4, GColorBlack);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_TUB, 4, TEAM_BLUE);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_BUMPER, 4, GColorBlack);
  // Hubs, so the tyres do not merge with the bumper into one black band.
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_HUB_A, 4,
                      GColorDarkGray);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_HUB_B, 4,
                      GColorDarkGray);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_WING, 4, GColorBlack);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_LAMP_A, 4,
                      TEAM_ORANGE);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_LAMP_B, 4,
                      TEAM_ORANGE);

  // The plumes come toward the viewer, so they go on last and over everything.
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_FLAME_A, 3,
                      TEAM_ORANGE);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_FLAME_B, 3,
                      TEAM_ORANGE);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_CORE_A, 3, GColorWhite);
  prv_fill_grid_shape(ctx, &pose, REAR_CX, REAR_CY, REAR_CORE_B, 3, GColorWhite);
}

static void prv_draw_ball(GContext *ctx, GPoint centre, int radius) {
  prv_fill_ellipse(ctx, centre, radius, radius, GColorBlack);
  prv_fill_ellipse(ctx, centre, radius - 1, radius - 1, GColorWhite);

  if (radius < 7) {
    return;
  }
  prv_fill_ellipse(ctx, centre, radius / 3, radius / 3, GColorBlack);
  for (int i = 0; i < 5; i++) {
    int32_t at = TRIG_MAX_ANGLE * i / 5;
    int reach = radius * 2 / 3;
    int patch = (radius / 4 < 2) ? 2 : radius / 4;
    GPoint on = GPoint(
        centre.x + (int16_t)(sin_lookup(at) * reach / TRIG_MAX_RATIO),
        centre.y - (int16_t)(cos_lookup(at) * reach / TRIG_MAX_RATIO));

    prv_fill_ellipse(ctx, on, patch, patch, GColorBlack);
  }
}

// The goal head-on: net, frame, and the chamfered shoulders that separate a
// Rocket League goal from a football one.
static void prv_draw_goal(GContext *ctx, GRect frame) {
  int right = frame.origin.x + frame.size.w;
  int foot = frame.origin.y + frame.size.h;
  const int bar = 5;
  const int shoulder = 14;
  RocketPoint left_shoulder[] = {
    { frame.origin.x, frame.origin.y }, { frame.origin.x + shoulder,
                                          frame.origin.y },
    { frame.origin.x, frame.origin.y + shoulder },
  };
  RocketPoint right_shoulder[] = {
    { right - 1, frame.origin.y }, { right - 1 - shoulder, frame.origin.y },
    { right - 1, frame.origin.y + shoulder },
  };

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, frame, 0, GCornerNone);

  // The net as a crosshatch. At this size a mesh is the only thing that reads
  // as depth; a flat fill reads as a hole.
  graphics_context_set_stroke_color(ctx, NET);
  graphics_context_set_stroke_width(ctx, 1);
  for (int x = frame.origin.x + 6; x < right - 4; x += 8) {
    graphics_draw_line(ctx, GPoint(x, frame.origin.y + 4), GPoint(x, foot - 2));
  }
  for (int y = frame.origin.y + 6; y < foot - 2; y += 8) {
    graphics_draw_line(ctx, GPoint(frame.origin.x + 4, y), GPoint(right - 4, y));
  }

  graphics_context_set_fill_color(ctx, TEAM_ORANGE);
  graphics_fill_rect(ctx, GRect(frame.origin.x, frame.origin.y, bar,
                                frame.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(right - bar, frame.origin.y, bar, frame.size.h),
                     0, GCornerNone);
  graphics_fill_rect(ctx, GRect(frame.origin.x, frame.origin.y, frame.size.w,
                                bar), 0, GCornerNone);
  prv_fill_polygon(ctx, left_shoulder, 3, TEAM_ORANGE);
  prv_fill_polygon(ctx, right_shoulder, 3, TEAM_ORANGE);

  // A white keyline inside the frame, because the goal in the game is lit and a
  // flat bar of orange is not.
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(frame.origin.x + bar, frame.origin.y + bar,
                                frame.size.w - (bar * 2),
                                frame.size.h - bar + 1));
}

// The stadium looking down the pitch: the far wall, the side walls falling away
// toward it, and the floor running back to the viewer.
static void prv_draw_downfield(GContext *ctx, GRect bounds) {
  const int horizon = ROCKET_ARENA_HORIZON;
  const int far = horizon - 46;
  const int flank = 36;
  const int width = bounds.size.w;
  RocketPoint left[] = { { 0, horizon - 100 }, { flank, far }, { flank, horizon },
                         { 0, horizon } };
  RocketPoint right[] = { { width, horizon - 100 }, { width - flank, far },
                          { width - flank, horizon }, { width, horizon } };

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, 0, width, far), 0, GCornerNone);

  // The far wall first, then the side walls over it — they have to occlude it
  // to read as nearer.
  graphics_context_set_fill_color(ctx, TEAM_BLUE_DEEP);
  graphics_fill_rect(ctx, GRect(0, far, width, horizon - far), 0, GCornerNone);
  prv_fill_polygon(ctx, left, 4, TEAM_BLUE_DEEP);
  prv_fill_polygon(ctx, right, 4, TEAM_BLUE_DEEP);

  // A lit edge along the top of each wall. This is what gives the scene its
  // depth: two lines running away to the same point say more about where the
  // viewer is standing than any amount of shading.
  graphics_context_set_stroke_color(ctx, TEAM_BLUE);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, GPoint(0, horizon - 100), GPoint(flank, far));
  graphics_draw_line(ctx, GPoint(width, horizon - 100),
                     GPoint(width - flank, far));
  graphics_draw_line(ctx, GPoint(flank, far), GPoint(width - flank, far));

  graphics_context_set_fill_color(ctx, FLOOR);
  graphics_fill_rect(ctx, GRect(0, horizon, width, bounds.size.h - horizon), 0,
                     GCornerNone);

  graphics_context_set_stroke_color(ctx, FLOOR_LINE);
  graphics_context_set_stroke_width(ctx, 2);
  for (int x = -360; x <= 560; x += 66) {
    graphics_draw_line(ctx, GPoint(width / 2, horizon),
                       GPoint(x, bounds.size.h));
  }

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(0, horizon), GPoint(width, horizon));
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  RocketArenaLayerData *data = (RocketArenaLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);

  prv_draw_downfield(ctx, bounds);
  prv_draw_goal(ctx, GOAL_FRAME);
  prv_draw_ball(ctx, BALL_CENTRE, BALL_RADIUS);

  if (data->vantage == ROCKET_VANTAGE_AERIAL) {
    // The shadow stays on the floor, well below and behind the car, and it is
    // the only thing in the frame saying the two are no longer in the same
    // place.
    prv_fill_ellipse(ctx, GPoint(104, 214), 28, 4, GColorBlack);
    prv_draw_profile(ctx, GPoint(106, 158), 94, -TRIG_MAX_ANGLE * 62 / 360);
  } else {
    prv_draw_rear(ctx, GPoint(100, 182), 100, -TRIG_MAX_ANGLE * 6 / 360);
  }
}

Layer *rocket_arena_layer_create(GRect frame, RocketVantage vantage) {
  Layer *layer = layer_create_with_data(frame, sizeof(RocketArenaLayerData));
  RocketArenaLayerData *data = (RocketArenaLayerData *)layer_get_data(layer);

  data->vantage = vantage;
  layer_set_update_proc(layer, prv_update_proc);
  return layer;
}

void rocket_arena_layer_destroy(Layer *layer) {
  layer_destroy(layer);
}
