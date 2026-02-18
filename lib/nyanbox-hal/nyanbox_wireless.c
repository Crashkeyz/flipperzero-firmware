#include "nyanbox_wireless.h"
#include "nyanbox_hal.h"
#include <string.h>

/**
 * NyanBox Wireless Communication Module Implementation
 */

static NyanBoxWirelessProtocol g_current_protocol = NyanBoxWireless_Protocol_BLE;
static NyanBoxWirelessState g_state = NyanBoxWireless_State_Idle;
static NyanBoxWirelessDataCallback g_data_callback = NULL;
static void* g_data_context = NULL;
static NyanBoxWirelessEventCallback g_event_callback = NULL;
static void* g_event_context = NULL;

// Simulated device lists
static NyanBoxBLEDevice g_ble_devices[10];
static uint8_t g_ble_device_count = 0;
static NyanBoxWiFiNetwork g_wifi_networks[10];
static uint8_t g_wifi_network_count = 0;

static void set_state(NyanBoxWirelessState new_state) {
    g_state = new_state;
    if(g_event_callback) {
        g_event_callback(new_state, g_event_context);
    }
}

bool nyanbox_wireless_init(NyanBoxWirelessProtocol protocol) {
    g_current_protocol = protocol;
    
    bool success = false;
    switch(protocol) {
        case NyanBoxWireless_Protocol_BLE:
            success = nyanbox_hal_wireless_init(NyanBoxWireless_BLE);
            break;
        case NyanBoxWireless_Protocol_WiFi:
            success = nyanbox_hal_wireless_init(NyanBoxWireless_WiFi);
            break;
        case NyanBoxWireless_Protocol_SubGHz_433MHz:
        case NyanBoxWireless_Protocol_SubGHz_868MHz:
        case NyanBoxWireless_Protocol_SubGHz_915MHz:
            success = nyanbox_hal_wireless_init(NyanBoxWireless_SubGHz);
            break;
    }
    
    if(success) {
        set_state(NyanBoxWireless_State_Idle);
    } else {
        set_state(NyanBoxWireless_State_Error);
    }
    
    return success;
}

void nyanbox_wireless_shutdown(void) {
    nyanbox_hal_wireless_disconnect();
    set_state(NyanBoxWireless_State_Idle);
}

NyanBoxWirelessState nyanbox_wireless_get_state(void) {
    return g_state;
}

void nyanbox_wireless_set_event_callback(NyanBoxWirelessEventCallback callback, void* context) {
    g_event_callback = callback;
    g_event_context = context;
}

void nyanbox_wireless_set_data_callback(NyanBoxWirelessDataCallback callback, void* context) {
    g_data_callback = callback;
    g_data_context = context;
}

/**
 * BLE Functions
 */
bool nyanbox_wireless_ble_start_scan(void) {
    if(g_current_protocol != NyanBoxWireless_Protocol_BLE) {
        return false;
    }
    
    set_state(NyanBoxWireless_State_Scanning);
    
    // Simulate finding some devices
    g_ble_device_count = 3;
    strncpy(g_ble_devices[0].name, "NyanBox-1", sizeof(g_ble_devices[0].name));
    g_ble_devices[0].rssi = -45;
    g_ble_devices[0].connectable = true;
    
    strncpy(g_ble_devices[1].name, "NyanBox-2", sizeof(g_ble_devices[1].name));
    g_ble_devices[1].rssi = -67;
    g_ble_devices[1].connectable = true;
    
    strncpy(g_ble_devices[2].name, "Unknown", sizeof(g_ble_devices[2].name));
    g_ble_devices[2].rssi = -82;
    g_ble_devices[2].connectable = false;
    
    set_state(NyanBoxWireless_State_Idle);
    return true;
}

uint8_t nyanbox_wireless_ble_get_devices(NyanBoxBLEDevice* devices, uint8_t max_devices) {
    uint8_t count = g_ble_device_count < max_devices ? g_ble_device_count : max_devices;
    memcpy(devices, g_ble_devices, count * sizeof(NyanBoxBLEDevice));
    return count;
}

bool nyanbox_wireless_ble_connect(const uint8_t* address) {
    set_state(NyanBoxWireless_State_Connecting);
    
    // Simulate connection
    nyanbox_hal_ble_start_advertising("NyanBox");
    
    set_state(NyanBoxWireless_State_Connected);
    return true;
}

bool nyanbox_wireless_ble_disconnect(void) {
    nyanbox_hal_ble_stop_advertising();
    set_state(NyanBoxWireless_State_Idle);
    return true;
}

