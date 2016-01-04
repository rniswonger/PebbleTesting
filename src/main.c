/*
  Add outside data using PebbleKit JS
  https://developer.getpebble.com/tutorials/watchface-tutorial/part3/
*/

#include <pebble.h>
static Window *s_main_window;
static GFont s_time_font;
static TextLayer *s_time_layer;
static GFont s_weather_font;
static TextLayer *s_weather_layer;
static GFont s_battery_font;
static int s_battery_level;
static TextLayer *s_battery_layer;

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  
  // write battery level into a buffer
  static char s_buffer[8];
  snprintf(s_buffer, sizeof(s_buffer), "%d%%", s_battery_level);
  text_layer_set_text(s_battery_layer, s_buffer);
  
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *cond_tuple = dict_find(iterator, KEY_CONDITIONS);
  
  // If the data is available, display it
  if (temp_tuple && cond_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°F", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", cond_tuple->value->cstring);
    
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");  
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send success!");
}


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
  
  // Get updates every 30 min
  if (tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    dict_write_uint8(iter, 0, 0);
    
    app_message_outbox_send();
  }
}


static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Custom fonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ALIEN_REGULAR_40));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ROBOTO_COND_REG_20));
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FFFHARMONY_8));
  
  // Create the time TextLayer
  s_time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50)
  );
  
  // Style the time text
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorRed);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Add time Layer as child layer to Window's root Layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  
  
  // Create the weather TextLayer
  s_weather_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(105, 100), bounds.size.w, 25)
  );
  
  // Style the weather text  
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorDarkGray);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  // Add weather Layer as child layer to Window's root Layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
  
  // Create battery TextLayer
  s_battery_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(135, 130), bounds.size.w, 10)
  );
  
  // Style battery text
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorDarkGray);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "??%");
  text_layer_set_font(s_battery_layer, s_battery_font);
  
  // Add battery layer as child layer to Window's root Layer
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_battery_layer);
  
  // Unload GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  fonts_unload_custom_font(s_battery_font);
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
  
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);  
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Register battery callback
  battery_state_service_subscribe(battery_callback);
  
  // Show battery at start
  battery_callback(battery_state_service_peek());
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