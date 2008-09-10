#ifndef _PILL_APPLET_H_
#define _PILL_APPLET_H_

#define _GNU_SOURCE

#include <string.h>
#include <glib.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>

#define LABEL_FROM_APPLET(applet) gtk_bin_get_child(GTK_BIN(gtk_bin_get_child(GTK_BIN(applet))))

typedef struct _application_state {
  PanelApplet* applet;
  struct timeval reset_interval;
  gboolean event_fired;
  struct timeval pill_taken_time;
  guint notification_id;
  guint timeout_event_id;
  GConfClient* client;
} ApplicationState;

#endif

