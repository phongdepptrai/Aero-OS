#ifndef SAL_H
#define SAL_H

#include <stdint.h>
#include <stddef.h>

// SAL API function prototypes
int sal_send(int dest_pid, const void *msg, size_t len);
int sal_recv(int src_pid, void *buf, size_t maxlen);
int sal_publish(const char *topic, const void *data, size_t len);
int sal_subscribe(const char *topic, void (*callback)(const void*, size_t));

// SAL message structure
struct sal_message {
    uint32_t sender_pid;
    uint32_t dest_pid;
    uint32_t msg_type;
    uint32_t length;
    uint8_t data[];
} __attribute__((packed));

// Topic structure for pub/sub
struct sal_topic {
    char name[64];
    void (*callback)(const void*, size_t);
    struct sal_topic *next;
};

// SAL constants
#define SAL_MAX_MESSAGE_SIZE 4096
#define SAL_MAX_TOPICS 256
#define AUTH_CHANNEL 1

#endif // SAL_H
