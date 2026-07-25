// linksland — the same hole in links weather: grey ceiling, dunes, bunkers.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=one-hue, type=LECO, polarity=light-on-dark
//
// The control against sunrise. Same split, same foreground band, but the sky is
// weather rather than colour — greys, with one bright strip on the horizon —
// which leaves green as the only chromatic thing on the display. It exists to
// answer whether the added background needed a sunrise to be worth having, or
// whether depth alone was doing the work and the palette was decoration.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define ROUGH GColorDarkGreen
#define STRIPE_A GColorMayGreen
#define STRIPE_B GColorIslamicGreen
#define SAND GColorLightGray
#define HORIZON 78
#define GROUND 146

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, GRect(0, 0, 200, 54), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0, 54, 200, 18), 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 72, 200, 8), 0, GCornerNone);

  // Dunes, with a gap in the middle for the hole to run through.
  for (int x = -10; x < 210; x += 21) {
    if (x > 74 && x < 128) {
      continue;
    }
    studio_fill_ellipse(ctx, GPoint(x, HORIZON + 8), 16, 12 + ((x * 5) % 9),
                        GColorBlack);
  }

  graphics_context_set_fill_color(ctx, ROUGH);
  graphics_fill_rect(ctx, GRect(0, HORIZON, 200, GROUND - HORIZON), 0,
                     GCornerNone);

  int y = HORIZON;
  int depth = 4;
  for (int band = 0; y < GROUND; band++) {
    graphics_context_set_fill_color(ctx, (band % 2) ? STRIPE_A : STRIPE_B);
    for (int row = y; row < y + depth && row < GROUND; row++) {
      int half = 6 + (82 * (row - HORIZON) / (GROUND - HORIZON));
      graphics_fill_rect(ctx, GRect(100 - half, row, half * 2, 1), 0,
                         GCornerNone);
    }
    y += depth;
    depth += 2;
  }

  // Revetted bunkers, which is what makes a hole read as links rather than park.
  studio_fill_ellipse(ctx, GPoint(46, 128), 26, 9, SAND);
  studio_fill_ellipse(ctx, GPoint(160, 112), 19, 7, SAND);
  studio_fill_ellipse(ctx, GPoint(122, 94), 11, 4, SAND);

  studio_fill_ellipse(ctx, GPoint(100, 82), 14, 5, STRIPE_B);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(99, 80, 3, 2), 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(100, 80), GPoint(100, 52));
  studio_fill_triangle(ctx, GPoint(124, 58), GPoint(101, 52), GPoint(101, 64),
                       GColorBlack);

  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(0, GROUND, 200, 228 - GROUND), 0, GCornerNone);
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent,
                      GColorWhite);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_scene_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_scene_layer, prv_draw_scene);
  layer_add_child(window_layer, s_scene_layer);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 150, 116, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(152, 154, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);

  s_time_layer = studio_text_layer(
      window_layer, GRect(10, 168, 180, 52),
      fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS), GColorWhite,
      GTextAlignmentLeft);
}

static void prv_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_scene_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("linksland", prv_load, prv_unload, prv_tick)
