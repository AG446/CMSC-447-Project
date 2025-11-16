/*
 * CMSC-447-Project
 * 
 * UMBC Student Accessibility Map Program.
 * Copyright 2025.
 * This program is property of University of Maryland Baltimore County (UMBC).
 * 
 * Program Devloped By:
 * - Benjamin Currie 
 * - Jack Xu
 */

#ifndef MAP_RENDER_H
#define MAP_RENDER_H

#include <gtk/gtk.h>
#include "map.h"

typedef struct In_Map_Button in_map_button_t;
typedef struct Map_Data_State map_data_state_t;
typedef struct Node_Popup node_popup_t;

struct In_Map_Button{
	double x;
	double y;
	double width;
	double height;
	double transition;
	bool was_pressed;
	bool mouse_on;
	bool pressed_on;
	bool visible;
};

in_map_button_t * create_map_button(double width,double height,map_data_state_t * mds);

struct Node_Popup{
	double x;
	double y;
	double full_width;
	double full_height;
	double width;
	double height;
	
	double transition;
	
	map_node_t * node_reference;
	
	in_map_button_t * close_button;
	in_map_button_t * set_start_button;
	in_map_button_t * set_end_button;
	
	bool visible;
};

node_popup_t * create_node_popup(double width,double height,map_data_state_t * mds);

struct Map_Data_State{
	cairo_surface_t * surface;
	GtkWidget * drawing_area;
	guint idle_drawing_function_id;
	
	double mouse_x;
	double mouse_y;
	bool mouse_down;
	bool p_mouse_down;
	
	in_map_button_t * zoom_in_button;
	in_map_button_t * zoom_out_button;
	in_map_button_t * home_button;
	
	node_popup_t * node_popup;
	
	in_map_button_t ** buttons;
	size_t n_buttons;
	size_t buttons_capacity;
};

map_data_state_t init_map_data_state(void);

void draw_map_drawing_area_callback (GtkDrawingArea *drawing_area,cairo_t *cr, int width,int height,map_data_state_t * mds);
void map_update(map_data_state_t * mds);

void resize_map_drawing_area_callback (GtkWidget *widget,int width,int height,map_data_state_t * mds);

#endif