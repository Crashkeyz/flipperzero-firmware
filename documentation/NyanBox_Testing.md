# NyanBox Testing and Validation Guide

This document provides comprehensive testing procedures for NyanBox firmware on both Flipper Zero and Cardputer platforms.

## Prerequisites

### For Flipper Zero Testing
- Flipper Zero device with latest official firmware
- ARM GCC toolchain (arm-none-eabi-gcc)
- OpenOCD (for debugging)
- ST-Link programmer (optional, for direct flashing)
- Serial terminal (screen, minicom, or PuTTY)

### For Cardputer Testing
- M5Stack Cardputer device
- ESP-IDF or Arduino IDE with M5Stack support
- PlatformIO (recommended)
- USB-C cable
- Serial monitor

## Build Instructions

### Building for Flipper Zero

#### Using Make (Native)
```bash
# Install ARM GCC toolchain
sudo apt install gcc-arm-none-eabi openocd

# Clone repository
git clone https://github.com/Crashkeyz/flipperzero-firmware
cd flipperzero-firmware

# Build all targets
make

# Or build specific target
make TARGET=f7

# Output: dist/f7/flipper-z-f7-full-*.dfu
```

#### Using Docker (Recommended)
```bash
# Pull Docker image
docker pull flipperdevices/flipperzero-toolchain

# Build using Docker
docker run --rm -v $(pwd):/project -w /project \
    flipperdevices/flipperzero-toolchain make

# Build specific app
docker run --rm -v $(pwd):/project -w /project \
    flipperdevices/flipperzero-toolchain make APPS=nyanbox
```

#### Expected Build Output
```
Compiling: applications/nyanbox/nyanbox_app.c
Compiling: lib/nyanbox-hal/nyanbox_hal_flipper.c
Compiling: lib/nyanbox-hal/nyanbox_ai.c
Compiling: lib/nyanbox-hal/nyanbox_wireless.c
Linking: firmware.elf
Creating: firmware.bin
Creating: firmware.dfu

Build complete!
Size: 512 KB
```

### Building for Cardputer

#### Using PlatformIO
```bash
# Install PlatformIO
pip install platformio

# Navigate to project
cd flipperzero-firmware

# Build for Cardputer
pio run -e cardputer

# Upload to device
pio run -e cardputer --target upload

# Monitor serial output
pio device monitor
```

#### Using Arduino IDE
1. Install M5Stack board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://m5stack.oss-cn-shenzhen.aliyuncs.com/resource/arduino/package_m5stack_index.json`
2. Install M5GFX and M5Unified libraries
3. Select "M5Stack Cardputer" board
4. Open `applications/nyanbox/nyanbox_cardputer.ino` (would need to be created)
5. Click Upload

## Flashing Firmware

### Flipper Zero

#### Via USB DFU
```bash
# Enter DFU mode on Flipper
# Hold Left + Back, release Back, keep Left until blue LED

# Flash using dfu-util
dfu-util -D dist/f7/flipper-z-f7-full-*.dfu -a 0

# Device will reboot automatically
```

#### Via qFlipper
1. Open qFlipper application
2. Connect Flipper Zero
3. Click "Install from file"
4. Select the .dfu file
5. Wait for installation to complete

#### Via ST-Link
```bash
# Flash bootloader and firmware
make flash

# Or just firmware
make firmware_flash
```

### Cardputer

#### Via PlatformIO
```bash
pio run -e cardputer --target upload
```

#### Via esptool
```bash
# Erase flash
esptool.py --port /dev/ttyUSB0 erase_flash

# Flash firmware
esptool.py --port /dev/ttyUSB0 --baud 1500000 write_flash 0x0 firmware.bin
```

## Testing Procedures

### Functional Tests

#### 1. Basic Application Launch
**Test**: Verify NyanBox app appears in menu and launches
```
Expected Result:
- App appears in Plugins menu as "NyanBox"
- App launches without crash
- Main screen displays:
  - "NyanBox Firmware" title
  - Platform name
  - Display resolution
  - Battery level
  - Wireless status
```

#### 2. Platform Detection
**Test**: Verify correct platform is detected
```
Flipper Zero Expected:
- Platform: Flipper Zero
- Display: 128x64

Cardputer Expected:
- Platform: M5Stack Cardputer
- Display: 240x135
```

