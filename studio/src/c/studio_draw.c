#include "studio_draw.h"

void studio_format(struct tm *now, char *time_text, size_t time_size,
                   char *date_text, size_t date_size) {
  DateTimeInfo info = {
    .hour = now->tm_hour,
    .minute = now->tm_min,
    .weekday = now->tm_wday,
    .month = now->tm_mon,
    .day_of_month = now->tm_mday,
  };
  if (time_text) {
    datetime_format_time(&info, clock_is_24h_style(), time_text, time_size);
  }
  if (date_text) {
    datetime_format_date(&info, date_text, date_size);
  }
}

void studio_upper(char *text) {
  for (; *text; text++) {
    if (*text >= 'a' && *text <= 'z') {
      *text -= 'a' - 'A';
    }
  }
}

GPoint studio_point_at(GPoint centre, int32_t angle, int length) {
  return GPoint(
      centre.x + (int16_t)(sin_lookup(angle) * length / TRIG_MAX_RATIO),
      centre.y - (int16_t)(cos_lookup(angle) * length / TRIG_MAX_RATIO));
}

int32_t studio_hour_angle(int hour, int minute) {
  return TRIG_MAX_ANGLE * ((hour % 12) * 60 + minute) / (12 * 60);
}

int32_t studio_minute_angle(int minute) {
  return TRIG_MAX_ANGLE * minute / 60;
}

static int prv_isqrt(int value) {
  int root = 0;
  while ((root + 1) * (root + 1) <= value) {
    root++;
  }
  return root;
}

void studio_fill_ellipse(GContext *ctx, GPoint centre, int radius_x,
                         int radius_y, GColor colour) {
  graphics_context_set_fill_color(ctx, colour);
  for (int dy = -radius_y; dy <= radius_y; dy++) {
    int half = radius_x * prv_isqrt((radius_y * radius_y) - (dy * dy))
               / radius_y;
    graphics_fill_rect(ctx, GRect(centre.x - half, centre.y + dy, half * 2, 1),
                       0, GCornerNone);
  }
}

void studio_fill_triangle(GContext *ctx, GPoint apex, GPoint from, GPoint to,
                          GColor colour) {
  const int steps = 24;
  graphics_context_set_stroke_color(ctx, colour);
  graphics_context_set_stroke_width(ctx, 1);
  for (int i = 0; i <= steps; i++) {
    graphics_draw_line(ctx, apex,
                       GPoint(from.x + ((to.x - from.x) * i / steps),
                              from.y + ((to.y - from.y) * i / steps)));
  }
}

void studio_battery_cell(GContext *ctx, GRect frame, int percent, GColor colour) {
  const int nub_width = 2;
  const int nub_gap = 1;
  int body_width = frame.size.w - nub_width - nub_gap;

  graphics_context_set_stroke_color(ctx, colour);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_rect(ctx, GRect(frame.origin.x, frame.origin.y, body_width,
                                frame.size.h));

  graphics_context_set_fill_color(ctx, colour);
  graphics_fill_rect(ctx, GRect(frame.origin.x + body_width + nub_gap,
                                frame.origin.y + (frame.size.h / 4), nub_width,
                                frame.size.h / 2), 0, GCornerNone);

  int track = body_width - 4;
  int fill = track * percent / 100;
  if (percent > 0 && fill < 1) {
    fill = 1;
  }
  if (fill > 0) {
    graphics_fill_rect(ctx, GRect(frame.origin.x + 2, frame.origin.y + 2, fill,
                                  frame.size.h - 4), 0, GCornerNone);
  }
}

TextLayer *studio_text_layer(Layer *parent, GRect frame, GFont font,
                             GColor colour, GTextAlignment alignment) {
  TextLayer *layer = text_layer_create(frame);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, colour);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, alignment);
  layer_add_child(parent, text_layer_get_layer(layer));
  return layer;
}
