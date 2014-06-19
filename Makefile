PREFIX = $(HOME)/.local/bin

all: dragon

dragon: dragon.c
	$(CC) --std=c99 -Wall $(DEFINES) dragon.c -o dragon `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`

install: dragon
	cp dragon $(PREFIX)
