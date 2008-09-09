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

static const gchar* PILL_TAKEN_MSG = "PILL TAKEN";
static const gchar* PILL_NOT_TAKEN_MSG = "PILL NOT TAKEN";
static const gchar* INTERVAL_KEY = "/apps/pill-applet/reset_interval";

// (60 seconds/minute, 30 minutes) - poll wallclock every half an hour :(
static const guint INTERVAL = 60 * 30;

// reset after 22 hours (60 seconds/minute * 60 minutes/hour * 22 hours)
static struct timeval g_reset_interval = { };
static gboolean g_event_fired = FALSE;
static struct timeval g_pill_taken_time = {};
static guint g_notification_id = NULL;


static void cleanup(GtkWidget* applet, gpointer data) {
  GConfClient* client = GCONF_CLIENT(data);
  gconf_client_notify_remove(client, g_notification_id);
  g_object_unref(GTK_OBJECT(client));
}


static gboolean try_reset_indicator(gpointer data) {
  struct timeval now = {};
  struct timeval elapsed_time = {};
  GtkWidget *label = NULL;
  gboolean rv = FALSE;
  
  label = GTK_WIDGET(data);
  gettimeofday(&now,NULL);
  timeradd(&g_pill_taken_time, &g_reset_interval, &elapsed_time);
  
  if(timercmp(&now, &elapsed_time, >)) {
    timerclear(&g_pill_taken_time);
    g_event_fired = FALSE;
    gtk_label_set_text(GTK_LABEL(label), PILL_NOT_TAKEN_MSG);
  } else {
    rv = TRUE;
  }
  return rv;
}

static void interval_changed(GConfClient *client,
			     guint cnxn_id,
			     GConfEntry *entry,
			     gpointer user_data) {
  GConfValue* value = NULL;
  int new_interval = 0;
  GtkWidget* label = NULL;

  label = GTK_WIDGET(user_data);
  value = gconf_entry_get_value(entry);
  new_interval = gconf_value_get_int(value);
  
  timerclear(&g_pill_taken_time);
  g_event_fired = FALSE;
  gtk_label_set_text(GTK_LABEL(label), PILL_NOT_TAKEN_MSG);
  g_reset_interval.tv_sec = new_interval;

}

static gboolean pill_taken(GtkWidget* event_box, GdkEventButton* event, gpointer data) {
  GtkWidget *label = GTK_WIDGET(data);
  if(!g_event_fired && (event->button == 1)) {
    g_event_fired = TRUE;
    gettimeofday(&g_pill_taken_time,NULL);
    gtk_label_set_text(GTK_LABEL(label), PILL_TAKEN_MSG);
    g_timeout_add_seconds(INTERVAL, try_reset_indicator, label);
  }
  return FALSE;
}


gboolean pill_applet_fill(PanelApplet* applet, const gchar* iid, gpointer data) {
  GtkWidget *label = NULL;
  GtkWidget *event_box = NULL;
  GConfClient* client;
  GConfValue* initial_interval = NULL;
  int interval = 0;

  if (strcmp(iid,"OAFIID:GNOME_PillApplet") != 0) {
    return FALSE;
  }

  if(!(client = gconf_client_get_default())) {
    return FALSE;
  }
  gconf_client_set_error_handling(client,GCONF_CLIENT_HANDLE_ALL);


  if(!(initial_interval = gconf_client_get(client,INTERVAL_KEY,NULL))) {
    g_object_unref(client);
    return FALSE;
  }

  if(!(interval = gconf_value_get_int(initial_interval))) {
    gconf_value_free(initial_interval);
    g_object_unref(client);
    return FALSE;
  }
  g_reset_interval.tv_sec = interval;
  g_reset_interval.tv_usec = 0;
  gconf_value_free(initial_interval);

  label = gtk_label_new(PILL_NOT_TAKEN_MSG);
  event_box = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(event_box), label);
  g_signal_connect(GTK_OBJECT(event_box), "button-press-event", G_CALLBACK(pill_taken),label);
  gtk_container_add(GTK_CONTAINER(applet),event_box);
  gtk_widget_show_all(GTK_WIDGET(applet));

  gconf_client_notify_add(client, "/apps/pill-applet",
			  interval_changed, label,
			  NULL, NULL);
  g_signal_connect(GTK_OBJECT(applet), "destroy", G_CALLBACK(cleanup), client);
  return TRUE;
}


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_PillApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "Pill Applet",
                             "0",
                             pill_applet_fill,
                             NULL);


    
