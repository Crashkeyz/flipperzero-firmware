#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NyanBox AI Module
 * Provides AI functionality for both Flipper Zero and Cardputer
 * Uses lightweight ML models suitable for embedded systems
 */

typedef enum {
    NyanBoxAI_Model_Unknown,
    NyanBoxAI_Model_TextClassification,
    NyanBoxAI_Model_ImageRecognition,
    NyanBoxAI_Model_AnomalyDetection,
    NyanBoxAI_Model_VoiceCommand
} NyanBoxAIModelType;

typedef enum {
    NyanBoxAI_Status_Uninitialized,
    NyanBoxAI_Status_Ready,
    NyanBoxAI_Status_Processing,
    NyanBoxAI_Status_Error
} NyanBoxAIStatus;

typedef struct {
    NyanBoxAIModelType type;
    const char* name;
    uint32_t version;
    uint32_t input_size;
    uint32_t output_size;
    uint32_t model_size_bytes;
} NyanBoxAIModelInfo;

typedef struct {
    float confidence;
    uint8_t class_id;
    char class_name[32];
} NyanBoxAIResult;

/**
 * Initialize AI subsystem
 */
bool nyanbox_ai_init(void);

/**
 * Get AI status
 */
NyanBoxAIStatus nyanbox_ai_get_status(void);

/**
 * Load AI model from memory
 */
bool nyanbox_ai_load_model(NyanBoxAIModelType type, const uint8_t* model_data, uint32_t size);

/**
 * Unload current model
 */
void nyanbox_ai_unload_model(void);

/**
 * Get current model info
 */
NyanBoxAIModelInfo nyanbox_ai_get_model_info(void);

/**
 * Run inference on input data
 */
bool nyanbox_ai_inference(const float* input, uint32_t input_size, NyanBoxAIResult* result);

/**
 * Text classification
 */
bool nyanbox_ai_classify_text(const char* text, NyanBoxAIResult* result);

/**
 * Image recognition (for devices with camera or image input)
 */
bool nyanbox_ai_recognize_image(const uint8_t* image_data, uint32_t width, uint32_t height, NyanBoxAIResult* result);

/**
 * Anomaly detection on sensor data
 */
bool nyanbox_ai_detect_anomaly(const float* sensor_data, uint32_t num_samples, NyanBoxAIResult* result);

/**
 * Voice command recognition
 */
bool nyanbox_ai_recognize_voice_command(const int16_t* audio_data, uint32_t num_samples, NyanBoxAIResult* result);

/**
 * Get available models list
 */
uint8_t nyanbox_ai_get_available_models(NyanBoxAIModelInfo* models, uint8_t max_models);

#ifdef __cplusplus
}
#endif
