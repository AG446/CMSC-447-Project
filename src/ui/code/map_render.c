#include "map_render.h"
#include "map_serial.h"

#define MAP_FONT_SIZE 14

//---------------------------------------------------------- FLT COLOR BEGIN --------------------------------------------------

void set_color(cairo_t * cr,flt_color_t color){
	cairo_set_source_rgba(cr,color.r,color.g,color.b,color.a);
}

flt_color_t sidewalk_clr = {"Sidewalk",1.0,1.0,0.4,1.0};
flt_color_t ramp_clr = {"Ramp",1.0,0.0,1.0,1.0};
flt_color_t stairs_clr = {"Stairs",1.0,0.2,0.2,1.0};
flt_color_t road_clr = {"Road",0.3,0.3,0.3,0.8};
flt_color_t construction_clr = {"Construction",1.0,0.6,0.0,1.0};
flt_color_t crosswalk_clr = {"Crosswalk",1.0,1.0,1.0,1.0};

flt_color_t * color_keys[N_COLOR_KEYS] = {
	&sidewalk_clr,
	&ramp_clr,
	&stairs_clr,
	&road_clr,
	&construction_clr,
	&crosswalk_clr
};
//---------------------------------------------------------- FLT COLOR END ----------------------------------------------------

//---------------------------------------------------------- MOUSE STATE BEGIN ------------------------------------------------
mouse_state_t init_mouse_state(void){
	mouse_state_t out;
	
	out.x = 0;
	out.y = 0;
	out.prev_x = 0;
	out.prev_y = 0;
	out.down = false;
	out.prev_down = false;
	
	return out;
}

void set_mouse_button_down(mouse_state_t * mouse_state,bool state){
	mouse_state->down = state;
}

void set_mouse_position(mouse_state_t * mouse_state,double mouse_x,double mouse_y){
	mouse_state->x = mouse_x;
	mouse_state->y = mouse_y;
}

void mouse_state_step(mouse_state_t * mouse_state){
	mouse_state->prev_down = mouse_state->down;
	mouse_state->prev_x = mouse_state->x;
	mouse_state->prev_y = mouse_state->y;
}

bool mouse_clicked(mouse_state_t mouse_state){
	return mouse_state.down && !mouse_state.prev_down;
}
//---------------------------------------------------------- MOUSE STATE END --------------------------------------------------

//---------------------------------------------------------- SCREEN PAN BEGIN -------------------------------------------------
#define ZOOM_TRANSITION_CONSTANT 0.03

screen_pan_t init_screen_pan(void){
	screen_pan_t out;
	
	out.zoom_jump = 1.4;
	
	out.min_zoom = 0.8;
	out.max_zoom = 6.0;
	out.default_zoom = 2.4;
	out.zoom_current = out.default_zoom;
	out.zoom_final = out.default_zoom;
	
	out.min_pan_x = -0.5;
	out.max_pan_x = 0.5;
	out.default_pan_x = 0.15;
	out.pan_x_current = out.default_pan_x;
	out.pan_x_final = out.default_pan_x;
	
	out.min_pan_y = -0.5;
	out.max_pan_y = 0.5;
	out.default_pan_y = -0.15;
	out.pan_y_current = out.default_pan_y;
	out.pan_y_final = out.default_pan_y;
	
	return out;
}

void screen_pan_update(screen_pan_t * screen_pan){
	if(screen_pan->pan_x_final < screen_pan->min_pan_x) screen_pan->pan_x_final = screen_pan->min_pan_x;
	else if(screen_pan->pan_x_final > screen_pan->max_pan_x) screen_pan->pan_x_final = screen_pan->max_pan_x;
	
	if(screen_pan->pan_y_final < screen_pan->min_pan_y) screen_pan->pan_y_final = screen_pan->min_pan_y;
	else if(screen_pan->pan_y_final > screen_pan->max_pan_y) screen_pan->pan_y_final = screen_pan->max_pan_y;
	
	if(screen_pan->zoom_final > screen_pan->max_zoom) screen_pan->zoom_final = screen_pan->max_zoom;
	else if(screen_pan->zoom_final < screen_pan->min_zoom) screen_pan->zoom_final = screen_pan->min_zoom;
	
	screen_pan->zoom_current = ZOOM_TRANSITION_CONSTANT*screen_pan->zoom_final + screen_pan->zoom_current*(1-ZOOM_TRANSITION_CONSTANT);
	screen_pan->pan_x_current = ZOOM_TRANSITION_CONSTANT*screen_pan->pan_x_final + screen_pan->pan_x_current*(1-ZOOM_TRANSITION_CONSTANT);
	screen_pan->pan_y_current = ZOOM_TRANSITION_CONSTANT*screen_pan->pan_y_final + screen_pan->pan_y_current*(1-ZOOM_TRANSITION_CONSTANT);
}

void screen_pan_home(screen_pan_t * screen_pan){
	screen_pan->zoom_final = screen_pan->default_zoom;
	screen_pan->pan_x_final = screen_pan->default_pan_x;
	screen_pan->pan_y_final = screen_pan->default_pan_y;
}

void screen_pan_zoom_in(screen_pan_t * screen_pan){
	screen_pan->zoom_final *= screen_pan->zoom_jump;
}

void screen_pan_zoom_out(screen_pan_t * screen_pan){
	screen_pan->zoom_final *= 1.0/screen_pan->zoom_jump;
}

//---------------------------------------------------------- SCREEN PAN END ---------------------------------------------------


//---------------------------------------------------------- Button Collection BEGIN ------------------------------------------
#define DEFAULT_BUTTONS_CAPACITY 4

button_collection_t init_button_collection(void){
	button_collection_t out;
	
	out.buttons_capacity = DEFAULT_BUTTONS_CAPACITY;
	out.n_buttons = 0;
	out.buttons = (in_map_button_t**) malloc(sizeof(in_map_button_t*) * out.buttons_capacity);
	
	return out;
}

void add_button_to_button_collection(button_collection_t * collection,in_map_button_t * button){
	if(collection->buttons_capacity == collection->n_buttons){
		collection->buttons_capacity *= 2;
		collection->buttons = (in_map_button_t**)realloc(collection->buttons,sizeof(in_map_button_t*) * collection->buttons_capacity);
	}
	
	collection->buttons[collection->n_buttons] = button;
	collection->n_buttons++;
}

bool update_button_collection(button_collection_t * collection,mouse_state_t mouse_state){
	bool mouse_over_button = false;
	for(size_t i = 0;i < collection->n_buttons;i++){
		update_button(collection->buttons[i],&mouse_over_button,mouse_state);
	}
	return mouse_over_button;
}

//---------------------------------------------------------- Button Collection END --------------------------------------------

//---------------------------------------------------------- IN MAP BUTTON BEGIN ----------------------------------------------
#define IN_MAP_BUTTON_TRANSITION_CONSTANT 0.03

in_map_button_t * create_map_button(double width,double height,button_collection_t * collection){
	in_map_button_t * out = (in_map_button_t*) malloc(sizeof(in_map_button_t));
	
	out->x = 0;
	out->y = 0;
	out->width = width;
	out->height = height;
	out->animation_transition = 0.0;
	out->was_pressed = false;
	out->mouse_on = false;
	out->pressed_on = false;
	out->visible = true;
	
	add_button_to_button_collection(collection,out);
	
	return out;
}

