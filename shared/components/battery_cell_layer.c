#include "battery_cell_layer.h"

typedef struct {
  GColor colour;
  int percent;
} BatteryCellLayerData;

// Pebble's battery handler takes no context pointer, so the subscription has to
// find its layer through a file static. That caps a watchface at one battery
// cell, which is one more than any of them need; a second create() would
// silently orphan the first, so it is refused instead.
static Layer *s_subscribed_layer;

static void prv_set_percent(Layer *layer, int percent) {
  BatteryCellLayerData *data = (BatteryCellLayerData *)layer_get_data(layer);
  if (data->percent == percent) {
    return;
  }
  data->percent = percent;
  layer_mark_dirty(layer);
}

static void prv_battery_handler(BatteryChargeState charge) {
  if (s_subscribed_layer) {
    prv_set_percent(s_subscribed_layer, charge.charge_percent);
  }
}

static void prv_update_proc(Layer *layer, GContext *ctx) {
  BatteryCellLayerData *data = (BatteryCellLayerData *)layer_get_data(layer);
  GRect bounds = layer_get_bounds(layer);
  const int nub_width = 2;
  const int nub_gap = 1;
  int body_width = bounds.size.w - nub_width - nub_gap;

  graphics_context_set_stroke_color(ctx, data->colour);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(0, 0, body_width, bounds.size.h));

  graphics_context_set_fill_color(ctx, data->colour);
  graphics_fill_rect(ctx, GRect(body_width + nub_gap, bounds.size.h / 4,
                                nub_width, bounds.size.h / 2),
                     0, GCornerNone);

  // The fill is the whole point of the icon, so it is never rounded away to
  // nothing: any charge above zero draws at least one column, which is what
  // makes 4% read as nearly empty rather than as empty.
  int track = body_width - 4;
  int fill = track * data->percent / 100;
  if (data->percent > 0 && fill < 1) {
    fill = 1;
  }
  if (fill > 0) {
    graphics_fill_rect(ctx, GRect(2, 2, fill, bounds.size.h - 4), 0,
                       GCornerNone);
  }
}

Layer *battery_cell_layer_create(GRect frame, GColor colour) {
  if (s_subscribed_layer) {
    return NULL;
  }

  Layer *layer = layer_create_with_data(frame, sizeof(BatteryCellLayerData));
  BatteryCellLayerData *data = (BatteryCellLayerData *)layer_get_data(layer);
  data->colour = colour;
  data->percent = battery_state_service_peek().charge_percent;
  layer_set_update_proc(layer, prv_update_proc);

  s_subscribed_layer = layer;
  battery_state_service_subscribe(prv_battery_handler);
  return layer;
}

void battery_cell_layer_destroy(Layer *layer) {
  if (layer == s_subscribed_layer) {
    battery_state_service_unsubscribe();
    s_subscribed_layer = NULL;
  }
  layer_destroy(layer);
}
