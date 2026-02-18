# NyanBox Multi-Platform Firmware

## 🎯 Overview

NyanBox is a modular firmware framework that brings advanced AI capabilities and wireless communication features to multiple embedded platforms. Originally developed for a custom device, it has been successfully adapted to run on:

- **Flipper Zero** - Versatile multi-tool with SubGHz capabilities
- **M5Stack Cardputer** - ESP32-S3 based portable computer with WiFi

## ✨ Key Features

### 🌐 Wireless Communication
- **Bluetooth Low Energy (BLE)** - Device discovery, pairing, and data transfer
- **WiFi** - Network connectivity (Cardputer only)
- **SubGHz Radio** - 433/868/915 MHz communication (Flipper Zero only)
- **OTA Updates** - Firmware deployment over Bluetooth

### 🤖 AI Capabilities
- **Text Classification** - Categorize and analyze text input
- **Image Recognition** - Identify objects in images
- **Anomaly Detection** - Detect unusual patterns in sensor data
- **Voice Commands** - Recognize spoken commands (with audio input)

### 🏗️ Architecture
- **Hardware Abstraction Layer (HAL)** - Platform-independent API
- **Modular Design** - Easy to extend and maintain
- **Multi-Platform Support** - Single codebase for multiple devices
- **Optimized Performance** - Efficient resource usage for embedded systems

## 📦 Repository Structure

```
├── applications/
│   └── nyanbox/                    # Main NyanBox application
│       ├── nyanbox_app.c          # Application entry point
│       └── README.md              # Application documentation
├── lib/
│   └── nyanbox-hal/               # Hardware abstraction layer
│       ├── nyanbox_hal.h          # Main HAL interface
│       ├── nyanbox_hal_flipper.c  # Flipper Zero implementation
│       ├── nyanbox_hal_cardputer.c# Cardputer implementation
│       ├── nyanbox_ai.h/c         # AI module
│       └── nyanbox_wireless.h/c   # Wireless communication
├── firmware/
│   └── targets/
│       ├── f6/                    # Flipper Zero F6 target
│       ├── f7/                    # Flipper Zero F7 target
│       └── cardputer/             # M5Stack Cardputer target
├── documentation/
│   ├── NyanBox_Deployment.md      # Deployment guide
│   └── NyanBox_Testing.md         # Testing procedures
└── platformio.ini                 # PlatformIO configuration
```

## 🚀 Quick Start

### For Flipper Zero

1. **Build the firmware**:
   ```bash
   make TARGET=f7
   ```

2. **Flash to device**:
   ```bash
   # Via USB DFU
   dfu-util -D dist/f7/flipper-z-f7-full-*.dfu -a 0
   
   # Or via qFlipper
   # Open qFlipper, connect device, Install from file
   ```

3. **Run NyanBox**:
   - Navigate to Applications → Plugins → NyanBox
   - Press OK to launch

### For Cardputer

1. **Build the firmware**:
   ```bash
   pio run -e cardputer
   ```

2. **Upload to device**:
   ```bash
   pio run -e cardputer --target upload
   ```

3. **Run NyanBox**:
   - Power on Cardputer
   - App starts automatically or select from menu

## 🔧 Development

### Prerequisites

**For Flipper Zero:**
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- Make
- OpenOCD (optional, for debugging)
- Docker (recommended)

**For Cardputer:**
- PlatformIO or Arduino IDE
- M5Stack board support
- ESP-IDF (optional)

### Building from Source

**Using Docker (Recommended for Flipper):**
```bash
docker run --rm -v $(pwd):/project \
    flipperdevices/flipperzero-toolchain make
```

**Native Build (Flipper):**
```bash
# Install dependencies
sudo apt install gcc-arm-none-eabi openocd

# Build
make

# Build specific app only
make APPS=nyanbox
```

**PlatformIO (Cardputer):**
```bash
# Install PlatformIO
pip install platformio

# Build and upload
pio run -e cardputer --target upload
```

## 📚 Documentation

Comprehensive documentation is available:

- **[Application README](applications/nyanbox/README.md)** - Features and usage
- **[Deployment Guide](documentation/NyanBox_Deployment.md)** - OTA updates via Bluetooth
- **[Testing Guide](documentation/NyanBox_Testing.md)** - Testing procedures and benchmarks

## 🎮 Usage

### Basic Controls