bool was_pressed_reset(in_map_button_t * button){
	bool out = button->was_pressed;
	button->was_pressed = false;
	return out;
}

bool update_button(in_map_button_t * button,bool * mouse_over_button,mouse_state_t mouse_state){
	if(!button->visible) return false;
	
	bool on = !*mouse_over_button && mouse_state.x > button->x && mouse_state.y > button->y && mouse_state.x < button->x + button->width && mouse_state.y < button->y + button->height;
	*mouse_over_button |= on;
	
	if(on){
		button->animation_transition = IN_MAP_BUTTON_TRANSITION_CONSTANT + button->animation_transition*(1-IN_MAP_BUTTON_TRANSITION_CONSTANT);
		button->mouse_on = true;
		button->pressed_on = mouse_state.down;
		
		if(mouse_clicked(mouse_state)){
			button->was_pressed = true;
		}
		return true;
	}else{
		button->mouse_on = false;
		button->pressed_on = false;
		button->animation_transition = button->animation_transition*(1-IN_MAP_BUTTON_TRANSITION_CONSTANT);
	}
	
	return false;
}
//---------------------------------------------------------- IN MAP BUTTON END ------------------------------------------------

//---------------------------------------------------------- ON SCREEN NODE BEGIN ---------------------------------------------
#define SELECTABLE_NODE_SIZE 1.5
#define SELECTABLE_NODE_EXPAND_FACTOR_AMOUNT 0.3

on_screen_node_t init_on_screen_node(map_node_t * node_ref,button_collection_t * collection){
	on_screen_node_t out;
	
	out.button = create_map_button(0,0,collection);
	
	out.node_reference = node_ref;
	
	return out;
}

void render_on_screen_node(on_screen_node_t * screen_node,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor){
	if(!screen_node->button->visible) return;
	
	float radius = screen_pan.zoom_current*SELECTABLE_NODE_SIZE*(1.0+screen_node->button->animation_transition*SELECTABLE_NODE_EXPAND_FACTOR_AMOUNT);
	
	double x = 0.0,y = 0.0;
	calculate_on_screen_pos(screen_node->node_reference->coordinate,&x,&y,surface,screen_pan,map_bounding_box,scaling_y_factor);
	
	cairo_t * cr = cairo_create (surface);
	screen_node->button->x = x-radius;
	screen_node->button->y = y-radius;
	screen_node->button->width = radius*2.0;
	screen_node->button->height = radius*2.0;
	
	cairo_arc(cr,x,y,radius,0,2.0*M_PI);
	cairo_close_path(cr);
	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr,0.0,0.0,0.0);
	cairo_set_line_width(cr,radius/2.0);
	cairo_stroke(cr);
	
	cairo_destroy (cr);
}
//---------------------------------------------------------- ON SCREEN NODE END -----------------------------------------------

//---------------------------------------------------------- NODE POPUP BEGIN -------------------------------------------------
#define CLOSE_BUTTON_HOVER_EXPAND_AMOUNT 1
#define CLOSE_BUTTON_LINE_THICKNESS 3
#define CLOSE_BUTTON_CORNER_RADIUS 4
#define CLOSE_BUTTON_X_SHAPE_PADDING 4

#define TEXT_BUTTON_HOVER_EXPAND_AMOUNT 3
#define TEXT_BUTTON_LINE_THICKNESS 2
#define TEXT_BUTTON_PADDING 8

#define NODE_POPUP_TICK_SIZE 16
#define NODE_POPUP_FRAME_PADDING 10
#define NODE_POPUP_TEXT_LINE_GAP 4
#define NODE_POPUP_LINE_THICKNESS 3
#define NODE_POPUP_TRANSITION_CONSTANT 0.03

node_popup_t init_node_popup(button_collection_t * collection){
	node_popup_t out;
	
	out.x = 0;
	out.y = 0;
	out.width = 0.0;
	out.height = 220.0;
	
	out.animation_transition = 0.5;
	
	out.node_reference = NULL;
	
	out.close_button = create_map_button(16,16,collection);
	out.set_start_button = create_map_button(64,32,collection);
	out.set_end_button = create_map_button(64,32,collection);
	
	out.disable_buttons = false;
	
	out.visible = false;
	
	return out;
}

void update_node_popup(node_popup_t * node_popup,map_pin_t * start_pin,map_pin_t * end_pin){
	if(node_popup->visible){
		node_popup->animation_transition = NODE_POPUP_TRANSITION_CONSTANT + node_popup->animation_transition*(1-NODE_POPUP_TRANSITION_CONSTANT);
	}else{
		node_popup->animation_transition = node_popup->animation_transition*(1-NODE_POPUP_TRANSITION_CONSTANT);
		return;
	}
	
	if(was_pressed_reset(node_popup->close_button)) close_node_popup(node_popup);
	
	node_popup->disable_buttons = (start_pin->visible && start_pin->node_reference == node_popup->node_reference) || (end_pin->visible && end_pin->node_reference == node_popup->node_reference);
	
	if(was_pressed_reset(node_popup->set_start_button)){
		if(!node_popup->disable_buttons) notify_map_pin(start_pin,node_popup->node_reference);
	}
	else if(was_pressed_reset(node_popup->set_end_button)){
		if(!node_popup->disable_buttons) notify_map_pin(end_pin,node_popup->node_reference);
	}
}

static void render_popup_close_button(in_map_button_t * button,cairo_surface_t * surface){
	if(!button->visible) return;
	
	cairo_t * cr = cairo_create(surface);
	
	double hover_expand_amount = button->animation_transition*CLOSE_BUTTON_HOVER_EXPAND_AMOUNT;
	
	{//red box background
		cairo_set_source_rgb (cr, 0.8,0.1,0.1);
		
		double corner_radius = CLOSE_BUTTON_CORNER_RADIUS;
		
		double inner_left = button->x + corner_radius - hover_expand_amount;
		double inner_right = button->x + button->width - corner_radius + hover_expand_amount;
		double inner_top = button->y + corner_radius - hover_expand_amount;
		double inner_bottom = button->y + button->height - corner_radius + hover_expand_amount;
		
		cairo_arc (cr, inner_right, inner_top, corner_radius, -M_PI/2, 0);
		cairo_arc (cr, inner_right, inner_bottom, corner_radius, 0, M_PI/2);
		cairo_arc (cr, inner_left, inner_bottom, corner_radius, M_PI/2, M_PI);
		cairo_arc (cr, inner_left, inner_top, corner_radius, M_PI, M_PI*3/2);
		
		cairo_close_path (cr);
		cairo_fill(cr);
	}
	
	{//diagonals to make x shape
		cairo_set_source_rgb(cr,1,1,1);
		cairo_set_line_width (cr, CLOSE_BUTTON_LINE_THICKNESS);
		
		double left = button->x + CLOSE_BUTTON_X_SHAPE_PADDING - hover_expand_amount;
		double right = button->x+button->width - CLOSE_BUTTON_X_SHAPE_PADDING + hover_expand_amount;
		double top = button->y + CLOSE_BUTTON_X_SHAPE_PADDING - hover_expand_amount;
		double bottom = button->y+button->height - CLOSE_BUTTON_X_SHAPE_PADDING + hover_expand_amount;
		
		cairo_move_to(cr, left, top);
		cairo_line_to(cr, right, bottom);
		cairo_stroke(cr);
		
		cairo_move_to(cr, left, bottom);
		cairo_line_to(cr, right, top);
		cairo_stroke(cr);
	}
	
	cairo_destroy (cr);
}

