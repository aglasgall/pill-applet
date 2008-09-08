#include <string.h>
#include <glib.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

static const gchar* PILL_TAKEN_MSG = "PILL TAKEN";
static const gchar* PILL_NOT_TAKEN_MSG = "PILL NOT TAKEN";
// (60 seconds/minute, 30 minutes) - poll wallclock every half an hour :(
static const guint INTERVAL = 60 * 30;
// reset after 22 hours (60 seconds/minute * 60 minutes/hour * 22 hours)
static const struct timeval RESET_INTERVAL = { 60 * 60 * 22, 0 };

static gboolean g_event_fired = FALSE;
static struct timeval g_pill_taken_time = {};

static gboolean reset_indicator(gpointer data) {
  struct timeval now = {};
  struct timeval elapsed_time = {};
  GtkWidget *label = NULL;
  
  label = GTK_WIDGET(data);
  gettimeofday(&now,NULL);
  timeradd(&g_pill_taken_time, &RESET_INTERVAL, &elapsed_time);
  
  if(timercmp(&now, &elapsed_time, >)) {
    timerclear(&g_pill_taken_time);
    g_event_fired = FALSE;
    gtk_label_set_text(GTK_LABEL(label), PILL_NOT_TAKEN_MSG);
    return FALSE;
  } else {
    return TRUE;
  }
}

static gboolean pill_taken(GtkWidget* event_box, GdkEventButton* event, gpointer data) {
  GtkWidget *label = GTK_WIDGET(data);
  if(!g_event_fired && (event->button == 1)) {
    g_event_fired = TRUE;
    gettimeofday(&g_pill_taken_time,NULL);
    gtk_label_set_text(GTK_LABEL(label), PILL_TAKEN_MSG);
    g_timeout_add_seconds(INTERVAL, reset_indicator, label);
  }
  return FALSE;
}


gboolean pill_applet_fill(PanelApplet* applet, const gchar* iid, gpointer data) {
  GtkWidget *label = NULL;
  GtkWidget *event_box = NULL;
  if (strcmp(iid,"OAFIID:GNOME_PillApplet") != 0) {
    return FALSE;
  }

  label = gtk_label_new(PILL_NOT_TAKEN_MSG);
  event_box = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(event_box), label);
  g_signal_connect(GTK_OBJECT(event_box), "button-press-event", G_CALLBACK(pill_taken),label);
  gtk_container_add(GTK_CONTAINER(applet),event_box);
  gtk_widget_show_all(GTK_WIDGET(applet));
  return TRUE;
}


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_PillApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "Pill Applet",
                             "0",
                             pill_applet_fill,
                             NULL);


    
