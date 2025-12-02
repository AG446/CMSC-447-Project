/*
 * CMSC-447-Project
 * * UMBC Student Accessibility Map Program.
 * Copyright 2025.
 * This program is property of University of Maryland Baltimore County (UMBC).
 * * * Program Devloped By:
 * - Alex Gallagher
 */

#include "map.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "text_proc.h"
#include <float.h>
#include <math.h>

/**
 * @brief Safely appends a string to a dynamic buffer, handling realloc and NULL dest.
 */
static int safe_str_append(char **dest, size_t *dest_len, const char *src) {
	if (src == NULL || strlen(src) == 0) {
		return 0; 
	}
	size_t src_len = strlen(src);
	size_t new_len = *dest_len + src_len;
	char *new_ptr;

	if (*dest == NULL) {
		new_ptr = (char *)malloc(new_len + 1);
		if (new_ptr == NULL) return -1;
		memcpy(new_ptr, src, src_len);
	} else {
		new_ptr = (char *)realloc(*dest, new_len + 1);
		if (new_ptr == NULL) return -1;
		memcpy(new_ptr + *dest_len, src, src_len);
	}

	*dest = new_ptr;
	*dest_len = new_len;
	(*dest)[new_len] = '\0';
	return 0;
}

/**
 * @brief Calculates the initial bearing (0-360 degrees) from node A to node B.
 * Uses standard Lat/Lon math. 0=N, 90=E.
 */
static double get_bearing_deg(map_node_t * a, map_node_t * b) {
	// Convert to radians
	double lat1_rad = a->coordinate.latitude * M_PI / 180.0;
	double lon1_rad = a->coordinate.longitude * M_PI / 180.0;
	double lat2_rad = b->coordinate.latitude * M_PI / 180.0;
	double lon2_rad = b->coordinate.longitude * M_PI / 180.0;

	double d_lon = lon2_rad - lon1_rad;

	double y = sin(d_lon) * cos(lat2_rad);
	double x = cos(lat1_rad) * sin(lat2_rad) -
			   sin(lat1_rad) * cos(lat2_rad) * cos(d_lon);
	
	double bearing_rad = atan2(y, x);
	double bearing_deg = (bearing_rad * 180.0 / M_PI + 360.0);
	
	return fmod(bearing_deg, 360.0);
}

/**
 * @brief Converts a degree bearing to a cardinal direction index (0-7).
 * 0=North, 1=NE, 2=East, etc.
 */
static int get_cardinality_idx(double bearing_deg) {
	// Offset by 22.5 to center the "North" sector on 0/360
	return (int)((bearing_deg + 22.5) / 45.0) & 7;
}

/**
 * @brief Calculates the distance between two nodes in feet using Haversine.
 */
static double calculate_distance_feet(map_node_t * a, map_node_t * b) {
	const double R_EARTH_FEET = 20902231.0;
	
	double lat1_rad = a->coordinate.latitude * M_PI / 180.0;
	double lon1_rad = a->coordinate.longitude * M_PI / 180.0;
	double lat2_rad = b->coordinate.latitude * M_PI / 180.0;
	double lon2_rad = b->coordinate.longitude * M_PI / 180.0;

	double d_lat = lat2_rad - lat1_rad;
	double d_lon = lon2_rad - lon1_rad;

	double hav_a = sin(d_lat / 2.0) * sin(d_lat / 2.0) +
				   cos(lat1_rad) * cos(lat2_rad) *
				   sin(d_lon / 2.0) * sin(d_lon / 2.0);
	
	double hav_c = 2.0 * atan2(sqrt(hav_a), sqrt(1.0 - hav_a));

	return R_EARTH_FEET * hav_c;
}

/**
 * @brief Calculates the smallest difference between two bearings.
 * Result is in range [-180, 180]. Positive = Right turn, Negative = Left turn.
 */
static double get_angle_diff(double b1, double b2) {
	double diff = b2 - b1;
	while (diff <= -180.0) diff += 360.0;
	while (diff > 180.0) diff -= 360.0;
	return diff;
}

/**
 * @brief Determines turn type (straight=0, left=1, right=2) at `curr` node.
 */
static int get_turn_type(map_node_t * prev, map_node_t * curr, map_node_t * next) {
	double bearing1 = get_bearing_deg(prev, curr);
	double bearing2 = get_bearing_deg(curr, next);
	double diff = get_angle_diff(bearing1, bearing2);

	// Angle threshold for a "turn" vs "straight" movement
	// 45 degrees is a standard threshold for "Slight" vs "Turn"
	if (fabs(diff) < 45.0) {
		return 0; // Straight
	} else if (diff > 0.0) {
		return 2; // Right
	} else {
		return 1; // Left
	}
}


/*
 * ============================================================================
 * BEGIN: Main Direction Generation Function
 * ============================================================================
 */

