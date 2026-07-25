// dimples — the whole face as one golf ball, time stamped across it.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// The most restrained of the pictorial readings: one object, centred, and
// nothing else on the display. The dimple lattice is doing all of the theming,
// which is the point — if a texture alone can say golf then the layout is free
// to be an ordinary centred stack, and this is the only Variant here that could
// pass for a normal watchface at arm's length.
//
// A golf ball is white, and this one is not, because the Batch holds polarity
// fixed at light-on-dark. That is a real tension and the obvious thing to Sweep
// next if this direction is the one worth having.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define SURFACE GColorDarkGreen
#define DIMPLE GColorIslamicGreen
#define HIGHLIGHT GColorMayGreen
#define BALL_X 100
#define BALL_Y 106
#define BALL_R 92

static Layer *s_ball_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_ball(Layer *layer, GContext *ctx) {
  GPoint centre = GPoint(BALL_X, BALL_Y);

  graphics_context_set_fill_color(ctx, SURFACE);
  graphics_fill_circle(ctx, centre, BALL_R);

  // Dimples on a staggered lattice, kept inside the silhouette so the rim stays
  // clean and the ball reads as round rather than as a tiled background.
  graphics_context_set_fill_color(ctx, DIMPLE);
  for (int row = 0; row * 17 < BALL_R * 2 + 17; row++) {
    int y = BALL_Y - BALL_R + (row * 17);
    int stagger = (row % 2) ? 9 : 0;
    for (int x = BALL_X - BALL_R + stagger; x <= BALL_X + BALL_R; x += 18) {
      int dx = x - BALL_X;
      int dy = y - BALL_Y;
      if ((dx * dx) + (dy * dy) < (BALL_R - 7) * (BALL_R - 7)) {
        graphics_fill_circle(ctx, GPoint(x, y), 4);
      }
    }
  }

  // A sliver of highlight so the ball has a lit side.
  graphics_context_set_stroke_color(ctx, HIGHLIGHT);
  graphics_context_set_stroke_width(ctx, 3);
  graphics_draw_arc(ctx, GRect(BALL_X - BALL_R, BALL_Y - BALL_R, BALL_R * 2,
                               BALL_R * 2),
                    GOvalScaleModeFitCircle, TRIG_MAX_ANGLE * 5 / 8,
                    TRIG_MAX_ANGLE * 7 / 8);
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent,
                      GColorWhite);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_ball_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_ball_layer, prv_draw_ball);
  layer_add_child(window_layer, s_ball_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(0, 74, 200, 56),
                                   studio_font(RESOURCE_ID_FONT_ARCHIVO_44),
                                   GColorWhite, GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(0, 134, 200, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentCenter);

  s_battery_layer = layer_create(GRect(81, 164, 38, 15));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  layer_destroy(s_ball_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("dimples", prv_load, prv_unload, prv_tick)
