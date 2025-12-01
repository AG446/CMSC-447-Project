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
typedef struct Button_Collection button_collection_t;
typedef struct On_Screen_Map on_screen_map_t;
typedef struct Node_Popup node_popup_t;
typedef struct Map_Pin map_pin_t;
typedef struct On_Screen_Node on_screen_node_t;
typedef struct Screen_Pan screen_pan_t;
typedef struct Mouse_State mouse_state_t;
typedef struct Zoom_Controls zoom_controls_t;
typedef struct Flt_Color flt_color_t;

//---------------------------------------------------------- FLT COLOR BEGIN --------------------------------------------------
struct Flt_Color{
	const char * color_name;
	double r,g,b,a;
};

void set_color(cairo_t * cr,flt_color_t color);

extern flt_color_t sidewalk_clr;
extern flt_color_t ramp_clr;
extern flt_color_t stairs_clr;
extern flt_color_t road_clr;
extern flt_color_t construction_clr;
extern flt_color_t crosswalk_clr;

#define N_COLOR_KEYS 6
extern flt_color_t * color_keys[N_COLOR_KEYS];
//---------------------------------------------------------- FLT COLOR END ----------------------------------------------------

//---------------------------------------------------------- MOUSE STATE BEGIN ------------------------------------------------
/*
 * Object to keep track of the mouse data for the on-screen map.
 */
struct Mouse_State{
	//mouse position
	double x;
	double y;
	double prev_x;
	double prev_y;
	
	//mouse button
	bool down;
	bool prev_down;
};

//initialize the mouse state
mouse_state_t init_mouse_state(void);

//set mouse button state
void set_mouse_button_down(mouse_state_t * mouse_state,bool state);

//set mouse position
void set_mouse_position(mouse_state_t * mouse_state,double mouse_x,double mouse_y);

//take a time step for the state of the mouse
void mouse_state_step(mouse_state_t * mouse_state);

//was the button clicked
bool mouse_clicked(mouse_state_t mouse_state);
//---------------------------------------------------------- MOUSE STATE END --------------------------------------------------

//---------------------------------------------------------- SCREEN PAN BEGIN -------------------------------------------------
/*
 * Object to keep track of the zoom and pan of the on-screen map.
 */
struct Screen_Pan{
	//zoom
	double zoom_jump;
	double min_zoom;
	double max_zoom;
	double default_zoom;
	double zoom_current;
	double zoom_final;
	
	//pan-x
	double min_pan_x;
	double max_pan_x;
	double default_pan_x;
	double pan_x_current;
	double pan_x_final;
	
	//pan-y
	double min_pan_y;
	double max_pan_y;
	double default_pan_y;
	double pan_y_current;
	double pan_y_final;
};

//initialize the screen pan object
screen_pan_t init_screen_pan(void);

//update the zoom state
void screen_pan_update(screen_pan_t * screen_pan);

//reset the pan and zoom to the default home position
void screen_pan_home(screen_pan_t * screen_pan);

//zoom in by a factor smoothly
void screen_pan_zoom_in(screen_pan_t * screen_pan);

//zoom out by a factor smoothly
void screen_pan_zoom_out(screen_pan_t * screen_pan);

//---------------------------------------------------------- SCREEN PAN END ---------------------------------------------------

//---------------------------------------------------------- Button Collection BEGIN ------------------------------------------
/*
 * Collection of all the buttons within the on-screen map
 */
struct Button_Collection{
	in_map_button_t ** buttons;
	size_t n_buttons;
	size_t buttons_capacity;
};

//initialize the collection of all buttons for the on-screen map
button_collection_t init_button_collection(void);

//add the button to the collection
void add_button_to_button_collection(button_collection_t * collection,in_map_button_t * button);

//update the collection of all buttons. Returns true if the mouse is over a button within the collection.
bool update_button_collection(button_collection_t * collection,mouse_state_t mouse_state);

//---------------------------------------------------------- Button Collection END --------------------------------------------

//---------------------------------------------------------- IN MAP BUTTON BEGIN ----------------------------------------------
/*
 * Buttons that are within the drawn map. Completely seperate from gtk buttons.
 */
struct In_Map_Button{
	//dimensions
	double x;
	double y;
	double width;
	double height;
	
	//smoothed transition value between 0 and 1. When mouse hovering the value is 1 otherwise it's 0.
	double animation_transition;
	
	//value true after button has been pressed
	bool was_pressed;
	
	//value true when the mouse is on the button
	bool mouse_on;
	
	//is the button currently being pressed. Used for changing color.
	bool pressed_on;
	
	//is the button activly visible
	bool visible;
};

//create a map button instance and add it to the list of all buttons
in_map_button_t * create_map_button(double width,double height,button_collection_t * collection);

//returns true if was pressed and resets the buttons click state
bool was_pressed_reset(in_map_button_t * button);

//update the state of the button
bool update_button(in_map_button_t * button,bool * mouse_already_over_button,mouse_state_t mouse_state);

//---------------------------------------------------------- IN MAP BUTTON END ------------------------------------------------

//---------------------------------------------------------- ON SCREEN NODE BEGIN ---------------------------------------------
/*
 * A wraper for map_node_t objects to make them behave like buttons for the on screen node
 */
struct On_Screen_Node{
	in_map_button_t * button;
	map_node_t * node_reference;
};

//initialize the on screen node (The clickable nodes on the on-screen map).
on_screen_node_t init_on_screen_node(map_node_t * node_ref,button_collection_t * collection);

//draw the on-screen node
void render_on_screen_node(on_screen_node_t * screen_node,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor);
//---------------------------------------------------------- ON SCREEN NODE END -----------------------------------------------