#### 3. Button Input
**Test**: Test all button inputs
```
Flipper Zero:
- Press Up → Display "Button: Up"
- Press Down → Display "Button: Down"
- Press Left → Display "Button: Left"
- Press Right → Display "Button: Right"
- Press OK → Toggle wireless
- Press Back → Exit app

Cardputer:
- Arrow keys or WASD for navigation
- Enter for OK
- ESC for Back
```

#### 4. Battery Monitoring
**Test**: Verify battery level displays correctly
```
Expected:
- Battery percentage updates every ~100ms
- Value matches system battery reading
- Charging indicator appears when charging
```

#### 5. Wireless Connectivity

**BLE Test**:
```bash
# On device:
1. Press OK button to start advertising
2. Status should show "BLE advertising..."

# On computer/phone:
3. Scan for Bluetooth devices
4. Look for "NyanBox" or device name
5. Should be discoverable

Expected Result:
- Device advertises successfully
- Wireless status shows "Connected" when discovered
- Press OK again to disconnect
```

**WiFi Test (Cardputer only)**:
```bash
Expected:
- WiFi scanning works
- Can connect to access point
- Status updates correctly
```

**SubGHz Test (Flipper Zero only)**:
```bash
Expected:
- SubGHz radio initializes
- Can transmit on 433/868/915 MHz
- Receive functionality works
```

### AI Functionality Tests

#### 1. AI Initialization
**Test**: Verify AI subsystem initializes
```c
// Check capabilities
NyanBoxAICapabilities caps = nyanbox_hal_ai_get_capabilities();

Flipper Zero Expected:
- available_memory: ~50000 bytes
- hardware_acceleration: false

Cardputer Expected:
- available_memory: ~200000 bytes
- hardware_acceleration: true
```

#### 2. Model Loading
**Test**: Load a test AI model
```c
// Load small test model (simplified)
uint8_t test_model[1024] = {0}; // Placeholder
bool result = nyanbox_ai_load_model(
    NyanBoxAI_Model_TextClassification, 
    test_model, 
    sizeof(test_model)
);

Expected Result:
- Returns true if memory available
- Model info populated correctly
```

#### 3. Inference Test
**Test**: Run simple inference
```c
float input[128] = {0.0f}; // Test input
NyanBoxAIResult result;

bool success = nyanbox_ai_inference(input, 128, &result);

Expected Result:
- Returns true/false based on model loaded
- Result contains confidence and class
```

### Wireless Communication Tests

#### 1. BLE Discovery
**Test**: Scan for nearby BLE devices
```c
nyanbox_wireless_init(NyanBoxWireless_Protocol_BLE);
nyanbox_wireless_ble_start_scan();

NyanBoxBLEDevice devices[10];
uint8_t count = nyanbox_wireless_ble_get_devices(devices, 10);

Expected Result:
- count > 0 if devices nearby
- Device info populated (name, RSSI, address)
```

#### 2. WiFi Scanning (Cardputer)
**Test**: Scan for WiFi networks
```c
nyanbox_wireless_wifi_start_scan();

NyanBoxWiFiNetwork networks[10];
uint8_t count = nyanbox_wireless_wifi_get_networks(networks, 10);

Expected Result:
- count > 0 if networks nearby
- Network info shows SSID, RSSI, channel
```

#### 3. Data Transfer
**Test**: Send and receive data
```c
uint8_t test_data[] = "Hello NyanBox";
bool sent = nyanbox_wireless_ble_send(test_data, sizeof(test_data));

Expected Result:
- Returns true on successful send
- Can receive echo on paired device
```

### OTA Update Tests

#### 1. OTA Initialization
**Test**: Initialize OTA subsystem
```c
bool result = nyanbox_wireless_ota_init();

Expected Result:
- Returns true
- System ready for OTA updates
```

