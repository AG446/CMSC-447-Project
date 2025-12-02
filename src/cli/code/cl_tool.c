#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map_obj_arr.h"
#include "cl_tool.h"
#include "text_proc.h"
#include "err_ctx.h"
#include <ctype.h>
#include "map_serial.h"

token_qual_t init_token_qual(const char * token){
	token_qual_t out;
	
	out.shared = false;
	
	if(token == NULL){
		out.strings = NULL;
		out.n_strings = 0;
	}else{
		out.strings = (char**) malloc(sizeof(char*) * 1);
		out.strings[0] = strdup(token);
		out.n_strings = 1;
	}
	
	return out;
}

token_qual_t (*itq)(const char * token) = init_token_qual;

token_qual_t init_multi_token_qual(size_t size){
	token_qual_t out;
	
	out.shared = false;
	
	out.strings = (char**) malloc(sizeof(char*) * size);
	for(size_t i = 0; i < size;i++) out.strings[i] = NULL;
	out.n_strings = size;
	
	return out;
}

void deinit_token_qual(token_qual_t * token_qual){
	if(token_qual == NULL) return;
	
	if(token_qual->strings == NULL) return;
	
	for(size_t i = 0;i < token_qual->n_strings;i++){
		if(token_qual->strings[i] != NULL) free(token_qual->strings[i]);
	}
	
	free(token_qual->strings);
}

bool token_qualifies(token_qual_t qual,const char * token){
	if(qual.strings == NULL) return true;
	if(token == NULL) return false;
	
	for(size_t i = 0;i < qual.n_strings;i++){
		if(qual.strings[i] != NULL && strcmp(qual.strings[i],token) == 0 ) return true;
	}
	
	return false;
}

phrase_command_t init_blank_phrase_command(void){
	phrase_command_t out;
	
	out.sequence = NULL;
	out.sequence_length = 0;
	out.help_msg = NULL;
	out.command_function = NULL;
	
	return out;
}

phrase_command_t init_phrase_command_L1(token_qual_t token_qual,const char * help_msg,command_f command_function){
	phrase_command_t out;
	
	if(help_msg != NULL) out.help_msg = strdup(help_msg);
	else out.help_msg = NULL;
	
	out.sequence_length = 1;
	out.sequence = (token_qual_t*) malloc(sizeof(token_qual_t) * 1);
	out.sequence[0] = token_qual;
	out.command_function = command_function;
	
	return out;
}

phrase_command_t init_phrase_command_L2(token_qual_t token_qual_1, token_qual_t token_qual_2,const char * help_msg,command_f command_function){
	phrase_command_t out;
	
	if(help_msg != NULL) out.help_msg = strdup(help_msg);
	else out.help_msg = NULL;
	
	out.sequence_length = 2;
	out.sequence = (token_qual_t*) malloc(sizeof(token_qual_t) * 2);
	out.sequence[0] = token_qual_1;
	out.sequence[1] = token_qual_2;
	out.command_function = command_function;
	
	return out;
}

phrase_command_t init_phrase_command_L3(token_qual_t token_qual_1, token_qual_t token_qual_2, token_qual_t token_qual_3,const char * help_msg,command_f command_function){
	phrase_command_t out;
	
	if(help_msg != NULL) out.help_msg = strdup(help_msg);
	else out.help_msg = NULL;
	
	out.sequence_length = 3;
	out.sequence = (token_qual_t*) malloc(sizeof(token_qual_t) * 3);
	out.sequence[0] = token_qual_1;
	out.sequence[1] = token_qual_2;
	out.sequence[2] = token_qual_3;
	out.command_function = command_function;
	
	return out;
}

