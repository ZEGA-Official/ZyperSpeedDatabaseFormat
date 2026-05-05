CC = gcc
CFLAGS = -std=c2x -Wall -Wextra -O2 -DNDEBUG -Iinclude
LDFLAGS = -static

SRC = src/db.c src/page.c src/storage.c src/parser.c src/wal.c src/rwlock.c src/utils.c
OBJ = $(SRC:.c=.o)
LIB = libzsdf.a
CLI = zsdf.exe

all: $(LIB) $(CLI)

$(LIB): $(OBJ)
	ar rcs $@ $^

$(CLI): tools/cli.c $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(LIB)
	$(CC) $(CFLAGS) -o test_db.exe test/test_db.c $(LIB)
	./test_db.exe
	$(CC) $(CFLAGS) -o test_parser.exe test/test_parser.c $(LIB)
	./test_parser.exe

bench: $(LIB)
	$(CC) $(CFLAGS) -o bench.exe test/bench.c $(LIB)
	./bench.exe

clean:
	rm -f $(OBJ) $(LIB) $(CLI) test_db.exe test_parser.exe bench.exe
