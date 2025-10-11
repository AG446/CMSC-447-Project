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

#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Map_Node map_node_t;
typedef struct Map_Edge map_edge_t;
typedef struct Coordinate cord_t;
typedef struct Map map_t;
typedef struct Map_Rect map_rect_t;
typedef struct Map_Polygon_Object mpo_t;
typedef struct Map_Path map_path_t;
typedef struct Saved_Paths saved_paths_t;
typedef struct Building building_t;

//---------------------------------------------------------- GEOMETRY PRIMITIVES BEGIN ------------------------------------------------
/*
 * Real world coordinate of something.
 */
struct Coordinate{
	//like x-coord
	double longitude;
	
	//like y-coord
	double latitude;
};

//Create a new coordinate instance. Not on heap.
cord_t create_cord(double lon,double lat);

//Print out a coordinate with its longitude and latitude. Tabs value lets you add tabs to every line of output.
void cord_to_output_stream(cord_t cord,size_t tabs,FILE * stream);

/*
 * A Rectangle formed by two coordinates
 */
struct Map_Rect{
	cord_t bottom_left;
	cord_t top_right;
};

//create an instance of a map rectangle. Not on heap.
map_rect_t create_map_rect(cord_t bottom_left,cord_t top_right);

//Print out a map rectangle and all of its member data. Tabs value lets you add tabs to every line of output.
void map_rect_to_output_stream(map_rect_t rect,size_t tabs,FILE * stream);




//MPO stands for Map-Polygon-Object

//is the object water
#define MPO_TYPE_WATER 1

//is the object tree
#define MPO_TYPE_TREE 2

//is the object a building
#define MPO_TYPE_BUILDING 3

/*
 * A closed polygon formed by a list of coordinates. This will be used to draw buildings, lakes etc.
 */
struct Map_Polygon_Object{
	cord_t * cords;//array
	size_t n_cords;
	uint8_t type;
	char * name;
};

//create a new map polygon object instance on the heap.
mpo_t * create_mpo(const cord_t * cord_arry, size_t n_cords, uint8_t type);

//delete a map polygon object from the heap.
void delete_map_mpo(mpo_t * mpo_ref);

//edit a misplaced coordinate
void set_mpo_cord(mpo_t * mpo,size_t index,cord_t new_cord);

//edit type
void set_mpo_type(mpo_t * mpo,uint8_t new_type);

//Give an MPO a name
void set_mpo_name(mpo_t * mpo,const char * name);

//clear the name from an mpo
void clear_mpo_name(mpo_t * mpo);

//Print out a map rectangle and all of its member data. Tabs value lets you add tabs to every line of output.
void mpo_to_output_stream(const mpo_t * mpo,size_t tabs,FILE * stream);
//---------------------------------------------------------- GEOMETRY PRIMITIVES END --------------------------------------------------


//--------------------------------------------------------------- BUILDING BEGIN ------------------------------------------------------
/*
 * An object that holds generic information about a building.
 */
struct Building{
	//An overall box that covers an entire building. This will be used to figure out if your mouse is on a building.
	map_rect_t building_bounding_box;
	
	//The list of all the name alias strings of a building. The zero-th element is the primary name.
	char ** possible_names;
	size_t n_possible_names;
	size_t possible_names_capacity;
	
	//The number of floors in that building.
	uint8_t n_floors;
};

//Creating a new building instance in the heap. It will need to be deleted.
building_t * create_building(const char * primary_name,map_rect_t building_bounding_box,size_t n_floors);

//Delete the building instance on the heap
void delete_building(building_t * building);

//Add an alternative name to a building to make search more effective.
void add_building_alias_name(building_t * building,const char * alias_name);

//Remove an alias name from the alias list. This might be used to removed misspelled text.
void remove_building_alias_name(building_t * building,const char * alias_name);

//Take an existing name from the alias list and swap it with the primary name
void change_primary_building_name(building_t * building,const char * primary_name);

//Fix an incorrect floor count assignment to a building
void set_building_floor_count(building_t * building,size_t new_floor_count);

//change the bounding box of a building if it was incorrect
void set_building_bounding_box(building_t * building,map_rect_t building_bounding_box);

//Get the main name of a building. (Warning) Might return NULL
const char * get_primary_building_name(const building_t * building);

//Print out the building object and all of its member data. Tabs value lets you add tabs to every line of output.
void building_to_output_stream(const building_t * building,size_t tabs,FILE * stream);
//--------------------------------------------------------------- BUILDING END --------------------------------------------------------

//---------------------------------------------------------- NODES BEGIN --------------------------------------------------------------
/*
 * If your outside it does not make sense to assign a floor number to a node.
 */