bool matches_phrase(phrase_command_t pc,char ** tokens,size_t n_tokens,map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	if(tokens == NULL || map == NULL || arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	if(n_tokens != pc.sequence_length) return false;
	
	for(size_t i = 0;i < n_tokens;i++){
		if(!token_qualifies(pc.sequence[i],tokens[i])) return false;
	}
	
	if(pc.command_function != NULL) pc.command_function(map,arr,collection,ctx);
	
	return true;
}

void deinit_phrase_command(phrase_command_t * pc,err_ctx_t * ctx){
	if(pc == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(pc->help_msg != NULL) free(pc->help_msg);
	if(pc->sequence == NULL) return;
	
	for(size_t i = 0;i < pc->sequence_length;i++){
		if(pc->sequence[i].shared) continue;
		deinit_token_qual(&(pc->sequence[i]));
	}
	
	free(pc->sequence);
}

command_collection_t init_command_collection(size_t size){
	command_collection_t out;
	
	out.n_commands = size;
	if(size == 0){
		out.commands = NULL;
		return out;
	}
	
	out.commands = (phrase_command_t*) malloc(sizeof(phrase_command_t) * size);
	for(size_t i = 0;i < size;i++){
		out.commands[i] = init_blank_phrase_command();
	}
	
	return out;
}

void deinit_command_collection(command_collection_t * collection,err_ctx_t * ctx){
	if(collection == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(collection->commands == NULL) return;
	
	for(size_t i = 0;i < collection->n_commands;i++){
		deinit_phrase_command(&(collection->commands[i]),ctx);
	}
	
	free(collection->commands);
}

void search_and_run_command(command_collection_t collection,char ** tokens,size_t n_tokens,map_t * map,map_obj_arr_t * arr,err_ctx_t * ctx){
	if(tokens == NULL || map == NULL || arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	for(size_t i = 0;i < collection.n_commands;i++){
		if(matches_phrase(collection.commands[i],tokens,n_tokens,map,arr,collection,ctx)) return;
	}
}

void phrase_command_to_output_stream(phrase_command_t pc,FILE * stream,err_ctx_t * ctx){
	fputs("-[",stream);
	fputs("\033[93m",stdout);
	for(size_t i = 0;i < pc.sequence_length;i++){
		token_qual_t current = pc.sequence[i];
		for(size_t j = 0;j < current.n_strings;j++){
			if(current.strings[j] != NULL){
				fputs(current.strings[j], stream);
			}
			if(j != current.n_strings-1) fputc('/',stream);
		}
		if(i != pc.sequence_length-1) fputc(' ',stream);
	}
	fputs("\033[0m",stdout);
	fputs("] ",stream);
	
	fputs("Description: \033[36m",stream);
	if(pc.help_msg != NULL){
		fputs(pc.help_msg,stream);
	}else{
		fputs("None",stream);
	}
	fputs("\033[0m\n",stream);
}

char * read_line(void){
	fputs("\033[93m>\033[0m ",stdout);
	fflush(stdout);
	
	size_t allocated_space = 16;
	char * output = (char*) malloc(allocated_space);
	size_t current_length = 0;
	
	int val;
	while((val = getc(stdin)) != '\n'){
		if(allocated_space == current_length){
			allocated_space *= 2;
			output = (char*) realloc(output,allocated_space);
		}
		output[current_length] = (char)val;
		current_length++;
	}
	
	if(allocated_space == current_length){
		allocated_space *= 2;
		output = (char*) realloc(output,allocated_space);
	}
	output[current_length] = '\0';
	
	return output;
}

void clear_terminal_screen(void){
	printf("\e[1;1H\e[2J");
}

bool is_halting_string(const char * str){
	if(str == NULL) return false;
	
	if(strcmp(str,"stop") == 0) return true;
	if(strcmp(str,"halt") == 0) return true;
	if(strcmp(str,"quit") == 0) return true;
	if(strcmp(str,"exit") == 0) return true;
	if(strcmp(str,"cancel") == 0) return true;
	
	return false;
}

double parse_double(const char * title,bool * canceled){
	double value = 0.0;
	bool valid = false;
	char extra;
	
	while(!valid){
		fputs(title,stdout);
		fputc(' ',stdout);
		char * double_string = read_line();
		c_str_lowercase(double_string);
		if(is_halting_string(double_string)){
			*canceled = true;
			free(double_string);
			return 0.0;
		}
	
		if(sscanf(double_string, " %lf %c", &value, &extra) == 1) {
			valid = true;
		}
		
		free(double_string);
	}
	
	return value;
}

size_t parse_index(const char * title,bool * canceled){
	size_t value = 0;
	bool valid = false;
	char extra;
	
	while(!valid){
		fputs(title,stdout);
		fputc(' ',stdout);
		char * index_string = read_line();
		c_str_lowercase(index_string);
		if(is_halting_string(index_string)){
			*canceled = true;
			free(index_string);
			return 0.0;
		}
		
		if(sscanf(index_string, " %lu %c", &value, &extra) == 1) {
			valid = true;
		}
		
		free(index_string);
	}
	
	return value;
}

size_t parse_index_in_range(const char * title,size_t min,size_t max,bool * canceled){
	size_t choice = 0;
	
	while(true){
		choice = parse_index(title,canceled);
		if(*canceled) break;
		
		if(choice < min || choice > max){
			fputs("Try again.\n",stdout);
		}else break;
	}
	
	return choice;
}

bool parse_confirmation(const char * title){
	fputs(title,stdout);
	fputc(' ',stdout);
	char * choice_string = read_line();
	
	bool out = strlen(choice_string) == 1 && tolower(choice_string[0]) == 'y';
	
	free(choice_string);
	
	return out;
}

bool parse_bool(const char * title,bool * canceled){
	fputs(title,stdout);
	fputc(' ',stdout);
	char * choice_string = read_line();
	
	c_str_lowercase(choice_string);
	
	bool out = false;;
	if((strcmp(choice_string,"t") == 0) || (strcmp(choice_string,"true") == 0)){
		out = true;
	}else if((strcmp(choice_string,"f") == 0) || (strcmp(choice_string,"false") == 0)){
		out = false;
	}else{
		*canceled = true;
	}
	
	free(choice_string);
	
	return out;
}

size_t parse_among_options(const char * title,const char ** options,size_t n_options,bool * canceled){
	fputs(title,stdout);
	fputc('\n',stdout);
	for(size_t i = 1;i <= n_options;i++){
		printf("%lu. %s\n",i,options[i-1]);
	}
	
	return parse_index_in_range("Choose",1,n_options,canceled);
}

uint8_t parse_mpo_type(bool * canceled){
	return (uint8_t) parse_among_options("### Choose an MPO type ###",mpo_type_names,N_MPO_TYPES,canceled);
}

uint8_t parse_edge_type(bool * canceled){
	return (uint8_t) parse_among_options("### Choose an Edge type ###",edge_type_names,N_EDGE_TYPES,canceled);
}

size_t parse_working_location(bool * canceled){
	const char * options[2] = {"Working Map","Working Set"};
	
	return parse_among_options("### Choose Where to Find ###",options,2,canceled);
}

cord_t parse_cord(bool * canceled){
	fputs("### Entering a Coordinate ###\n",stdout);
	
	float longitude = parse_double("Longitude",canceled);
	if(*canceled) return create_cord(0.0,0.0);
	
	float latitude = parse_double("Latitude",canceled);
	if(*canceled) return create_cord(0.0,0.0);
	
	return create_cord(longitude,latitude);
}

#define FETCH_NODE_N_OPTIONS 2
map_node_t * fetch_node(map_t * map,map_obj_arr_t * gws,err_ctx_t * ctx){
	bool canceled = false;
	size_t choice = parse_working_location(&canceled);
	if(canceled) return NULL;
	
	if(choice == WORKING_MAP){
		const char * options[FETCH_NODE_N_OPTIONS] = {
			"Find By Name",
			"Find By Index"
		};
		
		size_t chosen_option = parse_among_options("Choose How to Find Node",options,FETCH_NODE_N_OPTIONS,&canceled);
		if(canceled) return NULL;
		
		if(chosen_option == 1){
			fputs("Name of Node ",stdout);
			char * name = read_line();
			map_node_t * out = get_node_by_name_from_map(map,name,ctx);
			free(name);
			return out;
		}else if(chosen_option == 2){
			size_t node_index = parse_index_in_range("Index of Node in Map",0,get_map_node_count(*map)-1,&canceled);
			if(canceled) return NULL;
			return get_node_by_index_from_map(*map,node_index,ctx);
		}
	}else if(choice == WORKING_SET){
		size_t obj_index = parse_index("Node index",&canceled);
		if(canceled || !verify_map_obj_in_map_obj_arr(gws,obj_index,MO_TYPE_NODE,ctx)) return NULL;
		
		map_obj_t obj = get_map_obj_from_map_obj_arr(gws,obj_index,ctx);
		return obj.node;
	}
	
	return NULL;
}

#define FETCH_MPO_N_OPTIONS 2
mpo_t * fetch_mpo(map_t * map,map_obj_arr_t * gws,err_ctx_t * ctx){
	bool canceled = false;
	size_t choice = parse_working_location(&canceled);
	if(canceled) return NULL;
	
	if(choice == WORKING_MAP){
		const char * options[FETCH_MPO_N_OPTIONS] = {
			"Find By Name",
			"Find By Index"
		};
		
		size_t chosen_option = parse_among_options("Choose How to Find MPO",options,FETCH_MPO_N_OPTIONS,&canceled);
		if(canceled) return NULL;
		
		if(chosen_option == 1){
			fputs("Name of MPO ",stdout);
			char * name = read_line();
			mpo_t * out = get_mpo_by_name_from_map(map,name,ctx);
			free(name);
			return out;
		}else if(chosen_option == 2){
			size_t mpo_index = parse_index_in_range("Index of MPO in Map",0,get_map_mpo_count(*map)-1,&canceled);
			if(canceled) return NULL;
			return get_mpo_by_index_from_map(*map,mpo_index,ctx);
		}
	}else if(choice == WORKING_SET){
		size_t obj_index = parse_index("MPO index",&canceled);
		if(canceled || !verify_map_obj_in_map_obj_arr(gws,obj_index,MO_TYPE_MPO,ctx)) return NULL;
		
		map_obj_t obj = get_map_obj_from_map_obj_arr(gws,obj_index,ctx);
		return obj.mpo;
	}
	
	return NULL;
}

#define FETCH_BUILDING_N_OPTIONS 2
building_t * fetch_building(map_t * map,map_obj_arr_t * gws,err_ctx_t * ctx){
	bool canceled = false;
	size_t choice = parse_working_location(&canceled);
	if(canceled) return NULL;
	
	if(choice == WORKING_MAP){
		const char * options[FETCH_BUILDING_N_OPTIONS] = {
			"Find By Name",
			"Find By Index"
		};
		
		size_t chosen_option = parse_among_options("Choose How to Find Building",options,FETCH_BUILDING_N_OPTIONS,&canceled);
		if(canceled) return NULL;
		
		if(chosen_option == 1){
			fputs("Name of Building ",stdout);
			char * name = read_line();
			building_t * out = get_building_by_name_from_map(map,name,ctx);
			free(name);
			return out;
		}else if(chosen_option == 2){
			size_t building_index = parse_index_in_range("Index of Building in Map",0,get_map_building_count(*map)-1,&canceled);
			if(canceled) return NULL;
			return get_building_by_index_from_map(*map,building_index,ctx);
		}
	}else if(choice == WORKING_SET){
		size_t obj_index = parse_index("Building index",&canceled);
		if(canceled || !verify_map_obj_in_map_obj_arr(gws,obj_index,MO_TYPE_BUILDING,ctx)) return NULL;
		
		map_obj_t obj = get_map_obj_from_map_obj_arr(gws,obj_index,ctx);
		return obj.building;
	}
	
	return NULL;
}

void create_cord_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	cord_t cord = parse_cord(&canceled);
	if(!canceled){
		add_map_obj_to_map_obj_arr(arr,init_cord_map_obj(cord),ctx);
	}
}

void create_rect_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	size_t first_obj_index = parse_index("Coordinte Object Index (Bottom Left)",&canceled);
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,first_obj_index,MO_TYPE_CORD,ctx)) return;
	size_t second_obj_index = parse_index("Coordinte Object Index (Top Right)",&canceled);
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,second_obj_index,MO_TYPE_CORD,ctx)) return;
	
	if(first_obj_index == second_obj_index){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	size_t indexes[2] = {first_obj_index,second_obj_index};
	map_obj_t * removed = remove_map_objs_from_obj_arr(arr,indexes,2,ctx);
	map_obj_t first_obj = removed[0];
	map_obj_t second_obj = removed[1];
	free(removed);
	
	add_map_obj_to_map_obj_arr(arr,init_rect_map_obj( create_map_rect(first_obj.cord,second_obj.cord) ),ctx);
}