bool nyanbox_wireless_ble_send(const uint8_t* data, uint16_t length) {
    if(g_state != NyanBoxWireless_State_Connected) {
        return false;
    }
    
    set_state(NyanBoxWireless_State_Transmitting);
    bool success = nyanbox_hal_ble_send_data(data, length);
    set_state(NyanBoxWireless_State_Connected);
    
    return success;
}

/**
 * WiFi Functions
 */
bool nyanbox_wireless_wifi_start_scan(void) {
    if(g_current_protocol != NyanBoxWireless_Protocol_WiFi) {
        return false;
    }
    
    set_state(NyanBoxWireless_State_Scanning);
    
    // Simulate finding some networks
    g_wifi_network_count = 2;
    strncpy(g_wifi_networks[0].ssid, "NyanBox-WiFi", sizeof(g_wifi_networks[0].ssid));
    g_wifi_networks[0].rssi = -50;
    g_wifi_networks[0].channel = 6;
    g_wifi_networks[0].encrypted = true;
    
    strncpy(g_wifi_networks[1].ssid, "Guest-Network", sizeof(g_wifi_networks[1].ssid));
    g_wifi_networks[1].rssi = -75;
    g_wifi_networks[1].channel = 11;
    g_wifi_networks[1].encrypted = false;
    
    set_state(NyanBoxWireless_State_Idle);
    return true;
}

uint8_t nyanbox_wireless_wifi_get_networks(NyanBoxWiFiNetwork* networks, uint8_t max_networks) {
    uint8_t count = g_wifi_network_count < max_networks ? g_wifi_network_count : max_networks;
    memcpy(networks, g_wifi_networks, count * sizeof(NyanBoxWiFiNetwork));
    return count;
}

bool nyanbox_wireless_wifi_connect(const char* ssid, const char* password) {
    set_state(NyanBoxWireless_State_Connecting);
    // Simulate connection
    set_state(NyanBoxWireless_State_Connected);
    return true;
}

bool nyanbox_wireless_wifi_disconnect(void) {
    set_state(NyanBoxWireless_State_Idle);
    return true;
}

bool nyanbox_wireless_wifi_send(const uint8_t* data, uint16_t length, const char* host, uint16_t port) {
    if(g_state != NyanBoxWireless_State_Connected) {
        return false;
    }
    
    set_state(NyanBoxWireless_State_Transmitting);
    bool success = nyanbox_hal_wireless_send(data, length);
    set_state(NyanBoxWireless_State_Connected);
    
    return success;
}

/**
 * SubGHz Functions
 */
bool nyanbox_wireless_subghz_set_frequency(uint32_t frequency) {
    // Would configure the radio to the specified frequency
    return true;
}

bool nyanbox_wireless_subghz_start_rx(void) {
    set_state(NyanBoxWireless_State_Receiving);
    return true;
}

bool nyanbox_wireless_subghz_start_tx(void) {
    set_state(NyanBoxWireless_State_Transmitting);
    return true;
}

bool nyanbox_wireless_subghz_send(const uint8_t* data, uint16_t length) {
    return nyanbox_hal_wireless_send(data, length);
}

bool nyanbox_wireless_subghz_get_signal_info(NyanBoxSubGHzSignal* signal) {
    if(!signal) {
        return false;
    }
    
    // Would get actual signal information from radio
    signal->frequency = 433920000;
    signal->rssi = -65;
    signal->modulation = 0; // OOK
    signal->data_rate = 4800;
    
    return true;
}

/**
 * OTA Update Support
 */
static uint32_t g_ota_firmware_size = 0;
static uint32_t g_ota_bytes_written = 0;

bool nyanbox_wireless_ota_init(void) {
    g_ota_firmware_size = 0;
    g_ota_bytes_written = 0;
    return true;
}

bool nyanbox_wireless_ota_start(uint32_t firmware_size) {
    g_ota_firmware_size = firmware_size;
    g_ota_bytes_written = 0;
    return true;
}

bool nyanbox_wireless_ota_write_chunk(const uint8_t* data, uint16_t length) {
    if(g_ota_bytes_written + length > g_ota_firmware_size) {
        return false;
    }
    
    // Would write to flash or storage
    g_ota_bytes_written += length;
    
    return true;
}

bool nyanbox_wireless_ota_finalize(void) {
    if(g_ota_bytes_written != g_ota_firmware_size) {
        return false;
    }
    
    // Would verify and activate new firmware
    return true;
}

void nyanbox_wireless_ota_abort(void) {
    g_ota_firmware_size = 0;
    g_ota_bytes_written = 0;
}
