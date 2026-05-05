// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include "zeruntime_win.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Internal structures */
typedef struct {
    uint64_t magic;
    uint16_t version;
    uint16_t page_type;
    uint32_t checksum;
    uint32_t next_free_page;
    uint32_t page_checksum;
} PageHeader;

typedef struct {
    PageHeader header;
    uint8_t data[ZSDF_PAGE_SIZE - sizeof(PageHeader) - sizeof(uint32_t)];
    /* page_checksum stored at end */
} Page;

typedef struct {
    uint32_t page_count;
    uint32_t first_free_page;
    uint32_t root_meta_page;   /* page number of first META page */
    uint64_t creation_time;
} DbMeta;

struct zsdf_db {
    HANDLE hFile;
    HANDLE hMapping;
    void *mapped_base;
    size_t mapped_size;
    zeruntime_win_rwlock_t rwlock;
    int lock_mode;             /* 0=unlocked, 1=shared, 2=exclusive */
    uint64_t next_txn_id;
    DbMeta meta;
    /* In-memory table catalog (simplified) */
    /* ... */
};

/* Forward declarations */
static int db_load_meta(zsdf_db *db);
static int db_save_meta(zsdf_db *db);
static Page *db_get_page(zsdf_db *db, uint32_t pgno);
static uint32_t db_alloc_page(zsdf_db *db);
static void db_free_page(zsdf_db *db, uint32_t pgno);

int zsdf_open(const char *filename, zsdf_db **out) {
    zsdf_db *db = calloc(1, sizeof(zsdf_db));
    if (!db) return ZSDF_ERROR;

    db->hFile = zeruntime_win_createfile(filename, GENERIC_READ | GENERIC_WRITE,
                                         FILE_SHARE_READ, OPEN_ALWAYS);
    if (db->hFile == INVALID_HANDLE_VALUE) {
        free(db);
        return ZSDF_ERROR;
    }

    DWORD size_low = zeruntime_win_getfilesize(db->hFile, NULL);
    if (size_low == 0) {
        /* New database: create initial mapping and meta page */
        db->mapped_size = ZSDF_PAGE_SIZE;
        db->hMapping = zeruntime_win_createfilemapping(db->hFile, PAGE_READWRITE, 0, ZSDF_PAGE_SIZE, NULL);
        if (!db->hMapping) goto error;
        db->mapped_base = zeruntime_win_mapviewoffile(db->hMapping, FILE_MAP_ALL_ACCESS, 0, 0, ZSDF_PAGE_SIZE);
        if (!db->mapped_base) goto error;

        /* Initialize meta page */
        Page *meta = db_get_page(db, 0);
        memset(meta, 0, ZSDF_PAGE_SIZE);
        meta->header.magic = ZSDF_MAGIC;
        meta->header.version = 1;
        meta->header.page_type = 2; /* META */
        db->meta.root_meta_page = 0;
        db->meta.page_count = 1;
        db->meta.first_free_page = 0;
        db_save_meta(db);
    } else {
        /* Existing database: map entire file */
        db->mapped_size = size_low;
        db->hMapping = zeruntime_win_createfilemapping(db->hFile, PAGE_READWRITE, 0, size_low, NULL);
        if (!db->hMapping) goto error;
        db->mapped_base = zeruntime_win_mapviewoffile(db->hMapping, FILE_MAP_ALL_ACCESS, 0, 0, size_low);
        if (!db->mapped_base) goto error;

        /* Load meta */
        if (db_load_meta(db) != ZSDF_OK) goto error;
    }

    zeruntime_win_rwlock_init(&db->rwlock);
    db->lock_mode = 0;
    db->next_txn_id = 1;
    *out = db;
    return ZSDF_OK;

error:
    if (db->mapped_base) zeruntime_win_unmapviewoffile(db->mapped_base);
    if (db->hMapping) zeruntime_win_closehandle(db->hMapping);
    if (db->hFile != INVALID_HANDLE_VALUE) zeruntime_win_closehandle(db->hFile);
    free(db);
    return ZSDF_ERROR;
}

int zsdf_close(zsdf_db *db) {
    if (!db) return ZSDF_ERROR;
    /* Flush and unmap */
    zeruntime_win_flushviewoffile(db->mapped_base, db->mapped_size);
    zeruntime_win_unmapviewoffile(db->mapped_base);
    zeruntime_win_closehandle(db->hMapping);
    zeruntime_win_closehandle(db->hFile);
    free(db);
    return ZSDF_OK;
}

/* Internal helpers */
static Page *db_get_page(zsdf_db *db, uint32_t pgno) {
    return (Page *)((char *)db->mapped_base + pgno * ZSDF_PAGE_SIZE);
}

static int db_load_meta(zsdf_db *db) {
    Page *meta = db_get_page(db, 0);
    if (meta->header.magic != ZSDF_MAGIC) return ZSDF_ERROR;
    /* Copy meta fields from page data (simplified) */
    memcpy(&db->meta, meta->data, sizeof(DbMeta));
    return ZSDF_OK;
}

static int db_save_meta(zsdf_db *db) {
    Page *meta = db_get_page(db, 0);
    memcpy(meta->data, &db->meta, sizeof(DbMeta));
    /* Update checksum */
    /* ... */
    return ZSDF_OK;
}

static uint32_t db_alloc_page(zsdf_db *db) {
    if (db->meta.first_free_page != 0) {
        uint32_t pg = db->meta.first_free_page;
        Page *free_page = db_get_page(db, pg);
        db->meta.first_free_page = free_page->header.next_free_page;
        memset(free_page, 0, ZSDF_PAGE_SIZE);
        free_page->header.magic = ZSDF_MAGIC;
        free_page->header.version = 1;
        free_page->header.page_type = 0; /* DATA */
        return pg;
    }
    /* Extend file */
    size_t new_size = db->mapped_size + ZSDF_PAGE_SIZE;
    /* Unmap, extend file, remap (simplified) */
    zeruntime_win_unmapviewoffile(db->mapped_base);
    zeruntime_win_closehandle(db->hMapping);
    db->hMapping = zeruntime_win_createfilemapping(db->hFile, PAGE_READWRITE, 0, (DWORD)new_size, NULL);
    db->mapped_base = zeruntime_win_mapviewoffile(db->hMapping, FILE_MAP_ALL_ACCESS, 0, 0, new_size);
    db->mapped_size = new_size;
    uint32_t new_pg = (uint32_t)(new_size / ZSDF_PAGE_SIZE) - 1;
    Page *page = db_get_page(db, new_pg);
    memset(page, 0, ZSDF_PAGE_SIZE);
    page->header.magic = ZSDF_MAGIC;
    page->header.version = 1;
    page->header.page_type = 0;
    db->meta.page_count++;
    return new_pg;
}

static void db_free_page(zsdf_db *db, uint32_t pgno) {
    Page *page = db_get_page(db, pgno);
    page->header.page_type = 1; /* FREE */
    page->header.next_free_page = db->meta.first_free_page;
    db->meta.first_free_page = pgno;
}