void create_mpo_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	size_t n_cords = parse_index_in_range("Number of Coordinates",3,512,&canceled);
	if(canceled) return;
	
	size_t * obj_indexes = (size_t*) malloc(sizeof(size_t)*n_cords);
	
	for(size_t i = 0; i < n_cords;i++){
		printf("%lu ",i);
		size_t obj_index = parse_index("Coordinte Object Index",&canceled);
		
		if(canceled || !verify_map_obj_in_map_obj_arr(arr,obj_index,MO_TYPE_CORD,ctx)) {
			free(obj_indexes);
			return;
		}
		
		obj_indexes[i] = obj_index;
	}
	
	map_obj_t * removed = remove_map_objs_from_obj_arr(arr,obj_indexes,n_cords,ctx);
	free(obj_indexes);
	if(removed == NULL) return;
	
	cord_t * cord_arr = (cord_t*) malloc(sizeof(cord_t)*n_cords);
	for(size_t i = 0;i < n_cords;i++){
		cord_arr[i] = removed[i].cord;
	}
	free(removed);
	
	uint8_t chosen_mpo_type = parse_mpo_type(&canceled);
	if(canceled) free(cord_arr);
	
	add_map_obj_to_map_obj_arr(arr,init_mpo_map_obj( create_mpo(cord_arr,n_cords,chosen_mpo_type,ctx),ctx ),ctx);
	free(cord_arr);
}