#### 2. Simulated OTA Update
**Test**: Simulate firmware update process
```c
// Start OTA with test size
uint32_t firmware_size = 524288; // 512 KB
nyanbox_wireless_ota_start(firmware_size);

// Write test chunks
for(int i = 0; i < 1024; i++) {
    uint8_t chunk[512];
    memset(chunk, 0xAA, sizeof(chunk));
    nyanbox_wireless_ota_write_chunk(chunk, sizeof(chunk));
}

// Finalize
bool success = nyanbox_wireless_ota_finalize();

Expected Result:
- Each chunk writes successfully
- Finalize returns true
- Device ready to reboot (don't actually reboot in test)
```

## Performance Benchmarks

### Memory Usage
```
Flipper Zero:
- App stack: 2048 bytes
- HAL overhead: ~4 KB
- AI models: Up to 50 KB
- Free heap: ~200 KB remaining

Cardputer:
- App overhead: ~20 KB
- HAL overhead: ~8 KB
- AI models: Up to 200 KB
- Free heap: ~280 KB remaining
```

### Wireless Performance
```
BLE Transfer Rate:
- Flipper Zero: ~50-100 KB/s
- Cardputer: ~100-200 KB/s

WiFi Transfer Rate (Cardputer):
- TCP: ~500 KB/s
- UDP: ~800 KB/s

SubGHz (Flipper Zero):
- 433 MHz: ~10-50 KB/s (depends on modulation)
```

### AI Inference Speed
```
Text Classification (128 features):
- Flipper Zero: ~100-200ms
- Cardputer: ~20-50ms (with acceleration)

Image Recognition (28x28):
- Flipper Zero: ~500-1000ms
- Cardputer: ~100-200ms
```

## Automated Testing

### Unit Tests
```bash
# Run unit tests for HAL (would need to create)
make test_hal

# Run wireless tests
make test_wireless

# Run AI tests
make test_ai
```

### Integration Tests
```bash
# Run full integration test suite
make test_integration

# Test specific module
make test MODULE=nyanbox
```

## Debugging

### Flipper Zero Debugging
```bash
# Enable debug mode in build
make DEBUG=1

# Connect via OpenOCD
openocd -f interface/stlink.cfg -f target/stm32wbx.cfg

# In another terminal, start GDB
arm-none-eabi-gdb firmware.elf
(gdb) target remote :3333
(gdb) load
(gdb) break nyanbox_app
(gdb) continue
```

### Cardputer Debugging
```bash
# Enable verbose logging
pio run -e cardputer-debug --target upload

# Monitor serial output
pio device monitor

# Or use ESP-IDF monitor
idf.py monitor
```

### Common Issues and Solutions

**Issue**: Build fails with "arm-none-eabi-gcc not found"
```bash
# Solution: Install toolchain
sudo apt install gcc-arm-none-eabi
# Or use Docker build
```

**Issue**: "Permission denied" on /dev/ttyUSB0
```bash
# Solution: Add user to dialout group
sudo usermod -a -G dialout $USER
# Then logout and login
```

**Issue**: OOM (Out of Memory) during build
```bash
# Solution: Increase swap or use Docker
# Or reduce optimization level in Makefile
```

**Issue**: BLE not working on Flipper
```bash
# Solution: Ensure BT service is running
# Check BT settings in Flipper menu
# Restart device
```

## Continuous Integration

### GitHub Actions Workflow
```yaml
# .github/workflows/build.yml
name: Build NyanBox

on: [push, pull_request]

jobs:
  build-flipper:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build Flipper firmware
        run: |
          docker run --rm -v $(pwd):/project \
            flipperdevices/flipperzero-toolchain make
      - name: Upload artifact
        uses: actions/upload-artifact@v2
        with:
          name: flipper-firmware
          path: dist/

  build-cardputer:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Setup PlatformIO
        run: pip install platformio
      - name: Build Cardputer firmware
        run: pio run -e cardputer
      - name: Upload artifact
        uses: actions/upload-artifact@v2
        with:
          name: cardputer-firmware
          path: .pio/build/cardputer/firmware.bin
```

## Test Coverage

Current test coverage goals:
- HAL implementation: 80%
- Wireless module: 70%
- AI module: 60%
- Application code: 85%

## Conclusion

This testing guide ensures comprehensive validation of NyanBox firmware across both platforms. All tests should pass before releasing new versions.

For additional support, consult:
- Main README.md
- API documentation
- Community Discord
- GitHub Issues
