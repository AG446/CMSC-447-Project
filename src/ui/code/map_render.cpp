#include "map_render.h"

#define MAP_LINE_THICKNESS 4
#define TRANSITION_CONSTANT 0.03
#define DEFAULT_BUTTONS_CAPACITY 4
#define MAP_FONT_SIZE 14

in_map_button_t * create_map_button(double width,double height,map_data_state_t * mds){
	in_map_button_t * out = (in_map_button_t*) malloc(sizeof(in_map_button_t));
	
	out->x = 0;
	out->y = 0;
	out->width = width;
	out->height = height;
	out->transition = 0.0;
	out->was_pressed = false;
	out->mouse_on = false;
	out->pressed_on = false;
	out->visible = true;
	
	if(mds->buttons_capacity == mds->n_buttons){
		mds->buttons_capacity *= 2;
		mds->buttons = (in_map_button_t**)realloc(mds->buttons,sizeof(in_map_button_t*) * mds->buttons_capacity);
	}
	
	mds->buttons[mds->n_buttons] = out;
	mds->n_buttons++;
	
	return out;
}

static void render_popup_close_button(in_map_button_t * button,map_data_state_t * mds){
	if(!button->visible) return;
	
	cairo_t * cr = cairo_create (mds->surface);
	
	double radius = 4;
	double expand = button->transition*2;
	
	cairo_set_source_rgb (cr, 0.8,0.1,0.1);
	cairo_new_sub_path (cr);
	cairo_arc (cr, button->x + button->width + expand - radius, button->y + radius - expand, radius, -M_PI/2, 0);
	cairo_arc (cr, button->x + button->width + expand - radius, button->y + button->height + expand - radius, radius, 0, M_PI/2);
	cairo_arc (cr, button->x + radius - expand, button->y + button->height - radius + expand, radius, M_PI/2, M_PI);
	cairo_arc (cr, button->x + radius - expand, button->y + radius - expand, radius, M_PI, M_PI*3/2);
	cairo_close_path (cr);
	cairo_fill(cr);
	
	cairo_set_source_rgb(cr,1,1,1);
	cairo_set_line_width (cr, 2 + button->transition);
	
	double gap = 4;
	
	cairo_move_to(cr, button->x + gap - expand, button->y + gap - expand);
	cairo_line_to(cr, button->x+button->width - gap + expand, button->y+button->height - gap + expand);
	cairo_stroke(cr);
	
	cairo_move_to(cr, button->x + gap - expand, button->y + button->height - gap + expand);
	cairo_line_to(cr, button->x+button->width - gap + expand, button->y + gap - expand);
	cairo_stroke(cr);
	
	cairo_destroy (cr);
}

static void render_text_button(in_map_button_t * button,map_data_state_t * mds,const char * button_text){
	if(!button->visible) return;
	
	cairo_t * cr = cairo_create (mds->surface);
	
	double expand = button->transition*2;
	double radius = 6;
	
	if(button->pressed_on){
		cairo_set_source_rgb(cr,0.0,0.5,0.0);
	}else{
		cairo_set_source_rgb(cr,0.2,1,0.2);
	}
	cairo_new_sub_path (cr);
	cairo_arc (cr, button->x + button->width + expand - radius, button->y + radius - expand, radius, -M_PI/2, 0);
	cairo_arc (cr, button->x + button->width + expand - radius, button->y + button->height + expand - radius, radius, 0, M_PI/2);
	cairo_arc (cr, button->x + radius - expand, button->y + button->height - radius + expand, radius, M_PI/2, M_PI);
	cairo_arc (cr, button->x + radius - expand, button->y + radius - expand, radius, M_PI, M_PI*3/2);
	cairo_close_path (cr);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr,0,0,0);
	cairo_set_line_width (cr, MAP_LINE_THICKNESS/2);
	cairo_stroke(cr);
	
	float spacing = 20;
	
	cairo_set_font_size (cr, MAP_FONT_SIZE);
	cairo_text_extents_t extents;
	cairo_text_extents (cr, button_text, &extents);
	button->width = extents.width+spacing;
	
	cairo_set_font_size (cr, MAP_FONT_SIZE + expand/2);
	cairo_text_extents (cr, button_text, &extents);
	
	cairo_set_source_rgb(cr,0,0,0);
	cairo_move_to (cr, button->x + button->width/2 - extents.width/2, button->y+button->height+extents.height/2 - button->height/2);
	cairo_show_text (cr, button_text);
	
	cairo_destroy (cr);
}

