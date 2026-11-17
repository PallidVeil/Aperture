CFLAGS=-Wall -Wextra -Werror -Iinclude
SRCS=src/lexer.c src/rawmode.c src/main.c src/editor.c src/parser.c
CC=gcc

Aperture: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@