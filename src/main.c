/*
  Add outside data using PebbleKit JS
  https://developer.getpebble.com/tutorials/watchface-tutorial/part3/
*/

#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static GFont s_weather_font;
static TextLayer *s_weather_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Custom fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_KANIT_BOLD_45));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_KANIT_REGULAR_20));
  
  // Create the time TextLayer
  s_time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50)
  );
  
  // Style the time text
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add it as a child layer to the Window's root Layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  
  // Create the temperature TextLayer
  s_weather_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(125, 120), bounds.size.w, 25)
  );
  
  // Style the weather text  
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  // Add weather Layer to Window's root Layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  
  // Unload GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  
  // Set handlers to manage the elements inside the window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Show the window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Make sure the time is displayed when started
  update_time();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}