void create_node_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	size_t obj_index = parse_index("Coordinte Object Index",&canceled);
	
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,obj_index,MO_TYPE_CORD,ctx)) return;
	
	map_obj_t removed = remove_map_obj_from_map_obj_arr(arr,obj_index,ctx);
	
	add_map_obj_to_map_obj_arr(arr,init_node_map_obj( create_map_node(removed.cord),ctx),ctx);
}

void delete_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	size_t deletion_index = parse_index("Object Index",&canceled);
		
	if(canceled) return;
	
	delete_map_obj_from_map_obj_arr(arr,deletion_index,ctx);
}

void show_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	fputs("\033[36m### Current Working Map ###\n\033[0m",stdout);
	map_to_output_stream(*map,0,stdout,ctx);
}

void add_node_to_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	size_t obj_index = parse_index("Node index",&canceled);
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,obj_index,MO_TYPE_NODE,ctx)) return;
	
	map_obj_t removed = remove_map_obj_from_map_obj_arr(arr,obj_index,ctx);
	
	add_node_to_map(map,removed.node,ctx);
}

void add_mpo_to_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	size_t obj_index = parse_index("MPO index",&canceled);
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,obj_index,MO_TYPE_MPO,ctx)) return;
	
	map_obj_t removed = remove_map_obj_from_map_obj_arr(arr,obj_index,ctx);
	
	add_mpo_to_map(map,removed.mpo,ctx);
}

