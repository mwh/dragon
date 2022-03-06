PREFIX = $(HOME)/.local
MANPREFIX = $(PREFIX)/share/man
NAME = dragon

all: $(NAME)

$(NAME): dragon.c Makefile
	$(CC) --std=c99 -Wall $(DEFINES) dragon.c -o $(NAME) `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`

install: $(NAME)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(NAME) $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(NAME)
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	sed -e "s/dragon/$(NAME)/g" dragon.1 > $(DESTDIR)$(MANPREFIX)/man1/$(NAME).1
	chmod 644 $(DESTDIR)$(MANPREFIX)/man1/$(NAME).1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(NAME) $(DESTDIR)$(MANPREFIX)/man1/$(NAME).1

clean:
	rm -f $(NAME)