//---------------------------------------------------------- NODE POPUP BEGIN -------------------------------------------------
/*
 * A small window popup which is opened/notified when a on-screen node is clicked. The popup shows data about the node.
 */
struct Node_Popup{
	//dimensions
	double x;
	double y;
	double width;
	double height;
	
	//smoothed transition value between 0 and 1. When the popup is open value is 1 otherwise 0.
	double animation_transition;
	
	//node reference is used to calculate the on screen position of the popup and to get details of node.
	map_node_t * node_reference;
	
	//buttons which the popup has ownership over
	in_map_button_t * close_button;
	in_map_button_t * set_start_button;
	in_map_button_t * set_end_button;
	
	//grey out buttons if pin already locked TODO
	bool disable_buttons;
	
	//is the popup visible
	bool visible;
};

//initialize a node popup. Pass it reference to the button collection.
node_popup_t init_node_popup(button_collection_t * collection);

//update the state of the node popup
void update_node_popup(node_popup_t * node_popup,map_pin_t * start_pin,map_pin_t * end_pin);

//render the node popup
void render_node_popup(node_popup_t * node_popup,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor);

//show the popup and lock onto the new_node_reference
void notify_node_popup(node_popup_t * popup,map_node_t * new_node_reference);

//hide/close the node popup
void close_node_popup(node_popup_t * node_popup);
//---------------------------------------------------------- NODE POPUP END ---------------------------------------------------

//---------------------------------------------------------- MAP PIN BEGIN ----------------------------------------------------
/*
 * A pin which shows either the start or end destination on the on-screen map
 */
struct Map_Pin{
	//position
	double x,y;
	
	//color
	double r,g,b;
	
	//node reference is used to calculate the on screen position of the pin and to lock onto it
	map_node_t * node_reference;
	
	double animation_transition;
	
	//is the pin visible
	bool visible;
};

//initialize the map pin with a rgb color
map_pin_t init_map_pin(double r,double g,double b);

//update the map-pin state
void update_map_pin(map_pin_t * pin);

//render the map pin
void render_map_pin(map_pin_t * pin,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor);

//show the map pin and lock onto the new_node_reference
void notify_map_pin(map_pin_t * pin,map_node_t * new_node_reference);

//hide the map pin
void hide_map_pin(map_pin_t * pin);
//---------------------------------------------------------- MAP PIN END ------------------------------------------------------

//---------------------------------------------------------- ZOOM CONTROLS BEGIN ----------------------------------------------
/*
 * group of buttons to control the zoom of the on-screen map
 */
struct Zoom_Controls{
	in_map_button_t * zoom_in_button;
	in_map_button_t * zoom_out_button;
	in_map_button_t * home_button;
};

zoom_controls_t init_zoom_controls(button_collection_t * collection);
void update_zoom_controls(zoom_controls_t * zoom_controls,screen_pan_t * screen_pan);
void render_zoom_controls(zoom_controls_t * zoom_controls,cairo_surface_t * surface);
void recalculate_zoom_controls_position(zoom_controls_t * zoom_controls,int surface_width);
//---------------------------------------------------------- ZOOM CONTROLS END ------------------------------------------------

//---------------------------------------------------------- ON SCREEN MAP BEGIN ----------------------------------------------
/*
 * The complete state of the on-screen map
 */
struct On_Screen_Map{
	//associated widget and surface
	cairo_surface_t * surface;
	GtkWidget * drawing_area;
	
	//looping idle function to continously render the map
	guint idle_drawing_function_id;
	
	//map data
	map_sys_t map_sys;
	map_rect_t map_bounding_box;
	
	//translational states
	mouse_state_t mouse_state;
	screen_pan_t screen_pan;
	
	//interface objects
	zoom_controls_t zoom_controls;
	node_popup_t node_popup;
	map_pin_t start_pin;
	map_pin_t end_pin;
	on_screen_node_t * all_on_screen_nodes;
	size_t n_on_screen_nodes;
	
	//animation for the path
	double path_animation_transition;
	
	//node visibility options
	bool hide_non_auto_doors;
	
	//colletion of all buttons
	button_collection_t button_collection;
	
	gint64 last_micros;
	gint64 micros_debt;
	gint64 delay_time;
};

//initialize the on screen map
on_screen_map_t init_on_screen_map(const char * map_file_path);

//render callback for the on screen map
void render_on_screen_map_callback (GtkDrawingArea *drawing_area,cairo_t *cr, int width,int height,on_screen_map_t * screen_map);

//resize callback for the on screen map
void resize_on_screen_map_callback (GtkWidget *widget,int width,int height,on_screen_map_t * screen_map);

//update all the elements for the on screen map
void on_screen_map_update(on_screen_map_t * screen_map);

//clear the start and end location pin and close popup. Triggered with CLEAR (off-map) button.
void on_screen_map_clear_selection(on_screen_map_t * screen_map);

//calculate the path from the start pin to the end pin
void on_screen_map_find_path(on_screen_map_t * screen_map,edge_cost_function_f edge_cost_function);

//update which on-screen nodes are visible
void on_screen_map_set_hide_non_auto_doors(on_screen_map_t * screen_map,bool hide);

//looping idle function to continously render the map
gboolean idle_draw_function(on_screen_map_t * screen_map);

//---------------------------------------------------------- ON SCREEN MAP END ------------------------------------------------




//calculate on screen pixel coordinate from world coordinate
void calculate_on_screen_pos(cord_t cord,double * out_x,double * out_y,cairo_surface_t * surface,screen_pan_t screen_pan,map_rect_t map_bounding_box,double scaling_y_factor);

#endif