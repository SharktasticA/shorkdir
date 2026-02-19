CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

CFLAGS += -I.
LDFLAGS += -static

SRC = main.c

shorkdir: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shorkdir $(LDFLAGS)
	$(STRIP) shorkdir

PREFIX ?= /usr/
BINDIR = $(PREFIX)/bin

install: shorkdir
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 shorkdir $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/shorkdir

clean:
	rm -f shorkdir

.PHONY: install uninstall clean
