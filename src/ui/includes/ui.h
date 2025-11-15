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

#define N_PATH_FINDER_STRATEGY_OPTIONS 3
extern const char * path_finder_strategy_option_strings[N_PATH_FINDER_STRATEGY_OPTIONS+1];

typedef struct Screen_Data_State screen_data_state_t;

struct Screen_Data_State{
	char * start_location_text;
	char * end_location_text;
	
	bool hide_non_auto_doors;
	bool hide_interior_locations;
	unsigned int path_finder_strategy;
	
	map_data_state_t map_data_state;
	
	GtkWidget * window;
};

screen_data_state_t init_screen_data_state(void);
void screen_data_state_to_output_stream(screen_data_state_t state,FILE * stream);

void create_window (GtkApplication *app,screen_data_state_t * sds);

#endif