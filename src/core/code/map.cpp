#include "map.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

static void put_multitab(size_t n_tabs,FILE * stream){
	if(stream == NULL) return;
	for(size_t i = 0;i < n_tabs;i++) fputc('\t',stream);
}

cord_t create_cord(double lon,double lat){
	cord_t out;
	out.longitude = lon;
	out.latitude = lat;
	return out;
}

void cord_to_output_stream(cord_t cord,size_t tabs,FILE * stream){
	put_multitab(tabs,stream);
	fputs("Coordinate:\n",stream);
	
	put_multitab(tabs,stream);
	fputs("\tLongitude:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lf\n",cord.longitude);
	
	put_multitab(tabs,stream);
	fputs("\tLatitude:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lf\n",cord.latitude);
}

map_rect_t create_map_rect(cord_t bottom_left,cord_t top_right){
	map_rect_t out;
	out.bottom_left = bottom_left;
	out.top_right = top_right;
	return out;
}

void map_rect_to_output_stream(map_rect_t rect,size_t tabs,FILE * stream){
	put_multitab(tabs,stream);
	fputs("Map-Rect:\n",stream);
	
	put_multitab(tabs,stream);
	fputs("\tBottom left:\n",stream);
	cord_to_output_stream(rect.bottom_left,tabs+2,stream);
	
	put_multitab(tabs,stream);
	fputs("\tTop Right:\n",stream);
	cord_to_output_stream(rect.top_right,tabs+2,stream);
}

building_t * create_building(const char * primary_name,map_rect_t building_bounding_box,size_t n_floors){
	building_t * out = (building_t*) malloc(sizeof(building_t));
	
	out->n_floors = n_floors;
	out->possible_names_capacity = 0;
	out->n_possible_names = 0;
	out->possible_names = NULL;
	out->building_bounding_box = building_bounding_box;
	
	add_building_alias_name(out,primary_name);
	
	return out;
}

void delete_building(building_t * building){
	if(building == NULL) return;
	
	if(building->possible_names != NULL){
		for(size_t i = 0;i < building->n_possible_names;i++){
			free(building->possible_names[i]);
		}
		free(building->possible_names);
	}
	
	free(building);
}

#define DEFAULT_POSSIBLE_NAMES_CAPACITY 4

void add_building_alias_name(building_t * building,const char * alias_name){
	if(building == NULL || alias_name == NULL) return;//invalid parameters
	
	//ensure an array of strings exists
	if(building->possible_names == NULL){
		building->possible_names_capacity = DEFAULT_POSSIBLE_NAMES_CAPACITY;
		building->possible_names = (char**) malloc(sizeof(char*)*building->possible_names_capacity);
	}
	
	//resize strings array buffer if full
	if(building->n_possible_names == building->possible_names_capacity){
		building->possible_names_capacity *= 2;
		building->possible_names = (char**) realloc(building->possible_names,sizeof(char*)*building->possible_names_capacity);
	}
	
	size_t alias_length = strlen(alias_name);
	char * alias_string_cpy = (char*) malloc(alias_length+1);
	strcpy(alias_string_cpy,alias_name);
	
	//add new element to the strings array
	building->possible_names[building->n_possible_names] = alias_string_cpy;
	building->n_possible_names++;
}

void remove_building_alias_name(building_t * building,const char * alias_name){
	if(building == NULL || alias_name == NULL) return;//invalid parameters
	if(building->n_possible_names == 0) return;//array is empty
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < building->n_possible_names;i++){
		const char * current_alias = building->possible_names[i];
		
		if(strcmp(current_alias,alias_name) == 0){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found) return;
	
	//delete the string
	free(building->possible_names[matching_index]);
	
	//shift over data
	for(size_t i = matching_index;i < building->n_possible_names-1;i++){
		building->possible_names[i] = building->possible_names[i+1];
	}
	building->n_possible_names--;//shrink array
}

void change_primary_building_name(building_t * building,const char * primary_name){
	if(building == NULL || primary_name == NULL) return;//invalid parameters
	if(building->n_possible_names == 0) return;
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < building->n_possible_names;i++){
		const char * current_alias = building->possible_names[i];
		
		if(strcmp(current_alias,primary_name) == 0){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found) return;
	
	char * temp = building->possible_names[0];
	building->possible_names[0] = building->possible_names[matching_index];
	building->possible_names[matching_index] = temp;
}

