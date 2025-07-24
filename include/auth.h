#ifndef AUTH_H
#define AUTH_H

#include <stdint.h>

// Authentication message types
enum AuthMsgType {
    AUTH_VERIFY = 1,    // Request verification (biometric sample)
    AUTH_SUCCESS = 2,   // Authentication successful
    AUTH_FAILURE = 3    // Authentication failed
};

// Authentication message structure
struct AuthMsg {
    int type;           // AuthMsgType
    int user_id;        // User identifier
    uint32_t timestamp; // Authentication timestamp
    uint8_t security_token[32]; // Security token/hash
} __attribute__((packed));

// Biometric data structures
struct HRVData {
    uint32_t timestamp;
    float heart_rate;
    float hrv_score;
    float stress_level;
} __attribute__((packed));

struct EEGData {
    uint32_t timestamp;
    float alpha_waves;
    float beta_waves;
    float theta_waves;
    float delta_waves;
    float focus_level;
    float relaxation_level;
} __attribute__((packed));

// Authentication service constants
#define MAX_USERS 16
#define BIOMETRIC_SAMPLE_SIZE 256
#define AUTH_TIMEOUT_MS 30000

// Function prototypes for auth service
int auth_verify_user(int user_id, const void *biometric_data);
int auth_store_profile(int user_id, const void *profile_data);
void auth_service_main(void);

#endif // AUTH_H
