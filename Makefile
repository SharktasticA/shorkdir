CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

CFLAGS += -I.
LDFLAGS += -static

SRC = main.c

shorkdir-exec: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shorkdir-exec $(LDFLAGS)
	$(STRIP) shorkdir-exec

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

install: shorkdir-exec
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 shorkdir $(DESTDIR)$(BINDIR)
	install -m 755 shorkdir-exec $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/shorkdir
	rm -f $(DESTDIR)$(BINDIR)/shorkdir-exec

clean:
	rm -f shorkdir-exec

.PHONY: install uninstall clean