static void render_text_button(in_map_button_t * button,double r,double g,double b,const char * button_text,cairo_surface_t * surface){
	if(!button->visible) return;
	
	cairo_t * cr = cairo_create(surface);
	
	double hover_expand_amount = button->animation_transition*TEXT_BUTTON_HOVER_EXPAND_AMOUNT;
	
	{//button background
		double corner_radius = 6;
		
		double inner_left = button->x + corner_radius - hover_expand_amount;
		double inner_right = button->x + button->width - corner_radius + hover_expand_amount;
		double inner_top = button->y + corner_radius - hover_expand_amount;
		double inner_bottom = button->y + button->height - corner_radius + hover_expand_amount;
		
		cairo_arc (cr, inner_right, inner_top, corner_radius, -M_PI/2, 0);
		cairo_arc (cr, inner_right, inner_bottom, corner_radius, 0, M_PI/2);
		cairo_arc (cr, inner_left, inner_bottom, corner_radius, M_PI/2, M_PI);
		cairo_arc (cr, inner_left, inner_top, corner_radius, M_PI, M_PI*3/2);
		
		cairo_close_path (cr);
		
		if(button->pressed_on) cairo_set_source_rgb(cr,r*0.5,g*0.5,b*0.5);
		else cairo_set_source_rgb(cr,r,g,b);
		
		cairo_fill_preserve(cr);
		cairo_set_source_rgb(cr,0,0,0);
		cairo_set_line_width (cr, TEXT_BUTTON_LINE_THICKNESS);
		cairo_stroke(cr);
	}
	
	{//render text on button
		cairo_set_font_size (cr, MAP_FONT_SIZE);
		cairo_text_extents_t extents;
		cairo_text_extents (cr, button_text, &extents);
		button->width = extents.width+2*TEXT_BUTTON_PADDING;
		
		cairo_set_font_size (cr, MAP_FONT_SIZE + hover_expand_amount/2);
		cairo_text_extents (cr, button_text, &extents);
		
		cairo_set_source_rgb(cr,0,0,0);
		double center_x = button->x + button->width/2 - extents.width/2;
		double center_y = button->y + button->height/2 + extents.height/2;
		cairo_move_to (cr, center_x, center_y);
		cairo_show_text (cr, button_text);
	}
	
	cairo_destroy (cr);
}

void render_node_popup(node_popup_t * node_popup,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor){
	if(!node_popup->visible){
		if(node_popup->animation_transition < 0.3) return;
	}
	
	cairo_t * cr = cairo_create(surface);
	
	double width = node_popup->width * node_popup->animation_transition;
	double height = node_popup->height * node_popup->animation_transition;
	
	double x = 0.0,y = 0.0;
	if(node_popup->node_reference != NULL){
		calculate_on_screen_pos(node_popup->node_reference->coordinate,&x,&y,surface,screen_pan,map_bounding_box,scaling_y_factor);
		node_popup->x = x;
		node_popup->y = y;
	}
	
	double tick_size = NODE_POPUP_TICK_SIZE;
	
	double center_x = node_popup->x;
	double left = center_x - width/2;
	double right = center_x + width/2;
	double bottom = node_popup->y;
	double frame_bottom = bottom - tick_size;
	double frame_top = frame_bottom-height;
	
	{//draw the background frame
		
		cairo_set_source_rgba(cr,1,1,1,node_popup->animation_transition);
		cairo_move_to(cr, center_x, bottom);
		cairo_line_to(cr, center_x - tick_size,frame_bottom);
		cairo_line_to(cr, left,frame_bottom);
		cairo_line_to(cr, left,frame_top);
		cairo_line_to(cr, right,frame_top);
		cairo_line_to(cr, right,frame_bottom);
		cairo_line_to(cr, center_x + tick_size,frame_bottom);
		cairo_close_path(cr);
		cairo_fill_preserve (cr);
		cairo_set_source_rgba(cr,0,0,0,node_popup->animation_transition);
		cairo_set_line_width (cr, NODE_POPUP_LINE_THICKNESS);
		cairo_stroke(cr);
	}
	
	bool hide_inner_elements = node_popup->animation_transition < 0.8;//frame too small
	
	double gap = NODE_POPUP_FRAME_PADDING;
	double inner_right = right - gap;
	double inner_top = frame_top + gap;
	double inner_left = left + gap;
	double inner_bottom = frame_bottom - gap;
	
	double maximal_width = 0.0;//calculate the width of the frame
	
	{//draw close button
		node_popup->close_button->x = inner_right - node_popup->close_button->width;
		node_popup->close_button->y = inner_top;
		if(!hide_inner_elements) render_popup_close_button(node_popup->close_button,surface);
	}
	
	{//draw start button
		node_popup->set_start_button->x = inner_left;
		node_popup->set_start_button->y = inner_bottom - node_popup->set_start_button->height;
		double r = 0.0,g = 0.0,b = 0.0;
		if(node_popup->disable_buttons){
			r = 0.4;
			g = 0.4;
			b = 0.4;
		}else{
			r = 0.4;
			g = 0.6;
			b = 0.8;
		}
		if(!hide_inner_elements) render_text_button(node_popup->set_start_button,r,g,b,"Set as Start",surface);
	}
	
	{//draw end button
		node_popup->set_end_button->x = inner_left + node_popup->set_start_button->width + gap;
		node_popup->set_end_button->y = inner_bottom - node_popup->set_end_button->height;
		double r = 0.0,g = 0.0,b = 0.0;
		if(node_popup->disable_buttons){
			r = 0.4;
			g = 0.4;
			b = 0.4;
		}else{
			r = 0.8;
			g = 0.4;
			b = 0.4;
		}
		if(!hide_inner_elements) render_text_button(node_popup->set_end_button,r,g,b,"Set as End",surface);
	}
	
	maximal_width = fmaxf(node_popup->set_start_button->width + gap + node_popup->set_end_button->width, maximal_width );
	
	if(node_popup->node_reference != NULL){//draw the text
		cairo_set_font_size (cr, MAP_FONT_SIZE);
		double line_gap = NODE_POPUP_TEXT_LINE_GAP;
		double line_spacing = MAP_FONT_SIZE + line_gap;
		
		cairo_text_extents_t extents;
		
		double text_y = inner_top+MAP_FONT_SIZE;
		
		const map_node_t * node_ref = node_popup->node_reference;
		
		const char * node_name = get_map_node_name(node_ref,NULL);//TODO err ctx
		building_t * building = node_ref->associated_building;
		const char * building_name = building == NULL ? NULL : get_primary_building_name(building,NULL);//TODO err ctx
		double lon = node_ref->coordinate.longitude;
		double lat = node_ref->coordinate.latitude;
		char * floor_number_str = get_map_node_floor_number_name(node_ref,NULL);//TODO err_ctx_t
		const char * automatic_door_yes_no = node_adjacent_to_auto_door(node_ref,NULL) ? "Yes" : "No";//TODO err_ctx_t
		
		char * full_buffer = (char*) malloc(256);
		sprintf(
			full_buffer,
			"Location Name:\n\t%s\nAssociated Building:\n\t%s\nCoordinate:\n\t(lon=%lf,lat=%lf)\nFloor: %s\nAutomatic Door: %s"
			,node_name,building_name,lon,lat,floor_number_str,automatic_door_yes_no
		);
		
		free(floor_number_str);
		
		char * line = strtok(full_buffer, "\n");

		while (line != NULL) {
			double offset = 0.0;
			if(line[0] == '\t'){
				line++;
				offset += MAP_FONT_SIZE*2;
			}
			cairo_text_extents (cr, line, &extents);
			maximal_width = fmax(extents.width+offset,maximal_width);
			if(!hide_inner_elements){
				cairo_move_to (cr, inner_left+offset, text_y);
				cairo_show_text (cr, line);
			}
			line = strtok(NULL, "\n");
			text_y += line_spacing;
		}
		
		free(full_buffer);
	}
	
	node_popup->width = maximal_width + 2.0*gap;
	
	cairo_destroy (cr);
}

