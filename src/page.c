// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include "zeruntime_win.h"
#include <string.h>

/* CRC32 implementation (from utils.c) */
uint32_t zsdf_crc32(const void *data, size_t len);

bool zsdf_page_verify(Page *page) {
    uint32_t stored = page->header.page_checksum;
    page->header.page_checksum = 0;
    uint32_t computed = zsdf_crc32(page, ZSDF_PAGE_SIZE);
    page->header.page_checksum = stored;
    return computed == stored;
}

void zsdf_page_update_checksum(Page *page) {
    page->header.page_checksum = 0;
    page->header.page_checksum = zsdf_crc32(page, ZSDF_PAGE_SIZE);
}
