# NyanBox Bluetooth Deployment Guide

This guide explains how to deploy NyanBox firmware to devices over Bluetooth.

## Overview

NyanBox firmware supports Over-The-Air (OTA) updates via Bluetooth for both Flipper Zero and Cardputer platforms. This allows seamless firmware updates without physical connection.

## Supported Platforms

### Flipper Zero
- **Bluetooth**: BLE 5.0 via STM32WB55
- **Max Transfer Rate**: ~100 KB/s
- **Typical Firmware Size**: 500 KB - 1 MB
- **Update Time**: ~5-10 minutes

### M5Stack Cardputer
- **Bluetooth**: BLE/Classic via ESP32-S3
- **Max Transfer Rate**: ~200 KB/s
- **Typical Firmware Size**: 1-2 MB
- **Update Time**: ~5-15 minutes

## Prerequisites

### Hardware
- Flipper Zero or M5Stack Cardputer running NyanBox firmware
- Computer or mobile device with Bluetooth capability
- Charged battery (minimum 30% recommended)

### Software
- **Mobile**: NyanBox Companion App (iOS/Android)
- **Desktop**: Python 3.7+ with BluePy/Bleak library
- **Command Line**: `nyanbox-deploy` tool

## Deployment Methods

### Method 1: Mobile App (Recommended)

#### iOS/Android
1. Install NyanBox Companion App from App Store/Play Store
2. Open the app and enable Bluetooth
3. Tap "Scan for Devices"
4. Select your NyanBox device from the list
5. Tap "Update Firmware"
6. Select the firmware file (.dfu for Flipper, .bin for Cardputer)
7. Tap "Start Update"
8. Wait for completion (device will reboot automatically)

**App Features:**
- Automatic device discovery
- Progress tracking
- Checksum verification
- Automatic retry on failure
- Battery level monitoring

### Method 2: Desktop Tool

#### Installation
```bash
# Install via pip
pip install nyanbox-deploy

# Or clone and install from source
git clone https://github.com/nyanbox/deployment-tool
cd deployment-tool
pip install -e .
```

#### Usage
```bash
# Scan for devices
nyanbox-deploy scan

# Deploy firmware
nyanbox-deploy flash --device <device-name> --firmware <firmware-file>

# Example for Flipper Zero
nyanbox-deploy flash --device "NyanBox-Flipper" --firmware flipper-nyanbox-v1.0.dfu

# Example for Cardputer
nyanbox-deploy flash --device "NyanBox-Card" --firmware cardputer-nyanbox-v1.0.bin
```

#### Advanced Options
```bash
# Verify only (don't flash)
nyanbox-deploy verify --device <device-name> --firmware <firmware-file>

# Set custom MTU size
nyanbox-deploy flash --device <device-name> --firmware <firmware-file> --mtu 512

# Enable verbose logging
nyanbox-deploy flash --device <device-name> --firmware <firmware-file> --verbose
```

### Method 3: Manual BLE Connection

For advanced users who want to implement custom deployment tools.

#### Protocol Specification

**Service UUID**: `0000FF00-0000-1000-8000-00805F9B34FB`

**Characteristics:**
- **Control**: `0000FF01-0000-1000-8000-00805F9B34FB` (Write, Notify)
- **Data**: `0000FF02-0000-1000-8000-00805F9B34FB` (Write)
- **Status**: `0000FF03-0000-1000-8000-00805F9B34FB` (Read, Notify)

#### Command Format

All commands are sent to the Control characteristic:

```
Command Structure:
[CMD:1byte][LENGTH:2bytes][PAYLOAD:variable]

Commands:
0x01 - START_OTA (payload: firmware_size:4bytes)
0x02 - DATA_CHUNK (payload: chunk_data:512bytes max)
0x03 - END_OTA (payload: checksum:4bytes)
0x04 - ABORT_OTA
0x05 - GET_STATUS
```

#### Status Responses

Status notifications are received via Status characteristic:

```
Status Structure:
[STATUS:1byte][PROGRESS:2bytes][MESSAGE:variable]

Status Codes:
0x00 - READY
0x01 - RECEIVING
0x02 - VERIFYING
0x03 - INSTALLING
0x04 - COMPLETE
0xFF - ERROR (message contains error description)
```

#### Example Python Implementation

