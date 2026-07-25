// dusk — crest, an hour and a half later.
//
// Axes: time-display=digital, composition=asymmetric, complications=1-2,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// Identical geometry to crest, down to the crest function: the only thing
// changed is the light. The sky goes from a seven-band sunrise to greens draining
// into black, the sun is gone, the turf darkens and the trees fall to shapes. If
// the Session has been coasting on a colourful sky rather than on the drawing,
// this is where that shows, and the answer is worth knowing before anything gets
// built for real.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define GROUND 170
#define CREST_LEFT 120
#define CREST_FALL 20
#define SWELL 7

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static int prv_crest_at(int x) {
  int32_t along = (TRIG_MAX_ANGLE * 3 * x / 400) + (TRIG_MAX_ANGLE / 8);
  int base = CREST_LEFT + (CREST_FALL * x / 200);
  return base - (SWELL * sin_lookup(along) / TRIG_MAX_RATIO);
}

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_dusk_draw(ctx, GROUND);

  for (int x = 0; x < 200; x++) {
    int crest = prv_crest_at(x);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x, crest, 1, 228 - crest), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorIslamicGreen);
    graphics_fill_rect(ctx, GRect(x, crest - 2, 1, 2), 0, GCornerNone);
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
      window_layer, GRect(12, 148, 116, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(150, 150, 36, 14));
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

STUDIO_VARIANT("dusk", prv_load, prv_unload, prv_tick)
