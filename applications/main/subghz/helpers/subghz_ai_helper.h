#pragma once

#include <furi.h>
#include <stdbool.h>

// AI Commentary and Smart Suggestions System for Signalforge
// Provides intelligent feedback, mode suggestions, and user guidance

#define AI_MAX_RESPONSE_LEN 256
#define AI_MAX_SUGGESTION_LEN 128

typedef enum {
    AIToneNeutral,
    AIToneSarcastic,
    AIToneHelpful,
    AIToneWarning,
    AIToneCritical
} AITone;

typedef enum {
    AIContextSubGHz,
    AIContextNFC,
    AIContextIR,
    AIContextBluetooth,
    AIContextSystem
} AIContext;

typedef struct {
    char response[AI_MAX_RESPONSE_LEN];
    char suggestion[AI_MAX_SUGGESTION_LEN];
    AITone tone;
    bool requires_confirmation;
} AIResponse;

// Generate AI response based on context and operation
AIResponse* subghz_ai_generate_response(
    AIContext context,
    const char* operation,
    bool success,
    const char* details);

// Smart CounterMode suggestion based on protocol and receiver
uint8_t subghz_ai_suggest_counter_mode(
    const char* protocol,
    const char* receiver_model,
    uint32_t frequency);

// Validate .sub file and provide intelligent feedback
bool subghz_ai_validate_sub_file(
    const char* file_path,
    char* feedback_buffer,
    size_t buffer_size);

// Get snarky comment for failed operations
const char* subghz_ai_get_snark(AIContext context);

// Get helpful tip based on user action
const char* subghz_ai_get_tip(AIContext context, const char* action);

// Determine if operation needs user confirmation
bool subghz_ai_needs_confirmation(
    AIContext context,
    const char* operation,
    const char* target);

// Free AI response memory
void subghz_ai_free_response(AIResponse* response);