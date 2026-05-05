// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include "zeruntime_win.h"

void zsdf_rwlock_init(zsdf_db *db) {
    zeruntime_win_rwlock_init(&db->rwlock);
    db->lock_mode = 0;
}

void zsdf_rwlock_rdlock(zsdf_db *db) {
    zeruntime_win_rwlock_rdlock(&db->rwlock);
    db->lock_mode = 1;
}

void zsdf_rwlock_wrlock(zsdf_db *db) {
    zeruntime_win_rwlock_wrlock(&db->rwlock);
    db->lock_mode = 2;
}

void zsdf_rwlock_unlock(zsdf_db *db) {
    if (db->lock_mode == 1) {
        /* Release shared lock */
        /* Note: SRWLOCK requires matching release; we use a helper */
        zeruntime_win_rwlock_unlock_shared(&db->rwlock);
    } else if (db->lock_mode == 2) {
        zeruntime_win_rwlock_unlock_exclusive(&db->rwlock);
    }
    db->lock_mode = 0;
}