char * convert_path_to_directions_str(const map_path_t * path, err_ctx_t * ctx) {
	
	// --- Declarations ---
	const char * const card[] = {
		"north", "northeast", "east", "southeast", "south", "southwest", "west", "northwest"
	};
	const char * const dir[] = {
		"straight", "left", "right"
	};
	
	// Format strings
	const char * const F_START_BUILDING = "You are in the %s building on floor %d.\n";
	const char * const F_START_OUTSIDE = "You are outside.\n";
	const char * const F_ELEVATOR = "Take the elevator to floor %d.\n"; 
	const char * const F_TRAVEL = "Travel %s along the %s.\n";
	const char * const F_CONTINUE = "Continue along the %s for %.0f feet.\n";
	const char * const F_TURN = "Turn %s to face %s.\n";
	const char * const F_EXIT = "Exit the %s building.\n";
	const char * const F_ENTER = "Enter the %s building.\n";
	const char * const F_ARRIVE_FULL = "You have arrived at the %s on floor %d.\n";
	const char * const F_ARRIVE_NAME = "You have arrived at the %s.\n";
	const char * const F_ARRIVE_GENERIC = "You have arrived!\n"; 

	char * final_message = NULL;
	size_t final_len = 0;
	char buf[512]; 
	
	map_node_t * start_node = NULL;
	const char * start_building_name = NULL;
	
	size_t current_node_idx = 0;
	map_node_t * curr_node = NULL;
	map_node_t * next_node = NULL;
	map_edge_t * curr_edge = NULL;
	uint8_t current_edge_type = 0;

	const char * name = NULL;
	size_t elevator_end_idx = 0;
	map_node_t * dest_node = NULL;
	double start_bearing = 0.0;
	int card_idx = 0;
	
	double accumulated_distance = 0.0;
	size_t segment_end_idx = 0;
	map_node_t * node_a = NULL;
	map_node_t * node_b = NULL;
	map_node_t * node_c = NULL;
	map_edge_t * edge_b = NULL;
	
	int turn_type = 0;
	double next_bearing = 0.0;
	int next_card_idx = 0;
	double segment_bearing = 0.0;

	map_node_t * prev_node = NULL; 
	
	map_node_t * last_node = NULL;
	map_node_t * prev_last_node = NULL;
	const char * arrival_name = NULL;
	const char * arrival_building_name = NULL;
	
	// --- End Declarations ---

	if (path == NULL || path->n_nodes < 2 || path->n_edges < 1) {
		if (ctx) ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}

	// --- 1. Generate Start Message ---
	start_node = path->nodes[0];

	if (start_node->associated_building != NULL) {
		start_building_name = get_primary_building_name(start_node->associated_building, ctx);
	}
	
	if (start_building_name != NULL && start_node->floor_number != NODE_FLOOR_NUMBER_NONE) {
		snprintf(buf, sizeof(buf), F_START_BUILDING, start_building_name, start_node->floor_number);
		if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
	} else {
		if (safe_str_append(&final_message, &final_len, F_START_OUTSIDE) == -1) { free(final_message); return NULL; }
	}

	// --- 2. Main Segment-by-Segment Loop ---
	current_node_idx = 0;
	
	while (current_node_idx < path->n_nodes - 1) {
		curr_node = path->nodes[current_node_idx];
		next_node = path->nodes[current_node_idx + 1];
		curr_edge = path->edges[current_node_idx];
		current_edge_type = curr_edge->type;
		
		// --- A. Building Transition Check (Exit/Enter) ---
		
		// 1. Check for EXITING
		if (curr_node->associated_building != NULL && next_node->associated_building == NULL) {
			name = get_primary_building_name(curr_node->associated_building, ctx);
			if (name) {
				snprintf(buf, sizeof(buf), F_EXIT, name); 
				if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
			}
		} 
		// 2. Check for ENTERING
		else if (curr_node->associated_building == NULL && next_node->associated_building != NULL) {
			name = get_primary_building_name(next_node->associated_building, ctx);
			if (name) {
				snprintf(buf, sizeof(buf), F_ENTER, name); 
				if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
			}
		}

		// --- B. Mutual Exclusion Segment Handling ---
		
		// 1. Handle Elevator Shaft (Highest Priority) - SKIPS INTERMEDIATE FLOORS
		if (current_edge_type == EDGE_TYPE_ELEVATOR_SHAFT) {
			elevator_end_idx = current_node_idx;
			// Look ahead: consume all sequential elevator edges
			while (elevator_end_idx < path->n_edges &&
				   path->edges[elevator_end_idx]->type == EDGE_TYPE_ELEVATOR_SHAFT) {
				elevator_end_idx++;
			}
			dest_node = path->nodes[elevator_end_idx];
			
			if (dest_node->floor_number != NODE_FLOOR_NUMBER_NONE) {
				snprintf(buf, sizeof(buf), F_ELEVATOR, dest_node->floor_number);
				if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
				current_node_idx = elevator_end_idx;
			} else {
				current_node_idx++;
			}
		}
		
		// 2. Handle Doors/Stairs (Single-step)
		else if (current_edge_type == EDGE_TYPE_DOOR ||
			current_edge_type == EDGE_TYPE_AUTO_DOOR ||
			current_edge_type == EDGE_TYPE_STAIRS) {
			current_node_idx++;
		}

		// 3. Handle Aggregatable Travel Segments
		else {
			// C. Start of a new Travel Segment
			start_bearing = get_bearing_deg(curr_node, next_node);
			card_idx = get_cardinality_idx(start_bearing);
			
			snprintf(buf, sizeof(buf), F_TRAVEL, card[card_idx], edge_type_names[current_edge_type - 1]);
			if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
			
			// D. Path Smoothing Loop
			accumulated_distance = 0.0;
			segment_end_idx = current_node_idx + 1;
			accumulated_distance += calculate_distance_feet(curr_node, next_node);

			while (segment_end_idx < path->n_nodes - 1) {
				node_a = path->nodes[segment_end_idx - 1];
				node_b = path->nodes[segment_end_idx];
				node_c = path->nodes[segment_end_idx + 1];
				edge_b = path->edges[segment_end_idx];
				
				// Stop if edge type changes or is special
				if (edge_b->type != current_edge_type || 
					edge_b->type == EDGE_TYPE_ELEVATOR_SHAFT ||
					edge_b->type == EDGE_TYPE_DOOR ||
					edge_b->type == EDGE_TYPE_AUTO_DOOR ||
					edge_b->type == EDGE_TYPE_STAIRS) {
					break;
				}
				
				// Stop if there's a significant turn
				if (get_turn_type(node_a, node_b, node_c) != 0) { 
					break;
				}

				// Extra Check: Stop if the cumulative curve becomes too large (> 45 deg off start)
				// This prevents a "C" shape from being called "Straight"
				segment_bearing = get_bearing_deg(node_b, node_c);
				if (fabs(get_angle_diff(start_bearing, segment_bearing)) > 45.0) {
					break;
				}
				
				accumulated_distance += calculate_distance_feet(node_b, node_c);
				segment_end_idx++;
			}
			
			// E. Continuation Instruction
			if (accumulated_distance > 20.0) {
				snprintf(buf, sizeof(buf), F_CONTINUE, edge_type_names[current_edge_type - 1], accumulated_distance);
				if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
			}

			// F. Check for a Turn
			if (segment_end_idx < path->n_nodes - 1) {
				prev_node = path->nodes[segment_end_idx - 1];
				curr_node = path->nodes[segment_end_idx];
				next_node = path->nodes[segment_end_idx + 1];
				
				turn_type = get_turn_type(prev_node, curr_node, next_node);
				
				if (turn_type != 0) { 
					next_bearing = get_bearing_deg(curr_node, next_node);
					next_card_idx = get_cardinality_idx(next_bearing);
					
					snprintf(buf, sizeof(buf), F_TURN, dir[turn_type], card[next_card_idx]);
					if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
				}
			}

			current_node_idx = segment_end_idx;
		} 
	}

	// --- 3. Final Arrival Message ---
	last_node = path->nodes[path->n_nodes - 1];
	
	// Fallback "Enter" check if the loop ended exactly at a building entrance
	if (path->n_nodes > 1) {
		prev_last_node = path->nodes[path->n_nodes - 2];
		if(prev_last_node->associated_building == NULL && last_node->associated_building != NULL &&
		   path->edges[path->n_nodes - 2]->type != EDGE_TYPE_DOOR && 
		   path->edges[path->n_nodes - 2]->type != EDGE_TYPE_AUTO_DOOR)
		{
			name = get_primary_building_name(last_node->associated_building, ctx);
			if (name) {
				snprintf(buf, sizeof(buf), F_ENTER, name); 
				if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }
			}
		}
	}
	
	arrival_name = last_node->name;
	arrival_building_name = NULL;
	if (last_node->associated_building) {
		arrival_building_name = get_primary_building_name(last_node->associated_building, ctx);
	}
	
	if (arrival_name != NULL) {
		if (last_node->floor_number != NODE_FLOOR_NUMBER_NONE) {
			snprintf(buf, sizeof(buf), F_ARRIVE_FULL, arrival_name, last_node->floor_number);
		} else {
			snprintf(buf, sizeof(buf), F_ARRIVE_NAME, arrival_name);
		}
	} else if (arrival_building_name != NULL) {
		if (last_node->floor_number != NODE_FLOOR_NUMBER_NONE) {
			snprintf(buf, sizeof(buf), F_ARRIVE_FULL, arrival_building_name, last_node->floor_number);
		} else {
			snprintf(buf, sizeof(buf), F_ARRIVE_NAME, arrival_building_name);
		}
	} else {
		snprintf(buf, sizeof(buf), F_ARRIVE_GENERIC);
	}
	
	if (safe_str_append(&final_message, &final_len, buf) == -1) { free(final_message); return NULL; }

	return final_message;
}
