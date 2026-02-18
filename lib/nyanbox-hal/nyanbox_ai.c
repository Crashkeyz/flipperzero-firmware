#include "nyanbox_ai.h"
#include "nyanbox_hal.h"
#include <string.h>

/**
 * NyanBox AI Module Implementation
 * Lightweight AI inference for embedded systems
 */

static NyanBoxAIStatus g_ai_status = NyanBoxAI_Status_Uninitialized;
static NyanBoxAIModelInfo g_current_model = {0};
static const uint8_t* g_model_data = NULL;

bool nyanbox_ai_init(void) {
    if(!nyanbox_hal_ai_init()) {
        return false;
    }
    
    g_ai_status = NyanBoxAI_Status_Ready;
    return true;
}

NyanBoxAIStatus nyanbox_ai_get_status(void) {
    return g_ai_status;
}

bool nyanbox_ai_load_model(NyanBoxAIModelType type, const uint8_t* model_data, uint32_t size) {
    if(g_ai_status == NyanBoxAI_Status_Uninitialized) {
        return false;
    }
    
    // Check if we have enough memory
    NyanBoxAICapabilities caps = nyanbox_hal_ai_get_capabilities();
    if(size > caps.available_memory) {
        return false;
    }
    
    // Load the model
    if(!nyanbox_hal_ai_load_model(model_data, size)) {
        return false;
    }
    
    g_model_data = model_data;
    g_current_model.type = type;
    g_current_model.model_size_bytes = size;
    g_current_model.version = 1;
    
    switch(type) {
        case NyanBoxAI_Model_TextClassification:
            g_current_model.name = "Text Classifier";
            g_current_model.input_size = 128;
            g_current_model.output_size = 10;
            break;
        case NyanBoxAI_Model_ImageRecognition:
            g_current_model.name = "Image Recognizer";
            g_current_model.input_size = 784; // 28x28 image
            g_current_model.output_size = 10;
            break;
        case NyanBoxAI_Model_AnomalyDetection:
            g_current_model.name = "Anomaly Detector";
            g_current_model.input_size = 32;
            g_current_model.output_size = 1;
            break;
        case NyanBoxAI_Model_VoiceCommand:
            g_current_model.name = "Voice Command";
            g_current_model.input_size = 256;
            g_current_model.output_size = 5;
            break;
        default:
            g_current_model.name = "Unknown";
            break;
    }
    
    return true;
}

void nyanbox_ai_unload_model(void) {
    g_model_data = NULL;
    memset(&g_current_model, 0, sizeof(g_current_model));
}

NyanBoxAIModelInfo nyanbox_ai_get_model_info(void) {
    return g_current_model;
}

bool nyanbox_ai_inference(const float* input, uint32_t input_size, NyanBoxAIResult* result) {
    if(g_ai_status != NyanBoxAI_Status_Ready || g_model_data == NULL) {
        return false;
    }
    
    if(input_size != g_current_model.input_size) {
        return false;
    }
    
    g_ai_status = NyanBoxAI_Status_Processing;
    
    // Allocate output buffer
    float* output = malloc(g_current_model.output_size * sizeof(float));
    if(!output) {
        g_ai_status = NyanBoxAI_Status_Error;
        return false;
    }
    
    // Run inference through HAL
    bool success = nyanbox_hal_ai_inference(input, input_size, output, g_current_model.output_size);
    
    if(success && result) {
        // Find the class with highest confidence
        float max_conf = output[0];
        uint8_t max_idx = 0;
        
        for(uint32_t i = 1; i < g_current_model.output_size; i++) {
            if(output[i] > max_conf) {
                max_conf = output[i];
                max_idx = i;
            }
        }
        
        result->confidence = max_conf;
        result->class_id = max_idx;
        snprintf(result->class_name, sizeof(result->class_name), "Class_%d", max_idx);
    }
    
    free(output);
    g_ai_status = NyanBoxAI_Status_Ready;
    
    return success;
}