void notify_node_popup(node_popup_t * node_popup,map_node_t * new_node_reference){
	node_popup->visible = true;
	node_popup->close_button->visible = true;
	node_popup->set_start_button->visible = true;
	node_popup->set_end_button->visible = true;
	
	node_popup->animation_transition = 0.0;
	node_popup->node_reference = new_node_reference;
}

void close_node_popup(node_popup_t * node_popup){
	node_popup->visible = false;
	node_popup->close_button->visible = false;
	node_popup->set_start_button->visible = false;
	node_popup->set_end_button->visible = false;
}
//---------------------------------------------------------- NODE POPUP END ---------------------------------------------------

//---------------------------------------------------------- MAP PIN BEGIN ----------------------------------------------------
#define PIN_RADIUS 15
#define PIN_HEIGHT 50
#define PIN_OUTLINE_LINE_THICKNESS 3
#define PIN_TRANSITION_CONSTANT 0.03

map_pin_t init_map_pin(double r,double g,double b){
	map_pin_t out;
	
	out.r = r;
	out.g = g;
	out.b = b;
	
	out.x = 0.0;
	out.y = 0.0;
	
	out.node_reference = NULL;
	
	out.animation_transition = 0.0;
	
	out.visible = true;
	
	return out;
}

void update_map_pin(map_pin_t * pin){
	if(pin->visible){
		pin->animation_transition = PIN_TRANSITION_CONSTANT + pin->animation_transition*(1-PIN_TRANSITION_CONSTANT);
	}else{
		pin->animation_transition = PIN_TRANSITION_CONSTANT*0.5*pin->animation_transition + pin->animation_transition*(1-PIN_TRANSITION_CONSTANT);
	}
}

void render_map_pin(map_pin_t * pin,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor){
	if(pin->animation_transition < 0.2) return;
	
	if(pin->node_reference != NULL){
		calculate_on_screen_pos(pin->node_reference->coordinate,&(pin->x),&(pin->y),surface,screen_pan,map_bounding_box,scaling_y_factor);
	}
	
	cairo_t * cr = cairo_create(surface);
	
	double radius = PIN_RADIUS*pin->animation_transition;
	double pin_height = PIN_HEIGHT*pin->animation_transition;
	
	double theta = atan(radius/(pin_height-radius));
	
	cairo_set_source_rgba(cr,pin->r,pin->g,pin->b,0.8);
	cairo_arc (cr, pin->x, pin->y-pin_height+radius, radius, M_PI-theta, theta);
	cairo_line_to(cr,pin->x,pin->y);
	cairo_close_path(cr);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr,0,0,0,0.7);
	cairo_set_line_width (cr, PIN_OUTLINE_LINE_THICKNESS);
	cairo_stroke(cr);
	cairo_arc(cr,pin->x,pin->y-pin_height+radius,radius/2,0,2*M_PI);
	cairo_set_source_rgba(cr,1,1,1,0.8);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr,0,0,0,0.7);
	cairo_stroke(cr);
	
	cairo_destroy (cr);
}

void notify_map_pin(map_pin_t * pin,map_node_t * new_node_reference){
	pin->node_reference = new_node_reference;
	pin->visible = true;
	pin->animation_transition = 0.0;
}

void hide_map_pin(map_pin_t * pin){
	pin->visible = false;
}
//---------------------------------------------------------- MAP PIN END ------------------------------------------------------

//---------------------------------------------------------- ZOOM CONTROLS BEGIN ----------------------------------------------
#define ZOOM_CONTROLS_OUTLINE_THICKNESS 2
#define ZOOM_CONTROLS_PADDING 10
#define ZOOM_CONTROLS_BUTTON_SPACING 10
#define ZOOM_CONTROLS_BUTTON_EXPAND_AMOUNT 2
#define ZOOM_CONTROLS_PLUS_MINUS_LINE_THICKNESS 4

zoom_controls_t init_zoom_controls(button_collection_t * collection){
	zoom_controls_t out;
	
	out.zoom_in_button = create_map_button(40,40,collection);
	out.zoom_out_button = create_map_button(40,40,collection);
	out.home_button = create_map_button(40,40,collection);
	
	return out;
}

void update_zoom_controls(zoom_controls_t * zoom_controls,screen_pan_t * screen_pan){
	if(was_pressed_reset(zoom_controls->zoom_in_button)) screen_pan_zoom_in(screen_pan);
	else if(was_pressed_reset(zoom_controls->zoom_out_button)) screen_pan_zoom_out(screen_pan);
	else if(was_pressed_reset(zoom_controls->home_button)) screen_pan_home(screen_pan);
}

static double get_zoom_control_button_radius(in_map_button_t * button){
	return button->width/2 + button->animation_transition*ZOOM_CONTROLS_BUTTON_EXPAND_AMOUNT;
}

static void render_zoom_control_button_background(in_map_button_t * button,cairo_surface_t * surface){
	cairo_t * cr = cairo_create(surface);
	
	double radius = get_zoom_control_button_radius(button);
	
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_set_source_rgb (cr, 1,1,1);
	cairo_set_line_width (cr, ZOOM_CONTROLS_OUTLINE_THICKNESS);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb (cr, 0,0,0);
	cairo_stroke(cr);
	
	cairo_destroy (cr);
}

static void render_zoom_control_button_hover_fade(in_map_button_t * button,cairo_surface_t * surface){
	cairo_t * cr = cairo_create(surface);
	
	double radius = get_zoom_control_button_radius(button);
	
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_set_source_rgba (cr, 1-button->animation_transition,1-button->animation_transition,1-button->animation_transition,0.2);
	cairo_fill(cr);
	
	cairo_destroy (cr);
}

