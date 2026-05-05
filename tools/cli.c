// © 2026 Zega – Licensed under ZSAL
#include "zsdf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int callback(void *unused, int argc, char **argv, char **colnames) {
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", colnames[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: zsdf <database>\n");
        return 1;
    }
    zsdf_db *db;
    int rc = zsdf_open(argv[1], &db);
    if (rc != ZSDF_OK) {
        fprintf(stderr, "Cannot open database\n");
        return 1;
    }
    char *line = NULL;
    size_t len = 0;
    printf("ZSDF> ");
    while (getline(&line, &len, stdin) != -1) {
        if (strcmp(line, ".quit\n") == 0) break;
        char *err = NULL;
        rc = zsdf_exec(db, line, callback, NULL, &err);
        if (rc != ZSDF_OK) {
            fprintf(stderr, "Error: %s\n", err ? err : "unknown");
            free(err);
        }
        printf("ZSDF> ");
    }
    free(line);
    zsdf_close(db);
    return 0;
}
