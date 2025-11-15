#include "map_render.h"

in_map_button_t * init_map_button(double width,double height,void (*draw_func)(in_map_button_t * button,map_data_state_t * mds)){
	in_map_button_t * out = (in_map_button_t*) malloc(sizeof(in_map_button_t));;
	
	out->x = 0;
	out->y = 0;
	out->width = width;
	out->height = height;
	out->draw_func = draw_func;
	out->transition = 0.0;
	out->was_pressed = false;
	
	return out;
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
	
	mds->zoom_in_button->x = width-mds->zoom_in_button->width-gap;
	mds->zoom_in_button->y = gap;
	
	mds->zoom_out_button->x = width-mds->zoom_in_button->width-gap;
	mds->zoom_out_button->y = mds->zoom_in_button->height+2*gap;
}

#define TRANSITION_CONSTANT 0.03

void update_button(in_map_button_t * button,map_data_state_t * mds){
	bool on = mds->mouse_x > button->x && mds->mouse_y > button->y && mds->mouse_x < button->x + button->width && mds->mouse_y < button->y + button->height;
	
	if(on){
		if(mds->mouse_down && !mds->p_mouse_down) button->was_pressed = true;
		button->transition = TRANSITION_CONSTANT + button->transition*(1-TRANSITION_CONSTANT);
	}else{
		button->transition = button->transition*(1-TRANSITION_CONSTANT);
	}
}

void map_update(map_data_state_t * mds){
	for(size_t i = 0;i < mds->n_buttons;i++){
		update_button(mds->buttons[i],mds);
	}
	mds->p_mouse_down = mds->mouse_down;
}

#define BUTTON_SCALE_FACTOR 0.1

static void render_zoom_in_zoom_out_button(in_map_button_t * button,map_data_state_t * mds,bool zoom_in){
	cairo_t * cr = cairo_create (mds->surface);
	
	double scaling_factor = 1.0+button->transition*BUTTON_SCALE_FACTOR;
	
	double radius = button->width/2*scaling_factor;
	
	cairo_set_source_rgb (cr, 255,255,255);
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_fill (cr);
	
	cairo_set_source_rgb (cr, 0,0,0);
	cairo_set_line_width (cr, 5);
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_stroke (cr);
	
	double thickness = radius/4.0;
	double inner_radius = radius*0.6;
	
	if(zoom_in){
		cairo_set_source_rgb (cr, 255,0,0);
	}else{
		cairo_set_source_rgb (cr, 0,0,255);
	}
	cairo_rectangle(cr,button->x+button->width/2-inner_radius,button->y+button->height/2-thickness/2,inner_radius*2,thickness);
	if(zoom_in) cairo_rectangle(cr,button->x+button->width/2-thickness/2,button->y+button->height/2-inner_radius,thickness,inner_radius*2);
	cairo_fill (cr);
	
	cairo_set_source_rgba (cr, 0,0,0,button->transition*0.2);
	cairo_set_line_width (cr, 5);
	cairo_arc(cr, button->x+button->width/2, button->y+button->height/2, radius, 0, 2*M_PI);
	cairo_fill (cr);

	cairo_destroy (cr);
}

static void render_zoom_in_button(in_map_button_t * button,map_data_state_t * mds){
	render_zoom_in_zoom_out_button(button,mds,true);
}

static void render_zoom_out_button(in_map_button_t * button,map_data_state_t * mds){
	render_zoom_in_zoom_out_button(button,mds,false);
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
	
	out.zoom_in_button = init_map_button(40,40,render_zoom_in_button);
	out.zoom_out_button = init_map_button(40,40,render_zoom_out_button);
	
	out.n_buttons = 2;
	out.buttons = (in_map_button_t**) malloc(sizeof(in_map_button_t*) * out.n_buttons);
	out.buttons[0] = out.zoom_in_button;
	out.buttons[1] = out.zoom_out_button;
	
	return out;
}

static void clear_map_surface(map_data_state_t * mds){
	cairo_t * cr = cairo_create (mds->surface);

	cairo_set_source_rgb (cr, 0.2, 0.8, 0.5);
	cairo_paint (cr);

	cairo_destroy (cr);
}

static void map_draw_brush (map_data_state_t * mds){
	cairo_t *cr;
	
	cr = cairo_create (mds->surface);

	cairo_rectangle (cr,mds->mouse_x - 3, mds->mouse_y - 3, 6, 6);
	cairo_fill (cr);

	cairo_destroy (cr);
}

void draw_map_drawing_area_callback (GtkDrawingArea *drawing_area,cairo_t *cr, int width,int height,map_data_state_t * mds){
	
	clear_map_surface(mds);
	
	for(size_t i = 0;i < mds->n_buttons;i++){
		mds->buttons[i]->draw_func(mds->buttons[i],mds);
	}
	
	map_draw_brush (mds);
	
	cairo_set_source_surface (cr, mds->surface, 0, 0);
	cairo_paint (cr);
}