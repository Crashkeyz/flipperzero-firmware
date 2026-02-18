#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NyanBox Hardware Abstraction Layer
 * Platform-agnostic interface for NyanBox firmware
 * Supports: Flipper Zero, Cardputer by M5Stack
 */

typedef enum {
    NyanBoxPlatform_FlipperZero,
    NyanBoxPlatform_Cardputer,
    NyanBoxPlatform_Unknown
} NyanBoxPlatform;

typedef enum {
    NyanBoxButton_Up,
    NyanBoxButton_Down,
    NyanBoxButton_Left,
    NyanBoxButton_Right,
    NyanBoxButton_Ok,
    NyanBoxButton_Back
} NyanBoxButton;

typedef enum {
    NyanBoxButtonEvent_Press,
    NyanBoxButtonEvent_Release,
    NyanBoxButtonEvent_LongPress
} NyanBoxButtonEvent;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
} NyanBoxDisplayInfo;

typedef void (*NyanBoxButtonCallback)(NyanBoxButton button, NyanBoxButtonEvent event, void* context);

/**
 * Platform Detection
 */
NyanBoxPlatform nyanbox_hal_get_platform(void);
const char* nyanbox_hal_get_platform_name(void);

/**
 * Display Management
 */
void nyanbox_hal_display_init(void);
void nyanbox_hal_display_clear(void);
void nyanbox_hal_display_draw_pixel(uint16_t x, uint16_t y, bool on);
void nyanbox_hal_display_draw_text(uint16_t x, uint16_t y, const char* text);
void nyanbox_hal_display_update(void);
NyanBoxDisplayInfo nyanbox_hal_display_get_info(void);

/**
 * Button/Input Management
 */
void nyanbox_hal_input_init(NyanBoxButtonCallback callback, void* context);
bool nyanbox_hal_input_get_state(NyanBoxButton button);

/**
 * Wireless Communication
 */
typedef enum {
    NyanBoxWireless_BLE,
    NyanBoxWireless_WiFi,
    NyanBoxWireless_SubGHz
} NyanBoxWirelessType;

bool nyanbox_hal_wireless_init(NyanBoxWirelessType type);
bool nyanbox_hal_wireless_send(const uint8_t* data, uint16_t length);
int16_t nyanbox_hal_wireless_receive(uint8_t* buffer, uint16_t max_length);
bool nyanbox_hal_wireless_is_connected(void);
void nyanbox_hal_wireless_disconnect(void);

/**
 * Bluetooth Deployment Support
 */
typedef void (*NyanBoxBLEDataCallback)(const uint8_t* data, uint16_t length, void* context);

bool nyanbox_hal_ble_init(void);
bool nyanbox_hal_ble_start_advertising(const char* device_name);
bool nyanbox_hal_ble_stop_advertising(void);
void nyanbox_hal_ble_set_data_callback(NyanBoxBLEDataCallback callback, void* context);
bool nyanbox_hal_ble_send_data(const uint8_t* data, uint16_t length);

/**
 * GPIO Management
 */
typedef enum {
    NyanBoxGPIO_Mode_Input,
    NyanBoxGPIO_Mode_Output,
    NyanBoxGPIO_Mode_Analog
} NyanBoxGPIOMode;

bool nyanbox_hal_gpio_init(uint8_t pin, NyanBoxGPIOMode mode);
void nyanbox_hal_gpio_write(uint8_t pin, bool state);
bool nyanbox_hal_gpio_read(uint8_t pin);

/**
 * AI Functionality Support
 */
typedef struct {
    uint32_t model_size;
    uint32_t available_memory;
    bool hardware_acceleration;
} NyanBoxAICapabilities;

NyanBoxAICapabilities nyanbox_hal_ai_get_capabilities(void);
bool nyanbox_hal_ai_init(void);
bool nyanbox_hal_ai_load_model(const uint8_t* model_data, uint32_t size);
bool nyanbox_hal_ai_inference(const float* input, uint32_t input_size, float* output, uint32_t output_size);

/**
 * Power Management
 */
uint8_t nyanbox_hal_power_get_battery_level(void);
bool nyanbox_hal_power_is_charging(void);
void nyanbox_hal_power_sleep(void);

#ifdef __cplusplus
}
#endif
