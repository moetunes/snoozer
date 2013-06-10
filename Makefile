CFLAGS+= -Wall
LDADD+= -lX11 -lpthread
LDFLAGS=
EXEC=snoozer

PREFIX?= /usr/local
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

snoozer: snoozer.o
	$(CC) $(LDFLAGS) -s -O2 -ffast-math -fno-unit-at-a-time -o $@ $+ $(LDADD)

install: all
	install -Dm 755 snoozer $(DESTDIR)$(BINDIR)/snoozer

clean:
	rm -fv snoozer *.o

