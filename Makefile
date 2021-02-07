PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

all: dragon

dragon: dragon.c
	$(CC) --std=c99 -Wall $(DEFINES) dragon.c -o dragon `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`

install: dragon
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f dragon $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/dragon
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	cp -f dragon.1 $(DESTDIR)$(MANPREFIX)/man1/dragon.1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/dragon.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/dragon $(DESTDIR)$(MANPREFIX)/man1/dragon.1
