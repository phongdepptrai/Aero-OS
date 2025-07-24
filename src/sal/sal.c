#include "../include/sal/sal.h"
#include <stdint.h>

// Syscall numbers
enum Syscalls {
    SYS_SAL_SEND = 1,
    SYS_SAL_RECV = 2,
    SYS_SAL_PUBLISH = 3,
    SYS_SAL_SUBSCRIBE = 4,
};

// System call wrapper functions
static inline long syscall3(long num, long arg1, long arg2, long arg3) {
    long ret;
    asm volatile(
        "movl %1, %%edi; movl %2, %%esi; movl %3, %%edx; movl %4, %%eax; int $0x80"
        : "=a"(ret) 
        : "r"(arg1), "r"(arg2), "r"(arg3), "i"(num)
        : "edi", "esi", "edx", "memory"
    );
    return ret;
}

static inline long syscall4(long num, long arg1, long arg2, long arg3, long arg4) {
    long ret;
    asm volatile(
        "movl %1, %%edi; movl %2, %%esi; movl %3, %%edx; movl %4, %%ecx; movl %5, %%eax; int $0x80"
        : "=a"(ret) 
        : "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4), "i"(num)
        : "edi", "esi", "edx", "ecx", "memory"
    );
    return ret;
}

// SAL API implementation
int sal_send(int dest_pid, const void *msg, size_t len) {
    return (int)syscall3(SYS_SAL_SEND, dest_pid, (long)msg, len);
}

int sal_recv(int src_pid, void *buf, size_t maxlen) {
    return (int)syscall3(SYS_SAL_RECV, src_pid, (long)buf, maxlen);
}

int sal_publish(const char *topic, const void *data, size_t len) {
    // For now, implement as a special send to a broker process
    // TODO: Implement proper pub/sub mechanism
    return (int)syscall3(SYS_SAL_PUBLISH, (long)topic, (long)data, len);
}

int sal_subscribe(const char *topic, void (*callback)(const void*, size_t)) {
    // Register callback for topic
    // TODO: Implement proper subscription mechanism
    return (int)syscall3(SYS_SAL_SUBSCRIBE, (long)topic, (long)callback, 0);
}

// Kernel-side syscall implementations (stubs for now)
long sys_sal_send(int dest_pid, const void *buf, size_t size) {
    // TODO: Kernel logic to enqueue message to dest_pid's queue
    // For now, just return success
    (void)dest_pid; (void)buf; (void)size;
    return 0;
}

long sys_sal_recv(int src_pid, void *buf, size_t maxlen) {
    // TODO: Kernel logic to dequeue message from src_pid
    // If blocking, put this thread to sleep until a message arrives
    (void)src_pid; (void)buf; (void)maxlen;
    return 0;
}

long sys_sal_publish(const char *topic, const void *data, size_t len) {
    // TODO: Broadcast data to all subscribers of topic
    (void)topic; (void)data; (void)len;
    return 0;
}

long sys_sal_subscribe(const char *topic, void (*callback)(const void*, size_t)) {
    // TODO: Register callback for topic in subscription table
    (void)topic; (void)callback;
    return 0;
}
