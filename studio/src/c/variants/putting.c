// putting — the dial as a green, the hour hand as the pin, the minute as a ball.
//
// Axes: time-display=analogue, composition=centred, complications=1-2,
//       hue=one-hue, type=Gothic, polarity=light-on-dark
//
// The wildest reading in the Batch and the only analogue one: the face is a
// putting green seen from above, the hour hand is the flagstick planted in the
// cup, and the minute hand is the line of a putt with the ball at its far end.
// The question is whether an unfamiliar metaphor survives being a clock — the
// green's irregular edge deliberately gives up the ring of hour markers that
// normally does the reading, so this either works at a glance or not at all.
//
// The type axis is declared Gothic because nothing carries the time in type;
// only the date does, and an analogue face keeps its character in geometry.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define ROUGH GColorDarkGreen
#define SURFACE GColorIslamicGreen
#define CONTOUR GColorMayGreen
#define CENTRE_X 100
#define CENTRE_Y 100
#define POINTS 16

static Layer *s_green_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_date_text[16];
static int s_battery_percent;
static int s_hour;
static int s_minute;

// A green is never a circle. These radii, read round the compass, are what makes
// the edge look mown rather than drawn.
static const int s_edge[POINTS] = {
  80, 84, 82, 74, 68, 70, 78, 84, 86, 82, 72, 66, 68, 76, 82, 84,
};

static GPoint s_surface[POINTS];
static GPoint s_contour[POINTS];
static GPathInfo s_surface_info = { .num_points = POINTS, .points = s_surface };
static GPathInfo s_contour_info = { .num_points = POINTS, .points = s_contour };
static GPath *s_surface_path;
static GPath *s_contour_path;

static void prv_fill_triangle(GContext *ctx, GPoint apex, GPoint from,
                              GPoint to) {
  for (int i = 0; i <= 20; i++) {
    graphics_draw_line(ctx, apex,
                       GPoint(from.x + (to.x - from.x) * i / 20,
                              from.y + (to.y - from.y) * i / 20));
  }
}

static void prv_draw_green(Layer *layer, GContext *ctx) {
  GPoint centre = GPoint(CENTRE_X, CENTRE_Y);

  graphics_context_set_fill_color(ctx, SURFACE);
  gpath_draw_filled(ctx, s_surface_path);

  graphics_context_set_stroke_color(ctx, CONTOUR);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, s_contour_path);

  int32_t hour_angle = studio_hour_angle(s_hour, s_minute);
  int32_t minute_angle = studio_minute_angle(s_minute);

  // The putt: a line out from the cup with the ball sitting at its end.
  GPoint ball = studio_point_at(centre, minute_angle, 62);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_line(ctx, centre, ball);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, ball, 6);

  // The cup, and the pin standing in it.
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, centre, 7);

  GPoint tip = studio_point_at(centre, hour_angle, 52);
  GPoint hoist = studio_point_at(centre, hour_angle, 34);
  GPoint fly = studio_point_at(hoist, hour_angle + (TRIG_MAX_ANGLE / 4), 26);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_line(ctx, centre, tip);
  graphics_context_set_stroke_width(ctx, 1);
  prv_fill_triangle(ctx, fly, tip, hoist);
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent,
                      GColorWhite);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, ROUGH);

  GPoint centre = GPoint(CENTRE_X, CENTRE_Y);
  for (int i = 0; i < POINTS; i++) {
    int32_t angle = TRIG_MAX_ANGLE * i / POINTS;
    s_surface[i] = studio_point_at(centre, angle, s_edge[i]);
    s_contour[i] = studio_point_at(centre, angle, s_edge[i] * 3 / 5);
  }
  s_surface_path = gpath_create(&s_surface_info);
  s_contour_path = gpath_create(&s_contour_info);

  s_green_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_green_layer, prv_draw_green);
  layer_add_child(window_layer, s_green_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 194, 120, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(150, 199, 38, 15));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_green_layer);
  gpath_destroy(s_contour_path);
  gpath_destroy(s_surface_path);
}

static void prv_tick(struct tm *now) {
  studio_format(now, NULL, 0, s_date_text, sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;
  s_hour = now->tm_hour;
  s_minute = now->tm_min;

  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
  layer_mark_dirty(s_green_layer);
}

STUDIO_VARIANT("putting", prv_load, prv_unload, prv_tick)