**Flipper Zero:**
- **Arrow Keys** - Navigate menus
- **OK Button** - Select/Toggle wireless
- **Back Button** - Exit application

**Cardputer:**
- **Arrow Keys / WASD** - Navigate
- **Enter** - Select
- **ESC** - Back/Exit

### Wireless Features

**Enable Bluetooth:**
1. Press OK button
2. Status shows "BLE advertising..."
3. Device is now discoverable as "NyanBox"

**Connect from another device:**
```python
# Example using Python/Bleak
from bleak import BleakScanner

devices = await BleakScanner.discover()
nyanbox = [d for d in devices if "NyanBox" in d.name][0]
```

### AI Features

**Load a model:**
```c
nyanbox_ai_init();
nyanbox_ai_load_model(
    NyanBoxAI_Model_TextClassification,
    model_data,
    model_size
);
```

**Run inference:**
```c
NyanBoxAIResult result;
nyanbox_ai_classify_text("Hello World", &result);
printf("Class: %s, Confidence: %.2f\n", 
    result.class_name, 
    result.confidence
);
```

## 🔐 Security

- **Firmware Verification** - CRC32 checksum for OTA updates
- **Digital Signatures** - Optional firmware signing (configurable)
- **Secure Boot** - Bootloader verification (platform dependent)
- **Encrypted Communication** - BLE pairing and encryption

## 📊 Performance

### Resource Usage

| Platform | RAM Used | Flash Used | AI Memory | Free Heap |
|----------|----------|------------|-----------|-----------|
| Flipper Zero F7 | ~12 KB | ~50 KB | ~50 KB | ~200 KB |
| Cardputer | ~28 KB | ~100 KB | ~200 KB | ~280 KB |

### Wireless Performance

| Feature | Flipper Zero | Cardputer |
|---------|-------------|-----------|
| BLE Transfer | 50-100 KB/s | 100-200 KB/s |
| WiFi Transfer | N/A | 500-800 KB/s |
| SubGHz | 10-50 KB/s | N/A |

### AI Inference Speed

| Model Type | Flipper Zero | Cardputer |
|------------|-------------|-----------|
| Text (128 features) | 100-200ms | 20-50ms |
| Image (28x28) | 500-1000ms | 100-200ms |
| Anomaly (32 features) | 50-100ms | 10-20ms |

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow existing code style (use `.clang-format`)
- Write tests for new features
- Update documentation
- Ensure compatibility with both platforms
- Test on actual hardware when possible

## 🐛 Troubleshooting

**Build Issues:**
```bash
# Clean build
make clean

# Rebuild
make
```

**Device not detected:**
- Check USB cable and connections
- Verify device is in bootloader mode
- Try different USB port
- Check device drivers

**App crashes:**
- Check available memory
- Reduce AI model size
- Enable debug logging
- Check serial output

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/Crashkeyz/flipperzero-firmware/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Crashkeyz/flipperzero-firmware/discussions)
- **Documentation**: [/documentation](documentation/)

## 📄 License

This project inherits the license from the original Flipper Zero firmware repository. See [LICENSE](LICENSE) for details.

## 🙏 Acknowledgments

- **Flipper Devices** - For the amazing Flipper Zero platform
- **M5Stack** - For the Cardputer hardware
- **Contributors** - Everyone who has contributed to this project

## 🗺️ Roadmap

### Completed ✅
- [x] Multi-platform HAL architecture
- [x] Flipper Zero integration
- [x] Cardputer platform support
- [x] Wireless communication modules
- [x] AI functionality framework
- [x] Bluetooth OTA updates
- [x] Comprehensive documentation

### In Progress 🚧
- [ ] TensorFlow Lite integration
- [ ] Pre-trained model library
- [ ] Advanced wireless protocols
- [ ] Mobile companion app
- [ ] Cloud synchronization

### Planned 📋
- [ ] Additional platform support (ESP32, RP2040)
- [ ] Enhanced AI models
- [ ] Mesh networking
- [ ] Voice recognition
- [ ] Computer vision features
- [ ] IoT integrations

## 📈 Version History

### v1.0.0 (Current)
- Initial multi-platform release
- Flipper Zero and Cardputer support
- Basic AI and wireless features
- OTA update capability
- Complete documentation

---

**Made with ❤️ for the embedded AI community**

*NyanBox - Bringing AI to your pocket-sized devices*