bool nyanbox_ai_classify_text(const char* text, NyanBoxAIResult* result) {
    if(g_current_model.type != NyanBoxAI_Model_TextClassification) {
        return false;
    }
    
    // Convert text to feature vector (simplified)
    float* features = malloc(g_current_model.input_size * sizeof(float));
    if(!features) {
        return false;
    }
    
    // Simple text vectorization (character frequency, etc.)
    memset(features, 0, g_current_model.input_size * sizeof(float));
    uint32_t text_len = strlen(text);
    for(uint32_t i = 0; i < text_len && i < g_current_model.input_size; i++) {
        features[i] = (float)text[i] / 255.0f;
    }
    
    bool success = nyanbox_ai_inference(features, g_current_model.input_size, result);
    free(features);
    
    return success;
}

bool nyanbox_ai_recognize_image(const uint8_t* image_data, uint32_t width, uint32_t height, NyanBoxAIResult* result) {
    if(g_current_model.type != NyanBoxAI_Model_ImageRecognition) {
        return false;
    }
    
    // Convert image to normalized float array
    uint32_t expected_size = width * height;
    if(expected_size != g_current_model.input_size) {
        return false;
    }
    
    float* normalized = malloc(expected_size * sizeof(float));
    if(!normalized) {
        return false;
    }
    
    for(uint32_t i = 0; i < expected_size; i++) {
        normalized[i] = (float)image_data[i] / 255.0f;
    }
    
    bool success = nyanbox_ai_inference(normalized, expected_size, result);
    free(normalized);
    
    return success;
}

bool nyanbox_ai_detect_anomaly(const float* sensor_data, uint32_t num_samples, NyanBoxAIResult* result) {
    if(g_current_model.type != NyanBoxAI_Model_AnomalyDetection) {
        return false;
    }
    
    if(num_samples != g_current_model.input_size) {
        return false;
    }
    
    return nyanbox_ai_inference(sensor_data, num_samples, result);
}

bool nyanbox_ai_recognize_voice_command(const int16_t* audio_data, uint32_t num_samples, NyanBoxAIResult* result) {
    if(g_current_model.type != NyanBoxAI_Model_VoiceCommand) {
        return false;
    }
    
    // Convert audio samples to features (MFCC, etc.)
    // This is a simplified implementation
    float* features = malloc(g_current_model.input_size * sizeof(float));
    if(!features) {
        return false;
    }
    
    // Simple normalization
    uint32_t samples_to_use = num_samples < g_current_model.input_size ? num_samples : g_current_model.input_size;
    for(uint32_t i = 0; i < samples_to_use; i++) {
        features[i] = (float)audio_data[i] / 32768.0f;
    }
    
    // Pad with zeros if needed
    for(uint32_t i = samples_to_use; i < g_current_model.input_size; i++) {
        features[i] = 0.0f;
    }
    
    bool success = nyanbox_ai_inference(features, g_current_model.input_size, result);
    free(features);
    
    return success;
}

uint8_t nyanbox_ai_get_available_models(NyanBoxAIModelInfo* models, uint8_t max_models) {
    // Return list of available models
    // In a real implementation, this would scan storage for model files
    
    if(max_models < 4) {
        return 0;
    }
    
    models[0] = (NyanBoxAIModelInfo){
        .type = NyanBoxAI_Model_TextClassification,
        .name = "Text Classifier",
        .version = 1,
        .input_size = 128,
        .output_size = 10,
        .model_size_bytes = 50000
    };
    
    models[1] = (NyanBoxAIModelInfo){
        .type = NyanBoxAI_Model_ImageRecognition,
        .name = "Image Recognizer",
        .version = 1,
        .input_size = 784,
        .output_size = 10,
        .model_size_bytes = 100000
    };
    
    models[2] = (NyanBoxAIModelInfo){
        .type = NyanBoxAI_Model_AnomalyDetection,
        .name = "Anomaly Detector",
        .version = 1,
        .input_size = 32,
        .output_size = 1,
        .model_size_bytes = 20000
    };
    
    models[3] = (NyanBoxAIModelInfo){
        .type = NyanBoxAI_Model_VoiceCommand,
        .name = "Voice Command",
        .version = 1,
        .input_size = 256,
        .output_size = 5,
        .model_size_bytes = 75000
    };
    
    return 4;
}
