#include <string.h>
#include <glib.h>
#include <panel-applet.h>
#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <stdlib.h>

static gboolean pill_taken(GtkWidget* event_box, GdkEventButton* event, gpointer data) {
  GtkWidget *label = GTK_WIDGET(data);
  gtk_label_set_text(GTK_LABEL(label), "PILL TAKEN");
  return FALSE;
}


gboolean pill_applet_fill(PanelApplet* applet, const gchar* iid, gpointer data) {
  GtkWidget *label = NULL;
  GtkWidget *event_box = NULL;
  if (strcmp(iid,"OAFIID:GNOME_PillApplet") != 0) {
    return FALSE;
  }

  label = gtk_label_new("PILL NOT TAKEN");
  event_box = gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(event_box), label);
  gtk_signal_connect(event_box, "button-press-event", pill_taken,label);
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


    
