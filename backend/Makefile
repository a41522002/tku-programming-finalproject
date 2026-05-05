CC=gcc
CFLAGS=-Wall -Wextra -O2 -std=c11

poker: main.c card.c hand.c card.h hand.h
	$(CC) $(CFLAGS) -o poker main.c card.c hand.c

clean:
	rm -f poker
