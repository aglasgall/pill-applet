#include "pill-applet.h"


static const gchar* PILL_TAKEN_MSG = "PILL TAKEN";
static const gchar* PILL_NOT_TAKEN_MSG = "PILL NOT TAKEN";
static const gchar* INTERVAL_KEY = "/apps/pill-applet/reset_interval";
static const gchar* APP_CONFIG = "/apps/pill-applet";
// (60 seconds/minute, 30 minutes) - poll wallclock every half an hour :(
static const guint INTERVAL = 60 * 30;

// reset after 22 hours (60 seconds/minute * 60 minutes/hour * 22 hours)

static void cleanup(GtkWidget* applet, gpointer data) {
  ApplicationState* app = NULL;
  GConfClient* client = NULL;

  app = (ApplicationState*)data;
  client = GCONF_CLIENT(app->client);
  gconf_client_notify_remove(client, app->notification_id);
  gconf_client_remove_dir(client, APP_CONFIG, NULL);
  g_object_unref(client);
  g_free(app);
}

static void reset_app_state(ApplicationState* app) {
  timerclear(&(app->pill_taken_time));
  app->event_fired = FALSE;
  gtk_label_set_text(GTK_LABEL(LABEL_FROM_APPLET(app->applet)), PILL_NOT_TAKEN_MSG);

}

static gboolean try_reset_indicator(gpointer data) {
  struct timeval now = { 0,0 };
  struct timeval elapsed_time = { 0,0 };
  ApplicationState *app = NULL;
  gboolean rv = FALSE;

  app = (ApplicationState*)data;
  gettimeofday(&now,NULL);
  timeradd(&(app->pill_taken_time), &(app->reset_interval), &elapsed_time);
  
  if(timercmp(&now, &elapsed_time, >)) {
    reset_app_state(app);
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
  GSource* old_timer = NULL;
  ApplicationState* app = NULL;

  app = (ApplicationState*)user_data;
  label = GTK_WIDGET(LABEL_FROM_APPLET(app->applet));
  value = gconf_entry_get_value(entry);
  new_interval = gconf_value_get_int(value);
  
  // disable timeout event
  if (app->timeout_event_id) {
    old_timer = g_main_context_find_source_by_id(NULL,app->timeout_event_id);
    if(old_timer) {
      g_source_destroy(old_timer);
      app->timeout_event_id = 0;
    }
  }
  reset_app_state(app);
  app->reset_interval.tv_sec = new_interval;

}

static gboolean pill_taken(GtkWidget* event_box, GdkEventButton* event, gpointer data) {
  GtkWidget *label = NULL;
  ApplicationState* app = NULL;

  app = (ApplicationState*)data;
  label = LABEL_FROM_APPLET(app->applet);
  if(!(app->event_fired) && (event->button == 1)) {
    app->event_fired = TRUE;
    gettimeofday(&(app->pill_taken_time),NULL);
    gtk_label_set_text(GTK_LABEL(label), PILL_TAKEN_MSG);
    app->timeout_event_id = g_timeout_add_seconds(INTERVAL, try_reset_indicator, app);
  }
  return FALSE;
}


gboolean pill_applet_fill(PanelApplet* applet, const gchar* iid, gpointer data) {
  GtkWidget *label = NULL;
  GtkWidget *event_box = NULL;
  ApplicationState* app;
  GConfValue* initial_interval = NULL;
  int interval = 0;

  if (strcmp(iid,"OAFIID:GNOME_PillApplet") != 0) {
    return FALSE;
  }

  app = g_new0(ApplicationState, 1);
  app->applet = applet;
  if(!(app->client = gconf_client_get_default())) {
    return FALSE;
  }
  gconf_client_set_error_handling(app->client,GCONF_CLIENT_HANDLE_ALL);


  if(!(initial_interval = gconf_client_get(app->client,INTERVAL_KEY,NULL))) {
    g_object_unref(app->client);
    g_free(app);
    return FALSE;
  }

  if(!(interval = gconf_value_get_int(initial_interval))) {
    gconf_value_free(initial_interval);
    g_object_unref(app->client);
    g_free(app);
    return FALSE;
  }
  app->reset_interval.tv_sec = interval;
  app->reset_interval.tv_usec = 0;
  gconf_value_free(initial_interval);

  label = gtk_label_new(PILL_NOT_TAKEN_MSG);
  event_box = gtk_event_box_new();
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box),FALSE);
  gtk_container_add(GTK_CONTAINER(event_box), label);
  g_signal_connect(GTK_OBJECT(event_box), "button-press-event", G_CALLBACK(pill_taken),app);
  gtk_container_add(GTK_CONTAINER(applet),event_box);
  gtk_widget_show_all(GTK_WIDGET(applet));

  gconf_client_add_dir(app->client, APP_CONFIG,
		       GCONF_CLIENT_PRELOAD_NONE, NULL);
  app->notification_id = gconf_client_notify_add(app->client, 
					      "/apps/pill-applet/reset_interval",
					      interval_changed, app,
					      NULL, NULL);
  g_signal_connect(GTK_OBJECT(applet), "destroy", G_CALLBACK(cleanup), app);
  return TRUE;
}


PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_PillApplet_Factory",
                             PANEL_TYPE_APPLET,
                             "Pill Applet",
                             "0",
                             pill_applet_fill,
                             NULL)


    
