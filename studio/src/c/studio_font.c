#include "studio_font.h"

// Enough for every font bundled in the Studio, loaded at once by one Variant.
#define MAX_LOADED_FONTS 8

typedef struct {
  uint32_t resource_id;
  GFont font;
} LoadedFont;

static LoadedFont s_loaded[MAX_LOADED_FONTS];
static int s_loaded_count;

GFont studio_font(uint32_t resource_id) {
  // Asking twice for the same face is normal — a Variant may set it on several
  // layers — and loading it twice would leak the first copy.
  for (int i = 0; i < s_loaded_count; i++) {
    if (s_loaded[i].resource_id == resource_id) {
      return s_loaded[i].font;
    }
  }

  if (s_loaded_count >= MAX_LOADED_FONTS) {
    APP_LOG(APP_LOG_LEVEL_ERROR,
            "studio_font: more than %d fonts in one Variant", MAX_LOADED_FONTS);
    return fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  }

  GFont font = fonts_load_custom_font(resource_get_handle(resource_id));
  s_loaded[s_loaded_count++] = (LoadedFont) {
    .resource_id = resource_id,
    .font = font,
  };
  return font;
}

void studio_font_unload_all(void) {
  for (int i = 0; i < s_loaded_count; i++) {
    fonts_unload_custom_font(s_loaded[i].font);
  }
  s_loaded_count = 0;
}