void add_building_to_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	size_t obj_index = parse_index("Building index",&canceled);
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,obj_index,MO_TYPE_BUILDING,ctx)) return;
	
	map_obj_t removed = remove_map_obj_from_map_obj_arr(arr,obj_index,ctx);
	
	add_building_to_map(map,removed.building,ctx);
}

#define SET_NODE_PROP_N_OPTIONS 10
void set_node_property_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	map_node_t * node = fetch_node(map,arr,ctx);
	if(node == NULL) return;
	
	map_node_to_output_stream(node,0,stdout,ctx);
	
	const char * options[SET_NODE_PROP_N_OPTIONS] = {
		"Change Coordinate",
		"Set Name",
		"Set Picture File Path",
		"Set Associated Building",
		"Set Floor Number",
		"Set Selectable",
		"Clear Name",
		"Clear Picture File Path",
		"Clear Associated Building",
		"Clear Floor Number"
	};
	size_t chosen_option = parse_among_options("Choose which property to edit",options,SET_NODE_PROP_N_OPTIONS,&canceled);
	if(canceled) return;
	
	if(chosen_option == 1){
		cord_t cord = parse_cord(&canceled);
		if(canceled) return;
		set_map_node_cord(node,cord,ctx);
	}else if(chosen_option == 2){
		fputs("Name ",stdout);
		char * name = read_line();
		set_map_node_name(node,name,ctx);
		free(name);
	}else if(chosen_option == 3){
		fputs("Picture Path ",stdout);
		char * file_path = read_line();
		set_map_node_picture(node,file_path,ctx);
		free(file_path);
	}else if(chosen_option == 4){
		fputs("Select Building\n",stdout);
		building_t * fetched_building = fetch_building(map,arr,ctx);
		if(fetched_building == NULL) return;
		set_map_node_building(node,fetched_building,ctx);
	}else if(chosen_option == 5){
		size_t floor_number = parse_index_in_range("Floor Number (0,64)",0,64,&canceled);
		if(canceled) return;
		set_map_node_floor_number(node,floor_number,ctx);
	}else if(chosen_option == 6){
		bool selectable = parse_bool("Is it selectable (true/false)",&canceled);
		if(canceled) return;
		set_map_node_selectable(node,selectable,ctx);
	}else if(chosen_option == 7){
		clear_map_node_name(node,ctx);
	}else if(chosen_option == 8){
		clear_map_node_picture(node,ctx);
	}else if(chosen_option == 9){
		clear_map_node_building(node,ctx);
	}else if(chosen_option == 10){
		clear_map_node_floor_number(node,ctx);
	}
	
	map_node_to_output_stream(node,0,stdout,ctx);
}