static void render_zoom_in_zoom_out_button(in_map_button_t * button,bool zoom_in,cairo_surface_t * surface){
	if(!button->visible) return;
	
	cairo_t * cr = cairo_create(surface);
	
	double radius = get_zoom_control_button_radius(button);
	render_zoom_control_button_background(button,surface);
	
	double inner_radius = radius*0.6;
	
	if(zoom_in){
		cairo_set_source_rgb (cr, 1,0,0);
	}else{
		cairo_set_source_rgb (cr, 0,0,1);
	}
	
	double center_x = button->x+button->width/2;
	double center_y = button->y+button->height/2;
	
	cairo_set_line_width(cr,ZOOM_CONTROLS_PLUS_MINUS_LINE_THICKNESS);
	
	cairo_move_to(cr,center_x-inner_radius,center_y);
	cairo_line_to(cr,center_x+inner_radius,center_y);
	if(zoom_in){
		cairo_move_to(cr,center_x,center_y-inner_radius);
		cairo_line_to(cr,center_x,center_y+inner_radius);
	}
	cairo_stroke (cr);
	
	render_zoom_control_button_hover_fade(button,surface);

	cairo_destroy (cr);
}

static void render_zoom_in_button(in_map_button_t * button,cairo_surface_t * surface){
	render_zoom_in_zoom_out_button(button,true,surface);
}

static void render_zoom_out_button(in_map_button_t * button,cairo_surface_t * surface){
	render_zoom_in_zoom_out_button(button,false,surface);
}

#define HOUSE_ICON_N_POINTS 11
static const double house_icon_x[HOUSE_ICON_N_POINTS] = {-0.25,-0.25,0.25,0.25,0.75,0.75,1.0,0.0,-1.0,-0.75,-0.75};
static const double house_icon_y[HOUSE_ICON_N_POINTS] = {1,0.5,0.5,1,1,0,0,-1,0,0,1};

static void render_home_button(in_map_button_t * button,cairo_surface_t * surface){
	if(!button->visible) return;
	cairo_t * cr = cairo_create(surface);
	
	double radius = get_zoom_control_button_radius(button);
	render_zoom_control_button_background(button,surface);
	
	double inner_radius = radius*0.6;
	
	double center_x = button->x+button->width/2;
	double center_y = button->y+button->height/2;
	
	double house_icon_x_scaled[HOUSE_ICON_N_POINTS];
	double house_icon_y_scaled[HOUSE_ICON_N_POINTS];
	
	for(size_t i = 0;i < HOUSE_ICON_N_POINTS;i++){
		house_icon_x_scaled[i] = house_icon_x[i] * inner_radius + center_x;
		house_icon_y_scaled[i] = house_icon_y[i] * inner_radius + center_y;
	}
	
	cairo_set_source_rgb (cr, 0,0,0);
	cairo_move_to(cr, house_icon_x_scaled[0], house_icon_y_scaled[0]);
	for(size_t i = 1;i < HOUSE_ICON_N_POINTS;i++){
		cairo_line_to(cr, house_icon_x_scaled[i], house_icon_y_scaled[i]);
	}
	cairo_fill(cr);
	
	render_zoom_control_button_hover_fade(button,surface);
	
	cairo_destroy (cr);
}

void render_zoom_controls(zoom_controls_t * zoom_controls,cairo_surface_t * surface){
	render_zoom_in_button(zoom_controls->zoom_in_button,surface);
	render_zoom_out_button(zoom_controls->zoom_out_button,surface);
	render_home_button(zoom_controls->home_button,surface);
}

void recalculate_zoom_controls_position(zoom_controls_t * zoom_controls,int surface_width){
	double gap = ZOOM_CONTROLS_PADDING;
	double spacing = ZOOM_CONTROLS_BUTTON_SPACING;
	
	double current_y = gap;
	
	zoom_controls->zoom_in_button->x = surface_width - zoom_controls->zoom_in_button->width - gap;
	zoom_controls->zoom_in_button->y = current_y;
	
	current_y += zoom_controls->zoom_in_button->height + spacing;
	
	zoom_controls->zoom_out_button->x = surface_width - zoom_controls->zoom_in_button->width - gap;
	zoom_controls->zoom_out_button->y = current_y;
	
	current_y += zoom_controls->zoom_out_button->height + spacing;
	
	zoom_controls->home_button->x = surface_width - zoom_controls->home_button->width - gap;
	zoom_controls->home_button->y = current_y;
}
//---------------------------------------------------------- ZOOM CONTROLS END ------------------------------------------------

//---------------------------------------------------------- ON SCREEN MAP BEGIN ----------------------------------------------
#define PATH_TRANSITION_STEP 0.003

#define COLOR_KEYS_MARGIN 5
#define COLOR_KEYS_PADDING 3
#define COLOR_KEYS_SPACING 10
#define COLOR_KEY_SWATCH_SIZE 20
#define COLOR_KEYS_OUTLINE_THICKNESS 2

static size_t count_selectable_nodes(map_sys_t map_sys){
	size_t total_selectable_nodes = 0;
	
	for(size_t i = 0;i < get_map_node_count(map_sys.map);i++){
		map_node_t * node_ref = get_node_by_index_from_map(map_sys.map,i,NULL);//TODO add err_ctx_t
		if(node_ref->selectable) total_selectable_nodes++;
	}
	
	return total_selectable_nodes;
}

on_screen_map_t init_on_screen_map(const char * map_file_path){
	on_screen_map_t out;
	
	out.surface = NULL;
	out.drawing_area = NULL;
	out.idle_drawing_function_id = 0;
	
	out.mouse_state = init_mouse_state();
	
	out.map_sys = init_map_sys();
	out.map_sys.map = load_map_from_file(map_file_path,NULL);//TODO err ctx
	out.map_bounding_box = get_map_bounding_rect(out.map_sys.map);
	
	out.screen_pan = init_screen_pan();
	
	out.button_collection = init_button_collection();
	
	out.zoom_controls = init_zoom_controls(&(out.button_collection));
	
	out.node_popup = init_node_popup(&(out.button_collection));
	
	out.n_on_screen_nodes = count_selectable_nodes(out.map_sys);
	out.all_on_screen_nodes = (on_screen_node_t*) malloc(sizeof(on_screen_node_t) * out.n_on_screen_nodes);
	size_t write_index = 0;
	for(size_t i = 0; i < get_map_node_count(out.map_sys.map);i++){
		map_node_t * node_ref = out.map_sys.map.all_nodes[i];
		if(node_ref->selectable){
			out.all_on_screen_nodes[write_index] = init_on_screen_node(node_ref,&(out.button_collection));
			write_index++;
		}
	}
	
	out.hide_non_auto_doors = false;
	on_screen_map_set_hide_non_auto_doors(&out,out.hide_non_auto_doors);
	out.show_building_names = true;
	
	out.start_pin = init_map_pin(0.2,0.4,1.0);
	out.start_pin.visible = false;
	out.end_pin = init_map_pin(1.0,0.0,0.0);
	out.end_pin.visible = false;
	out.path_animation_transition = 0.0;
	
	out.last_micros = 0;
	out.micros_debt = 0;
	out.delay_time = 0;
	
	return out;
}

