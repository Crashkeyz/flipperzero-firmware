#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NyanBox Wireless Communication Module
 * Supports BLE, WiFi, and SubGHz communication
 */

typedef enum {
    NyanBoxWireless_Protocol_BLE,
    NyanBoxWireless_Protocol_WiFi,
    NyanBoxWireless_Protocol_SubGHz_433MHz,
    NyanBoxWireless_Protocol_SubGHz_868MHz,
    NyanBoxWireless_Protocol_SubGHz_915MHz
} NyanBoxWirelessProtocol;

typedef enum {
    NyanBoxWireless_State_Idle,
    NyanBoxWireless_State_Scanning,
    NyanBoxWireless_State_Connecting,
    NyanBoxWireless_State_Connected,
    NyanBoxWireless_State_Transmitting,
    NyanBoxWireless_State_Receiving,
    NyanBoxWireless_State_Error
} NyanBoxWirelessState;

typedef struct {
    char ssid[32];
    int8_t rssi;
    uint8_t channel;
    bool encrypted;
} NyanBoxWiFiNetwork;

typedef struct {
    char name[32];
    uint8_t address[6];
    int8_t rssi;
    bool connectable;
} NyanBoxBLEDevice;

typedef struct {
    uint32_t frequency;
    int8_t rssi;
    uint8_t modulation;
    uint16_t data_rate;
} NyanBoxSubGHzSignal;

typedef void (*NyanBoxWirelessDataCallback)(const uint8_t* data, uint16_t length, void* context);
typedef void (*NyanBoxWirelessEventCallback)(NyanBoxWirelessState state, void* context);

/**
 * Initialize wireless subsystem
 */
bool nyanbox_wireless_init(NyanBoxWirelessProtocol protocol);

/**
 * Shutdown wireless subsystem
 */
void nyanbox_wireless_shutdown(void);

/**
 * Get current state
 */
NyanBoxWirelessState nyanbox_wireless_get_state(void);

/**
 * Set event callback
 */
void nyanbox_wireless_set_event_callback(NyanBoxWirelessEventCallback callback, void* context);

/**
 * Set data receive callback
 */
void nyanbox_wireless_set_data_callback(NyanBoxWirelessDataCallback callback, void* context);

/**
 * BLE Functions
 */
bool nyanbox_wireless_ble_start_scan(void);
uint8_t nyanbox_wireless_ble_get_devices(NyanBoxBLEDevice* devices, uint8_t max_devices);
bool nyanbox_wireless_ble_connect(const uint8_t* address);
bool nyanbox_wireless_ble_disconnect(void);
bool nyanbox_wireless_ble_send(const uint8_t* data, uint16_t length);

/**
 * WiFi Functions
 */
bool nyanbox_wireless_wifi_start_scan(void);
uint8_t nyanbox_wireless_wifi_get_networks(NyanBoxWiFiNetwork* networks, uint8_t max_networks);
bool nyanbox_wireless_wifi_connect(const char* ssid, const char* password);
bool nyanbox_wireless_wifi_disconnect(void);
bool nyanbox_wireless_wifi_send(const uint8_t* data, uint16_t length, const char* host, uint16_t port);

/**
 * SubGHz Functions
 */
bool nyanbox_wireless_subghz_set_frequency(uint32_t frequency);
bool nyanbox_wireless_subghz_start_rx(void);
bool nyanbox_wireless_subghz_start_tx(void);
bool nyanbox_wireless_subghz_send(const uint8_t* data, uint16_t length);
bool nyanbox_wireless_subghz_get_signal_info(NyanBoxSubGHzSignal* signal);

/**
 * OTA Update Support
 */
bool nyanbox_wireless_ota_init(void);
bool nyanbox_wireless_ota_start(uint32_t firmware_size);
bool nyanbox_wireless_ota_write_chunk(const uint8_t* data, uint16_t length);
bool nyanbox_wireless_ota_finalize(void);
void nyanbox_wireless_ota_abort(void);

#ifdef __cplusplus
}
#endif
