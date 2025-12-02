#include "ui.h"
#include "map_render.h"
#include "text_proc.h"

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
	
	out.path_finder_strategy_index = 0;
	
	out.on_screen_map = init_on_screen_map("assets/maps/campus.map");
	
	out.window = NULL;
	
	return out;
}

void screen_data_state_to_output_stream(screen_data_state_t state,FILE * stream){
	fputs("\n\n",stream);
	
	fprintf(stream,"Hide Non Auto Doors: %d\n",state.on_screen_map.hide_non_auto_doors);
	fprintf(stream,"Show Building Names: %d\n",state.on_screen_map.show_building_names);
	
	fputs("Path Finder Strategy: ",stream);
	fputs(path_strategies[state.path_finder_strategy_index]->strategy_name,stream);
	fputc('\n',stream);
}

static void populate_search_results_into_text_view(GtkWidget * text_view,search_results_t results){
	char * results_str = NULL;
	
	if(results.n_results > 0){
		size_t total_length = 0;
		const char ** results_name = (const char**) malloc(sizeof(const char*) * results.n_results);
		for(size_t i = 0;i < results.n_results;i++){
			map_node_t * current_node = results.results[i];
			const char * current_node_name = get_map_node_name(current_node,NULL);//TODO add err_ctx_t
			
			results_name[i] = current_node_name;
			total_length += strlen(current_node_name);
		}
		
		const char * node_selected_str = "Node Selected: ";
		total_length += strlen(node_selected_str) + results.n_results + 1;
		
		results_str = (char*) malloc(total_length+1);
		
		size_t at = 0;
		strcpy(results_str+at,node_selected_str);
		at += strlen(node_selected_str);
		
		for(size_t i = 0;i < results.n_results;i++){
			strcpy(results_str+at,results_name[i]);
			at += strlen(results_name[i]);
			if(i == 0){
				results_str[at] = '\n';
				at++;
			}
			
			results_str[at] = '\n';
			at++;
		}
		results_str[at] = '\0';
		free(results_name);
	}else{
		results_str = strdup("No results found");
	}
	
	GtkTextBuffer * text_buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (text_buffer,results_str,-1);
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(text_view),text_buffer);
}

static void start_location_activate (GtkWidget * widget,screen_data_state_t * sds){
	GtkEntryBuffer * buffer = gtk_entry_get_buffer (GTK_ENTRY(widget));
	char * text = strdup(gtk_entry_buffer_get_text(buffer));
	
	search_results_t results = get_nodes_with_closest_match(sds->on_screen_map.map_sys.map,text);
	populate_search_results_into_text_view(sds->text_view,results);
	if(results.n_results > 0){
		on_screen_map_notify_start(&(sds->on_screen_map),results.results[0]);
	}
	deinit_search_results(&results);
	
	free(text);
}

static void end_location_activate (GtkWidget * widget,screen_data_state_t * sds){
	GtkEntryBuffer * buffer = gtk_entry_get_buffer (GTK_ENTRY(widget));
	char * text = strdup(gtk_entry_buffer_get_text(buffer));
	
	search_results_t results = get_nodes_with_closest_match(sds->on_screen_map.map_sys.map,text);
	populate_search_results_into_text_view(sds->text_view,results);
	if(results.n_results > 0){
		on_screen_map_notify_end(&(sds->on_screen_map),results.results[0]);
	}
	deinit_search_results(&results);
	
	free(text);
}


static void create_location_label_and_entry(const char * label_name,GtkWidget ** entry_assign,GtkWidget * grid,size_t row,GCallback entry_callback,GCallback clear_callback,screen_data_state_t * sds){
	GtkWidget * label = gtk_label_new(label_name);
	{
		gtk_widget_add_css_class(label,"location-labels");
	}
	
	GtkWidget * find_location = gtk_button_new_with_label("Clear");
	{
		gtk_widget_add_css_class(find_location,"clear-buttons");
		if(clear_callback != NULL) g_signal_connect(find_location, "clicked", G_CALLBACK (clear_callback), sds);
	}
	
	GtkWidget * entry = gtk_entry_new();
	{
		gtk_widget_add_css_class(entry,"location-entries");
		if(entry_callback != NULL) g_signal_connect(entry, "activate", G_CALLBACK (entry_callback), sds);
		gtk_widget_set_hexpand(entry, TRUE);
		*entry_assign = entry;
	}
	
	gtk_grid_attach (GTK_GRID (grid), label,0,row,1,1);
	gtk_grid_attach (GTK_GRID (grid), entry,1,row,1,1);
	gtk_grid_attach (GTK_GRID (grid), find_location,2,row,1,1);
}

static void start_location_clear_callback(GtkButton * self,screen_data_state_t * sds){
	GtkEntryBuffer * entry_buffer = gtk_entry_buffer_new ("",-1);
	gtk_entry_set_buffer (GTK_ENTRY(sds->start_location_entry),entry_buffer);
}

static void end_location_clear_callback(GtkButton * self,screen_data_state_t * sds){
	GtkEntryBuffer * entry_buffer = gtk_entry_buffer_new ("",-1);
	gtk_entry_set_buffer (GTK_ENTRY(sds->end_location_entry),entry_buffer);
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
		create_location_label_and_entry("Start Location:",&(sds->start_location_entry),location_grid,0,G_CALLBACK (start_location_activate),G_CALLBACK(start_location_clear_callback),sds);
		create_location_label_and_entry("End Location:",&(sds->end_location_entry),location_grid,1,G_CALLBACK (end_location_activate),G_CALLBACK(end_location_clear_callback),sds);
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
	
	GtkTextBuffer * text_buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (text_buffer,"",-1);
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(sds->text_view),text_buffer);
}