#define NODE_FLOOR_NUMBER_NONE -128
/*
 * A node/location on the map with some details
 */
struct Map_Node{
	//location of node
	cord_t coordinate;
	
	//name of node if applicable
	char * name;
	
	//file path of node if applicable
	char * picture_file_path;
	
	//list of outgoing edges
	map_edge_t ** outgoing_edges;
	size_t n_outgoing_edges;
	size_t outgoing_edges_capacity;
	
	//associated building if applicable
	building_t * associated_building;
	
	//floor number of node if applicable
	int8_t floor_number;
	
	//whether or not the node is selectable with the mouse
	bool selectable;
	
	//use for intermediate calculation during A* search
	double cost_temp;
	
	//a temporary index which is used for file saving purposes
	size_t index_temp;
};

//Create a map_node_t object in the heap. This will need to be freed.
map_node_t * create_map_node(cord_t coordinate);

//Delete a map_node_t object.
void delete_map_node(map_node_t * node);

//Give a node a name.
void set_map_node_name(map_node_t * node,const char * name);

//clear the name field from a node
void clear_map_node_name(map_node_t * node);

//Set an image for node based on a file path.
void set_map_node_picture(map_node_t * node,const char * file_path);

//clear the file name field from a node
void clear_map_node_picture(map_node_t * node);

//Set a nodes floor number if applicable
void set_map_node_floor_number(map_node_t * node,int8_t floor_number);

//If a node doesn't have a floor number, like those outside then clear it
void clear_map_node_floor_number(map_node_t * node);

//Make a node selectable with the mouse or not.
void set_map_node_selectable(map_node_t * node,bool selectable);

//Set the building in which the node resides.
void set_map_node_building(map_node_t * node,building_t * building);

//If you accidently assigned a building to a node you can clear it.
void clear_map_node_building(map_node_t * node);

//edit a misplaced coordinate
void set_map_node_cord(map_node_t * node,cord_t new_cord);

//is the node next to an automatic door?
bool node_adjacent_to_auto_door(map_node_t * node);

//Print out a map node and show all of its member data. Tabs value lets you add tabs to every line of output.
void map_node_to_output_stream(const map_node_t * node,size_t tabs,FILE * stream);
//---------------------------------------------------------- NODES END ----------------------------------------------------------------



//---------------------------------------------------------- EDGES BEGIN --------------------------------------------------------------
//Is the edge a sidewalk
#define EDGE_TYPE_SIDEWALK 1

//Is the edge type a road
#define EDGE_TYPE_ROAD 2

//Is the edge type stairs
#define EDGE_TYPE_STAIRS 3

//Is the edge type a ramp
#define EDGE_TYPE_RAMP 4

//Is the edge type a hallway
#define EDGE_TYPE_HALLWAY 5

//Is the edge an elevator shaft
#define EDGE_TYPE_ELEVATOR_SHAFT 6

//Is the edge an overpass
#define EDGE_TYPE_OVERPASS 7

//Does the edge cross through door
#define EDGE_TYPE_DOOR 8

//Does the edge cross an automatic door
#define EDGE_TYPE_AUTO_DOOR 9

//Does the edge use a crosswalk
#define EDGE_TYPE_CROSSWALK 10

struct Map_Edge{
	map_node_t * a;
	map_node_t * b;
	uint8_t type;
};

//Create a map_edge_t object in the heap. This will need to be freed.
map_edge_t * create_map_edge(uint8_t type,map_node_t * a,map_node_t * b);

//Delete a map_edge_t object.
void delete_map_edge(map_edge_t * edge);

//change the type of the edge
void set_map_edge_type(map_edge_t * edge,uint8_t type);

//Print out a map edge and show all of its member data. Tabs value lets you add tabs to every line of output.
void map_edge_to_output_stream(const map_edge_t * edge,size_t tabs,FILE * stream);
//---------------------------------------------------------- EDGES END ----------------------------------------------------------------



//---------------------------------------------------------- MAP BEGIN ----------------------------------------------------------------
struct Search_Filter_Options{
	char * start_position_text;
	char * end_position_text;
	bool exclude_stairs;
	bool exclude_non_auto_doors;
	bool exclude_interiors;
};



/*
 * The map is all the nodes, all the edges and all the map-polygon-objects
 */
struct Map{
	//array of nodes
	map_node_t ** all_nodes;
	size_t n_nodes;
	size_t node_capacity;
	
	//array of edges (these are not manipulated directly)
	map_edge_t ** all_edges;
	size_t n_edges;
	size_t edge_capacity;
	
	//array of buildings
	building_t ** all_buildings;
	size_t n_buildings;
	size_t buildings_capacity;
	
