#include <pebble.h>

#define BUFSIZE 20
static Window *sMyWindow;
static TextLayer *sTimeTextLayer;
static TextLayer *sDateTextLayer;
static TextLayer *sBatteryTextLayer;
// static GFont sTimeFont;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time();
void getRoman(char* buffer, int buflen, int number);
void romanStrFromTime(char* buffer, int numChars, struct tm *tick_time, bool use24Hr);
void romanDateStrFromTime(char* buffer, int numChars, struct tm *tick_time);
void romanBatteryStr(char* buffer, int numChars);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static char sTimeTextBuffer[BUFSIZE];
static char sDateTextBuffer[BUFSIZE];
static char sBatteryTextBuffer[BUFSIZE];

static void main_window_load(Window *window) 
{
    // Create GBitmap, then set to created BitmapLayer
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
    s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

    sTimeTextLayer = text_layer_create(GRect(5, 65, 139, 50));
    sDateTextLayer = text_layer_create(GRect(12, 6, 120, 20));
    sBatteryTextLayer = text_layer_create(GRect(54, 146, 36, 16));

    text_layer_set_background_color(sTimeTextLayer, GColorClear);
    text_layer_set_text_color(sTimeTextLayer, GColorBlack);
    text_layer_set_font(sTimeTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_text_alignment(sTimeTextLayer, GTextAlignmentCenter);

    text_layer_set_background_color(sDateTextLayer, GColorClear);
    text_layer_set_text_color(sDateTextLayer, GColorWhite);
    text_layer_set_font(sDateTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(sDateTextLayer, GTextAlignmentCenter);

    text_layer_set_background_color(sBatteryTextLayer, GColorClear);
    text_layer_set_text_color(sBatteryTextLayer, GColorWhite);
    text_layer_set_font(sBatteryTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(sBatteryTextLayer, GTextAlignmentCenter);

    layer_add_child(window_get_root_layer(window), text_layer_get_layer(sTimeTextLayer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(sDateTextLayer));
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(sBatteryTextLayer));

}

static void main_window_unload(Window *window) 
{
    text_layer_destroy(sTimeTextLayer);
    text_layer_destroy(sDateTextLayer);
    text_layer_destroy(sBatteryTextLayer);
    
    //    fonts_unload_custom_font(sTimeFont);
    // Destroy GBitmap
    gbitmap_destroy(s_background_bitmap);

    // Destroy BitmapLayer
    bitmap_layer_destroy(s_background_layer);
}

static void update_time() 
{
     // Get a tm structure
     time_t temp = time(NULL); 
     struct tm *tick_time = localtime(&temp);
    
    romanStrFromTime(sTimeTextBuffer, BUFSIZE, tick_time, clock_is_24h_style());
    romanDateStrFromTime(sDateTextBuffer, BUFSIZE, tick_time);
    romanBatteryStr(sBatteryTextBuffer, BUFSIZE);

  // Display this time on the TextLayer
    text_layer_set_text(sTimeTextLayer, sTimeTextBuffer);
    text_layer_set_text(sDateTextLayer, sDateTextBuffer);
    text_layer_set_text(sBatteryTextLayer, sBatteryTextBuffer);
}

void romanStrFromTime(char* buffer, int numChars, struct tm *tick_time, bool use24Hr)
{
    char strHrs[8];
    char strMin[8];
    
    if(clock_is_24h_style() == true) 
    {
        getRoman(strHrs, 8, tick_time->tm_hour);
    }
    else
    {
        getRoman(strHrs, 8, tick_time->tm_hour % 12);        
    }
    
    getRoman(strMin, 8, tick_time->tm_min);
    
    strcpy(buffer,strHrs);
    strcat(buffer,":");
    strcat(buffer,strMin);

}

void romanDateStrFromTime(char* buffer, int numChars, struct tm *tick_time)
{
    char strMonth[8];
    char strDay[8];
    getRoman(strMonth, 8, tick_time->tm_mon + 1);
    getRoman(strDay, 8, tick_time->tm_mday);
    
    strcpy(buffer,strMonth);
    strcat(buffer,"/");
    strcat(buffer,strDay);
}

void romanBatteryStr(char* buffer, int numChars)
{    
    static char s_battery_buffer[16];

    BatteryChargeState charge_state = battery_state_service_peek();
    if (charge_state.is_charging) 
    {
        snprintf(s_battery_buffer, sizeof(s_battery_buffer), "Chg");
    } 
    else 
    {
        getRoman(s_battery_buffer, sizeof(s_battery_buffer), (int)(charge_state.charge_percent / 10));
    }
    
    
    strcpy(buffer,s_battery_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) 
{
    update_time();
}

void handle_init(void) 
{
   sMyWindow = window_create();
    
   window_set_window_handlers(sMyWindow, (WindowHandlers) 
   {
       .load = main_window_load,
       .unload = main_window_unload
   });

   window_stack_push(sMyWindow, true);
   update_time();
   tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}


void handle_deinit(void) 
{
   window_destroy(sMyWindow);
}

int main(void) 
{
    handle_init();
    app_event_loop();
    handle_deinit();
}

void getRoman(char* buffer, int buflen, int number) 
{
    
    char* roman[] = {"M","XM","CM","D","XD","CD","C","XC","L","XL","X","IX","V","IV","I"};
    int arab[] = {1000, 990, 900, 500, 490, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
    char* result = calloc(buflen, sizeof (char));
    int i = 0;
    while (number > 0 || 15 == (i - 1)) 
    {
        while ((number - arab[i]) >= 0) 
        {
            number -= arab[i];
            strcat(result,roman[i]);
        }
        i++;
    }
    strcpy(buffer,result);
    free(result);
}