void create_building_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	fputs("Building name ",stdout);
	char * building_name = read_line();
	
	size_t rect_index = parse_index("Bounding Box Rectangle index",&canceled);
	if(canceled || !verify_map_obj_in_map_obj_arr(arr,rect_index,MO_TYPE_RECT,ctx)){
		free(building_name);
		return;
	}
	
	size_t n_floors = parse_index_in_range("Number of Floors (0,64)",0,64,&canceled);
	if(canceled){
		free(building_name);
		return;
	}
	
	map_obj_t removed = remove_map_obj_from_map_obj_arr(arr,rect_index,ctx);
	map_rect_t building_rect = removed.rect;
	
	add_map_obj_to_map_obj_arr(arr,init_building_map_obj( create_building(building_name,building_rect,n_floors,ctx),ctx),ctx);
	
	free(building_name);
}

#define SET_MPO_PROP_N_OPTIONS 4
void set_mpo_property_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	mpo_t * mpo = fetch_mpo(map,arr,ctx);
	if(mpo == NULL) return;
	
	mpo_to_output_stream(mpo,0,stdout,ctx);
	
	const char * options[SET_MPO_PROP_N_OPTIONS] = {
		"Edit Coordinate",
		"Set Name",
		"Set Type",
		"Clear Name"
	};
	
	size_t chosen_option = parse_among_options("Choose which property to edit",options,SET_MPO_PROP_N_OPTIONS,&canceled);
	if(canceled) return;
	
	if(chosen_option == 1){
		size_t index = parse_index_in_range("Index To Edit",0,get_mpo_size(mpo,ctx)-1,&canceled);
		if(canceled) return;
		cord_t new_cord = parse_cord(&canceled);
		if(canceled) return;
		set_mpo_cord(mpo,index,new_cord,ctx);
	}else if(chosen_option == 2){
		fputs("Name ",stdout);
		char * name = read_line();
		set_mpo_name(mpo,name,ctx);
		free(name);
	}else if(chosen_option == 3){
		uint8_t mpo_type = parse_mpo_type(&canceled);
		if(canceled) return;
		set_mpo_type(mpo,mpo_type,ctx);
	}else if(chosen_option == 4){
		clear_mpo_name(mpo,ctx);
	}
	
	mpo_to_output_stream(mpo,0,stdout,ctx);
}

#define SET_BUILDING_PROP_N_OPTIONS 5
void set_building_property_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	building_t * building = fetch_building(map,arr,ctx);
	if(building == NULL) return;
	
	building_to_output_stream(building,0,stdout,ctx);
	
	const char * options[SET_BUILDING_PROP_N_OPTIONS] = {
		"Change Bounding Box",
		"Add alias name",
		"Remove alias name",
		"Change Primary Name",
		"Set Floor Count"
	};
	
	size_t chosen_option = parse_among_options("Choose which property to edit",options,SET_BUILDING_PROP_N_OPTIONS,&canceled);
	if(canceled) return;
	
	if(chosen_option == 1){
		size_t rect_index = parse_index("Bounding Box Rectangle index",&canceled);
		if(canceled || !verify_map_obj_in_map_obj_arr(arr,rect_index,MO_TYPE_RECT,ctx)) return;
		map_obj_t removed = remove_map_obj_from_map_obj_arr(arr,rect_index,ctx);
		map_rect_t bounding_box = removed.rect;
		
		set_building_bounding_box(building,bounding_box,ctx);
	}else if(chosen_option == 2){
		fputs("Alias Name ",stdout);
		char * alias_name = read_line();
		add_building_alias_name(building,alias_name,ctx);
		free(alias_name);
	}else if(chosen_option == 3){
		fputs("Name to remove ",stdout);
		char * name = read_line();
		remove_building_alias_name(building,name,ctx);
		free(name);
	}else if(chosen_option == 4){
		fputs("Primary Name ",stdout);
		char * name = read_line();
		change_primary_building_name(building,name,ctx);
		free(name);
	}else if(chosen_option == 5){
		size_t n_floors = parse_index_in_range("Number of Floors (0,64)",0,64,&canceled);
		if(canceled) return;
		set_building_floor_count(building,n_floors,ctx);
	}
	
	building_to_output_stream(building,0,stdout,ctx);
}