void set_building_floor_count(building_t * building,size_t new_floor_count){
	if(building == NULL) return;
	building->n_floors = new_floor_count;
}

void set_building_bounding_box(building_t * building,map_rect_t building_bounding_box){
	if(building == NULL) return;
	
	building->building_bounding_box = building_bounding_box;
}

const char * get_primary_building_name(const building_t * building){
	if(building == NULL) return NULL;
	if(building->n_possible_names == 0) return NULL;
	
	return building->possible_names[0];
}

void building_to_output_stream(const building_t * building,size_t tabs,FILE * stream){
	if(building == NULL || stream == NULL) return;
	
	put_multitab(tabs,stream);
	fprintf(stream,"Building %p:\n",building);
	
	if(building->n_possible_names > 0){
		put_multitab(tabs,stream);
		fputs("\tPrimary Name:\n",stream);
		put_multitab(tabs,stream);
		fputs("\t\t",stream);
		fputs(building->possible_names[0],stream);
		fputc('\n',stream);
	}
	
	put_multitab(tabs,stream);
	fputs("\tN floors:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%d\n",building->n_floors);
	
	if(building->n_possible_names > 1){
		put_multitab(tabs,stream);
		fputs("\tAlias Names:\n",stream);
		for(size_t i = 1;i < building->n_possible_names;i++){
			put_multitab(tabs,stream);
			fputs("\t\t",stream);
			fputs(building->possible_names[i],stream);
			fputc('\n',stream);
		}
	}
	
	put_multitab(tabs,stream);
	fputs("\tBounding Box:\n",stream);
	map_rect_to_output_stream(building->building_bounding_box,tabs+2,stream);
}

map_node_t * create_map_node(cord_t coordinate) {
	map_node_t * output = (map_node_t *) malloc(sizeof(map_node_t));
	
	output->coordinate = coordinate;
	output->picture_file_path = NULL;
	output->name = NULL;
	output->outgoing_edges = NULL;
	output->n_outgoing_edges = 0;
	output->outgoing_edges_capacity = 0;
	output->selectable = false;
	output->floor_number = NODE_FLOOR_NUMBER_NONE;
	output->associated_building = NULL;
	output->cost_temp = 0.0;
	output->index_temp = 0;
	
	return output;
}

void delete_map_node(map_node_t * node){
	if(node == NULL) return;
	if(node->picture_file_path != NULL) {
		free(node->picture_file_path);
	}
	if(node->name != NULL){
		free(node->name);
	}
	free(node->outgoing_edges);
	free(node);
}

void set_map_node_cord(map_node_t * node,cord_t new_cord){
	if(node == NULL) return;
	
	node->coordinate = new_cord;
}

void set_map_node_name(map_node_t * node,const char * name){
	if(node == NULL) return;
	
	if(node->name != NULL) free(node->name);
	
	size_t name_length = strlen(name);
	char * name_cpy = (char*) malloc(name_length+1);
	strcpy(name_cpy,name);
	
	node->name = name_cpy;
}

void set_map_node_picture(map_node_t * node,const char * file_path){
	if(node == NULL) return;
	
	if(node->picture_file_path != NULL) free(node->picture_file_path);
	
	size_t path_length = strlen(file_path);
	char * path_cpy = (char*) malloc(path_length+1);
	strcpy(path_cpy,file_path);
	
	node->picture_file_path = path_cpy;
}

void set_map_node_floor_number(map_node_t * node,int8_t floor_number){
	if(node == NULL) return;
	
	node->floor_number = floor_number;
}

void clear_map_node_floor_number(map_node_t * node){
	if(node == NULL) return;
	
	node->floor_number = NODE_FLOOR_NUMBER_NONE;
}

void set_map_node_selectable(map_node_t * node,bool selectable){
	if(node == NULL) return;
	
	node->selectable = selectable;
}

void set_map_node_building(map_node_t * node,building_t * building){
	if(node == NULL || building == NULL) return;
	
	node->associated_building = building;
}

