#include "ui.h"
#include "map_render.h"

static void map_pointer_motion_event (GtkEventController *gesture,gdouble x,gdouble y,screen_data_state_t * sds){
	set_mouse_position(&(sds->on_screen_map.mouse_state),x,y);
}

static void map_mouse_pressed (GtkGestureClick * gesture,int n_press,double x,double y,screen_data_state_t * sds){
	set_mouse_button_down(&(sds->on_screen_map.mouse_state),true);
}

static void map_mouse_released (GtkGestureClick * gesture,gint n_press,gdouble x,gdouble y,screen_data_state_t * sds){
	set_mouse_button_down(&(sds->on_screen_map.mouse_state),false);
}

screen_data_state_t init_screen_data_state(void){
	screen_data_state_t out;
	
	out.start_location_text = NULL;
	out.end_location_text = NULL;
	
	out.hide_interior_locations = true;
	
	out.path_finder_strategy_index = 0;
	
	out.on_screen_map = init_on_screen_map("assets/maps/campus.map");
	
	out.window = NULL;
	
	return out;
}

void screen_data_state_to_output_stream(screen_data_state_t state,FILE * stream){
	fputs("\n\n",stream);
	fputs("Start Location Text: ",stream);
	if(state.start_location_text != NULL) fputs(state.start_location_text,stream);
	fputc('\n',stream);
	
	fputs("End Location Text: ",stream);
	if(state.end_location_text != NULL) fputs(state.end_location_text,stream);
	fputc('\n',stream);
	
	fprintf(stream,"Hide Non Auto Doors: %d\n",state.on_screen_map.hide_non_auto_doors);
	fprintf(stream,"Hide Interior Locations: %d\n",state.hide_interior_locations);
	
	fputs("Path Finder Strategy: ",stream);
	fputs(path_strategies[state.path_finder_strategy_index]->strategy_name,stream);
	fputc('\n',stream);
}

static void start_location_activate (GtkWidget *widget,screen_data_state_t * sds){
	GtkEntryBuffer * buffer = gtk_entry_get_buffer (GTK_ENTRY(widget));
	char * text = strdup(gtk_entry_buffer_get_text(buffer));
	if(sds->start_location_text != NULL) free(sds->start_location_text);
	sds->start_location_text = text;
	
	screen_data_state_to_output_stream(*sds,stdout);
}

static void end_location_activate (GtkWidget *widget,screen_data_state_t * sds){
	GtkEntryBuffer * buffer = gtk_entry_get_buffer (GTK_ENTRY(widget));
	char * text = strdup(gtk_entry_buffer_get_text(buffer));
	if(sds->end_location_text != NULL) free(sds->end_location_text);
	sds->end_location_text = text;
	
	screen_data_state_to_output_stream(*sds,stdout);
}


static void create_location_label_and_entry(const char * label_name,GtkWidget * grid,size_t row,GCallback entry_callback,screen_data_state_t * sds){
	GtkWidget * label = gtk_label_new(label_name);
	{
		gtk_widget_add_css_class(label,"location-labels");
	}
	
	GtkWidget * find_location = gtk_button_new_with_label("Find");
	{
		gtk_widget_add_css_class(find_location,"find-buttons");
	}
	
	GtkWidget * entry = gtk_entry_new();
	{
		gtk_widget_add_css_class(entry,"location-entries");
		if(entry_callback != NULL) g_signal_connect(entry, "activate", G_CALLBACK (entry_callback), sds);
		gtk_widget_set_hexpand(entry, TRUE);
	}
	
	gtk_grid_attach (GTK_GRID (grid), label,0,row,1,1);
	gtk_grid_attach (GTK_GRID (grid), entry,1,row,1,1);
	gtk_grid_attach (GTK_GRID (grid), find_location,2,row,1,1);
}

