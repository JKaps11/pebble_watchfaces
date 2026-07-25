// verge — the foreground cut on the diagonal, so the hole keeps one corner.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// A middle position between band and overlay: there is still a black ground
// under the numerals, but its edge runs on a slant, so the fairway carries on
// past the type and reaches the bottom of the display on the right. The date and
// the battery move up onto the sky to keep the wedge clear, which is what makes
// this asymmetric rather than another split — mass low left, mass high right,
// and the hole running between them.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define VERGE_LEFT 128
#define VERGE_RIGHT 174

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_course_draw(ctx, 228);

  for (int x = 0; x < 200; x++) {
    int edge = VERGE_LEFT + ((VERGE_RIGHT - VERGE_LEFT) * x / 200);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x, edge, 1, 228 - edge), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorMayGreen);
    graphics_fill_rect(ctx, GRect(x, edge - 2, 1, 2), 0, GCornerNone);
  }
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
      window_layer, GRect(70, 8, 120, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentRight);

  s_battery_layer = layer_create(GRect(154, 34, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);

  s_time_layer = studio_text_layer(window_layer, GRect(10, 164, 180, 56),
                                   studio_font(RESOURCE_ID_FONT_BARLOW_52),
                                   GColorWhite, GTextAlignmentLeft);
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

STUDIO_VARIANT("verge", prv_load, prv_unload, prv_tick)