void clear_map_node_building(map_node_t * node){
	if(node == NULL) return;
	
	node->associated_building = NULL;
}

#define DEFAULT_OUTGOING_EDGES_CAPACITY 2

static void add_outgoing_edge_to_node(map_node_t * node,map_edge_t * edge){
	if(node == NULL || edge == NULL) return;
	
	if(node->outgoing_edges == NULL){
		node->outgoing_edges_capacity = DEFAULT_OUTGOING_EDGES_CAPACITY;
		node->outgoing_edges = (map_edge_t**) malloc(sizeof(map_edge_t*)*node->outgoing_edges_capacity);
	}
	
	if(node->outgoing_edges_capacity == node->n_outgoing_edges){
		node->outgoing_edges_capacity *= 2;
		node->outgoing_edges = (map_edge_t**) realloc(node->outgoing_edges,sizeof(map_edge_t*)*node->outgoing_edges_capacity);
	}
	
	node->outgoing_edges[node->n_outgoing_edges] = edge;
	node->n_outgoing_edges++;
}

static void remove_outgoing_edge_from_node(map_node_t * node,map_edge_t * edge){
	if(node == NULL || edge == NULL) return;
	if(node->n_outgoing_edges == 0) return;
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < node->n_outgoing_edges;i++){
		map_edge_t * current_edge = node->outgoing_edges[i];
		
		if(current_edge == edge){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found) return;
	
	//shift over data
	for(size_t i = matching_index;i < node->n_outgoing_edges-1;i++){
		node->outgoing_edges[i] = node->outgoing_edges[i+1];
	}
	node->n_outgoing_edges--;//shrink array
}

void map_node_to_output_stream(const map_node_t * node,size_t tabs,FILE * stream){
	if(node == NULL || stream == NULL) return;
	
	put_multitab(tabs,stream);
	fprintf(stream,"Map-Node %p:\n",node);
	cord_to_output_stream(node->coordinate,tabs+1,stream);
	
	if(node->name != NULL){
		put_multitab(tabs,stream);
		fputs("\tName:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%s\n",node->name);
	}
	
	if(node->picture_file_path != NULL){
		put_multitab(tabs,stream);
		fputs("\tPicture File:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%s\n",node->picture_file_path);
	}
	
	if(node->floor_number != NODE_FLOOR_NUMBER_NONE){
		put_multitab(tabs,stream);
		fputs("\tFloor Number:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%u\n",node->floor_number);
	}
	
	put_multitab(tabs,stream);
	fputs("\tIs Selectable:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%s\n",node->selectable ? "True":"False");
	
	if(node->associated_building != NULL){
		put_multitab(tabs,stream);
		fputs("\tAssociated Building:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%s\n",get_primary_building_name(node->associated_building));
	}
	
	if(node->n_outgoing_edges > 0){
		put_multitab(tabs,stream);
		fputs("\tN outgoing edges:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lu\n",node->n_outgoing_edges);
		put_multitab(tabs,stream);
		fputs("\tOutgoing edges:\n",stream);
		for(size_t i = 0;i < node->n_outgoing_edges;i++){
			put_multitab(tabs,stream);
			fprintf(stream,"\t\t%p\n",node->outgoing_edges[i]);
		}
	}
}

map_edge_t * create_map_edge(uint8_t type,map_node_t * a,map_node_t * b) {
	map_edge_t * output = (map_edge_t *)malloc(sizeof(map_edge_t));
	output->a = a;
	output->b = b;
	output->type = type;
	
	add_outgoing_edge_to_node(a,output);
	add_outgoing_edge_to_node(b,output);
	
	return output;
}

void delete_map_edge(map_edge_t * edge){
	if(edge == NULL) return;
	free(edge);
}

void set_map_edge_type(map_edge_t * edge,uint8_t type){
	if(edge == NULL) return;
	
	edge->type = type;
}