static GtkWidget * create_location_frame(screen_data_state_t * sds){
	GtkWidget * umbc_logo_image = gtk_image_new_from_file ("assets/ui_images/umbc_logo.png");
	{
		gtk_widget_set_name(umbc_logo_image, "location-frame-image");
		gtk_image_set_pixel_size (GTK_IMAGE(umbc_logo_image), 80);
		gtk_widget_set_valign(umbc_logo_image,GTK_ALIGN_START);
	}
	
	GtkWidget * location_grid = gtk_grid_new ();
	{
		create_location_label_and_entry("Start Location:",location_grid,0,G_CALLBACK (start_location_activate),sds);
		create_location_label_and_entry("End Location:",location_grid,1,G_CALLBACK (end_location_activate),sds);
		gtk_grid_attach(GTK_GRID(location_grid), umbc_logo_image, 3, 0, 1, 2);
	}
	
	GtkWidget * location_frame = gtk_frame_new(NULL);
	{
		GtkWidget * location_frame_label = gtk_label_new ("Location Search Fields");
		gtk_widget_add_css_class(location_frame_label, "frame-label");
		gtk_frame_set_label_widget(GTK_FRAME(location_frame),location_frame_label);
		gtk_frame_set_label_align(GTK_FRAME(location_frame),0.5f);
		gtk_widget_set_name(location_frame, "location-frame");
		gtk_frame_set_child(GTK_FRAME(location_frame),location_grid);
	}
	
	return location_frame;
}

static GtkWidget * create_toggle_option_box(const char * toggle_name,bool initial_toggle_state,GCallback toggle_callback,screen_data_state_t * sds){
	GtkWidget * label = gtk_label_new(toggle_name);
	{
		gtk_widget_set_halign(label,GTK_ALIGN_START);
		gtk_widget_set_hexpand(label,TRUE);
	}
	
	GtkWidget * toggle_switch = gtk_switch_new();
	{
		gtk_switch_set_active(GTK_SWITCH(toggle_switch),initial_toggle_state);
		gtk_widget_set_halign(toggle_switch,GTK_ALIGN_END);
		if(toggle_callback != NULL) g_signal_connect (toggle_switch, "state-set", G_CALLBACK (toggle_callback), sds);
	}
	
	GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	{
		gtk_box_append(GTK_BOX(box), label);
		gtk_box_append(GTK_BOX(box), toggle_switch);
		gtk_widget_set_valign(box,GTK_ALIGN_START);
		gtk_widget_set_vexpand(box,FALSE);
		gtk_widget_set_hexpand(box,FALSE);
	}
	
	return box;
}

static GtkWidget * create_dropdown_option_box(const char * dropdown_name,const char * const * strings,GCallback dropdown_callback,screen_data_state_t * sds){
	GtkWidget * label = gtk_label_new(dropdown_name);
	{
		gtk_widget_set_halign(label,GTK_ALIGN_START);
		gtk_widget_set_hexpand(label,TRUE);
	}
	GtkWidget * dropdown = gtk_drop_down_new_from_strings(strings);
	{
		gtk_widget_set_hexpand(dropdown,FALSE);
		gtk_widget_set_vexpand(dropdown,FALSE);
		if(dropdown_callback != NULL) g_signal_connect(dropdown,"notify::selected-item", G_CALLBACK(dropdown_callback), sds);
	}
	
	GtkWidget * box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	{
		gtk_box_append(GTK_BOX(box), label);
		gtk_box_append(GTK_BOX(box), dropdown);
		gtk_widget_set_valign(box,GTK_ALIGN_START);
		gtk_widget_set_vexpand(box,FALSE);
	}
	
	return box;
}

static void clear_button_clicked_callback(GtkButton * self,screen_data_state_t * sds){
	on_screen_map_clear_selection(&(sds->on_screen_map));
}

static void go_button_clicked_callback(GtkButton * self,screen_data_state_t * sds){
	on_screen_map_find_path(&(sds->on_screen_map),path_strategies[sds->path_finder_strategy_index]->edge_cost_function);
}

