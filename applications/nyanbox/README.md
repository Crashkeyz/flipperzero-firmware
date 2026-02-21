# NyanBox Firmware - Multi-Platform Support

This directory contains the NyanBox firmware adaptation that works on both **Flipper Zero** and **M5Stack Cardputer** platforms.

## Overview

NyanBox firmware has been adapted to run on multiple hardware platforms with full support for:
- ✅ Wireless Communication (BLE, WiFi*, SubGHz*)
- ✅ AI Functionality (Text Classification, Image Recognition, Anomaly Detection, Voice Commands)
- ✅ Bluetooth Deployment for OTA updates
- ✅ Modular and maintainable architecture

*WiFi support available on Cardputer only
*SubGHz support available on Flipper Zero only

## Supported Platforms

### 1. Flipper Zero
- **MCU**: STM32WB55 (Dual-core Cortex-M4F + M0+)
- **Display**: 128x64 monochrome LCD
- **Wireless**: BLE, SubGHz (433/868/915 MHz)
- **Memory**: 256KB RAM, 1MB Flash
- **Features**: Full NyanBox feature set with SubGHz radio

### 2. M5Stack Cardputer
- **MCU**: ESP32-S3 (Dual-core Xtensa LX7)
- **Display**: 240x135 IPS LCD (16-bit color)
- **Wireless**: BLE, WiFi 2.4GHz
- **Memory**: 512KB SRAM, 8MB Flash, 2MB PSRAM
- **Features**: Full NyanBox feature set with WiFi and enhanced AI capabilities

## Architecture

The firmware uses a **Hardware Abstraction Layer (HAL)** to provide platform-agnostic APIs:

```
applications/nyanbox/          # Main NyanBox application
lib/nyanbox-hal/              # Platform abstraction layer
├── nyanbox_hal.h             # Main HAL interface
├── nyanbox_hal_flipper.c     # Flipper Zero implementation
├── nyanbox_hal_cardputer.c   # Cardputer implementation
├── nyanbox_ai.h/c            # AI functionality module
└── nyanbox_wireless.h/c      # Wireless communication module
```

## Features

### Wireless Communication
The wireless module supports multiple protocols:

**BLE (Bluetooth Low Energy)**
- Device scanning and discovery
- Connection management
- Data transfer
- OTA firmware updates

**WiFi (Cardputer only)**
- Network scanning
- AP connection
- TCP/UDP communication
- OTA updates over WiFi

**SubGHz (Flipper Zero only)**
- 433/868/915 MHz support
- Transmit and receive
- Signal analysis
- Multiple modulation schemes

### AI Functionality
Four AI model types are supported:

1. **Text Classification** - Categorize text input
2. **Image Recognition** - Identify objects in images
3. **Anomaly Detection** - Detect unusual patterns in sensor data
4. **Voice Commands** - Recognize spoken commands

AI models can be loaded dynamically and run on-device with:
- Flipper Zero: CPU-based inference
- Cardputer: ESP32-S3 vector acceleration

### Bluetooth Deployment
Both platforms support firmware deployment via Bluetooth:

1. Device advertises as "NyanBox"
2. Connect via BLE
3. Send firmware chunks
4. Automatic verification and installation
5. Device reboots with new firmware

## Building

### For Flipper Zero

```bash
# Standard Flipper build process
make APPS=nyanbox

# Or build everything
make
```

### For Cardputer

**Note**: Cardputer requires ESP32 toolchain. Two options:

**Option 1: PlatformIO**
```bash
# Install PlatformIO
pip install platformio

# Build for Cardputer
pio run -e cardputer
```

**Option 2: Arduino IDE**
```
1. Install M5Stack board support
2. Select "M5Stack Cardputer" board
3. Open nyanbox_app.ino
4. Compile and upload
```

## Usage

### On Flipper Zero

1. Navigate to Applications → NyanBox
2. Use directional buttons to navigate
3. Press OK to toggle wireless connection
4. Press Back to exit

### On Cardputer

1. Power on Cardputer
2. NyanBox app starts automatically (or select from menu)
3. Use keyboard or buttons to interact
4. ESC to exit

## API Reference

### Platform Detection
```c
NyanBoxPlatform platform = nyanbox_hal_get_platform();
const char* name = nyanbox_hal_get_platform_name();
```

### Display Management
```c
nyanbox_hal_display_init();
NyanBoxDisplayInfo info = nyanbox_hal_display_get_info();
nyanbox_hal_display_draw_text(10, 20, "Hello NyanBox!");
nyanbox_hal_display_update();
```

### Wireless Communication
```c
// BLE Example
nyanbox_wireless_init(NyanBoxWireless_Protocol_BLE);
nyanbox_wireless_ble_start_advertising("NyanBox");
nyanbox_wireless_ble_send(data, length);
```

### AI Inference
```c
// Initialize AI
nyanbox_ai_init();

// Load model
nyanbox_ai_load_model(NyanBoxAI_Model_TextClassification, model_data, size);

// Run inference
NyanBoxAIResult result;
nyanbox_ai_classify_text("Hello world", &result);
printf("Class: %s, Confidence: %.2f\n", result.class_name, result.confidence);
```

## OTA Updates

### Deploy firmware via Bluetooth

**From mobile app or computer:**

1. Scan for "NyanBox" device
2. Connect to device
3. Send OTA command with firmware size
4. Transfer firmware in chunks (max 512 bytes per packet)
5. Device verifies and installs automatically

**Protocol:**
```
1. Connect to NyanBox BLE service
2. Write to OTA characteristic: [START][size:4bytes]
3. Write chunks: [DATA][chunk:512bytes]
4. Write final: [END]
5. Device responds: [OK] or [ERROR]
```

## Platform Differences

| Feature | Flipper Zero | Cardputer |
|---------|-------------|-----------|
| Display | 128x64 mono | 240x135 color |
| Input | 5-way + Back | Full keyboard |
| WiFi | ❌ | ✅ |
| SubGHz | ✅ | ❌ |
| BLE | ✅ | ✅ |
| AI Acceleration | CPU only | Vector extensions |
| RAM for AI | ~50KB | ~200KB |
| Battery | 2000mAh | 1500mAh |

## Development

### Adding New Platforms

To add support for a new platform:

1. Create target directory: `firmware/targets/<platform>/`
2. Implement HAL: `lib/nyanbox-hal/nyanbox_hal_<platform>.c`
3. Define platform constants in `nyanbox_hal.h`
4. Update build system to include new target
5. Test all features on new platform

### Customization

The modular architecture allows easy customization:

- **Add new wireless protocols**: Extend `nyanbox_wireless.c`
- **Add new AI models**: Update `nyanbox_ai.c`
- **Custom UI**: Modify `nyanbox_app.c`
- **Platform-specific features**: Add to platform HAL implementation

## Troubleshooting

**Flipper Zero:**
- Ensure latest firmware is installed
- Check that BLE is not in use by other apps
- Verify SubGHz region settings

**Cardputer:**
- Ensure M5Stack libraries are installed
- Check ESP32 board support version
- Verify sufficient flash space for AI models

## Contributing

Contributions welcome! Please:
1. Follow existing code style
2. Test on both platforms
3. Update documentation
4. Submit pull request

## License

See main repository LICENSE file.

## Support

- Issues: Report on GitHub
- Discussions: Use GitHub Discussions
- Documentation: See `/documentation` folder

---

**NyanBox** - Multi-platform firmware with AI and wireless capabilities