void render_node_popup(node_popup_t * popup,map_data_state_t * mds){
	if(!popup->visible){
		popup->close_button->visible = false;
		popup->set_start_button->visible = false;
		popup->set_end_button->visible = false;
		if(popup->transition < 0.3) return;
	}else{
		popup->close_button->visible = true;
		popup->set_start_button->visible = true;
		popup->set_end_button->visible = true;
	}
	
	cairo_t * cr = cairo_create (mds->surface);
	
	double tick_size = 16;
	
	cairo_set_source_rgba(cr,1,1,1,popup->transition);
	cairo_move_to(cr, popup->x, popup->y);
	cairo_line_to(cr, popup->x - tick_size,popup->y - tick_size);
	cairo_line_to(cr, popup->x - popup->width/2,popup->y - tick_size);
	cairo_line_to(cr, popup->x - popup->width/2,popup->y - tick_size-popup->height);
	cairo_line_to(cr, popup->x + popup->width/2,popup->y - tick_size-popup->height);
	cairo_line_to(cr, popup->x + popup->width/2,popup->y - tick_size);
	cairo_line_to(cr, popup->x + tick_size,popup->y - tick_size);
	cairo_close_path(cr);
	cairo_fill_preserve (cr);
	cairo_set_source_rgba(cr,0,0,0,popup->transition);
	cairo_set_line_width (cr, MAP_LINE_THICKNESS);
	cairo_stroke(cr);
	
	if(popup->transition < 0.8){
		cairo_destroy (cr);
		return;
	}
	
	double gap = 8;
	
	popup->close_button->x = popup->x + popup->width/2 - popup->close_button->width - gap;
	popup->close_button->y = popup->y - popup->height - popup->close_button->height + gap;
	render_popup_close_button(popup->close_button,mds);
	
	popup->set_start_button->x = popup->x - popup->width/2 + gap;
	popup->set_start_button->y = popup->y - popup->set_start_button->height - tick_size - gap;
	render_text_button(popup->set_start_button,mds,"Set as Start");
	
	popup->set_end_button->x = popup->x - popup->width/2 + gap + popup->set_start_button->width + gap;
	popup->set_end_button->y = popup->y - popup->set_end_button->height - tick_size - gap;
	render_text_button(popup->set_end_button,mds,"Set as End");
	
	double line_gap = 4;
	
	cairo_set_font_size (cr, MAP_FONT_SIZE);
	
	double text_x = popup->x - popup->width/2 + gap;
	double text_y = popup->y-popup->height - tick_size + MAP_FONT_SIZE + gap;
	double line_spacing = MAP_FONT_SIZE + line_gap;
	
	if(popup->node_reference != NULL){
		char * buffer = (char*) malloc(128);
		double lon = popup->node_reference->coordinate.longitude;
		double lat = popup->node_reference->coordinate.latitude;
		
		sprintf(buffer, "Location Name:");
		cairo_move_to (cr, text_x, text_y);
		cairo_show_text (cr, buffer);
		
		text_y += line_spacing;
		
		const char * node_name = get_map_node_name(popup->node_reference,NULL);//TODO err ctx
		sprintf(buffer, "        %s", node_name == NULL ? "None given" : node_name);
		cairo_move_to (cr, text_x, text_y);
		cairo_show_text (cr, buffer);
		
		text_y += line_spacing;
		
		building_t * building = popup->node_reference->associated_building;
		if(building != NULL){
			
			sprintf(buffer, "Associated Building:");
			cairo_move_to (cr, text_x, text_y);
			cairo_show_text (cr, buffer);
			
			text_y += line_spacing;
			
			const char * building_name = get_primary_building_name(building,NULL);//TODO err ctx
			sprintf(buffer, "        %s", building_name);
			cairo_move_to (cr, text_x, text_y);
			cairo_show_text (cr, buffer);
			
			text_y += line_spacing;
		}
		
		sprintf(buffer, "Precise Location:");
		cairo_move_to (cr, text_x, text_y);
		cairo_show_text (cr, buffer);
		
		text_y += line_spacing;
		
		sprintf(buffer, "        (lon=%lf, lat=%lf)", lon,lat);
		cairo_move_to (cr, text_x, text_y);
		cairo_show_text (cr, buffer);
		
		free(buffer);
	}
	
	cairo_destroy (cr);
}