	//array of map polygon objects
	mpo_t ** mpos;
	size_t n_mpos;
	size_t mpo_capacity;
	
	map_node_t * active_start;
	map_node_t * active_end;
	map_path_t * active_path;
	double (*active_edge_cost_function)(const map_edge_t * edge_ref);
};

//Create a map object. Not on heap.
map_t init_map(void);

//Clear heap data from within the map.
void clear_map(map_t * map);

//add building to map
void add_building_to_map(map_t * map,building_t * building);

//remove building from map
void remove_building_from_map(map_t * map,building_t * building);

//remove building by name
void remove_building_by_name(map_t * map,const char * name);

//add node to map
void add_node_to_map(map_t * map,map_node_t * node);//TODO

//remove node from map
void remove_node_from_map(map_t * map,map_node_t * node);//TODO

//remove node from map by name
void remove_node_by_name(map_t * map,const char * node_name);//TODO

//connect two nodes in a map
void connect_nodes_in_map(map_t * map,map_node_t * node_a,map_node_t * node_b,uint8_t edge_type);//TODO

//disconnect two nodes in a map
void disconnect_nodes_in_map(map_t * map,map_node_t * node_a,map_node_t * node_b);//TODO

//change the connection edge type
void set_connection_type(map_t * map,map_node_t * node_a,map_node_t * node_b,uint8_t new_edge_type);//TODO

//add a map polygon object to the map
void add_mpo_to_map(map_t * map,mpo_t * mpo);//TODO

//remove a map polygon object from the map
void remove_mpo_from_map(map_t * map,mpo_t * mpo);//TODO

//Print out a map and all its member data. Tabs value lets you add tabs to every line of output.
void map_to_output_stream(map_t map,size_t tabs,FILE * stream);//TODO

struct Map_Path{
	map_node_t ** nodes;//ordered list that defines a path, does not own nodes
	size_t n_nodes;//number of elements in this list
	char * name;//default NULL
};

struct Saved_Paths{
	map_path_t ** paths;
	size_t n_paths;
	size_t paths_capacity;
};
//---------------------------------------------------------- MAP END ------------------------------------------------------------------






//---------------------------------------------------------- FUNCTIONS BEGIN ----------------------------------------------------------




/*
 * The cost of an edge for people in wheelchairs
 */
double calculate_wheelchair_edge_cost(const map_edge_t * edge_ref);

/*
 * The cost of an edge for a walker
 */
double calculate_walker_edge_cost(const map_edge_t * edge_ref);

/*
 * The cost of an edge for transfering heavy equipment
 */
double calculate_deliverer_edge_cost(const map_edge_t * edge_ref);

/*
 * The cost of an edge for a driver
 */
double calculate_driver_edge_cost(const map_edge_t * edge_ref);

/*
 * Find a path which is the least cost given the edge_cost_function. Use Dijkstra's Algorithm
 * the start node is active_start and end node is active_end
 * store best path into active_path
 */
void find_best_path(map_t * map_ref);






/*
 * give a name to a path
 */
void set_map_path_name(map_path_t * map_path_ref,const char * path_name);

/*
 * Delete the map_path_t object
 * dev-note: Ensure that the name is freed.
 */
void delete_map_path(map_path_t * map_path_ref);

/*
 * copy a map path object
 */
map_path_t * copy_map_path(const map_path_t * map_path_ref);







/*
 * Get the box that encompases all map nodes.
 * This function is used to rescale our points to screen space.
 * 
 * Ex. If we have 3 nodes at (0,1), (-1,5), (2,3)
 * Then return the rectangle with a bottom left corner (-1,1) and top right corner (2,5)
 */
map_rect_t get_map_bounding_rect(const map_t * map_ref);






/*
 * create a saved_paths_t object
 */
saved_paths_t init_saved_paths();

/*
 * free all data in saved_paths_t and reset it
 */
void clear_saved_paths(saved_paths_t * saved_paths);

/*
 * filter map paths from saved_paths_t object, fuzzy search
 */
map_path_t ** filter_saved_paths(const char * path_name,const saved_paths_t * map_ref,size_t * n_results_out);






/*
 * Save the saved_paths_t object to a file
 */
void save_saved_paths(const saved_paths_t * saved_paths_ref,FILE * file);

/*
 * initialize the saved_paths_t object from file
 */
void init_saved_paths_from_file(saved_paths_t * saved_paths_ref,FILE * file);

/*
 * initialized a map from a file
 */
void init_map_from_file(map_t * map_ref,FILE * file);

/*
 * Save a map to a file
 */
void save_map_to_file(const map_t * map_ref,FILE * file);


//---------------------------------------------------------- FUNCTIONS END ------------------------------------------------------------


void do_thing();

#endif