#define DELETE_FROM_MAP_N_DEL_OPTIONS 3
#define DELETE_FROM_MAP_N_NODE_DELETE_OPTIONS 2
void delete_from_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	const char * del_options[DELETE_FROM_MAP_N_DEL_OPTIONS] = {
		"Remove Node",
		"Remove MPO",
		"Remove Building"
	};
	
	size_t chosen_option = parse_among_options("Choose what to delete from the map",del_options,DELETE_FROM_MAP_N_DEL_OPTIONS,&canceled);
	if(canceled) return;
	
	const char * node_delete_options[DELETE_FROM_MAP_N_NODE_DELETE_OPTIONS] = {
		"Remove By Name",
		"Remove By Index"
	};
	
	if(chosen_option == 1){
		chosen_option = parse_among_options("Choose How to Delete Node",node_delete_options,DELETE_FROM_MAP_N_NODE_DELETE_OPTIONS,&canceled);
		if(canceled) return;
		
		if(chosen_option == 1){
			fputs("Node Name ",stdout);
			char * name = read_line();
			remove_node_by_name_from_map(map,name,ctx);
			free(name);
		}else if(chosen_option == 2){
			size_t node_index = parse_index_in_range("Index of Node in Map",0,get_map_node_count(*map)-1,&canceled);
			if(canceled) return;
			remove_node_from_map_by_index(map,node_index,ctx);
		}
	}else if(chosen_option == 2){
		chosen_option = parse_among_options("Choose How to Delete MPO",node_delete_options,DELETE_FROM_MAP_N_NODE_DELETE_OPTIONS,&canceled);
		if(canceled) return;
		
		if(chosen_option == 1){
			fputs("MPO Name ",stdout);
			char * name = read_line();
			remove_mpo_from_map_by_name(map,name,ctx);
			free(name);
		}else if(chosen_option == 2){
			size_t mpo_index = parse_index_in_range("Index of MPO in Map",0,get_map_mpo_count(*map)-1,&canceled);
			if(canceled) return;
			remove_mpo_from_map_by_index(map,mpo_index,ctx);
		}
	}else if(chosen_option == 3){
		chosen_option = parse_among_options("Choose How to Delete Building",node_delete_options,DELETE_FROM_MAP_N_NODE_DELETE_OPTIONS,&canceled);
		if(canceled) return;
		
		if(chosen_option == 1){
			fputs("Building Name ",stdout);
			char * name = read_line();
			remove_building_by_name_from_map(map,name,ctx);
			free(name);
		}else if(chosen_option == 2){
			size_t building_index = parse_index_in_range("Index of Building in Map",0,get_map_building_count(*map)-1,&canceled);
			if(canceled) return;
			remove_building_from_map_by_index(map,building_index,ctx);
		}
	}
}

