#include <gtk/gtk.h>
#include "ui.h"

/*
static void print_hello (GtkWidget *widget,gpointer   data){
	g_print ("Hello World\n");
}
*/

static void activate (GtkApplication *app,gpointer user_data){
	GtkWidget *window;

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "UMBC Navigator");
	gtk_window_set_default_size (GTK_WINDOW (window), 1024, 768);
	gtk_window_present (GTK_WINDOW (window));
	
	GtkWidget * start_location_entry = gtk_entry_new();
	GtkWidget * end_location_entry = gtk_entry_new();
	
	
	GtkWidget *button_2 = gtk_button_new_with_label ("Hello World 2");
	
	GtkWidget * hpaned = gtk_paned_new (GTK_ORIENTATION_VERTICAL);
	
	GtkWidget * frame_1 = gtk_frame_new("Location Search Fields");
	
	GtkWidget * query_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_append (GTK_BOX (query_box), start_location_entry);
	gtk_box_append (GTK_BOX (query_box), end_location_entry);
	gtk_box_set_spacing(GTK_BOX(query_box),5);
	
	gtk_frame_set_child(GTK_FRAME(frame_1),query_box);
	
	
	gtk_paned_set_start_child (GTK_PANED (hpaned), frame_1);
	gtk_paned_set_shrink_start_child (GTK_PANED (hpaned), FALSE);
	gtk_paned_set_resize_start_child(GTK_PANED (hpaned),FALSE);
	
	GtkWidget * frame_2 = gtk_frame_new("Random");
	gtk_frame_set_child(GTK_FRAME(frame_2),button_2);
	gtk_paned_set_end_child (GTK_PANED (hpaned), frame_2);
	
	
	
	gtk_window_set_child (GTK_WINDOW (window), hpaned);
	
	gtk_widget_set_size_request(frame_2, -1, -1);
}

int main (int argc,char **argv){
	GtkApplication *app;
	int status;

	app = gtk_application_new ("com.ag446.cmsc_447_project", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
