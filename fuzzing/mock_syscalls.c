// mock_syscalls.c
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Mock definitions for BOLOS SDK syscalls

void os_exit(uint32_t code) {
    printf("[mock] os_exit called with code %u\n", code);
    exit(code);  // Simulate exit
}

void os_longjmp(uint32_t exception) {
    printf("[mock] os_longjmp called with exception %u\n", exception);
    exit(exception);  // Simplified longjmp simulation
}

void os_throw(uint32_t exception) {
    printf("[mock] os_throw called with exception %u\n", exception);
    exit(exception);
}

void os_sched_exit(uint8_t code) {
    printf("[mock] os_sched_exit(%u)\n", code);
    exit(code);
}

void os_sched_yield(void) {
    printf("[mock] os_sched_yield\n");
}

void io_seproxyhal_general_status(void) {
    printf("[mock] io_seproxyhal_general_status\n");
}

void io_seproxyhal_power_off(void) {
    printf("[mock] io_seproxyhal_power_off\n");
    exit(0);
}

// Memory functions
void os_memmove(void *dst, const void *src, size_t len) {
    memmove(dst, src, len);
}

void os_memset(void *dst, int val, size_t len) {
    memset(dst, val, len);
}

// Crypto mocks

int cx_sha256_init(void *hash) {
    printf("[mock] cx_sha256_init called\n");
    return 0;
}

int cx_sha256_update(void *hash, const uint8_t *in, size_t len) {
    printf("[mock] cx_sha256_update called with %zu bytes\n", len);
    return 0;
}

int cx_sha256_final(void *hash, uint8_t *out) {
    printf("[mock] cx_sha256_final called\n");
    memset(out, 0x42, 32);  // Fake hash output
    return 0;
}

// Dummy implementations for BIP32
int os_perso_derive_node_bip32(uint32_t curve, const uint32_t *path, uint32_t pathLength, uint8_t *privateKey, uint8_t *chainCode) {
    printf("[mock] os_perso_derive_node_bip32\n");
    if (privateKey) memset(privateKey, 0x11, 32);
    if (chainCode) memset(chainCode, 0x22, 32);
    return 0;
}

// Add more mocks as needed...