```python
import asyncio
from bleak import BleakClient, BleakScanner

SERVICE_UUID = "0000FF00-0000-1000-8000-00805F9B34FB"
CONTROL_UUID = "0000FF01-0000-1000-8000-00805F9B34FB"
DATA_UUID = "0000FF02-0000-1000-8000-00805F9B34FB"
STATUS_UUID = "0000FF03-0000-1000-8000-00805F9B34FB"

async def deploy_firmware(device_name, firmware_path):
    # Scan for device
    device = await BleakScanner.find_device_by_name(device_name)
    if not device:
        print(f"Device {device_name} not found")
        return
    
    # Read firmware
    with open(firmware_path, 'rb') as f:
        firmware_data = f.read()
    
    firmware_size = len(firmware_data)
    print(f"Firmware size: {firmware_size} bytes")
    
    async with BleakClient(device) as client:
        print(f"Connected to {device_name}")
        
        # Subscribe to status notifications
        def status_handler(sender, data):
            status = data[0]
            progress = int.from_bytes(data[1:3], 'little')
            print(f"Status: {status}, Progress: {progress}%")
        
        await client.start_notify(STATUS_UUID, status_handler)
        
        # Send START_OTA command
        start_cmd = bytes([0x01, 0x04, 0x00]) + firmware_size.to_bytes(4, 'little')
        await client.write_gatt_char(CONTROL_UUID, start_cmd)
        await asyncio.sleep(0.5)
        
        # Send firmware in chunks
        chunk_size = 512
        for i in range(0, firmware_size, chunk_size):
            chunk = firmware_data[i:i+chunk_size]
            await client.write_gatt_char(DATA_UUID, chunk)
            
            # Progress update
            progress = (i / firmware_size) * 100
            print(f"Progress: {progress:.1f}%", end='\r')
            
            await asyncio.sleep(0.01)  # Small delay to avoid overwhelming device
        
        print("\nAll chunks sent. Verifying...")
        
        # Calculate checksum (CRC32)
        import zlib
        checksum = zlib.crc32(firmware_data)
        
        # Send END_OTA command with checksum
        end_cmd = bytes([0x03, 0x04, 0x00]) + checksum.to_bytes(4, 'little')
        await client.write_gatt_char(CONTROL_UUID, end_cmd)
        
        # Wait for completion
        await asyncio.sleep(5)
        
        print("Firmware deployed successfully!")

# Usage
asyncio.run(deploy_firmware("NyanBox-Flipper", "firmware.dfu"))
```

## Troubleshooting

### Device Not Found
- **Solution**: Ensure device is powered on and in range
- **Solution**: Check that Bluetooth is enabled on both devices
- **Solution**: Try power cycling the device
- **Solution**: Move closer to the device (within 10 meters)

### Connection Drops During Update
- **Solution**: Ensure battery is charged (>30%)
- **Solution**: Keep devices close together during update
- **Solution**: Disable power saving mode on mobile device
- **Solution**: Close other Bluetooth applications

### Verification Failed
- **Solution**: Re-download firmware file (may be corrupted)
- **Solution**: Check that firmware is for correct platform
- **Solution**: Try updating again with lower MTU size

### Device Won't Boot After Update
- **Solution**: Enter bootloader mode manually
  - **Flipper**: Hold Left + Back, release Back, release Left when LED blinks
  - **Cardputer**: Hold Boot button while powering on
- **Solution**: Flash recovery firmware via USB

### Slow Transfer Speed
- **Solution**: Ensure no other Bluetooth devices are active
- **Solution**: Update Bluetooth drivers on computer
- **Solution**: Reduce MTU size (try 256 or 128)
- **Solution**: Use USB deployment instead

## Security

### Firmware Verification
All firmware updates are verified using:
- **CRC32 checksum**: Ensures data integrity
- **Digital signature**: Verifies firmware authenticity (optional)
- **Version check**: Prevents downgrade attacks (configurable)

### Secure Boot
If secure boot is enabled:
1. Firmware must be signed with trusted key
2. Signature is verified before installation
3. Unsigned firmware is rejected

### Best Practices
- Only install firmware from trusted sources
- Verify checksums match official releases
- Keep bootloader locked in production
- Enable signature verification
- Regular security updates

## Building Firmware for Deployment

### Flipper Zero
```bash
# Build firmware
make TARGET=f7

# Output: dist/f7/flipper-z-f7-full-*.dfu
```

### Cardputer
```bash
# Using PlatformIO
pio run -e cardputer

# Output: .pio/build/cardputer/firmware.bin
```

## API Reference

See the NyanBox Wireless API documentation for programmatic access:
- `nyanbox_wireless_ota_init()` - Initialize OTA subsystem
- `nyanbox_wireless_ota_start(size)` - Begin update
- `nyanbox_wireless_ota_write_chunk(data, len)` - Write firmware chunk
- `nyanbox_wireless_ota_finalize()` - Complete update
- `nyanbox_wireless_ota_abort()` - Cancel update

## Support

- **Documentation**: https://nyanbox.dev/docs
- **Issues**: https://github.com/nyanbox/firmware/issues
- **Discord**: https://discord.gg/nyanbox
- **Email**: support@nyanbox.dev

## License

NyanBox firmware deployment tools are licensed under MIT License.
See LICENSE file for details.