static GtkWidget * create_go_clear_button_box(screen_data_state_t * sds){
	
	GtkWidget * go_button = gtk_button_new_with_label("GO");
	{
		gtk_widget_add_css_class(go_button,"big-buttons");
		gtk_widget_set_name(go_button, "go-button");
		gtk_widget_set_hexpand(go_button,TRUE);
		gtk_widget_set_vexpand(go_button,FALSE);
		g_signal_connect(go_button,"clicked",G_CALLBACK(go_button_clicked_callback),sds);
	}
	
	GtkWidget * clear_button = gtk_button_new_with_label("CLEAR");
	{
		gtk_widget_add_css_class(clear_button,"big-buttons");
		gtk_widget_set_name(clear_button, "clear-button");
		gtk_widget_set_hexpand(clear_button,TRUE);
		gtk_widget_set_vexpand(clear_button,FALSE);
		g_signal_connect(clear_button,"clicked",G_CALLBACK(clear_button_clicked_callback),sds);
	}
	
	GtkWidget * go_clear_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	{
		gtk_box_append(GTK_BOX(go_clear_box), go_button);
		gtk_box_append(GTK_BOX(go_clear_box), clear_button);
		gtk_widget_set_valign(go_clear_box,GTK_ALIGN_END);
		gtk_widget_set_hexpand(go_clear_box,TRUE);
		gtk_widget_set_vexpand(go_clear_box,FALSE);
	}
	
	return go_clear_box;
}

static gboolean hide_non_auto_doors_switch_state_set_callback(GtkSwitch * self,gboolean state,screen_data_state_t * sds){
	on_screen_map_set_hide_non_auto_doors(&(sds->on_screen_map),state);
	
	screen_data_state_to_output_stream(*sds,stdout);
	return FALSE;
}

static gboolean hide_interior_locations_switch_state_set_callback(GtkSwitch * self,gboolean state,screen_data_state_t * sds){
	sds->hide_interior_locations = state;
	screen_data_state_to_output_stream(*sds,stdout);
	return FALSE;
}

static void path_finder_dropdown_callback(GtkDropDown * self,GParamSpec* pspec, screen_data_state_t * sds){
	sds->path_finder_strategy_index = gtk_drop_down_get_selected (self);
	screen_data_state_to_output_stream(*sds,stdout);
}

const char* HELP_DIALOG_CONTENT = 
"<big>Welcome to the UMBC Interactive Accessibility Map!</big>\n\n"
"This map is designed to help you navigate campus by pathfinding using accessibly-modified doors, elevators, and ramps. It does so by calculating the shortest path to take from any location on campus to another!\n\n"
"Start by inputting a **source location**. This could be anywhere on campus - you can either select a location on the map to the left by clicking on it, or you can type in a building or abbreviation in the search bar on the right.\n\n"
"Next, input a **destination**. It's just like before! Choose anywhere else you'd like to go on the map, either by clicking on it or by searching it up.\n\n"
"Finally, press the **Calculate** button, and the rest will go on its own! You can see the route the algorithm generated on the map, or you can read from the instructions below to understand where it wants you to go!\n\n"
"When you're finished, you can save that route by clicking the **Save** button at the bottom. If you ever want to see it again, select the **Saved Routes** button and you'll be able to see a list of the routes you have saved before. Just pick it, and it will instantly reload back onto the screen like before!\n\n"
"Thanks for using our app! If you have any feedback you'd like for us to know, feel free to use the QR code at the bottom to help us make the app better for you!)\n\n";

static void help_button_clicked (GtkButton* self,screen_data_state_t * sds){
	GtkWidget *dialog;
	dialog = gtk_dialog_new_with_buttons ("Learn about UMBC navigator",GTK_WINDOW(sds->window),GTK_DIALOG_DESTROY_WITH_PARENT,NULL,NULL);gtk_widget_show(dialog);
	gtk_window_set_default_size (GTK_WINDOW (dialog), 600, 600);
	gtk_window_set_resizable(GTK_WINDOW(dialog),false);
	
	
	GtkWidget * content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
	GtkWidget * label = gtk_label_new(NULL);
	gtk_label_set_markup (GTK_LABEL (label), HELP_DIALOG_CONTENT);
	gtk_label_set_wrap(GTK_LABEL(label),TRUE);
	gtk_widget_set_halign(label,GTK_ALIGN_START);
	gtk_widget_set_vexpand(content_area,TRUE);
	gtk_box_append (GTK_BOX (content_area), label);
	
	
}