void map_edge_to_output_stream(const map_edge_t * edge,size_t tabs,FILE * stream){
	if(edge == NULL || stream == NULL) return;
	
	put_multitab(tabs,stream);
	fprintf(stream,"Edge %p:\n",edge);
	
	put_multitab(tabs,stream);
	fputs("\tType:\n",stream);
	put_multitab(tabs,stream);
	fputs("\t\t",stream);
	if(edge->type == EDGE_TYPE_SIDEWALK){
		fputs("sidewalk",stream);
	}else if(edge->type == EDGE_TYPE_ROAD){
		fputs("road",stream);
	}else if(edge->type == EDGE_TYPE_STAIRS){
		fputs("stairs",stream);
	}else if(edge->type == EDGE_TYPE_RAMP){
		fputs("ramp",stream);
	}else if(edge->type == EDGE_TYPE_HALLWAY){
		fputs("hallway",stream);
	}else if(edge->type == EDGE_TYPE_ELEVATOR_SHAFT){
		fputs("elevator shaft",stream);
	}else if(edge->type == EDGE_TYPE_OVERPASS){
		fputs("overpass",stream);
	}else if(edge->type == EDGE_TYPE_DOOR){
		fputs("door",stream);
	}else if(edge->type == EDGE_TYPE_AUTO_DOOR){
		fputs("automatic door",stream);
	}else if(edge->type == EDGE_TYPE_CROSSWALK){
		fputs("crosswalk",stream);
	}
	fputc('\n',stream);
	
	put_multitab(tabs,stream);
	fputs("\tNode-A:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%p\n",edge->a);
	
	put_multitab(tabs,stream);
	fputs("\tNode-B:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%p\n",edge->b);
}

mpo_t * create_mpo(const cord_t * cord_arry, size_t n_cords, uint8_t type){
	mpo_t * output =  (mpo_t*)malloc(sizeof(mpo_t));
	output->cords = (cord_t*)malloc(sizeof(cord_t)*n_cords);
	output->n_cords = n_cords;
	for(size_t i =0; i<n_cords; i++) {
		output->cords[i] = cord_arry[i];
	}
	output->type = type;
	return output;
}

void delete_map_mpo(mpo_t * mpo_ref){
	if(mpo_ref == NULL) return;
	free(mpo_ref->cords);
	free(mpo_ref);
}

void mpo_to_output_stream(const mpo_t * mpo,size_t tabs,FILE * stream){
	if(mpo == NULL || stream == NULL) return;
	
	put_multitab(tabs,stream);
	fprintf(stream,"Map-Polygon-Object %p:\n",mpo);
	
	put_multitab(tabs,stream);
	fputs("\tType:\n",stream);
	put_multitab(tabs,stream);
	fputs("\t\t",stream);
	if(mpo->type == MPO_TYPE_WATER){
		fputs("water",stream);
	}else if(mpo->type == MPO_TYPE_TREE){
		fputs("tree",stream);
	}else if(mpo->type == MPO_TYPE_BUILDING){
		fputs("building",stream);
	}
	fputc('\n',stream);
	
	put_multitab(tabs,stream);
	fputs("\tN Coordinates:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lu\n",mpo->n_cords);
	
	put_multitab(tabs,stream);
	fputs("\tCoordinate Array:\n",stream);
	for(size_t i = 0;i < mpo->n_cords;i++){
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lu\n",i);
		cord_to_output_stream(mpo->cords[i],tabs+2,stream);
	}
}

void set_mpo_type(mpo_t * mpo,uint8_t new_type){
	if(mpo == NULL) return;
	
	mpo->type = new_type;
}

void set_mpo_cord(mpo_t * mpo,size_t index,cord_t new_cord){
	if(mpo == NULL) return;
	if(!(index < mpo->n_cords)) return;//out of bounds
	
	mpo->cords[index] = new_cord;
}

void do_thing(){
}

map_t init_map(void){
	map_t map;
	map.node_capacity = 0;
	//makes an array of size node_capacity
	map.all_nodes= NULL;
	map.n_nodes = 0;
	map.edge_capacity = 0;
	map.all_edges = NULL;
	map.n_edges = 0;
	map.mpo_capacity = 0;
	map.mpos = NULL;
	map.n_mpos = 0;
	map.active_start = NULL;
	map.active_end = NULL;
	map.active_path = NULL;
	map.active_edge_cost_function = NULL;
	return map;
}

