#include <pebble.h>
#include "resources.h"
#include "scroll_text_layer.h"
#include "bahn_interface.h"
#include "connection_details_window.h"


static Window* s_window;
static ScrollTextLayer* s_scroll_layer;
static StatusBarLayer *s_status_bar;

#define NUM_FONTS 6

static char* fonts[6] = {
  FONT_KEY_GOTHIC_18,
  FONT_KEY_GOTHIC_18_BOLD,
  FONT_KEY_GOTHIC_24,
  FONT_KEY_GOTHIC_24_BOLD,
  FONT_KEY_GOTHIC_28,
  FONT_KEY_GOTHIC_28_BOLD,
};

static uint8_t font = 3;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  font += 1;
  if (font >= NUM_FONTS) {
    font = 0;
  }
  scroll_text_layer_set_text(s_scroll_layer, "");
  scroll_text_layer_set_system_font(s_scroll_layer, fonts[font]);
  scroll_text_layer_set_text(s_scroll_layer, BahnInterface_get_current_connection_details());
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window* window) {
  if (!s_scroll_layer) {
    s_status_bar = addStatusBar(window);
    GRect scroll_layer_bounds = computeEffectiveWindowBounds(window, s_status_bar);
#if defined(PBL_ROUND)
    int original_height = scroll_layer_bounds.size.h;
    int original_width = scroll_layer_bounds.size.w;
    
    scroll_layer_bounds.size.w -= 20;
    scroll_layer_bounds.size.h = original_height * 2 / 3;
    
    scroll_layer_bounds.origin.x = (original_width - scroll_layer_bounds.size.w) / 2;
    scroll_layer_bounds.origin.y = (original_height - scroll_layer_bounds.size.h) / 2;
#endif
    s_scroll_layer = scroll_text_layer_create(scroll_layer_bounds);
  }
  scroll_text_layer_set_system_font(s_scroll_layer, fonts[font]);
  scroll_text_layer_set_text(s_scroll_layer, BahnInterface_get_current_connection_details());
  scroll_text_layer_add_to_window(s_scroll_layer, window);
  scroll_text_layer_set_callbacks(s_scroll_layer, (ScrollLayerCallbacks) {
    .click_config_provider = click_config_provider
  });
}

static void window_unload(Window *window) {
  scroll_text_layer_destroy(s_scroll_layer);
  s_scroll_layer = NULL;
  status_bar_layer_destroy(s_status_bar);
  s_status_bar = NULL;
}

static void show_window() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

void ConnectionDetailsWindow_show(ConnectionResult* result) {  
  ConnectionDetailsWindow_destroy();

  BahnInterface_read_connection_details(result, show_window);
}

void ConnectionDetailsWindow_destroy() {
  if (!s_window) {
    return;
  }
  
  window_destroy(s_window);
  s_window = NULL;
}
