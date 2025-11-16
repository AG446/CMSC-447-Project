#include "ui.h"

int main (int argc,char **argv){
	screen_data_state_t screen_state = init_screen_data_state();
	
	GtkApplication *app;
	int status;

	app = gtk_application_new ("com.ag446.cmsc_447_project", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect (app, "activate", G_CALLBACK (create_window), &screen_state);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	
	return status;
}