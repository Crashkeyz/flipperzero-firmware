#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include "../../lib/nyanbox-hal/nyanbox_hal.h"

/**
 * NyanBox Application for Flipper Zero
 * Main application implementing NyanBox features with AI and wireless capabilities
 */

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    NotificationApp* notifications;
    bool running;
    uint8_t battery_level;
    bool wireless_connected;
    char status_text[64];
} NyanBoxApp;

typedef enum {
    NyanBoxEventInput,
    NyanBoxEventTick,
} NyanBoxEventType;

typedef struct {
    NyanBoxEventType type;
    InputEvent input;
} NyanBoxEvent;

static void nyanbox_draw_callback(Canvas* canvas, void* ctx) {
    NyanBoxApp* app = ctx;
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    
    // Title
    canvas_draw_str(canvas, 2, 10, "NyanBox Firmware");
    
    // Platform info
    canvas_set_font(canvas, FontSecondary);
    const char* platform = nyanbox_hal_get_platform_name();
    char platform_str[64];
    snprintf(platform_str, sizeof(platform_str), "Platform: %s", platform);
    canvas_draw_str(canvas, 2, 22, platform_str);
    
    // Display info
    NyanBoxDisplayInfo display = nyanbox_hal_display_get_info();
    char display_str[64];
    snprintf(display_str, sizeof(display_str), "Display: %dx%d", display.width, display.height);
    canvas_draw_str(canvas, 2, 32, display_str);
    
    // Battery level
    char battery_str[32];
    snprintf(battery_str, sizeof(battery_str), "Battery: %d%%", app->battery_level);
    canvas_draw_str(canvas, 2, 42, battery_str);
    
    // Wireless status
    const char* wireless_status = app->wireless_connected ? "Connected" : "Disconnected";
    char wireless_str[64];
    snprintf(wireless_str, sizeof(wireless_str), "Wireless: %s", wireless_status);
    canvas_draw_str(canvas, 2, 52, wireless_str);
    
    // Status text
    canvas_draw_str(canvas, 2, 62, app->status_text);
}

static void nyanbox_input_callback(InputEvent* input_event, void* ctx) {
    NyanBoxApp* app = ctx;
    NyanBoxEvent event = {.type = NyanBoxEventInput, .input = *input_event};
    furi_message_queue_put(app->event_queue, &event, 0);
}

static void nyanbox_button_handler(NyanBoxButton button, NyanBoxButtonEvent event, void* context) {
    NyanBoxApp* app = context;
    
    if(event == NyanBoxButtonEvent_Press) {
        switch(button) {
            case NyanBoxButton_Up:
                snprintf(app->status_text, sizeof(app->status_text), "Button: Up");
                break;
            case NyanBoxButton_Down:
                snprintf(app->status_text, sizeof(app->status_text), "Button: Down");
                break;
            case NyanBoxButton_Left:
                snprintf(app->status_text, sizeof(app->status_text), "Button: Left");
                break;
            case NyanBoxButton_Right:
                snprintf(app->status_text, sizeof(app->status_text), "Button: Right");
                break;
            case NyanBoxButton_Ok:
                // Toggle wireless connection
                if(app->wireless_connected) {
                    nyanbox_hal_wireless_disconnect();
                    app->wireless_connected = false;
                    snprintf(app->status_text, sizeof(app->status_text), "Wireless disconnected");
                } else {
                    nyanbox_hal_wireless_init(NyanBoxWireless_BLE);
                    nyanbox_hal_ble_start_advertising("NyanBox");
                    app->wireless_connected = true;
                    snprintf(app->status_text, sizeof(app->status_text), "BLE advertising...");
                }
                break;
            case NyanBoxButton_Back:
                app->running = false;
                break;
        }
    }
}

int32_t nyanbox_app(void* p) {
    UNUSED(p);
    
    NyanBoxApp* app = malloc(sizeof(NyanBoxApp));
    app->event_queue = furi_message_queue_alloc(8, sizeof(NyanBoxEvent));
    app->running = true;
    app->battery_level = 0;
    app->wireless_connected = false;
    snprintf(app->status_text, sizeof(app->status_text), "Ready");
    
    // Initialize HAL
    nyanbox_hal_display_init();
    nyanbox_hal_input_init(nyanbox_button_handler, app);
    nyanbox_hal_ble_init();
    
    // Get battery level
    app->battery_level = nyanbox_hal_power_get_battery_level();
    
    // Setup GUI
    app->gui = furi_record_open("gui");
    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, nyanbox_draw_callback, app);
    view_port_input_callback_set(app->view_port, nyanbox_input_callback, app);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    // Setup notifications
    app->notifications = furi_record_open("notification");
    
    NyanBoxEvent event;
    while(app->running) {
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == NyanBoxEventInput) {
                // Map Flipper input to NyanBox button
                NyanBoxButton button;
                switch(event.input.key) {
                    case InputKeyUp:
                        button = NyanBoxButton_Up;
                        break;
                    case InputKeyDown:
                        button = NyanBoxButton_Down;
                        break;
                    case InputKeyLeft:
                        button = NyanBoxButton_Left;
                        break;
                    case InputKeyRight:
                        button = NyanBoxButton_Right;
                        break;
                    case InputKeyOk:
                        button = NyanBoxButton_Ok;
                        break;
                    case InputKeyBack:
                        button = NyanBoxButton_Back;
                        break;
                    default:
                        continue;
                }
                
                NyanBoxButtonEvent btn_event;
                switch(event.input.type) {
                    case InputTypePress:
                        btn_event = NyanBoxButtonEvent_Press;
                        break;
                    case InputTypeRelease:
                        btn_event = NyanBoxButtonEvent_Release;
                        break;
                    case InputTypeLong:
                        btn_event = NyanBoxButtonEvent_LongPress;
                        break;
                    default:
                        continue;
                }
                
                nyanbox_button_handler(button, btn_event, app);
                view_port_update(app->view_port);
            }
        } else {
            // Periodic update
            app->battery_level = nyanbox_hal_power_get_battery_level();
            app->wireless_connected = nyanbox_hal_wireless_is_connected();
            view_port_update(app->view_port);
        }
    }
    
    // Cleanup
    if(app->wireless_connected) {
        nyanbox_hal_wireless_disconnect();
    }
    
    view_port_enabled_set(app->view_port, false);
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close("gui");
    furi_record_close("notification");
    furi_message_queue_free(app->event_queue);
    free(app);
    
    return 0;
}