node_popup_t * create_node_popup(double width,double height,map_data_state_t * mds){
	node_popup_t * out = (node_popup_t*) malloc(sizeof(node_popup_t));
	
	out->x = 0;
	out->y = 0;
	out->full_width = width;
	out->full_height = height;
	out->width = 0;
	out->height = 0;
	
	out->transition = 0.5;
	
	out->node_reference = NULL;
	
	out->close_button = create_map_button(16,16,mds);
	out->set_start_button = create_map_button(64,32,mds);
	out->set_end_button = create_map_button(64,32,mds);
	
	out->visible = true;
	
	return out;
}

static void update_popup(node_popup_t * popup){
	popup->width = popup->full_width * popup->transition;
	popup->height = popup->full_height * popup->transition;
	
	if(popup->visible){
		popup->transition = TRANSITION_CONSTANT + popup->transition*(1-TRANSITION_CONSTANT);
	}else{
		popup->transition = 0.2*TRANSITION_CONSTANT + popup->transition*(1-TRANSITION_CONSTANT);
	}
	if(popup->close_button->was_pressed){
		popup->visible = false;
		popup->close_button->was_pressed = false;
	}
}

void resize_map_drawing_area_callback (GtkWidget *widget,int width,int height,map_data_state_t * mds){
	if (mds->surface != NULL){
		cairo_surface_destroy (mds->surface);
		mds->surface = NULL;
	}

	if (gtk_native_get_surface (gtk_widget_get_native (widget))){
		mds->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, gtk_widget_get_width (widget),gtk_widget_get_height (widget));
	}
	
	double gap = 16;
	
	double current_y = gap;
	
	mds->zoom_in_button->x = width-mds->zoom_in_button->width-gap;
	mds->zoom_in_button->y = current_y;
	
	current_y += mds->zoom_in_button->height+gap;
	
	mds->zoom_out_button->x = width-mds->zoom_in_button->width-gap;
	mds->zoom_out_button->y = current_y;
	
	current_y += mds->zoom_out_button->height+gap;
	
	mds->home_button->x = width-mds->home_button->width-gap;
	mds->home_button->y = current_y;
}

static bool update_button(in_map_button_t * button,map_data_state_t * mds,bool mouse_already_over_button){
	if(!button->visible) return false;
	
	bool on = mds->mouse_x > button->x && mds->mouse_y > button->y && mds->mouse_x < button->x + button->width && mds->mouse_y < button->y + button->height;
	
	if(on){
		button->transition = TRANSITION_CONSTANT + button->transition*(1-TRANSITION_CONSTANT);
		button->mouse_on = true;
		button->pressed_on = mds->mouse_down;
		if(!mouse_already_over_button && mds->mouse_down && !mds->p_mouse_down){
			button->was_pressed = true;
		}
		return true;
	}else{
		button->mouse_on = false;
		button->transition = button->transition*(1-TRANSITION_CONSTANT);
	}
	
	return false;
}

void map_update(map_data_state_t * mds){
	bool mouse_already_over_button = false;
	for(size_t i = 0;i < mds->n_buttons;i++){
		if(update_button(mds->buttons[i],mds,mouse_already_over_button)){
			mouse_already_over_button = true;
		}
	}
	update_popup(mds->node_popup);
	mds->p_mouse_down = mds->mouse_down;
	if(rand() < 4000000){
		mds->node_popup->visible = true;
	}
}

#define BUTTON_SCALE_FACTOR 0.1

static void render_zoom_in_zoom_out_button(in_map_button_t * button,map_data_state_t * mds,bool zoom_in){
	if(!button->visible) return;
	
	cairo_t * cr = cairo_create (mds->surface);
	
	double scaling_factor = 1.0+button->transition*BUTTON_SCALE_FACTOR;
	
	double radius = button->width/2*scaling_factor;
	
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_set_source_rgb (cr, 1,1,1);
	cairo_set_line_width (cr, MAP_LINE_THICKNESS);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb (cr, 0,0,0);
	cairo_stroke(cr);
	
	double thickness = radius/4.0;
	double inner_radius = radius*0.6;
	
	if(zoom_in){
		cairo_set_source_rgb (cr, 1,0,0);
	}else{
		cairo_set_source_rgb (cr, 0,0,1);
	}
	cairo_rectangle(cr,button->x+button->width/2-inner_radius,button->y+button->height/2-thickness/2,inner_radius*2,thickness);
	if(zoom_in) cairo_rectangle(cr,button->x+button->width/2-thickness/2,button->y+button->height/2-inner_radius,thickness,inner_radius*2);
	cairo_fill (cr);
	
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_set_source_rgba (cr, 1-button->transition,1-button->transition,1-button->transition,0.2);
	cairo_fill(cr);

	cairo_destroy (cr);
}