static void clear_map_surface(on_screen_map_t * screen_map){
	cairo_t * cr = cairo_create (screen_map->surface);

	cairo_set_source_rgb (cr, 0.2, 0.8, 0.5);
	cairo_paint (cr);

	cairo_destroy (cr);
}

static void set_to_edge_color_and_thickness(cairo_t * cr,const map_edge_t * edge,on_screen_map_t * screen_map){
	uint8_t type = edge->type;
	
	if(type == EDGE_TYPE_SIDEWALK){
		set_color(cr,sidewalk_clr);
	}else if(type == EDGE_TYPE_RAMP){
		set_color(cr,ramp_clr);
	}else if(type == EDGE_TYPE_STAIRS){
		set_color(cr,stairs_clr);
	}else if(type == EDGE_TYPE_ROAD){
		set_color(cr,road_clr);
	}else if(type == EDGE_TYPE_CONSTRUCTION){
		set_color(cr,construction_clr);
	}else if(type == EDGE_TYPE_CROSSWALK){
		set_color(cr,crosswalk_clr);
	}else{
		cairo_set_source_rgb(cr,0,0,0);
	}
	
	double weight_factor = 1.0;
	
	if(type == EDGE_TYPE_ROAD){
		weight_factor = 3;
	}else if(type == EDGE_TYPE_STAIRS){
		weight_factor = 1.0;
	}
	
	cairo_set_line_width (cr, fmax(weight_factor*screen_map->screen_pan.zoom_current,weight_factor));
}

