#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct Map_Node map_node_t;
typedef struct Map_Edge map_edge_t;
typedef struct Coordinate coord_t;
typedef struct Map map_t;
typedef struct Map_Rect map_rect_t;
typedef struct Map_Polygon_Object mpo_t;
typedef struct Map_Path map_path_t;
typedef struct Saved_Paths saved_paths_t;


//---------------------------------------------------------- GEOMETRY PRIMITIVES BEGIN ------------------------------------------------
/*
 * Real world coordinate of something.
 */
struct Coordinate{
	double longitude;//like x-coord
	double latitude;//like y-coord
};

/*
 * A Rectangle formed by two coordinates
 */
struct Map_Rect{
	coord_t bottom_left;
	coord_t top_right;
};


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
	coord_t * cords;//array
	size_t n_cords;
	uint8_t type;
};
//---------------------------------------------------------- GEOMETRY PRIMITIVES END --------------------------------------------------



//---------------------------------------------------------- NODES BEGIN --------------------------------------------------------------
//This is a "notable" location because it is some kind of location that can be selected/clicked on the map.
#define NODE_TYPE_NOTABLE_LOCATION 1

//This is not a notable location, merely a way of joining EDGES. This is useful for defining curves and sidewalk intersection.
#define NODE_TYPE_JOIN 2

/*
 * A node/location on the map with some details
 */
struct Map_Node{
	coord_t coordinate;//location of node
	char * name;//Notable locations have names, if not this should be NULL.
	map_edge_t * outgoing_edges;//all the edges connected to this node
	size_t n_outgoing_edges;//number of outgoing edges connected to this node
	size_t outgoing_edges_capacity;//the memory capacity of outgoing edges (defualt should be 2)
	uint8_t type;//the node type
};
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

struct Map_Edge{
	map_node_t * a;
	map_node_t * b;
	uint8_t type;
};
//---------------------------------------------------------- EDGES END ----------------------------------------------------------------



//---------------------------------------------------------- MAP BEGIN ----------------------------------------------------------------
/*
 * The map is all the nodes, all the edges and all the map-polygon-objects
 */
struct Map{
	map_node_t ** all_nodes;//array of map_node_t pointers
	size_t n_nodes;
	map_edge_t ** all_edges;//array of map_edge_t pointers
	size_t n_edges;
	mpo_t ** mpos;//array of mpo_t pointers (these are map polygon objects)
	size_t n_mpos;
	
	map_node_t * active_start;
	map_node_t * active_end;
	map_path_t * active_path;
	double (*active_edge_cost_function)(const map_edge_t * edge_ref);
};

struct Map_Path{
	map_node_t ** nodes;//ordered list that defines a path, does not own nodes
	size_t n_nodes;//number of elements in this list
	char * name;//default NULL
};

struct Saved_Paths{
	map_path_t * paths;
	size_t n_paths;
	size_t n_paths_capacity;
};
//---------------------------------------------------------- MAP END ------------------------------------------------------------------






//---------------------------------------------------------- FUNCTIONS BEGIN ----------------------------------------------------------
/*
 * Create a map_node_t object in the heap.
 * This will need to be freed.
 */
map_node_t * create_map_node(uint8_t type,coord_t coordinate);

/*
 * Delete a map_node_t object.
 * dev-note: Ensure that the name is freed.
 */
void delete_map_node(map_node_t * node);

/*
 * Set the name of a node.
 * dev-note: The node will store its own string copy
 */
void set_map_node_name(map_node_t * node_ref,const char * name_ref);






/*
 * Create a map_edge_t object in the heap.
 * This will need to be freed.
 */
map_edge_t * create_map_edge(uint8_t type,map_node_t * a,map_node_t * b);

/*
 * Delete a map_edge_t object.
 */
void delete_map_edge(map_edge_t * edge);



/*
 * Create a map object
 */
map_t init_map(void);

/*
 * initialize the map with some test setup of your choosing. For debugging and devlopement purposes
 */
void init_map_for_testing(map_t * map_ref);

/*
 * free all data in map_t and reset it
 */
void clear_map(map_t * map_ref);

/*
 * filter locations from map and return a list of map_node_t, fuzzy search
 */
map_node_t ** filter_locations(const char * location_name,const map_t * map_ref,size_t * n_results_out);




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
saved_paths_t init_saved_paths(void);

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




/*
 * format
 * n:node_index, longitude_double, latitude_double, name_string, type_name
 * Example "n:15, 89.1, -20.3, "commons east entrance", NOTABLE_LOCATION" will return a node with those properties
 */
map_node_t * create_map_node_from_text(const char * text);

/*
 * format
 * e: node_index_1, node_index_2, type_name
 * Example "e: 25, 30, ROAD" will return a node with those properties
 */
map_edge_t * create_map_edge_from_text(const char * text);

/*
 * format
 * p: name_string, node_index_1, node_index_2, node_index_3, ...
 * Example "p: getting to class, 25, 37, 12, 15"
 */
map_path_t * create_map_path_from_text(const char * text);

//---------------------------------------------------------- FUNCTIONS END ------------------------------------------------------------

#endif