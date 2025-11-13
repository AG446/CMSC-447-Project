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

#ifndef CL_TOOL_H
#define CL_TOOL_H

#include <stdint.h>
#include "map.h"
#include "map_obj_arr.h"



typedef struct Token_Qualifer token_qual_t;
typedef struct Phrase_Command phrase_command_t;
typedef struct Command_Collection command_collection_t;
typedef void (*command_f)(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);



//---------------------------------------------------------- TOKEN QUALIFIER BEGIN --------------------------------------------
/*
 * List of synonyms for a token. Usually just 1 string.
 * For example `quit` and `exit` are effectivly synonyms so we should group them under one qualifer.
 */
struct Token_Qualifer{
	char ** strings;
	size_t n_strings;
	bool shared;
};

//initialize a token qualifier based on a single string/token
token_qual_t init_token_qual(const char * token);

//shortcut function, exactly the same as init_token_qual
extern token_qual_t (*itq)(const char * token);

/*
 * Initialize a token qualifier which has multiple satisfactory tokens/strings. 
 * Provide the size here then set `strings` array directly. Make sure the strings are all on the heap.
*/
token_qual_t init_multi_token_qual(size_t size);

//Clear heap data from within the token qualifier.
void deinit_token_qual(token_qual_t * token_qual);

//Does the provided token/string satisfy the qualifer
bool token_qualifies(token_qual_t qual,const char * token);
//---------------------------------------------------------- TOKEN QUALIFIER END ----------------------------------------------



//---------------------------------------------------------- PHRASE COMMAND BEGIN ---------------------------------------------
/*
 * The phrase to tigger a command.
 * A phrase is a sequence of token qualifers.
 */
struct Phrase_Command{
	token_qual_t * sequence;
	size_t sequence_length;
	char * help_msg;
	command_f command_function;
};

//initialize a phrase command that is blank and has no heap data
phrase_command_t init_blank_phrase_command(void);

//create a phrase command made up of 1 token. You can pass NULL to either `help_msg` or `command_function`.
phrase_command_t init_phrase_command_L1(token_qual_t token_qual,const char * help_msg,command_f command_function);

//create a phrase command made up of 2 tokens. You can pass NULL to either `help_msg` or `command_function`.
phrase_command_t init_phrase_command_L2(token_qual_t token_qual_1, token_qual_t token_qual_2,const char * help_msg,command_f command_function);

//create a phrase command made up of 3 tokens. You can pass NULL to either `help_msg` or `command_function`.
phrase_command_t init_phrase_command_L3(token_qual_t token_qual_1, token_qual_t token_qual_2, token_qual_t token_qual_3,const char * help_msg,command_f command_function);

//Does the provided tokens array match/satisfy the phrase command. Note this function automatically calls the command_function.
bool matches_phrase(phrase_command_t pc,char ** tokens,size_t n_tokens,map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//Clear heap data from within the phrase command object. Note that if a token has the `shared` flag true, then it will not deinit it.
void deinit_phrase_command(phrase_command_t * pc,err_ctx_t * ctx);

//Print the phrase command instance. This is basically just the help message.
void phrase_command_to_output_stream(phrase_command_t pc,FILE * stream,err_ctx_t * ctx);
//---------------------------------------------------------- PHRASE COMMAND END -----------------------------------------------



//---------------------------------------------------------- COMMAND COLLECTION BEGIN -----------------------------------------
/*
 * The collection of all commands.
 */
struct Command_Collection{
	phrase_command_t * commands;
	size_t n_commands;
};

//initialize the command collection. The size is the number of commands.
command_collection_t init_command_collection(size_t size);

//deinitialize the command collection data from the heap
void deinit_command_collection(command_collection_t * collection,err_ctx_t * ctx);

//given the tokens and the provided data find the command in the collection and run the associated command_function
void search_and_run_command(command_collection_t collection,char ** tokens,size_t n_tokens,map_t * map,map_obj_arr_t * arr,err_ctx_t * ctx);
//---------------------------------------------------------- COMMAND COLLECTION END -------------------------------------------

//read a single line from the command line.
char * read_line();

//clear the whole terminal screen.
void clear_terminal_screen(void);

//is the string some way of exiting. Ex. exit, quit
bool is_halting_string(const char * str);

//parse a double from the command line
double parse_double(const char * title,bool * canceled);

//parse an index form the command line
size_t parse_index(const char * title,bool * canceled);

//parse an index within the range (max included) from the command line
size_t parse_index_in_range(const char * title,size_t min,size_t max,bool * canceled);

//Parse whether the user is confirming a yes or no.
bool parse_confirmation(const char * title);

//Parse a bool, true or false, from the command line.
bool parse_bool(const char * title,bool * canceled);

//parse the chosen open from a list of options within the command line.
size_t parse_among_options(const char * title,const char ** options,size_t n_options,bool * canceled);

//parse the MPO type from the command line
uint8_t parse_mpo_type(bool * canceled);

//Parse the edge type from the command line
uint8_t parse_edge_type(bool * canceled);

//parse whether the user want to obtain a map_obj_t from the map or the working set.
#define WORKING_MAP 1
#define WORKING_SET 2
size_t parse_working_location(bool * canceled);

//Parse a coordinate from the command line
cord_t parse_cord(bool * canceled);

//fetch a node from either the map or the working set.
map_node_t * fetch_node(map_t * map,map_obj_arr_t * gws,err_ctx_t * ctx);

//fetch an MPO from either the map or the working set.
mpo_t * fetch_mpo(map_t * map,map_obj_arr_t * gws,err_ctx_t * ctx);

//fetch a building from either the map or the working set.
building_t * fetch_building(map_t * map,map_obj_arr_t * gws,err_ctx_t * ctx);



//the command_f for adding a cord to the working set
void create_cord_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for adding a rectangle to the working set
void create_rect_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for adding an MPO to the working set
void create_mpo_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for adding a node to the working set
void create_node_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for adding a building to the working set
void create_building_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for delete an object from within the working set
void delete_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for showing all the data from within the map
void show_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to add a node from the working set to the map
void add_node_to_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to add an MPO from the working set to the map
void add_mpo_to_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for adding a building from the working set to the map
void add_building_to_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for setting/adding the properties of a node
void set_node_property_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for setting/adding the properties of an MPO
void set_mpo_property_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f for setting/adding the properties of a building
void set_building_property_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to delete objects from within the map
void delete_from_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to connect nodes from within the map
void connect_nodes_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to disconnect nodes from within the map
void disconnect_nodes_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to set the connection type or edge between two nodes from within the map
void set_edge_type_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to load the map from a file
void load_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to save the map to a file
void save_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to get the help message
void help_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

//the command_f to clear all the objects from the working set.
void clear_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx);

#endif