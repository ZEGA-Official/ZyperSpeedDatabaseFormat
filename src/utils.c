// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include <stdint.h>

/* Fast CRC32 (software) */
uint32_t zsdf_crc32(const void *data, size_t len) {
    const uint8_t *p = data;
    uint32_t crc = 0xFFFFFFFF;
    while (len--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}
