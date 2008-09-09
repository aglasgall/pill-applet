CFLAGS=`pkg-config --cflags libpanelapplet-2.0 gconf-2.0` -g -Wall -Werror -pedantic -std=c99
LDFLAGS=`pkg-config --libs libpanelapplet-2.0 gconf-2.0`
GCONF_CONFIG_SOURCE=$(shell /usr/bin/gconftool-2 --get-default-source)
export GCONF_CONFIG_SOURCE
GCONF_SCHEMA_DIR=/usr/share/gconf/schemas
BINDIR=/usr/local/bin
BONOBO_DIR=/usr/lib/bonobo/servers

pill-applet: pill-applet.o

all: pill-applet

install: all
	/usr/bin/install pill-applet $(BINDIR)
	/usr/bin/install -m 644 pill-applet.server $(BONOBO_DIR)
	/usr/bin/install -m 644 pill-applet.schemas $(GCONF_SCHEMA_DIR)
	/usr/bin/gconftool-2 --makefile-install-rule $(GCONF_SCHEMA_DIR)/pill-applet.schemas

clean:
	rm pill-applet.o pill-applet
