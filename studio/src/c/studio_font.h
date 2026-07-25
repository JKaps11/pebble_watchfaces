#pragma once

#include <pebble.h>

// Custom fonts for the type-character axis.
//
// Pebble's 30 system fonts offer only two families large enough to carry a
// watchface — LECO and Bitham; Gothic stops at 28 px — so without these the type
// axis has about two usable positions and every digital Variant converges on the
// same look however the other axes vary.
//
// A system font is a one-liner and needs no cleanup. A custom one has to be
// loaded from a resource and unloaded again, which is enough friction that a
// Variant would reach for a system font by default. So loading goes through
// here, every font handed out is remembered, and main.c releases the lot when
// the Variant unloads — a Variant asks for a font and never thinks about it
// again.
//
//   text_layer_set_font(layer, studio_font(RESOURCE_ID_FONT_WALLPOET_44));
//
// The bundled faces are subset to digits, colon, space and AM/PM. They are for
// the time; anything else should use a system font.
GFont studio_font(uint32_t resource_id);

// Releases every font handed out by studio_font. Called by the entry point.
void studio_font_unload_all(void);
