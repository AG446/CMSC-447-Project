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

#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include "map_render.h"

typedef struct Screen_Data_State screen_data_state_t;

struct Screen_Data_State{
	char * start_location_text;
	char * end_location_text;
	
	bool hide_interior_locations;
	unsigned int path_finder_strategy_index;
	
	on_screen_map_t on_screen_map;
	
	GtkWidget * window;
};

screen_data_state_t init_screen_data_state(void);
void screen_data_state_to_output_stream(screen_data_state_t state,FILE * stream);

void create_window (GtkApplication *app,screen_data_state_t * sds);

#endif