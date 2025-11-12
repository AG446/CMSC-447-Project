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

#ifndef MAP_OBJ_ARR_H
#define MAP_OBJ_ARR_H

#include "map.h"

typedef struct Map_Object map_obj_t;
typedef struct Map_Object_Array map_obj_arr_t;

//---------------------------------------------------------- MAP OBJECT BEGIN -------------------------------------------------
/*
 * The generic working object is just a container for various map objects.
 * A union is used to save on memory since a generic working object can only be
 * one type at a time.
 */

#define MO_TYPE_NULL 0
#define MO_TYPE_CORD 1
#define MO_TYPE_MPO 2
#define MO_TYPE_RECT 3
#define MO_TYPE_NODE 4
#define MO_TYPE_BUILDING 5

struct Map_Object{
	union{
		cord_t cord;
		map_rect_t rect;
		mpo_t * mpo;
		map_node_t * node;
		building_t * building;
	};
	uint8_t type;
};

//initialize a blank map_obj_t without a type
map_obj_t init_null_map_obj(void);

//initialize a cord map_obj_t
map_obj_t init_cord_map_obj(cord_t cord);

//initialize a rect map_obj_t
map_obj_t init_rect_map_obj(map_rect_t rect);

//initialize a map polygon map_obj_t
map_obj_t init_mpo_map_obj(mpo_t * mpo,err_ctx_t * ctx);

//initialize a map node map_obj_t
map_obj_t init_node_map_obj(map_node_t * node,err_ctx_t * ctx);

//initialize a building map_obj_t
map_obj_t init_building_map_obj(building_t * building,err_ctx_t * ctx);

//clear all the memory in the heap held by the map_node_t
void deinit_map_obj(map_obj_t * obj,err_ctx_t * ctx);

//---------------------------------------------------------- MAP OBJECT END ---------------------------------------------------


//---------------------------------------------------------- MAP OBJECT ARRAY BEGIN -------------------------------------------
/*
 * A dynamic array of map_obj_t instances which is used for the working set in the CLI
 */
struct Map_Object_Array{
	map_obj_t * objects;
	size_t n_objects;
	size_t objects_capacity;
};

//initialize a map_obj_arr_t
map_obj_arr_t init_map_obj_arr(void);

//delete all the heap data within the map_obj_arr_t
void deinit_map_obj_arr(map_obj_arr_t * arr,err_ctx_t * ctx);

//clear the list within the map_obj_arr_t. Note: this does not free the memory block for the array until deinit_map_obj_arr is called.
void clear_map_obj_arr(map_obj_arr_t * arr,err_ctx_t * ctx);

//add a map_obj_t to the map_obj_arr_t
void add_map_obj_to_map_obj_arr(map_obj_arr_t * arr,map_obj_t obj,err_ctx_t * ctx);

//remove and return the map_obj_t from the map_obj_arr_t
map_obj_t remove_map_obj_from_map_obj_arr(map_obj_arr_t * arr,size_t index,err_ctx_t * ctx);

//get the map_obj_t instance from the map_obj_arr_t, read only
const map_obj_t get_map_obj_from_map_obj_arr(map_obj_arr_t * arr,size_t index,err_ctx_t * ctx);

//Remove multiple map_obj_t instances from the map_obj_arr_t. Note: can return null upon error Ex. when there are duplicate indexes provided
map_obj_t * remove_map_objs_from_obj_arr(map_obj_arr_t * arr,size_t * indexes,size_t n_indexes,err_ctx_t * ctx);

//delete the map_obj_t instance from the map_obj_arr_t at the provided index
void delete_map_obj_from_map_obj_arr(map_obj_arr_t * gws,size_t index,err_ctx_t * ctx);

//verify that a map_obj_t is of an expected type. Returns false if its not the expected type and throws an error in the error context.
bool verify_map_obj_in_map_obj_arr(map_obj_arr_t * arr,size_t index,uint8_t expected_type,err_ctx_t * ctx);

//Print out a map_obj_arr_t to the stream.
void map_obj_arr_to_output_stream(const map_obj_arr_t arr,FILE * stream,err_ctx_t * ctx);

//---------------------------------------------------------- MAP OBJECT ARRAY END ---------------------------------------------

#endif