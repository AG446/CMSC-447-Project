#include "cl_tool.h"
#include "string.h"
#include "text_proc.h"

int main(){
	err_ctx_t err_ctx = create_err_ctx();
	
	token_qual_t delete_tq = init_multi_token_qual(5);
	delete_tq.shared = true;
	delete_tq.strings[0] = strdup("delete");
	delete_tq.strings[1] = strdup("del");
	delete_tq.strings[2] = strdup("rem");
	delete_tq.strings[3] = strdup("remove");
	delete_tq.strings[4] = strdup("pop");
	
	token_qual_t create_tq = itq("create");
	create_tq.shared = true;
	
	token_qual_t node_tq = itq("node");
	node_tq.shared = true;
	
	token_qual_t nodes_tq = itq("nodes");
	nodes_tq.shared = true;
	
	token_qual_t mpo_tq = itq("mpo");
	mpo_tq.shared = true;
	
	token_qual_t map_tq = itq("map");
	map_tq.shared = true;
	
	token_qual_t add_tq = itq("add");
	add_tq.shared = true;
	
	token_qual_t prop_tq = init_multi_token_qual(2);
	prop_tq.strings[0] = strdup("property");
	prop_tq.strings[1] = strdup("prop");
	prop_tq.shared = true;
	
	token_qual_t set_tq = itq("set");
	set_tq.shared = true;
	
	token_qual_t building_tq = init_multi_token_qual(2);
	building_tq.shared = true;
	building_tq.strings[0] = strdup("building");
	building_tq.strings[1] = strdup("build");
	
	command_collection_t command_collection = init_command_collection(22);
	command_collection.commands[0] = init_phrase_command_L1(itq("help"),							"Show this screen.",help_command);
	command_collection.commands[1] = init_phrase_command_L1(itq("clear"),							"Clear the working set.",clear_command);
	command_collection.commands[2] = init_phrase_command_L1(delete_tq,								"Delete object from the working set.",delete_command);
	command_collection.commands[3] = init_phrase_command_L2(create_tq,itq("cord"),					"Create a coordinate and add it to the working set.",create_cord_command);
	command_collection.commands[4] = init_phrase_command_L1(itq("cc"),								"Fast create cord.",create_cord_command);
	command_collection.commands[5] = init_phrase_command_L2(create_tq,node_tq,						"Create a node and add it to the working set.",create_node_command);
	command_collection.commands[6] = init_phrase_command_L2(create_tq,mpo_tq,						"Create an MPO and add it to the working set.",create_mpo_command);
	command_collection.commands[7] = init_phrase_command_L2(create_tq,itq("rect"),					"Create a rectangle and add it to the working set.",create_rect_command);
	command_collection.commands[8] = init_phrase_command_L2(create_tq,building_tq,					"Create a building and add it to the working set.",create_building_command);
	command_collection.commands[9] = init_phrase_command_L2(itq("show"),map_tq,						"Show the entire map and all its data.",show_map_command);
	command_collection.commands[10] = init_phrase_command_L2(add_tq,node_tq,						"Add a node to the map.",add_node_to_map_command);
	command_collection.commands[11] = init_phrase_command_L2(add_tq,mpo_tq,							"Add an MPO to the map.",add_mpo_to_map_command);
	command_collection.commands[12] = init_phrase_command_L2(add_tq,building_tq,					"Add a building to the map.",add_building_to_map_command);
	command_collection.commands[13] = init_phrase_command_L2(map_tq,delete_tq,						"Delete an element from the map.",delete_from_map_command);
	command_collection.commands[14] = init_phrase_command_L2(itq("connect"),nodes_tq,				"Connect nodes in the map.",connect_nodes_command);
	command_collection.commands[15] = init_phrase_command_L2(itq("disconnect"),nodes_tq,			"Disconnect nodes within the map.",disconnect_nodes_command);
	command_collection.commands[16] = init_phrase_command_L2(itq("load"),map_tq,					"Load a map from a file.",load_map_command);
	command_collection.commands[17] = init_phrase_command_L2(itq("save"),map_tq,					"Save the map to a file.",save_map_command);
	command_collection.commands[18] = init_phrase_command_L3(set_tq,node_tq,prop_tq,				"Set, change or add properties to a node.",set_node_property_command);
	command_collection.commands[19] = init_phrase_command_L3(set_tq,mpo_tq,prop_tq,					"Set, change or add properties to an MPO.",set_mpo_property_command);
	command_collection.commands[20] = init_phrase_command_L3(set_tq,building_tq,prop_tq,			"Set, change or add properties to a building.",set_building_property_command);
	command_collection.commands[21] = init_phrase_command_L3(set_tq,itq("connection"),itq("type"),	"Change the connection type between nodes.",set_edge_type_command);
	
	map_t working_map = init_map();
	
	map_obj_arr_t working_set = init_map_obj_arr();
	
	while(true){
		//clear_terminal_screen();
		map_obj_arr_to_output_stream(working_set,stdout,&err_ctx);//show the working set
		
		char * line = read_line();
		c_str_lowercase(line);
		
		if(is_halting_string(line)){
			free(line);
			break;
		}
		
		size_t n_tokens = 0;
		char ** tokens = split_into_tokens(line,&n_tokens);
		free(line);
		search_and_run_command(command_collection,tokens,n_tokens,&working_map,&working_set,&err_ctx);
		delete_tokens(tokens,n_tokens);
		
		errs_to_output_stream(&err_ctx,stdout);
		reset_err_ctx(&err_ctx);
		
		fputs("Press ENTER to continue\n",stdout);
		getc(stdin);
	}
	
	deinit_map_obj_arr(&working_set,&err_ctx);
	deinit_map(&working_map,&err_ctx);
	
	deinit_command_collection(&command_collection,&err_ctx);
	deinit_token_qual(&delete_tq);
	deinit_token_qual(&create_tq);
	deinit_token_qual(&building_tq);
	deinit_token_qual(&node_tq);
	deinit_token_qual(&nodes_tq);
	deinit_token_qual(&add_tq);
	deinit_token_qual(&mpo_tq);
	deinit_token_qual(&map_tq);
	deinit_token_qual(&set_tq);
	deinit_token_qual(&prop_tq);
}
