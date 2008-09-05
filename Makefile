CFLAGS=`pkg-config --cflags libpanelapplet-2.0` -g
LDFLAGS=`pkg-config --libs libpanelapplet-2.0`
BINDIR=/usr/local/bin
BONOBO_DIR=/usr/lib/bonobo/servers

pill-applet: pill-applet.o

all: pill-applet

install: all
	/usr/bin/install pill-applet $(BINDIR)
	/usr/bin/install -m 644 pill-applet.server $(BONOBO_DIR)

clean:
	rm pill-applet.o pill-applet