static GtkWidget * create_options_frame(screen_data_state_t * sds){
	GtkWidget * hide_non_auto_door_option = create_toggle_option_box("Hide Non-Automatic Doors",sds->on_screen_map.hide_non_auto_doors,G_CALLBACK(hide_non_auto_doors_switch_state_set_callback),sds);
	GtkWidget * hide_interior_option = create_toggle_option_box("Hide Interior Locations",sds->hide_interior_locations,G_CALLBACK(hide_interior_locations_switch_state_set_callback),sds);
	
	GtkWidget * help_button = gtk_button_new();
	{
		GtkWidget * help_button_image = gtk_image_new_from_file ("assets/ui_images/help.png");
		gtk_image_set_pixel_size(GTK_IMAGE(help_button_image),32);
		gtk_button_set_child (GTK_BUTTON (help_button), help_button_image);
		gtk_widget_set_hexpand(help_button,FALSE);
		gtk_widget_set_halign(help_button,GTK_ALIGN_START);
		gtk_widget_set_name(help_button,"help-button");
		g_signal_connect(help_button,"clicked",G_CALLBACK(help_button_clicked),sds);
	}
	
	GtkWidget * google_form_link = gtk_link_button_new_with_label ("https://forms.gle/xDagBdgFJ7LyttHg6","Link to Feedback Form");
	GtkWidget * phone_number_info = gtk_label_new("Call this number to request repairs:");
	GtkWidget * phone_number_label = gtk_label_new("(410) 455-2550");
	
	GtkWidget * options_top_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	{
		gtk_widget_set_valign(options_top_box,GTK_ALIGN_START);
		gtk_widget_set_vexpand(options_top_box,FALSE);
		
		gtk_box_append(GTK_BOX(options_top_box), help_button);
		
		gtk_box_append(GTK_BOX(options_top_box), phone_number_info);
		gtk_box_append(GTK_BOX(options_top_box), phone_number_label);
		gtk_box_append(GTK_BOX(options_top_box), google_form_link);
		
		gtk_box_append(GTK_BOX(options_top_box), hide_non_auto_door_option);
		gtk_box_append(GTK_BOX(options_top_box), hide_interior_option);
	}
	
	const char * path_finder_strategy_option_strings[N_PATH_STRATEGIES+1];
	for(size_t i = 0;i < N_PATH_STRATEGIES;i++) path_finder_strategy_option_strings[i] = path_strategies[i]->strategy_name;
	path_finder_strategy_option_strings[N_PATH_STRATEGIES] = NULL;
	GtkWidget * path_finder_strategy_dropdown = create_dropdown_option_box("Path Finder Strategy",path_finder_strategy_option_strings,G_CALLBACK(path_finder_dropdown_callback),sds);
	
	GtkWidget * go_clear_box = create_go_clear_button_box(sds);
	
	GtkWidget * options_bottom_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	{
		gtk_widget_set_vexpand(options_bottom_box,FALSE);
		gtk_box_append(GTK_BOX(options_bottom_box), path_finder_strategy_dropdown);
		gtk_box_append (GTK_BOX (options_bottom_box), go_clear_box);
		gtk_widget_set_valign(options_bottom_box,GTK_ALIGN_END);
	}
	
	GtkWidget * options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	{
		gtk_widget_set_vexpand(options_box,TRUE);
		gtk_box_append (GTK_BOX (options_box), options_top_box);
		gtk_box_append (GTK_BOX (options_box), options_bottom_box);
		gtk_box_set_homogeneous(GTK_BOX (options_box),TRUE);
	}
	
	GtkWidget * options_frame_label = gtk_label_new ("Options");
	{
		gtk_widget_add_css_class(options_frame_label, "frame-label");
	}
	
	GtkWidget * options_frame = gtk_frame_new(NULL);
	{
		gtk_frame_set_label_widget(GTK_FRAME(options_frame),options_frame_label);
		gtk_frame_set_label_align(GTK_FRAME(options_frame),0.5f);
		
		gtk_widget_set_name(options_frame,"options-frame");
		gtk_widget_set_hexpand(options_frame,FALSE);
		gtk_frame_set_child(GTK_FRAME(options_frame),options_box);
	}
	
	return options_frame;
}