static void render_edges(on_screen_map_t * screen_map){
	cairo_t * cr = cairo_create (screen_map->surface);
	
	for(size_t i = 0;i < get_map_edge_count(screen_map->map_sys.map);i++){
		const map_edge_t * edge = get_edge_by_index_from_map(screen_map->map_sys.map,i,NULL);//TODO err_ctx_t
		const map_node_t * node_a = get_edge_node_a(edge,NULL);//TODO err_ctx_t
		const map_node_t * node_b = get_edge_node_b(edge,NULL);//TODO err_ctx_t
		
		double start_x = 0,start_y = 0;
		calculate_on_screen_pos(node_a->coordinate,&start_x,&start_y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
		double end_x = 0,end_y = 0;
		calculate_on_screen_pos(node_b->coordinate,&end_x,&end_y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
		
		cairo_move_to(cr,start_x,start_y);
		cairo_line_to(cr,end_x,end_y);
		set_to_edge_color_and_thickness(cr,edge,screen_map);
		cairo_stroke(cr);
	}
	
	cairo_destroy(cr);
}

static void render_mpo(const mpo_t * mpo,on_screen_map_t * screen_map){
	cairo_t * cr = cairo_create (screen_map->surface);
	
	for(size_t j = 0;j < get_mpo_size(mpo,NULL);j++){//TODO err_ctx_t
		
		cord_t cord = get_mpo_cord(mpo,j,NULL);//TODO err_ctx_t
		double x = 0.0,y = 0.0;
		calculate_on_screen_pos(cord,&x,&y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
		
		if(j == 0) cairo_move_to(cr,x,y);
		else cairo_line_to(cr,x,y);
	}
	cairo_close_path(cr);
	
	cairo_set_source_rgba(cr,0.7,0.7,0.7,0.9);
	cairo_fill_preserve(cr);
	cairo_set_line_width (cr, fmax(screen_map->screen_pan.zoom_current,3));
	cairo_set_source_rgb(cr,0,0,0);
	cairo_stroke(cr);
	
	cairo_destroy(cr);
}

static void render_mpos(on_screen_map_t * screen_map){
	for(size_t i = 0;i < get_map_mpo_count(screen_map->map_sys.map);i++){
		const mpo_t * mpo = get_mpo_by_index_from_map(screen_map->map_sys.map,i,NULL);//TODO err_ctx_t
		
		render_mpo(mpo,screen_map);
	}
}

static void render_selectable_nodes(on_screen_map_t * screen_map){
	for(size_t i = 0;i < screen_map->n_on_screen_nodes;i++){
		on_screen_node_t * screen_node = &(screen_map->all_on_screen_nodes[i]);
		render_on_screen_node(screen_node,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
	}
}

static void render_building_name(const building_t * building,on_screen_map_t * screen_map){
	cairo_t * cr = cairo_create (screen_map->surface);
	
	const char * building_name = get_primary_building_name(building,NULL);//TODO err_ctx_t
	
	map_rect_t building_bounding_box = get_building_bounding_box(building,NULL);//TODO err_ctx_t
	double bl_x = 0.0,tr_x = 0.0;
	double bl_y = 0.0,tr_y = 0.0;
	
	calculate_on_screen_pos(building_bounding_box.bottom_left,&bl_x,&bl_y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
	calculate_on_screen_pos(building_bounding_box.top_right,&tr_x,&tr_y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
	
	double mean_x = (bl_x + tr_x)*0.5;
	double mean_y = (bl_y + tr_y)*0.5;
	
	double font_size = 5*screen_map->screen_pan.zoom_current;
	cairo_set_font_size (cr, font_size);
	
	cairo_text_extents_t extents;
	cairo_text_extents (cr, building_name , &extents);
	
	cairo_rectangle(cr,mean_x-extents.width/2,mean_y-font_size,extents.width,extents.height+font_size/2);
	cairo_set_source_rgba(cr,1.0,1.0,1.0,0.6);
	cairo_fill(cr);
	
	cairo_move_to (cr, mean_x-extents.width/2,mean_y);
	cairo_set_source_rgb(cr,0,0,0);
	cairo_show_text (cr, building_name);
	
	cairo_destroy(cr);
}

static void render_building_names(on_screen_map_t * screen_map){
	if(!screen_map->show_building_names) return;
	
	for(size_t i = 0;i < get_map_building_count(screen_map->map_sys.map);i++){
		const building_t * building = get_building_by_index_from_map(screen_map->map_sys.map,i,NULL);//TODO err_ctx_t
		render_building_name(building,screen_map);
	}
}

static void render_path(on_screen_map_t * screen_map){
	cairo_t * cr = cairo_create (screen_map->surface);
	
	if(screen_map->map_sys.active_path == NULL) return;
	
	map_path_t * path = screen_map->map_sys.active_path;
	
	double * x_values = (double*) malloc(sizeof(double) * path->n_nodes);
	double * y_values = (double*) malloc(sizeof(double) * path->n_nodes);
	double * cummalative_dist = (double*) malloc(sizeof(double) * path->n_nodes);
	double total_dist = 0.0;
	
	double x_prev = 0.0,y_prev = 0.0;
	for(size_t i = 0;i < path->n_nodes;i++){
		cord_t cord = path->nodes[i]->coordinate;
		double x = 0.0,y = 0.0;
		calculate_on_screen_pos(cord,&x,&y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
		x_values[i] = x;
		y_values[i] = y;
		
		double dx = 0.0,dy = 0.0;
		if(i != 0){
			dx = x - x_prev;
			dy = y - y_prev;
		}
		
		double dist = sqrt(dx*dx+dy*dy);
		total_dist += dist;
		cummalative_dist[i] = total_dist;
		
		x_prev = x;
		y_prev = y;
	}
	
	//normalize
	for(size_t i = 0;i < path->n_nodes;i++){
		cummalative_dist[i] /= total_dist;
	}
	
	cairo_set_source_rgba(cr,0.3,0.5,1.0,0.8);
	cairo_set_line_width(cr,2*screen_map->screen_pan.zoom_current);
	
	for(size_t i = 0;i < path->n_nodes;i++){
		bool halt_after = cummalative_dist[i] > screen_map->path_animation_transition;
		
		if(i == 0) cairo_move_to(cr,x_values[i],y_values[i]);
		else{
			if(!halt_after){
				cairo_line_to(cr,x_values[i],y_values[i]);
			}else{
				double prev_x = x_values[i-1],prev_y = y_values[i-1];
				double final_x = x_values[i],final_y = y_values[i];
				
				double normalized_line_length = cummalative_dist[i] - cummalative_dist[i-1];
				double line_percent = (screen_map->path_animation_transition - cummalative_dist[i-1])/normalized_line_length;
				
				double lerp_x = final_x*line_percent + prev_x*(1.0-line_percent);
				double lerp_y = final_y*line_percent + prev_y*(1.0-line_percent);
				
				cairo_line_to(cr,lerp_x,lerp_y);
			}
		}
		
		if(halt_after) break;
	}
	
	cairo_stroke(cr);
	
	free(x_values);
	free(y_values);
	free(cummalative_dist);
	
	cairo_destroy(cr);
}

static void render_color_keys(on_screen_map_t * screen_map){
	cairo_t * cr = cairo_create (screen_map->surface);
	
	cairo_set_font_size(cr,MAP_FONT_SIZE);
	
	double width = 0;
	for(size_t i = 0;i < N_COLOR_KEYS;i++){
		cairo_text_extents_t extents;
		cairo_text_extents(cr,color_keys[i]->color_name,&extents);
		width = fmax(width,extents.width);
	}
	width += COLOR_KEY_SWATCH_SIZE + COLOR_KEYS_SPACING + 2.0*COLOR_KEYS_PADDING;
	
	cairo_set_source_rgba(cr,1.0,1.0,1.0,0.5);
	double height = N_COLOR_KEYS*(COLOR_KEYS_SPACING + COLOR_KEY_SWATCH_SIZE) - COLOR_KEYS_SPACING + 2.0*COLOR_KEYS_PADDING;
	cairo_rectangle(cr,COLOR_KEYS_MARGIN,COLOR_KEYS_MARGIN,width,height);
	cairo_fill_preserve(cr);
	cairo_set_line_width(cr,COLOR_KEYS_OUTLINE_THICKNESS);
	cairo_set_source_rgb(cr,0.0,0.0,0.0);
	cairo_stroke(cr);
	
	for(size_t i = 0;i < N_COLOR_KEYS;i++){
		double swatch_x = COLOR_KEYS_MARGIN + COLOR_KEYS_PADDING;
		double swatch_y = COLOR_KEYS_MARGIN+COLOR_KEYS_PADDING+i*(COLOR_KEYS_SPACING + COLOR_KEY_SWATCH_SIZE);
		cairo_rectangle(cr,swatch_x,swatch_y,COLOR_KEY_SWATCH_SIZE,COLOR_KEY_SWATCH_SIZE);
		set_color(cr,*(color_keys[i]));
		cairo_fill(cr);
		
		cairo_set_source_rgb(cr,0,0,0);
		
		double text_x = swatch_x + COLOR_KEY_SWATCH_SIZE + COLOR_KEYS_SPACING;
		double text_y = swatch_y + COLOR_KEY_SWATCH_SIZE/2 + MAP_FONT_SIZE/2;
		
		cairo_move_to(cr,text_x,text_y);
		cairo_show_text(cr,color_keys[i]->color_name);
	}
	
	cairo_destroy(cr);
}

void render_on_screen_map_callback (GtkDrawingArea *drawing_area,cairo_t *cr, int width,int height,on_screen_map_t * screen_map){
	clear_map_surface(screen_map);
	
	render_edges(screen_map);
	render_mpos(screen_map);
	render_selectable_nodes(screen_map);
	render_building_names(screen_map);
	render_path(screen_map);
	
	cairo_surface_t * surface = screen_map->surface;
	screen_pan_t screen_pan = screen_map->screen_pan;
	map_rect_t map_bounding_box = screen_map->map_bounding_box;
	double scaling_y_factor = screen_map->map_sys.map.scaling_y_factor;
	
	render_node_popup(&(screen_map->node_popup),surface,screen_pan,map_bounding_box,scaling_y_factor);
	
	render_zoom_controls(&(screen_map->zoom_controls),surface);
	
	render_map_pin(&(screen_map->start_pin),surface,screen_pan,map_bounding_box,scaling_y_factor);
	render_map_pin(&(screen_map->end_pin),surface,screen_pan,map_bounding_box,scaling_y_factor);
	
	render_color_keys(screen_map);
	
	//paint the surface
	cairo_set_source_surface (cr, screen_map->surface, 0, 0);
	cairo_paint (cr);
}

void resize_on_screen_map_callback (GtkWidget *widget,int width,int height,on_screen_map_t * screen_map){
	if (screen_map->surface != NULL){
		cairo_surface_destroy (screen_map->surface);
		screen_map->surface = NULL;
	}

	if (gtk_native_get_surface (gtk_widget_get_native (widget))){
		screen_map->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, gtk_widget_get_width (widget),gtk_widget_get_height (widget));
	}
	
	recalculate_zoom_controls_position(&(screen_map->zoom_controls),width);
}

static void update_map_drag(on_screen_map_t * screen_map){
	if(screen_map->surface == NULL) return;
	
	mouse_state_t mouse_state = screen_map->mouse_state;
	screen_pan_t * screen_pan = &(screen_map->screen_pan);
	double surface_width = cairo_image_surface_get_width(screen_map->surface);
	double surface_height = cairo_image_surface_get_height(screen_map->surface);
	
	double delta_mouse_x = mouse_state.x - mouse_state.prev_x;
	double delta_mouse_y = mouse_state.y - mouse_state.prev_y;
	
	if(mouse_state.down){
		screen_pan->pan_x_final += delta_mouse_x/(screen_pan->zoom_current * surface_width);
		screen_pan->pan_x_current = screen_pan->pan_x_final;
		screen_pan->pan_y_final += delta_mouse_y/(screen_pan->zoom_current * surface_height);
		screen_pan->pan_y_current = screen_pan->pan_y_final;
	}
}

static void check_for_clicked_nodes(on_screen_map_t * screen_map){
	for(size_t i = 0;i < screen_map->n_on_screen_nodes;i++){
		on_screen_node_t * r_node = &(screen_map->all_on_screen_nodes[i]);
		if(was_pressed_reset(r_node->button)){
			notify_node_popup(&(screen_map->node_popup),r_node->node_reference);
			break;
		}
	}
}

void on_screen_map_update(on_screen_map_t * screen_map){
	bool mouse_over_button = update_button_collection(&(screen_map->button_collection),screen_map->mouse_state);
	
	update_map_pin(&screen_map->start_pin);
	update_map_pin(&screen_map->end_pin);
	
	if(!mouse_over_button) update_map_drag(screen_map);
	
	update_zoom_controls(&(screen_map->zoom_controls),&(screen_map->screen_pan));
	
	check_for_clicked_nodes(screen_map);
	
	update_node_popup(&(screen_map->node_popup),&(screen_map->start_pin),&(screen_map->end_pin));
	
	if(screen_map->map_sys.active_path != NULL){
		screen_map->path_animation_transition += PATH_TRANSITION_STEP;
		if(screen_map->path_animation_transition > 1.0) screen_map->path_animation_transition = 1.0;
	}
	
	screen_pan_update(&(screen_map->screen_pan));
	mouse_state_step(&(screen_map->mouse_state));
}

void on_screen_map_clear_selection(on_screen_map_t * screen_map){
	close_node_popup(&(screen_map->node_popup));
	hide_map_pin(&(screen_map->start_pin));
	hide_map_pin(&(screen_map->end_pin));
	
	delete_map_path(screen_map->map_sys.active_path);
	screen_map->map_sys.active_path = NULL;
}

void on_screen_map_find_path(on_screen_map_t * screen_map,edge_cost_function_f edge_cost_function){
	if(!screen_map->start_pin.visible || !screen_map->end_pin.visible) return;
	
	screen_map->map_sys.active_start = screen_map->start_pin.node_reference;
	screen_map->map_sys.active_end = screen_map->end_pin.node_reference;
	
	screen_map->map_sys.active_edge_cost_function = edge_cost_function;
	
	find_best_path(&(screen_map->map_sys),NULL);//TODO add err_ctx_t
	
	screen_map->path_animation_transition = 0.0;
}

static void update_on_screen_node_visibility(on_screen_map_t * screen_map){
	for(size_t i = 0;i < screen_map->n_on_screen_nodes;i++){
		bool visible = true;
		
		on_screen_node_t * on_screen_node = &(screen_map->all_on_screen_nodes[i]);
		bool adj_to_auto_door = node_adjacent_to_auto_door(on_screen_node->node_reference,NULL);//TODO err_ctx_t
		
		if(screen_map->hide_non_auto_doors && !adj_to_auto_door) visible = false;
		
		on_screen_node->button->visible = visible;
	}
}

void on_screen_map_set_hide_non_auto_doors(on_screen_map_t * screen_map,bool hide){
	screen_map->hide_non_auto_doors = hide;
	update_on_screen_node_visibility(screen_map);
}

void on_screen_map_set_show_building_names(on_screen_map_t * screen_map,bool show){
	screen_map->show_building_names = show;
}

void on_screen_map_notify_start(on_screen_map_t * screen_map,map_node_t * start){
	notify_map_pin(&(screen_map->start_pin),start);
	notify_node_popup(&(screen_map->node_popup),start);
	on_screen_map_update_focus(screen_map,start);
}

void on_screen_map_notify_end(on_screen_map_t * screen_map,map_node_t * end){
	notify_map_pin(&(screen_map->end_pin),end);
	notify_node_popup(&(screen_map->node_popup),end);
	on_screen_map_update_focus(screen_map,end);
}

void on_screen_map_update_focus(on_screen_map_t * screen_map,map_node_t * node){
	double x = 0.0,y = 0.0;
	calculate_on_screen_pos(node->coordinate,&x,&y,screen_map->surface,screen_map->screen_pan,screen_map->map_bounding_box,screen_map->map_sys.map.scaling_y_factor);
	
	double surface_width = cairo_image_surface_get_width(screen_map->surface);
	double surface_height = cairo_image_surface_get_height(screen_map->surface);
	
	screen_map->screen_pan.pan_x_final += (surface_width/2.0-x)/(screen_map->screen_pan.zoom_current * surface_width);
	screen_map->screen_pan.pan_y_final += (surface_height/2.0-y)/(screen_map->screen_pan.zoom_current * surface_height);
}

gboolean idle_draw_function(on_screen_map_t * screen_map) {
	gint64 frame_time = 1000000/120;
	gint64 update_time = 4000;//4 milliseconds per update (DONT CHANGE)
	
	gint64 current_micros = g_get_monotonic_time();
	gint64 delta_micros = current_micros - screen_map->last_micros;
	if(delta_micros > 10*update_time){
		delta_micros = 0;
	}
	screen_map->last_micros = current_micros;
	
	gint64 time_debt = delta_micros + screen_map->micros_debt;
	if(time_debt > 10*update_time){
		time_debt = 0;
	}
	
	while(time_debt > update_time){
		on_screen_map_update(screen_map);
		time_debt -= update_time;
	}
	
	screen_map->micros_debt = time_debt;//add debt
	
	//printf("%ld %ld\n",screen_map->delay_time,delta_micros);
	
	//slow down or speed up idle draw function call rate based
	if(delta_micros < frame_time/2){
		screen_map->delay_time += 100;
	}else if(delta_micros > 100){
		screen_map->delay_time -= 100;
	}
	g_usleep(screen_map->delay_time);
	
	gtk_widget_queue_draw(screen_map->drawing_area);
	return G_SOURCE_CONTINUE;
}
//---------------------------------------------------------- ON SCREEN MAP END ------------------------------------------------

void calculate_on_screen_pos(cord_t cord,double * out_x,double * out_y,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor){
	double x = cord.longitude;
	double y = cord.latitude;
	
	double bl_x = map_bounding_box.bottom_left.longitude;
	double bl_y = map_bounding_box.bottom_left.latitude;
	double tr_x = map_bounding_box.top_right.longitude;
	double tr_y = map_bounding_box.top_right.latitude;
	
	double x_offset = (bl_x + tr_x)*0.5;
	double y_offset = (bl_y + tr_y)*0.5;
	
	double map_wid_raw = tr_x - bl_x;
	double map_hei_raw = tr_y - bl_y;
	
	double map_wid = map_wid_raw;
	double map_hei = map_hei_raw*scaling_y_factor;
	
	double surface_wid = cairo_image_surface_get_width(surface);
	double surface_hei = cairo_image_surface_get_height(surface);
	
	double screen_scale_factor = fmin(surface_wid, surface_hei);
	double scale_factor = screen_scale_factor/fmax(map_wid,map_hei);
	
	double scale_x = scale_factor;
	double scale_y = scale_factor*scaling_y_factor;
	
	x -= x_offset;
	y -= y_offset;
	
	x *= scale_x*screen_pan.zoom_current;
	y *= scale_y*screen_pan.zoom_current;
	
	x += surface_wid/2.0;
	y += surface_hei/2.0;
	
	y = surface_hei - y;
	
	x += screen_pan.pan_x_current * screen_pan.zoom_current * cairo_image_surface_get_width(surface);
	y += screen_pan.pan_y_current * screen_pan.zoom_current * cairo_image_surface_get_height(surface);
	
	*out_x = x;
	*out_y = y;
}