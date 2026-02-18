# Cardputer by M5Stack Target Configuration
# ESP32-S3 based platform

# MCU Configuration
MCU_FAMILY = ESP32
MCU = ESP32S3

# Compiler flags for ESP32-S3
CFLAGS += -DPLATFORM_CARDPUTER
CFLAGS += -DESP32
CFLAGS += -DESP32S3
CFLAGS += -DCONFIG_FREERTOS_HZ=1000

# Memory configuration
# ESP32-S3 has:
# - 512KB SRAM
# - 8MB Flash (typical)
# - 2MB PSRAM (optional)
RAM_EXEC = 384K
RAM_SHARED = 64K
FLASH_ADDRESS = 0x0

# Display configuration
DISPLAY_WIDTH = 240
DISPLAY_HEIGHT = 135
DISPLAY_TYPE = ST7789

# Wireless capabilities
HAS_BLE = 1
HAS_WIFI = 1
HAS_SUBGHZ = 0

# Build targets
TARGET_DIR = $(PROJECT_ROOT)/firmware/targets/cardputer

# Include paths
CFLAGS += -I$(TARGET_DIR)/Inc
CFLAGS += -I$(TARGET_DIR)/furi_hal

# Note: This target would require:
# - ESP-IDF toolchain
# - M5Stack Cardputer SDK
# - ESP32 Arduino framework (optional)
# 
# For full implementation, integrate with:
# - platformio.ini for ESP32 builds
# - Arduino IDE with M5Stack board support
# - ESP-IDF native build system