static GtkWidget * create_map_frame(screen_data_state_t * sds){
	GtkWidget * map_frame_label = gtk_label_new ("Interactive Map");
	{
		gtk_widget_set_name(map_frame_label,"map-frame-label");
	}
	
	GtkWidget * map_drawing_area = gtk_drawing_area_new ();
	sds->on_screen_map.idle_drawing_function_id = g_idle_add((GSourceFunc)idle_draw_function,&(sds->on_screen_map));
	sds->on_screen_map.drawing_area = map_drawing_area;
	{
		gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (map_drawing_area), (GtkDrawingAreaDrawFunc)(render_on_screen_map_callback), &(sds->on_screen_map), NULL);
		g_signal_connect_after (GTK_DRAWING_AREA (map_drawing_area), "resize", G_CALLBACK (resize_on_screen_map_callback), &(sds->on_screen_map));
	}
	
	GtkEventController * mouse_move = gtk_event_controller_motion_new();
	{
		g_signal_connect (mouse_move, "motion", G_CALLBACK (map_pointer_motion_event), sds);
	}
	
	GtkGesture * press = gtk_gesture_click_new ();
	{
		gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (press), GDK_BUTTON_PRIMARY);
		g_signal_connect (press, "pressed", G_CALLBACK (map_mouse_pressed), sds);
		g_signal_connect (press, "released", G_CALLBACK (map_mouse_released), sds);
	}
	
	gtk_widget_add_controller (map_drawing_area, GTK_EVENT_CONTROLLER (press));
	gtk_widget_add_controller (map_drawing_area, GTK_EVENT_CONTROLLER (mouse_move));
	
	GtkWidget * map_frame = gtk_frame_new(NULL);
	{
		gtk_frame_set_label_widget(GTK_FRAME(map_frame),map_frame_label);
		gtk_frame_set_label_align(GTK_FRAME(map_frame),0.5f);
		gtk_widget_set_name(map_frame,"map-frame");
		gtk_widget_set_hexpand(map_frame,TRUE);
		gtk_widget_set_vexpand(map_frame,TRUE);
		gtk_frame_set_child(GTK_FRAME(map_frame),map_drawing_area);
		gtk_widget_set_size_request (map_frame, 500, -1);
	}
	
	return map_frame;
}

gboolean window_close_request_callback(GtkWindow* self,screen_data_state_t * sds){
	if (sds->on_screen_map.idle_drawing_function_id != 0) {
		g_source_remove(sds->on_screen_map.idle_drawing_function_id);
		sds->on_screen_map.idle_drawing_function_id = 0;
	}
	return FALSE;
}

void create_window (GtkApplication *app,screen_data_state_t * sds){
	GtkCssProvider *css_provider = gtk_css_provider_new();
	{
		gtk_css_provider_load_from_file(css_provider, g_file_new_for_path("assets/style.css"));
	}
	GdkDisplay *display = gdk_display_get_default();
	{
		gtk_style_context_add_provider_for_display(display, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	
	GtkWidget * map_frame = create_map_frame(sds);
	GtkWidget * options_frame = create_options_frame(sds);
	
	GtkWidget * hpaned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
	{
		gtk_widget_set_vexpand(hpaned, TRUE);
		gtk_paned_set_start_child (GTK_PANED (hpaned), map_frame);
		gtk_paned_set_end_child (GTK_PANED (hpaned), options_frame);
		gtk_paned_set_shrink_start_child(GTK_PANED (hpaned),FALSE);
		gtk_paned_set_shrink_end_child(GTK_PANED (hpaned),FALSE);
		gtk_paned_set_resize_end_child (GTK_PANED (hpaned),FALSE);
	}
	
	GtkWidget * location_frame = create_location_frame(sds);
	
	GtkWidget * screen_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	{
		gtk_box_append(GTK_BOX(screen_box), location_frame);
		gtk_box_append(GTK_BOX(screen_box), hpaned);
	}
	
	GtkWidget * window = gtk_application_window_new (app);
	{
		gtk_window_set_title (GTK_WINDOW (window), "UMBC Navigator");
		gtk_window_set_default_size (GTK_WINDOW (window), 1024, 768);
		gtk_window_set_child (GTK_WINDOW (window), screen_box);
		gtk_window_present (GTK_WINDOW (window));
		g_signal_connect(window, "close-request", G_CALLBACK(window_close_request_callback), sds);
	}
	
	sds->window = window;
}
