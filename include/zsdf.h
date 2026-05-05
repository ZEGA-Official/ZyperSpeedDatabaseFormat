// © 2026 Zega – Licensed under ZSAL
#ifndef ZSDF_H
#define ZSDF_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return codes */
#define ZSDF_OK           0
#define ZSDF_ERROR        1
#define ZSDF_BUSY         5
#define ZSDF_ROW          100
#define ZSDF_DONE         101

/* Page constants */
#define ZSDF_PAGE_SIZE    4096
#define ZSDF_MAGIC        0xDEADBEEFCAFEBABEULL

/* Column types */
typedef enum {
    ZSDF_TYPE_INTEGER = 1,
    ZSDF_TYPE_REAL    = 2,
    ZSDF_TYPE_TEXT    = 3
} ZsdfType;

/* Opaque structures */
typedef struct zsdf_db zsdf_db;
typedef struct zsdf_stmt zsdf_stmt;

/* Database lifecycle */
int zsdf_open(const char *filename, zsdf_db **db);
int zsdf_close(zsdf_db *db);

/* Simple SQL execution */
int zsdf_exec(zsdf_db *db, const char *sql,
              int (*callback)(void*,int,char**,char**), void *arg,
              char **errmsg);

/* Prepared statement API */
int zsdf_prepare(zsdf_db *db, const char *sql, int nByte,
                 zsdf_stmt **stmt, const char **pzTail);
int zsdf_step(zsdf_stmt *stmt);
int zsdf_finalize(zsdf_stmt *stmt);

/* Binding */
int zsdf_bind_int(zsdf_stmt *stmt, int idx, int val);
int zsdf_bind_text(zsdf_stmt *stmt, int idx, const char *val, int len,
                   void(*destructor)(void*));

/* Result access */
int zsdf_column_int(zsdf_stmt *stmt, int iCol);
const char *zsdf_column_text(zsdf_stmt *stmt, int iCol);
int zsdf_column_count(zsdf_stmt *stmt);

/* Error handling */
const char *zsdf_errmsg(zsdf_db *db);

#ifdef __cplusplus
}
#endif

#endif /* ZSDF_H */