#define CONNECT true
#define DISCONNECT false
#define N_CONNECT_DISCONNECT_OPTIONS 2
void connect_disconnect_nodes_command(map_t * map,err_ctx_t * ctx,bool connect_mode){
	bool canceled = false;
	
	uint8_t chosen_edge_type = 0;
	if(connect_mode == CONNECT){
		chosen_edge_type = parse_edge_type(&canceled);
		if(canceled) return;
	}
	
	size_t chosen_connection_option = 0;
	
	if(connect_mode == CONNECT){
		const char * connect_options[N_CONNECT_DISCONNECT_OPTIONS] = {
			"Connect By Index",
			"Connect By Names"
		};
		chosen_connection_option = parse_among_options("Choose how to connect nodes",connect_options,N_CONNECT_DISCONNECT_OPTIONS,&canceled);
	}else if(connect_mode == DISCONNECT){
		const char * connect_options[N_CONNECT_DISCONNECT_OPTIONS] = {
			"Disconnect By Index",
			"Disconnect By Names"
		};
		chosen_connection_option = parse_among_options("Choose how to connect nodes",connect_options,N_CONNECT_DISCONNECT_OPTIONS,&canceled);
	}
	if(canceled) return;
	
	if(chosen_connection_option == 1){
		size_t first_node_index = parse_index_in_range("Index of First Node in Map",0,get_map_node_count(*map)-1,&canceled);
		if(canceled) return;
		
		size_t second_node_index = parse_index_in_range("Index of Second Node in Map",0,get_map_node_count(*map)-1,&canceled);
		if(canceled) return;
		
		if(connect_mode == CONNECT) connect_nodes_in_map_by_indices(map,first_node_index,second_node_index,chosen_edge_type,ctx);
		else if(connect_mode == DISCONNECT) disconnect_nodes_in_map_by_indices(map,first_node_index,second_node_index,ctx);
	}else if(chosen_connection_option == 2){
		fputs("First Node Name ",stdout);
		char * first_node_name = read_line();
		
		fputs("Second Node Name ",stdout);
		char * second_node_name = read_line();
		
		if(connect_mode == CONNECT) connect_nodes_in_map_by_names(map,first_node_name,second_node_name,chosen_edge_type,ctx);
		else if(connect_mode == DISCONNECT) disconnect_nodes_in_map_by_names(map,first_node_name,second_node_name,ctx);
		free(first_node_name);
		free(second_node_name);
	}
}

void connect_nodes_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	connect_disconnect_nodes_command(map,ctx,true);
}

void disconnect_nodes_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	connect_disconnect_nodes_command(map,ctx,false);
}

#define SET_EDGE_TYPE_N_OPTIONS 2
void set_edge_type_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	bool canceled = false;
	
	const char * options[SET_EDGE_TYPE_N_OPTIONS] = {
		"Seach By Node Index Pair",
		"Seach By Node Name Pair"
	};
	
	size_t chosen_option = parse_among_options("Chooose how to edit a connection type",options,SET_EDGE_TYPE_N_OPTIONS,&canceled);
	if(canceled) return;
	
	uint8_t chosen_edge_type = parse_edge_type(&canceled);
	if(canceled) return;
	
	if(chosen_option == 1){
		size_t first_node_index = parse_index_in_range("Index of First Node in Map",0,get_map_node_count(*map)-1,&canceled);
		if(canceled) return;
		
		size_t second_node_index = parse_index_in_range("Index of Second Node in Map",0,get_map_node_count(*map)-1,&canceled);
		if(canceled) return;
		
		set_connection_type_for_nodes_by_indices(map,first_node_index,second_node_index,chosen_edge_type,ctx);
	}else if(chosen_option == 2){
		fputs("First Node Name ",stdout);
		char * first_node_name = read_line();
		
		fputs("Second Node Name ",stdout);
		char * second_node_name = read_line();
		
		set_connection_type_for_nodes_by_name(map,first_node_name,second_node_name,chosen_edge_type,ctx);
		free(first_node_name);
		free(second_node_name);
	}
}

void load_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	fputs("(Load) File Name of Map ",stdout);
	char * file_name = read_line();
	
	deinit_map(map,ctx);
	*map = load_map_from_file(file_name,ctx);
	
	free(file_name);
}

void save_map_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	fputs("(Save) File Name of Map ",stdout);
	char * file_name = read_line();
	
	save_map_to_file(map,file_name);
	
	free(file_name);
}

void help_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	for(size_t i = 0;i < collection.n_commands;i++){
		phrase_command_to_output_stream(collection.commands[i],stdout,ctx);
	}
}

void clear_command(map_t * map,map_obj_arr_t * arr,command_collection_t collection,err_ctx_t * ctx){
	if(parse_confirmation("Are you Sure?")) clear_map_obj_arr(arr,ctx);
	else fputs("Not cleared.\n",stdout);
}