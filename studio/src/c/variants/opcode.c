// opcode — the listing pared to a single instruction. The zero end of the Spread.
//
// Axes: time-display=digital, composition=corner-anchored, complications=0,
//       hue=one-hue, type=custom, polarity=light-on-dark
//
// If an assembly listing is going to read as one, it has to do it with the shape
// of the source rather than with the amount of it — the assembler directives, the
// label in column one, the operands hanging off a fixed tab stop. This carries
// nothing but the time so that the question is asked cleanly: does five lines of
// scaffolding around one number read as a listing, or as a joke about one?

#include <pebble.h>
#include "../studio_draw.h"
#include "../studio_font.h"
#include "../variant.h"

#define MARGIN 12
#define PHOSPHOR GColorScreaminGreen
#define TOP 24
#define PITCH 26
#define LINE_HEIGHT 26

static TextLayer *s_lines[5];
static char s_time_line[24];

static TextLayer *prv_line(Layer *parent, int row) {
  return studio_text_layer(
      parent, GRect(MARGIN, TOP + (row * PITCH), 200 - (MARGIN * 2),
                    LINE_HEIGHT),
      studio_font(RESOURCE_ID_FONT_MONO_18), PHOSPHOR, GTextAlignmentLeft);
}

static void prv_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  for (unsigned i = 0; i < ARRAY_LENGTH(s_lines); i++) {
    s_lines[i] = prv_line(window_layer, i);
  }

  text_layer_set_text(s_lines[0], "  ORG $0000");
  text_layer_set_text(s_lines[1], "CLOCK:");
  text_layer_set_text(s_lines[3], "  RET");
  text_layer_set_text(s_lines[4], "  END");
}

static void prv_unload(Window *window) {
  for (unsigned i = 0; i < ARRAY_LENGTH(s_lines); i++) {
    text_layer_destroy(s_lines[i]);
  }
}

static void prv_tick(struct tm *now) {
  char time_text[16];
  studio_format(now, time_text, sizeof(time_text), NULL, 0);
  snprintf(s_time_line, sizeof(s_time_line), "  LD  A,%s", time_text);
  text_layer_set_text(s_lines[2], s_time_line);
}

STUDIO_VARIANT("opcode", prv_load, prv_unload, prv_tick)
