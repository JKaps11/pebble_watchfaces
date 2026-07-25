// sprinkler — the head cover golfers actually pace off, centred in the fairway.
//
// Axes: time-display=digital, composition=centred, complications=1-2,
//       hue=one-hue, type=Bitham, polarity=light-on-dark
//
// yardage's idea taken to its cleanest form: one dark disc set into mown turf,
// and nothing else on the display. A sprinkler head is the flattest, plainest
// object on a golf course and is already a ring around a number, so the theme
// and the layout are the same shape — which is the argument for it over the
// rectangular plate. The risk is the opposite of the pictorial Variants: with no
// horizon and no pin, this may simply be a round watchface on a green ground.

#include <pebble.h>
#include "../studio_draw.h"
#include "../variant.h"

#define TURF GColorDarkGreen
#define STRIPE GColorMayGreen
#define ACCENT GColorInchworm
#define CENTRE_X 100
#define CENTRE_Y 104
#define DISC_R 84

static Layer *s_head_layer;
static TextLayer *s_label_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static Layer *s_battery_layer;

static char s_time_text[16];
static char s_date_text[16];
static int s_battery_percent;

static void prv_draw_head(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  for (int y = 0; y < bounds.size.h; y += 15) {
    graphics_context_set_fill_color(ctx, ((y / 15) % 2) ? STRIPE : TURF);
    graphics_fill_rect(ctx, GRect(0, y, bounds.size.w, 15), 0, GCornerNone);
  }

  GPoint centre = GPoint(CENTRE_X, CENTRE_Y);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, centre, DISC_R);

  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 2);
  graphics_draw_circle(ctx, centre, DISC_R);
  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, centre, DISC_R - 8);

  // The two lifting slots, which is what stops the disc reading as a plain ring.
  graphics_context_set_fill_color(ctx, ACCENT);
  graphics_fill_rect(ctx, GRect(CENTRE_X - 13, CENTRE_Y - DISC_R + 15, 26, 3),
                     1, GCornersAll);
  graphics_fill_rect(ctx, GRect(CENTRE_X - 13, CENTRE_Y + DISC_R - 18, 26, 3),
                     1, GCornersAll);

  graphics_context_set_stroke_color(ctx, ACCENT);
  graphics_draw_line(ctx, GPoint(CENTRE_X - 58, 134), GPoint(CENTRE_X + 58, 134));
}

static void prv_draw_battery(Layer *layer, GContext *ctx) {
  studio_battery_cell(ctx, layer_get_bounds(layer), s_battery_percent, ACCENT);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, TURF);

  s_head_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_head_layer, prv_draw_head);
  layer_add_child(window_layer, s_head_layer);

  s_label_layer = studio_text_layer(
      window_layer, GRect(30, 46, 140, 18),
      fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), ACCENT,
      GTextAlignmentCenter);
  text_layer_set_text(s_label_layer, "TO CENTRE");

  s_time_layer = studio_text_layer(
      window_layer, GRect(20, 70, 160, 54),
      fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS), GColorWhite,
      GTextAlignmentCenter);

  s_date_layer = studio_text_layer(
      window_layer, GRect(30, 140, 140, 22),
      fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GColorWhite,
      GTextAlignmentCenter);

  s_battery_layer = layer_create(GRect(82, 166, 36, 14));
  layer_set_update_proc(s_battery_layer, prv_draw_battery);
  layer_add_child(window_layer, s_battery_layer);
}

static void prv_unload(Window *window) {
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_label_layer);
  layer_destroy(s_head_layer);
}

static void prv_tick(struct tm *now) {
  studio_format(now, s_time_text, sizeof(s_time_text), s_date_text,
                sizeof(s_date_text));
  s_battery_percent = battery_state_service_peek().charge_percent;

  text_layer_set_text(s_time_layer, s_time_text);
  text_layer_set_text(s_date_layer, s_date_text);
  layer_mark_dirty(s_battery_layer);
}

STUDIO_VARIANT("sprinkler", prv_load, prv_unload, prv_tick)