void clear_map(map_t * map_ref){
	if(map_ref->all_nodes != NULL) {
		for(size_t i=0; i < map_ref->n_nodes; i++) {
			delete_map_node(map_ref->all_nodes[i]);
		}
		free(map_ref->all_nodes);
	}

	if(map_ref->all_edges != NULL) {
		for(size_t i=0; i < map_ref->n_edges; i++) {
			delete_map_edge(map_ref->all_edges[i]);
		}
		free(map_ref->all_edges);
	}
	if(map_ref->mpos != NULL) {
		for(size_t i=0; i < map_ref->n_mpos; i++) {
			delete_map_mpo(map_ref->mpos[i]);
		}
		free(map_ref->mpos);
	}
	if(map_ref->active_path != NULL) {
		delete_map_path(map_ref->active_path);
	}
}

void delete_map_path(map_path_t * map_path_ref) {
	if(map_path_ref == NULL) return;
	free(map_path_ref->nodes);
	free(map_path_ref->name);
}

map_path_t * copy_map_path(const map_path_t * map_path_ref){
	map_path_t * out = (map_path_t*) malloc(sizeof(map_path_t));

	if(map_path_ref == NULL){
		out->nodes = NULL;
		out->n_nodes = 0;
		out->name = NULL;
		return out;
	}

	if(map_path_ref->nodes != NULL){
		out->nodes = (map_node_t**) malloc(sizeof(map_node_t*)*map_path_ref->n_nodes);
		for(size_t i = 0;i < map_path_ref->n_nodes;i++){
			out->nodes[i] = map_path_ref->nodes[i];
		}
		out->n_nodes = map_path_ref->n_nodes;
	}else{
		out->nodes = NULL;
		out->n_nodes = 0;
	}

	if(map_path_ref->name != NULL){
		size_t ref_name_length = strlen(map_path_ref->name);
		char * new_string = (char*) malloc(ref_name_length+1);
		strcpy(new_string,map_path_ref->name);

		out->name = new_string;
	}else{
		out->name = NULL;
	}

	return out;
}

saved_paths_t init_saved_paths(){
	saved_paths_t out;

	out.paths = NULL;
	out.n_paths = 0;
	out.paths_capacity = 0;

	return out;
}

void clear_saved_paths(saved_paths_t * saved_paths){
	if(saved_paths == NULL) return;

	if(saved_paths->paths != NULL){
		for(size_t i = 0;i < saved_paths->n_paths;i++){
			delete_map_path(saved_paths->paths[i]);
		}
		free(saved_paths->paths);
	}
}
/*
 * returns a number between 0 and 1 which corresponds to how similar to tokens are
 * 0.0 means the two tokens are not similar
 * 1.0 menas the strings are identical
 */
static float token_similarity_score(const char * a,const char * b){
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);
	
	
	if(a_len > b_len) return token_similarity_score(b,a);
	float score = 0;
	
	if(strcmp(a,b) == 0) return 1.0f;
	
	bool * used = (bool*) malloc(sizeof(bool)*b_len);
	for(size_t i = 0;i < b_len;i++) used[i] = false;
	
	for(size_t i = 0;i < b_len;i++){
		float this_loop_correctness = 0.0f;
		bool last_correct = false;
		for(size_t j = 0;j < a_len;j++){
			size_t first_index = i+j;
			size_t second_index = j;
			
			char b_char;
			if(first_index < b_len){
				b_char = b[first_index];
			}else{
				b_char = '\0';
			}
			char a_char = a[second_index];
			
			//printf("%c %c\n",a_char,b_char);
			
			if(a_char == b_char){
				if(!used[first_index]) this_loop_correctness += last_correct ? 1.0f : 0.5f;
				used[first_index] = true;
				last_correct = true;
			}else{
				last_correct = false;
			}
			
		}
		//printf("\n");
		
		if(this_loop_correctness <= 2.0f) continue;
		float correctness_percentage = this_loop_correctness/a_len;
		score += correctness_percentage;
	}
	free(used);
	
	return score*((float)a_len)/((float)b_len);
}

/*
 * Counts the number of tokens in a string seperated by spaces or tabs
 */
static size_t count_tokens(const char * input){
	size_t token_count = 0;
	bool last_was_space = true;
	for(size_t i = 0; input[i] != '\0';i++){
		char c = input[i];
		bool space = isspace(c);
		
		if(!space && last_was_space){
			token_count++;
		}
		last_was_space = space;
	}
	
	return token_count;
}

