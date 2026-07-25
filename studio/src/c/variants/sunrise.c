// sunrise — fairway's view, opened up: dawn sky, the hole, the time underfoot.
//
// Axes: time-display=digital, composition=split, complications=1-2,
//       hue=multi, type=custom, polarity=light-on-dark
//
// fairway had the composition right and the legibility wrong: its mowing stripes
// ran straight under the numerals. The fix is the split — the scene gets the top
// two thirds and stops dead at a black foreground band that the type owns
// outright, so the picture can be as busy as it likes. Everything else is the
// added background: a graded dawn sky, a treeline, and the stripes narrowing to
// a point so the hole reads as going away from you rather than lying flat.

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define ROUGH GColorDarkGreen
#define STRIPE_A GColorMayGreen
#define STRIPE_B GColorIslamicGreen
#define HORIZON 76
#define GROUND 146

static Layer *s_scene_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

// Top of the sky down to the horizon. Dark overhead, warm where the sun is.
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

static void prv_draw_scene(Layer *layer, GContext *ctx) {
  for (int band = 0; band < 7; band++) {
    graphics_context_set_fill_color(ctx, prv_sky_band(band));
    graphics_fill_rect(ctx, GRect(0, band * 11, 200, 12), 0, GCornerNone);
  }

  studio_fill_ellipse(ctx, GPoint(152, 74), 19, 19, GColorYellow);

  // A treeline, so the horizon is an edge rather than a colour change.
  for (int x = -6; x < 210; x += 17) {
    int height = 9 + ((x * 7) % 11);
    studio_fill_ellipse(ctx, GPoint(x, HORIZON + 4), 11, height, GColorBlack);
  }

  // The hole, narrowing away from the viewer.
  graphics_context_set_fill_color(ctx, ROUGH);
  graphics_fill_rect(ctx, GRect(0, HORIZON, 200, GROUND - HORIZON), 0,
                     GCornerNone);

  int y = HORIZON;
  int depth = 4;
  for (int band = 0; y < GROUND; band++) {
    graphics_context_set_fill_color(ctx, (band % 2) ? STRIPE_A : STRIPE_B);
    for (int row = y; row < y + depth && row < GROUND; row++) {
      int half = 6 + (84 * (row - HORIZON) / (GROUND - HORIZON));
      graphics_fill_rect(ctx, GRect(100 - half, row, half * 2, 1), 0,
                         GCornerNone);
    }
    y += depth;
    depth += 2;
  }

  // The green at the far end, with the pin standing on the skyline.
  studio_fill_ellipse(ctx, GPoint(100, 80), 15, 6, GColorIslamicGreen);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, GRect(99, 78, 3, 2), 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_line(ctx, GPoint(100, 78), GPoint(100, 50));
  studio_fill_triangle(ctx, GPoint(124, 56), GPoint(101, 50), GPoint(101, 62),
                       GColorWhite);

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

  s_time_layer = studio_text_layer(window_layer, GRect(10, 166, 180, 56),
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

STUDIO_VARIANT("sunrise", prv_load, prv_unload, prv_tick)