static void render_zoom_in_button(in_map_button_t * button,map_data_state_t * mds){
	render_zoom_in_zoom_out_button(button,mds,true);
}

static void render_zoom_out_button(in_map_button_t * button,map_data_state_t * mds){
	render_zoom_in_zoom_out_button(button,mds,false);
}

static void render_home_button(in_map_button_t * button,map_data_state_t * mds){
	if(!button->visible) return;
	cairo_t * cr = cairo_create (mds->surface);
	
	double scaling_factor = 1.0+button->transition*BUTTON_SCALE_FACTOR;
	
	double radius = button->width/2*scaling_factor;
	
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_set_source_rgb (cr, 1,1,1);
	cairo_set_line_width (cr, MAP_LINE_THICKNESS);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb (cr, 0,0,0);
	cairo_stroke(cr);
	
	double inner_radius = radius*0.6;
	
	
	double house_x[11] = {-0.25,-0.25,0.25,0.25,0.75,0.75,1.0,0.0,-1.0,-0.75,-0.75};
	double house_y[11] = {1,0.5,0.5,1,1,0,0,-1,0,0,1};
	
	for(size_t i = 0;i < 11;i++){
		house_x[i] = house_x[i]*inner_radius + button->x+button->width/2;
		house_y[i] = house_y[i]*inner_radius + button->y+button->height/2;
	}
	
	cairo_set_source_rgb (cr, 0,0,0);
	cairo_move_to(cr, house_x[0], house_y[0]);
	for(size_t i = 1;i < 11;i++){
		cairo_line_to(cr, house_x[i], house_y[i]);
	}
	cairo_fill(cr);
	
	
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_set_source_rgba (cr, 1-button->transition,1-button->transition,1-button->transition,0.2);
	cairo_fill(cr);
	
	cairo_destroy (cr);
}

map_data_state_t init_map_data_state(void){
	map_data_state_t out;
	
	out.surface = NULL;
	out.drawing_area = NULL;
	out.idle_drawing_function_id = 0;
	out.mouse_x = 0;
	out.mouse_y = 0;
	out.mouse_down = false;
	out.p_mouse_down = false;
	
	out.buttons_capacity = DEFAULT_BUTTONS_CAPACITY;
	out.n_buttons = 0;
	out.buttons = (in_map_button_t**) malloc(sizeof(in_map_button_t*) * out.buttons_capacity);
	
	out.zoom_in_button = create_map_button(40,40,&out);
	out.zoom_out_button = create_map_button(40,40,&out);
	out.home_button = create_map_button(40,40,&out);
	
	out.node_popup = create_node_popup(300,180,&out);
	out.node_popup->x = 300;
	out.node_popup->y = 300;
	map_node_t * test_node = create_map_node(create_cord(-76.71372,39.25297));
	set_map_node_name(test_node,"Northern door",NULL);//TODO err ctx
	building_t * build = create_building("ITE",create_map_rect(create_cord(0,0),create_cord(1,1)),2,NULL);//TODO err ctx
	set_map_node_building(test_node,build,NULL);//TODO err ctx
	out.node_popup->node_reference = test_node;
	
	return out;
}

static void clear_map_surface(map_data_state_t * mds){
	cairo_t * cr = cairo_create (mds->surface);

	cairo_set_source_rgb (cr, 0.2, 0.8, 0.5);
	cairo_paint (cr);

	cairo_destroy (cr);
}

void draw_map_drawing_area_callback (GtkDrawingArea *drawing_area,cairo_t *cr, int width,int height,map_data_state_t * mds){
	
	clear_map_surface(mds);
	
	render_zoom_in_button(mds->zoom_in_button,mds);
	render_zoom_out_button(mds->zoom_out_button,mds);
	render_home_button(mds->home_button,mds);
	
	render_node_popup(mds->node_popup,mds);
	
	//map_draw_brush (mds);
	
	cairo_set_source_surface (cr, mds->surface, 0, 0);
	cairo_paint (cr);
}