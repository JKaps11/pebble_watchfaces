// pinhigh — the same green, with a ground for the numerals to stand on.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// greenside and this differ in exactly one decision: whether the type gets a
// black ground or a keyline. Everything else — the vantage, the pin, the cup,
// the bunker — is the same call into the same scene. The crest here is level
// rather than tilted, because the near fringe of a green is a rolled edge and
// not a bank, and because holding it level is what makes the comparison against
// crest worth anything.

#include <pebble.h>
#include "../golf_course.h"
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define GROUND 156
#define CREST 140
#define SWELL 10

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static int prv_crest_at(int x) {
  int32_t along = (TRIG_MAX_ANGLE * 2 * x / 400) + (TRIG_MAX_ANGLE * 3 / 8);
  return CREST - (SWELL * sin_lookup(along) / TRIG_MAX_RATIO);
}

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  golf_green_draw(ctx, GROUND);

  for (int x = 0; x < 200; x++) {
    int crest = prv_crest_at(x);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(x, crest, 1, 228 - crest), 0, GCornerNone);
    graphics_context_set_fill_color(ctx, GColorInchworm);
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
      window_layer, GRect(12, 152, 116, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentLeft);

  s_battery_layer = layer_create(GRect(152, 156, 36, 14));
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

STUDIO_VARIANT("pinhigh", prv_load, prv_unload, prv_tick)
