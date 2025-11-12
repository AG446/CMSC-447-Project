#include <gtk/gtk.h>
#include "ui.h"

/*
static void print_hello (GtkWidget *widget,gpointer   data){
	g_print ("Hello World\n");
}
*/

struct Screen_Data_State{
	char * start_location_text;
	char * end_location_text;
};

static void activate (GtkApplication *app,gpointer user_data){
	GtkWidget *window;

	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "UMBC Navigator");
	gtk_window_set_default_size (GTK_WINDOW (window), 1024, 768);
	gtk_window_present (GTK_WINDOW (window));
	
	GtkWidget * screen_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget * location_frame = gtk_frame_new("Location Search Fields");
	
	GtkWidget * location_grid = gtk_grid_new ();
	gtk_grid_set_column_spacing(GTK_GRID (location_grid),10);
	gtk_grid_set_row_spacing(GTK_GRID (location_grid),5);
	
	GtkWidget * start_location_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (start_location_label), "<big>Start Location: </big>");
	GtkWidget * start_location_entry = gtk_entry_new();
	gtk_widget_set_hexpand(start_location_entry, TRUE);
	gtk_grid_attach (GTK_GRID (location_grid), start_location_label,0,0,1,1);
	gtk_grid_attach (GTK_GRID (location_grid), start_location_entry,1,0,1,1);
	
	GtkWidget * end_location_label = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (end_location_label), "<big>End Location: </big>");
	GtkWidget * end_location_entry = gtk_entry_new();
	gtk_widget_set_hexpand(end_location_entry, TRUE);
	gtk_grid_attach (GTK_GRID (location_grid), end_location_label,0,1,1,1);
	gtk_grid_attach (GTK_GRID (location_grid), end_location_entry,1,1,1,1);
	
	
	gtk_widget_set_margin_start(location_grid, 10);
	gtk_widget_set_margin_end(location_grid, 10);
	gtk_widget_set_margin_top(location_grid, 10);
	gtk_widget_set_margin_bottom(location_grid, 10);
	
	gtk_frame_set_child(GTK_FRAME(location_frame),location_grid);
	gtk_box_append(GTK_BOX(screen_box), location_frame);
	
	
	GtkWidget * map_frame = gtk_frame_new(NULL);
	GtkWidget * map_frame_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (map_frame_label), "<big>Interactive Map</big>");
	gtk_frame_set_label_widget(GTK_FRAME(map_frame),map_frame_label);
	gtk_frame_set_label_align(GTK_FRAME(map_frame),0.5f);
	GtkWidget * map_drawing_area = gtk_drawing_area_new ();
	gtk_frame_set_child(GTK_FRAME(map_frame),map_drawing_area);
	
	GtkWidget * options_frame = gtk_frame_new(NULL);
	GtkWidget * options_frame_label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (options_frame_label), "<big>Options</big>");
	gtk_frame_set_label_widget(GTK_FRAME(options_frame),options_frame_label);
	gtk_frame_set_label_align(GTK_FRAME(options_frame),0.5f);
	GtkWidget * mech_door_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget * mech_doors_label = gtk_label_new("Hide Non-Mechanical Doors");
	GtkWidget * mech_doors_switch = gtk_switch_new();
	GtkWidget * go_button = gtk_button_new_with_label("GO");
	gtk_widget_set_hexpand(go_button,TRUE);
	GtkWidget * clear_button = gtk_button_new_with_label("CLEAR");
	gtk_widget_set_hexpand(clear_button,TRUE);
	GtkWidget * go_clear_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_spacing(GTK_BOX(go_clear_box),10);
	gtk_box_append(GTK_BOX(go_clear_box), go_button);
	gtk_box_append(GTK_BOX(go_clear_box), clear_button);
	gtk_box_append(GTK_BOX(mech_door_box), mech_doors_label);
	gtk_widget_set_halign(mech_doors_label,GTK_ALIGN_START);
	gtk_widget_set_halign(mech_doors_switch,GTK_ALIGN_END);
	gtk_widget_set_hexpand(mech_doors_label,TRUE);
	gtk_box_append(GTK_BOX(mech_door_box), mech_doors_switch);
	GtkWidget * options_grid = gtk_grid_new();
	gtk_grid_attach (GTK_GRID (options_grid), mech_door_box,0,0,1,1);
	
	
	gtk_grid_attach (GTK_GRID (options_grid), go_clear_box,0,1,1,1);
	
	gtk_widget_set_valign(mech_door_box,GTK_ALIGN_START);
	gtk_widget_set_valign(go_clear_box,GTK_ALIGN_END);
	
	gtk_grid_set_column_spacing(GTK_GRID (options_grid),20);
	
	gtk_grid_set_row_homogeneous(GTK_GRID (options_grid),TRUE);
	
	gtk_widget_set_margin_start(options_grid, 10);
	gtk_widget_set_margin_end(options_grid, 10);
	gtk_frame_set_child(GTK_FRAME(options_frame),options_grid);
	
	
	GtkWidget * hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_vexpand(hpaned, TRUE);
	gtk_paned_set_start_child (GTK_PANED (hpaned), map_frame);
	gtk_paned_set_end_child (GTK_PANED (hpaned), options_frame);
	gtk_paned_set_shrink_end_child(GTK_PANED (hpaned),FALSE);
	gtk_widget_set_size_request (map_frame, 500, -1);
	
	gtk_box_append(GTK_BOX(screen_box), hpaned);
	
	gtk_window_set_child (GTK_WINDOW (window), screen_box);
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