static void go_button_clicked_callback(GtkButton * self,screen_data_state_t * sds){
	on_screen_map_find_path(&(sds->on_screen_map),path_strategies[sds->path_finder_strategy_index]->edge_cost_function);
	if(sds->on_screen_map.map_sys.active_path == NULL) return;
	char * path_text = convert_path_to_directions_str(sds->on_screen_map.map_sys.active_path, NULL);//TODO err_ctx_t
	if(path_text == NULL) return;
	
	GtkTextBuffer * text_buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (text_buffer,path_text,-1);
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(sds->text_view),text_buffer);
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

static gboolean show_building_names_switch_state_set_callback(GtkSwitch * self,gboolean state,screen_data_state_t * sds){
	on_screen_map_set_show_building_names(&(sds->on_screen_map),state);
	
	screen_data_state_to_output_stream(*sds,stdout);
	return FALSE;
}

static void path_finder_dropdown_callback(GtkDropDown * self,GParamSpec* pspec, screen_data_state_t * sds){
	sds->path_finder_strategy_index = gtk_drop_down_get_selected (self);
	screen_data_state_to_output_stream(*sds,stdout);
}

const char* HELP_DIALOG_CONTENT = 
"<big>Welcome to UMBC Navigator!</big>\n\n"
"This map is designed to help you navigate campus by pathfinding using accessibly-modified doors, elevators, and ramps. It does so by calculating the shortest path to take from any location on campus to another!\n\n"
"Start by inputting a source location. This could be anywhere on campus - you can either select a location on the map to the left by clicking on it, or you can type in a building or abbreviation in the search bar on the right. Sources are indicated with blue.\n\n"
"Next, input a destination. It's just like before! Choose anywhere else you'd like to go on the map, either by clicking on it or by searching it up. Destinations are indicated with red.\n\n"
"Finally, press the Go button, and the rest will happen on its own! You can see the route the algorithm generated on the map, or you can read from the instructions on the right to understand where it wants you to go!\n\n"
"Along the options sidebar on the right, we have many options for you to choose from. You can choose to toggle visual features on the map, or you can change the strategy used between calculating routes that use elevators only, preferring elevators (but still with some stairs), or stairs be prioritized over elevators.\n\n"
"If, along your travels, you run into an accessible passageway, like a door or elevator, that does not appear in functioning order, feel free to call the number on the side of the application. This is the official UMBC number to call to request a work order to be put in to repair that item as soon as possible.\n\n"
"Thanks for using our app! If you have any feedback you'd like for us to know, feel free to click the link button on the side to help us make the app better for you!\n\n";

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
	GtkWidget * hide_interior_option = create_toggle_option_box("Show Building Names",sds->on_screen_map.show_building_names,G_CALLBACK(show_building_names_switch_state_set_callback),sds);
	
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
	
	
	GtkWidget * text_view = gtk_text_view_new();
	{
		gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view),GTK_WRAP_WORD);
		gtk_widget_set_vexpand(text_view,TRUE);
		sds->text_view = text_view;
	}
	
	GtkWidget * scrolled_window = gtk_scrolled_window_new();
	{
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		gtk_widget_set_vexpand(scrolled_window, TRUE);
		gtk_widget_set_hexpand(scrolled_window, TRUE);
		gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);
	}
	
	GtkWidget * options_top_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	{
		gtk_widget_set_vexpand(options_top_box,TRUE);
		
		gtk_box_append(GTK_BOX(options_top_box), help_button);
		
		gtk_box_append(GTK_BOX(options_top_box), phone_number_info);
		gtk_box_append(GTK_BOX(options_top_box), phone_number_label);
		gtk_box_append(GTK_BOX(options_top_box), google_form_link);
		
		gtk_box_append(GTK_BOX(options_top_box), hide_non_auto_door_option);
		gtk_box_append(GTK_BOX(options_top_box), hide_interior_option);
		
		gtk_box_append(GTK_BOX(options_top_box), scrolled_window);
	}
	
	const char * path_finder_strategy_option_strings[N_PATH_STRATEGIES+1];
	for(size_t i = 0;i < N_PATH_STRATEGIES;i++) path_finder_strategy_option_strings[i] = path_strategies[i]->strategy_name;
	path_finder_strategy_option_strings[N_PATH_STRATEGIES] = NULL;
	GtkWidget * path_finder_strategy_dropdown = create_dropdown_option_box("Path Finder Strategy",path_finder_strategy_option_strings,G_CALLBACK(path_finder_dropdown_callback),sds);
	
	GtkWidget * go_clear_box = create_go_clear_button_box(sds);
	
	GtkWidget * options_bottom_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	{
		gtk_box_append(GTK_BOX(options_bottom_box), path_finder_strategy_dropdown);
		gtk_box_append (GTK_BOX (options_bottom_box), go_clear_box);
		gtk_widget_set_valign(options_bottom_box,GTK_ALIGN_END);
		gtk_widget_set_vexpand(options_bottom_box,FALSE);
	}
	
	GtkWidget * options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	{
		gtk_widget_set_vexpand(options_box,TRUE);
		gtk_box_append (GTK_BOX (options_box), options_top_box);
		gtk_box_append (GTK_BOX (options_box), options_bottom_box);
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
		gtk_widget_set_vexpand(options_frame,TRUE);
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

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 1080

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
		gtk_window_set_default_size (GTK_WINDOW (window), WINDOW_WIDTH, WINDOW_HEIGHT);
		gtk_window_set_child (GTK_WINDOW (window), screen_box);
		gtk_window_present (GTK_WINDOW (window));
		g_signal_connect(window, "close-request", G_CALLBACK(window_close_request_callback), sds);
	}
	
	sds->window = window;
}