/*
 * convert a c-string to lowercase
 */
static void c_str_lowercase(char * str){
	size_t str_len = strlen(str);
	for(size_t i = 0;i < str_len;i++){
		str[i] = tolower(str[i]);
	}
}

/*
 * Returns a new c-string on the heap which is the first token in the input string
 * It also converts the string to lower case
 */
static char * get_first_token(const char * input){
	size_t index_of_first_space;
	size_t index = 0;
	while(!(input[index] == '\0' || isspace(input[index]))){
		index++;
	}
	index_of_first_space = index;
	char * output = (char*) malloc(index_of_first_space+1);
	memcpy(output,input,index_of_first_space);
	output[index_of_first_space] = '\0';
	c_str_lowercase(output);
	return output;
}

/*
 * Splits an input string into its tokens seperated by spaces or tabs
 */
static char ** split_into_tokens(const char * input,size_t * n_tokens_out){
	size_t token_count = count_tokens(input);
	*n_tokens_out = token_count;
	
	char ** tokens = (char**)malloc(sizeof(char*)*token_count);
	
	bool last_was_space = true;
	size_t at = 0;
	for(size_t i = 0; input[i] != '\0';i++){
		char c = input[i];
		bool space = isspace(c);
		
		if(!space && last_was_space){
			tokens[at] = get_first_token(input+i);
			at++;
		}
		last_was_space = space;
	}
	
	return tokens;
}

/*
 * Delete tokens from the heap
 */
static void delete_tokens(char ** tokens,size_t n_tokens){
	for(size_t i = 0;i < n_tokens;i++){
		free(tokens[i]);
	}
	free(tokens);
}

/*
 * Compare phrases by trying to re-order tokens
 */
float phrase_similarity_score(const char * phrase_1,const char * phrase_2){
	size_t n_phrase_1_tokens;
	char ** phrase_1_tokens = split_into_tokens(phrase_1,&n_phrase_1_tokens);
	
	size_t n_phrase_2_tokens;
	char ** phrase_2_tokens = split_into_tokens(phrase_2,&n_phrase_2_tokens);
	
	float score = 0.0f;
	
	for(size_t i = 0;i < n_phrase_1_tokens;i++){
		for(size_t j = 0;j < n_phrase_2_tokens;j++){
			char * phrase_1_token = phrase_1_tokens[i];
			char * phrase_2_token = phrase_2_tokens[j];
			score += token_similarity_score(phrase_1_token,phrase_2_token);
		}
	}
	
	delete_tokens(phrase_2_tokens,n_phrase_2_tokens);
	delete_tokens(phrase_1_tokens,n_phrase_1_tokens);
	
	return score;
}

/*
 * convert a map polygon object into a stream of bytes
 */
