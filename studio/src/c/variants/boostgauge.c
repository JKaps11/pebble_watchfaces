// boostgauge — the boost meter borrowed whole, reading the battery.
//
// Axes: time-display=digital, composition=corner-anchored, complications=1-2,
//       hue=multi, type=Bitham, polarity=light-on-dark
//
// Every other Variant here quotes the game's scenery. This one quotes its HUD,
// which is the part a player actually looks at, and the boost meter happens to
// be a battery gauge already — so the theme is doing work rather than
// decorating. Bitham and a black ground keep the type deliberately plain: the
// question is whether one borrowed instrument is enough to carry a face.

#include <pebble.h>
#include "../rocket_arena.h"
#include "../studio_draw.h"
#include "../variant.h"

#define GAUGE_X 144
#define GAUGE_Y 158
#define GAUGE_OUTER 44
#define GAUGE_INNER 31
#define SEGMENTS 22

static Layer *s_gauge_layer;
static Layer *s_car_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_boost_layer;
static TextLayer *s_label_layer;

static char s_time_text[16];
static char s_date_text[16];
static char s_boost_text[8];
static int s_battery_percent;

// The meter is a three-quarter arc opening downward, drawn as separate spokes
// rather than a filled ring — the game's meter is segmented, and a segment count
// is something the eye can read without measuring an angle.
static void prv_draw_gauge(Layer *layer, GContext *ctx) {
  const int32_t sweep = TRIG_MAX_ANGLE * 3 / 4;
  const int32_t start = -sweep / 2;
  int lit = (SEGMENTS * s_battery_percent + 99) / 100;
  GPoint centre = GPoint(GAUGE_X, GAUGE_Y);

  graphics_context_set_stroke_width(ctx, 5);
  for (int i = 0; i < SEGMENTS; i++) {
    int32_t angle = start + (sweep * i / (SEGMENTS - 1));
    graphics_context_set_stroke_color(ctx, (i < lit) ? ROCKET_BOOST
                                                     : GColorOxfordBlue);
    graphics_draw_line(ctx, studio_point_at(centre, angle, GAUGE_INNER),
                       studio_point_at(centre, angle, GAUGE_OUTER));
  }
}

static void prv_draw_car(Layer *layer, GContext *ctx) {
  rocket_octane_draw(ctx, GRect(34, 0, 60, 29), ROCKET_BLUE, true);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  s_time_layer = studio_text_layer(
      window_layer, GRect(10, 18, 182, 52),
      fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GColorWhite,
      GTextAlignmentLeft);

  s_date_layer = studio_text_layer(
      window_layer, GRect(12, 72, 180, 24),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), ROCKET_BOOST,
      GTextAlignmentLeft);

  s_gauge_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_gauge_layer, prv_draw_gauge);
  layer_add_child(window_layer, s_gauge_layer);

  s_boost_layer = studio_text_layer(
      window_layer, GRect(GAUGE_X - 40, GAUGE_Y - 22, 80, 34),
      fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GColorWhite,
      GTextAlignmentCenter);

  s_label_layer = studio_text_layer(
      window_layer, GRect(GAUGE_X - 40, GAUGE_Y + 10, 80, 20),
      fonts_get_system_font(FONT_KEY_GOTHIC_14), GColorLightGray,
      GTextAlignmentCenter);
  text_layer_set_text(s_label_layer, "BOOST");

  s_car_layer = layer_create(GRect(0, 172, 120, 36));
  layer_set_update_proc(s_car_layer, prv_draw_car);
  layer_add_child(window_layer, s_car_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_car_layer);
  text_layer_destroy(s_label_layer);
  text_layer_destroy(s_boost_layer);
  layer_destroy(s_gauge_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;
  snprintf(s_boost_text, sizeof(s_boost_text), "%d", s_battery_percent);

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  text_layer_set_text(s_boost_layer, s_boost_text);
  layer_mark_dirty(s_gauge_layer);
}

STUDIO_VARIANT("boostgauge", prv_load, prv_unload, prv_tick)
