CC=gcc
CFLAGS=-Wall -Wextra

first: main.c safe_string.c safe_string.h
	$(CC) -o app main.c safe_string.c $(CFLAGS)

clean:
	rm app