static uint8_t * convert_mpo_to_binary(const mpo_t * mpo,size_t * buffer_size){
	*buffer_size = sizeof(uint8_t)+sizeof(size_t)+sizeof(cord_t)*mpo->n_cords;
	uint8_t * buffer = (uint8_t *) malloc(*buffer_size);
	
	size_t current_offset = 0;
	
	memcpy(buffer+current_offset,&(mpo->type),sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(buffer+current_offset,&(mpo->n_cords),sizeof(size_t));
	current_offset += sizeof(size_t);
	
	memcpy(buffer+current_offset,mpo->cords,sizeof(cord_t)*mpo->n_cords);
	
	return buffer;
}

/*
 * convert a stream of bytes into a map polygon object
 */
static mpo_t * convert_binary_to_mpo(const uint8_t * buffer){
	mpo_t * out = (mpo_t*) malloc(sizeof(mpo_t));
	
	size_t current_offset = 0;
	
	memcpy(&(out->type),buffer+current_offset,sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(&(out->n_cords),buffer+current_offset,sizeof(size_t));
	current_offset += sizeof(size_t);
	
	out->cords = (cord_t*) malloc(sizeof(cord_t)*out->n_cords);
	
	memcpy(out->cords,buffer+current_offset,sizeof(cord_t)*out->n_cords);
	
	return out;
}


/*
 * get index of node
 */
size_t get_node_index(map_node_t * node,map_node_t ** all_nodes){
	return ((size_t)node-((size_t)&(all_nodes[0])))/sizeof(map_node_t);
}

/*
 * convert a map edge object into a stream of bytes
 */
static uint8_t * convert_map_edge_to_binary(const map_edge_t * edge,map_node_t ** all_nodes,size_t * buffer_size){
	*buffer_size = sizeof(uint8_t)+sizeof(size_t)+sizeof(size_t);
	uint8_t * buffer = (uint8_t *) malloc(*buffer_size);
	
	size_t current_offset = 0;
	
	memcpy(buffer+current_offset,&(edge->type),sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	size_t node_a_index = get_node_index(edge->a,all_nodes);
	memcpy(buffer+current_offset,&(node_a_index),sizeof(size_t));
	current_offset += sizeof(size_t);
	
	size_t node_b_index = get_node_index(edge->b,all_nodes);
	memcpy(buffer+current_offset,&(node_b_index),sizeof(size_t));
	
	return buffer;
}

/*
 * convert a stream of bytes into an edge object
 */
static map_edge_t * convert_binary_to_map_edge(const uint8_t * buffer,map_node_t ** all_nodes){
	map_edge_t * out = (map_edge_t*) malloc(sizeof(map_edge_t));
	
	size_t current_offset = 0;
	
	memcpy(&(out->type),buffer+current_offset,sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	size_t index;
	memcpy(&(index),buffer+current_offset,sizeof(size_t));
	out->a = all_nodes[index];
	current_offset += sizeof(size_t);
	
	memcpy(&(index),buffer+current_offset,sizeof(size_t));
	out->b = all_nodes[index];
	
	return out;
}

/*
 * convert a map node object into a stream of bytes
 */
/*
static uint8_t * convert_map_node_to_binary(const map_node_t * node,size_t * buffer_size){
	size_t total_alias_name_length = 0;
	for(size_t i = 0;i < node->n_possible_names;i++){
		char * string = node->possible_names[i];
		size_t string_len = strlen(string);
		total_alias_name_length += string_len+1;
	}
	
	size_t picture_file_path_len = 0;
	if(node->picture_file_path != NULL) picture_file_path_len = strlen(node->picture_file_path);
	
	*buffer_size = 
		sizeof(uint8_t) + //type
		sizeof(cord_t) + //coordinate
		sizeof(size_t) + //n_possible_names
		total_alias_name_length + //array of strings
		picture_file_path_len+1 + //picture file path name
		sizeof(size_t);//n_outgoing_edges
	
	uint8_t * buffer = (uint8_t *) malloc(*buffer_size);
	
	size_t current_offset = 0;
	
	memcpy(buffer+current_offset,&(node->type),sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(buffer+current_offset,&(node->coordinate),sizeof(cord_t));
	current_offset += sizeof(cord_t);
	
	memcpy(buffer+current_offset,&(node->n_possible_names),sizeof(size_t));
	current_offset += sizeof(size_t);
	
	
	for(size_t i = 0;i < node->n_possible_names;i++){
		char * string = node->possible_names[i];
		size_t string_len = strlen(string);
		memcpy(buffer+current_offset,string,string_len+1);
		current_offset += string_len+1;
	}
	
	if(node->picture_file_path != NULL){
		memcpy(buffer+current_offset,node->picture_file_path,picture_file_path_len+1);
		current_offset += picture_file_path_len+1;
	}else{
		memcpy(buffer+current_offset,"\0",1);
		current_offset++;
	}
	
	memcpy(buffer+current_offset,&(node->n_outgoing_edges),sizeof(size_t));
	
	return buffer;
}
*/

/*
 * convert a stream of bytes into an node object
 */
/*
static map_node_t * convert_binary_to_map_node(const uint8_t * buffer){
	map_node_t * out = (map_node_t*) malloc(sizeof(map_node_t));
	
	size_t current_offset = 0;
	
	memcpy(&(out->type),buffer+current_offset,sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(&(out->coordinate),buffer+current_offset,sizeof(cord_t));
	current_offset += sizeof(cord_t);
	
	memcpy(&(out->n_possible_names),buffer+current_offset,sizeof(size_t));
	current_offset += sizeof(size_t);
	
	out->possible_names = (char**) malloc(sizeof(char*)*out->n_possible_names);
	
	size_t next_null_char_index;
	for(size_t i = 0; i < out->n_possible_names;i++){
		next_null_char_index = 0;
		while((char)buffer[current_offset+next_null_char_index] != '\0'){
			next_null_char_index++;
		}
		char * possible_name = (char*) malloc(next_null_char_index+1);
		memcpy(possible_name,buffer+current_offset,next_null_char_index+1);
		current_offset+=next_null_char_index+1;
		out->possible_names[i] = possible_name;
	}
	next_null_char_index = 0;
	while((char)buffer[current_offset+next_null_char_index] != '\0'){
		next_null_char_index++;
	}
	if(next_null_char_index == 0){
		out->picture_file_path = NULL;
		current_offset++;
	}else{
		out->picture_file_path = (char*) malloc(next_null_char_index+1);
		memcpy(out->picture_file_path,buffer+current_offset,next_null_char_index+1);
		current_offset += next_null_char_index+1;
	}
	
	memcpy(&(out->n_outgoing_edges),buffer+current_offset,sizeof(size_t));
	
	out->outgoing_edges = (map_edge_t**) malloc(sizeof(map_edge_t*)*out->n_outgoing_edges);
	for(size_t i = 0;i < out->n_outgoing_edges;i++) out->outgoing_edges[i] = NULL;
	
	return out;
}
*/

void file_save_test(){
	FILE *write_ptr;
	
	mpo_t mpo_test;
	mpo_test.type = MPO_TYPE_TREE;
	mpo_test.n_cords = 4;
	mpo_test.cords = (cord_t*) malloc(sizeof(cord_t)*mpo_test.n_cords);
	mpo_test.cords[0] = {0.0,0.0};
	mpo_test.cords[1] = {0.0,1.0};
	mpo_test.cords[2] = {1.0,1.0};
	mpo_test.cords[3] = {1.0,0.0};
	
	size_t buffer_size;
	uint8_t * buffer = convert_mpo_to_binary(&mpo_test,&buffer_size);
	
	write_ptr = fopen("test.bin","wb");  // w for write, b for binary

	fwrite(buffer,buffer_size,1,write_ptr); // write 10 bytes from our buffer
	
	free(mpo_test.cords);
	
	fclose(write_ptr);
}

void file_open_test(){
	FILE *read_ptr;
	
	mpo_t * mpo_test;
	
	read_ptr = fopen("test.bin","rb");

	uint8_t buffer[256];
	for(int i = 0;i < 256;i++) buffer[i] = 0;
	
	size_t file_size;
	
	fseek(read_ptr, 0, SEEK_END); // seek to end of file
	file_size = ftell(read_ptr); // get current file pointer
	fseek(read_ptr, 0, SEEK_SET);
	
	fread(buffer,file_size,1,read_ptr);
	
	mpo_test = convert_binary_to_mpo(buffer);
	
	/*
	printf("%d %ld\n",mpo_test->type,mpo_test->n_cords);
	for(size_t i = 0;i < mpo_test->n_cords;i++){
		printf("(%lf %lf),\n",mpo_test->cords[i].longitude,mpo_test->cords[i].latitude);
	}
	*/
	//mpo_to_string(mpo_test, stdout);
	
	fclose(read_ptr);
}

void best_match_test(){
	char input[64];
	const char * locations[6] = {
		"Information Technology Computing ITE",
		"Library",
		"Engineering ENG",
		"Interdisciplinary Life Sciences ILS",
		"Mathematics MATH",
		"Janet and Walter Sondheim"
	};

	input[0] = '\0';
	scanf(" %[^\n]",input);
	const char * best = NULL;
	
	float max = 0.0f;
	for(size_t i = 0;i < 6;i++){
		const char * loc = locations[i];
	float current_score = phrase_similarity_score(input,loc);
		if(current_score > max){
			max = current_score;
			best = loc;
		}
	}
	
	printf("Best Guess is : %s\n",best);
}

/*
map_node_t ** filter_locations(const char * location_name,const map_t * map_ref,size_t max_results){
	
}
*/
