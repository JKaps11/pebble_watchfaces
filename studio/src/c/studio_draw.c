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
