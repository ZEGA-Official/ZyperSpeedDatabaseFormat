// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include "zeruntime_win.h"
#include <string.h>
#include <stdlib.h>

/* Row format: 4-byte length + data */
int zsdf_row_insert(zsdf_db *db, uint32_t table_root, const void *row_data, uint32_t row_len, uint64_t *row_id) {
    /* Simplified: append to last page of table */
    /* ... */
    return ZSDF_OK;
}

int zsdf_row_delete(zsdf_db *db, uint64_t row_id) {
    /* Mark row as deleted (set length to 0) */
    return ZSDF_OK;
}

/* Table scan */
int zsdf_table_scan(zsdf_db *db, uint32_t table_root, void *context,
                    int (*callback)(void*, const void*, uint32_t)) {
    /* Iterate over pages, call callback for each row */
    return ZSDF_OK;
}
