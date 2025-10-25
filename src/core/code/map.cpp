#include "map.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>


err_ctx_t create_err_ctx(){
	err_ctx_t out;
	out.flags = 0;
	return out;
}

void reset_err_ctx(err_ctx_t * ctx){
	ctx->flags = 0;
}

void errs_to_output_stream(const err_ctx_t * ctx,FILE * stream){
	if(ctx->flags == 0){
		fputs("No errors encountered!\n",stream);
		return;
	}
	
	fputs("Errors Collected:\n",stream);
	if((ctx->flags & ERROR_INVALID_PARAM) != 0){
		fputs("\tError: Invalid parameter!\n",stream);
	}else if((ctx->flags & ERROR_DUPLICATE_PARAMETER) != 0){
		fputs("\tError: Duplicate parameters received!\n",stream);
	}else if((ctx->flags & ERROR_OUT_OF_BOUNDS_INDEX) != 0){
		fputs("\tError: Index passed is out of bounds!\n",stream);
	}else if((ctx->flags & ERROR_OBJECT_NOT_FOUND) != 0){
		fputs("\tError: Object not found!\n",stream);
	}
}

bool err_encountered(const err_ctx_t * ctx){
	return ctx->flags != 0;
}

//MEMORY PARAMETERS

#define DEFAULT_POSSIBLE_NAMES_CAPACITY 1
#define DEFAULT_OUTGOING_EDGES_CAPACITY 1
#define DEFAULT_BUILDINGS_CAPACITY 1
#define DEFAULT_NODES_CAPACITY 1
#define DEFAULT_EDGES_CAPACITY 1
#define DEFAULT_MPO_CAPACITY 1

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

building_t * create_building(const char * primary_name,map_rect_t building_bounding_box,size_t n_floors,err_ctx_t * ctx){
	if(primary_name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	building_t * out = (building_t*) malloc(sizeof(building_t));
	
	out->n_floors = n_floors;
	out->possible_names_capacity = 0;
	out->n_possible_names = 0;
	out->possible_names = NULL;
	out->building_bounding_box = building_bounding_box;
	
	add_building_alias_name(out,primary_name,ctx);
	
	return out;
}

void delete_building(building_t * building,err_ctx_t * ctx){
	if(building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(building->possible_names != NULL){
		for(size_t i = 0;i < building->n_possible_names;i++){
			free(building->possible_names[i]);
		}
		free(building->possible_names);
	}
	
	free(building);
}

void add_building_alias_name(building_t * building,const char * alias_name,err_ctx_t * ctx){
	if(building == NULL || alias_name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;//invalid parameters
	}
	
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

void remove_building_alias_name(building_t * building,const char * alias_name,err_ctx_t * ctx){
	if(building == NULL || alias_name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;//invalid parameters
	}
	if(building->n_possible_names == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;//array is empty
	}
	
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
	
	if(!found){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	//delete the string
	free(building->possible_names[matching_index]);
	
	//shift over data
	for(size_t i = matching_index;i < building->n_possible_names-1;i++){
		building->possible_names[i] = building->possible_names[i+1];
	}
	building->n_possible_names--;//shrink array
}

void change_primary_building_name(building_t * building,const char * primary_name,err_ctx_t * ctx){
	if(building == NULL || primary_name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;//invalid parameters
	}
	if(building->n_possible_names == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
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
	
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	char * temp = building->possible_names[0];
	building->possible_names[0] = building->possible_names[matching_index];
	building->possible_names[matching_index] = temp;
}

void set_building_floor_count(building_t * building,size_t new_floor_count,err_ctx_t * ctx){
	if(building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	building->n_floors = new_floor_count;
}

void set_building_bounding_box(building_t * building,map_rect_t building_bounding_box,err_ctx_t * ctx){
	if(building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	building->building_bounding_box = building_bounding_box;
}

const char * get_primary_building_name(const building_t * building,err_ctx_t * ctx){
	if(building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	if(building->n_possible_names == 0) return NULL;//I dont throw error here because this is perfectly valid possibility
	return building->possible_names[0];
}

void building_to_output_stream(const building_t * building,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(building == NULL || stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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
	output->previous = NULL;
	
	return output;
}

void delete_map_node(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
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

void clear_map_node_name(map_node_t * node){
	if(node == NULL) return;
	if(node->name == NULL) return;
	
	free(node->name);
	node->name = NULL;
}

void set_map_node_picture(map_node_t * node,const char * file_path){
	if(node == NULL) return;
	
	if(node->picture_file_path != NULL) free(node->picture_file_path);
	
	size_t path_length = strlen(file_path);
	char * path_cpy = (char*) malloc(path_length+1);
	strcpy(path_cpy,file_path);
	
	node->picture_file_path = path_cpy;
}

void clear_map_node_picture(map_node_t * node){
	if(node == NULL) return;
	if(node->picture_file_path == NULL) return;
	
	free(node->picture_file_path);
	node->picture_file_path = NULL;
}

bool node_adjacent_to_auto_door(map_node_t * node){
	if(node == NULL) return false;
	if(node->n_outgoing_edges == 0) return false;
	
	for(size_t i = 0;i < node->n_outgoing_edges;i++){
		map_edge_t * connected_edge = node->outgoing_edges[i];
		
		if(connected_edge->type == EDGE_TYPE_AUTO_DOOR){
			return true;
		}
	}
	
	return false;
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

static void remove_outgoing_edge_from_node_by_index(map_node_t * node,size_t index){
	if(node == NULL) return;
	if(!(index < node->n_outgoing_edges)) return;
	
	//shift over data
	for(size_t i = index;i < node->n_outgoing_edges-1;i++){
		node->outgoing_edges[i] = node->outgoing_edges[i+1];
	}
	node->n_outgoing_edges--;//shrink array
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
	
	remove_outgoing_edge_from_node_by_index(node,matching_index);
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
		fprintf(stream,"\t\t%s\n",get_primary_building_name(node->associated_building,NULL));//TODO add error context
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
	fputs("\tNode 1:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%p %s\n",edge->a,(edge->a->name == NULL) ? "" : edge->a->name);
	
	put_multitab(tabs,stream);
	fputs("\tNode 2:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%p %s\n",edge->b,(edge->b->name == NULL) ? "" : edge->b->name);
}

mpo_t * create_mpo(const cord_t * cord_arry, size_t n_cords, uint8_t type,err_ctx_t * ctx){
	if(cord_arry == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	mpo_t * output =  (mpo_t*)malloc(sizeof(mpo_t));
	output->cords = (cord_t*)malloc(sizeof(cord_t)*n_cords);
	output->n_cords = n_cords;
	for(size_t i =0; i<n_cords; i++) {
		output->cords[i] = cord_arry[i];
	}
	output->type = type;
	output->name = NULL;
	
	return output;
}

void set_mpo_name(mpo_t * mpo,const char * name,err_ctx_t * ctx){
	if(mpo == NULL || name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(mpo->name != NULL) free(mpo->name);
	
	size_t name_length = strlen(name);
	char * name_cpy = (char*) malloc(name_length+1);
	strcpy(name_cpy,name);
	
	mpo->name = name_cpy;
}

const char * get_mpo_name(const mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	return mpo->name;
}

void clear_mpo_name(mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(mpo->name == NULL) return;//dont throw error here because perfectly valid case
	
	free(mpo->name);
	mpo->name = NULL;
}

void delete_mpo(mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(mpo->name != NULL) free(mpo->name);
	
	free(mpo->cords);
	free(mpo);
}

void mpo_to_output_stream(const mpo_t * mpo,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(mpo == NULL || stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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
	
	if(mpo->name != NULL){
		put_multitab(tabs,stream);
		fputs("\tName:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%s\n",mpo->name);
	}
	
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

void set_mpo_type(mpo_t * mpo,uint8_t new_type,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	mpo->type = new_type;
}

void set_mpo_cord(mpo_t * mpo,size_t index,cord_t new_cord,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(!(index < mpo->n_cords)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	
	mpo->cords[index] = new_cord;
}

map_t init_map(void){
	map_t map;
	map.all_nodes= NULL;
	map.n_nodes = 0;
	map.node_capacity = 0;
	
	map.all_edges = NULL;
	map.n_edges = 0;
	map.edge_capacity = 0;
	
	map.all_buildings = NULL;
	map.n_buildings = 0;
	map.buildings_capacity = 0;
	
	map.all_mpos = NULL;
	map.n_mpos = 0;
	map.mpo_capacity = 0;
	
	return map;
}

void clear_map(map_t * map,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(map->all_nodes != NULL) {
		for(size_t i = 0; i < map->n_nodes;i++) {
			delete_map_node(map->all_nodes[i],ctx);
		}
		free(map->all_nodes);
	}

	if(map->all_edges != NULL) {
		for(size_t i = 0; i < map->n_edges;i++) {
			delete_map_edge(map->all_edges[i]);
		}
		free(map->all_edges);
	}
	
	if(map->all_buildings != NULL){
		for(size_t i = 0;i < map->n_buildings;i++){
			delete_building(map->all_buildings[i],ctx);
		}
		free(map->all_buildings);
	}
	
	if(map->all_mpos != NULL) {
		for(size_t i = 0; i < map->n_mpos;i++) {
			delete_mpo(map->all_mpos[i],ctx);
		}
		free(map->all_mpos);
	}
}

void add_building_to_map(map_t * map,building_t * building){
	if(map == NULL || building == NULL) return;
	
	if(map->all_buildings == NULL){
		map->buildings_capacity = DEFAULT_BUILDINGS_CAPACITY;
		map->all_buildings = (building_t**) malloc(sizeof(building_t*)*map->buildings_capacity);
	}
	
	if(map->buildings_capacity == map->n_buildings){
		map->buildings_capacity *= 2;
		map->all_buildings = (building_t**) realloc(map->all_buildings,sizeof(building_t*)*map->buildings_capacity);
	}
	
	map->all_buildings[map->n_buildings] = building;
	map->n_buildings++;
}

building_t * get_building_by_index(map_t * map,size_t index){
	if(map == NULL) return NULL;
	if(!(index < map->n_buildings)) return NULL;//out of bounds
	
	return map->all_buildings[index];
}

building_t * get_building_by_name(map_t * map, const char * name){
	if(map == NULL || name == NULL) return NULL;//invalid parameters
	
	for(size_t i = 0;i < map->n_buildings;i++){
		building_t * current_building = map->all_buildings[i];
		
		for(size_t j = 0;j < current_building->n_possible_names;j++){
			const char * possible_name = current_building->possible_names[j];
			
			if(strcmp(possible_name,name) == 0){
				return current_building;
			}
		}
	}
	
	return NULL;
}

void remove_building_from_map_by_index(map_t * map,size_t index){
	if(map == NULL) return;//invalid parameter
	if(!(index < map->n_buildings)) return;//out of bounds
	
	building_t * building_in_question = map->all_buildings[index];
	
	//remove all references to that building within the map
	for(size_t i = 0;i < map->n_nodes;i++){
		map_node_t * current_node = map->all_nodes[i];
		if(current_node->associated_building == building_in_question){
			clear_map_node_building(current_node);
		}
	}
	
	//delete the building
	delete_building(building_in_question,NULL);//TODO add error context
	
	//shift over data
	for(size_t i = index;i < map->n_buildings-1;i++){
		map->all_buildings[i] = map->all_buildings[i+1];
	}
	map->n_buildings--;//shrink array
}

void remove_building_from_map(map_t * map,building_t * building){
	if(map == NULL || building == NULL) return;//invalid parameters
	if(map->n_buildings == 0) return;//array is empty
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_buildings;i++){
		building_t * current_building = map->all_buildings[i];
		
		if(current_building == building){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found) return;
	
	remove_building_from_map_by_index(map,matching_index);
}

void remove_building_by_name_from_map(map_t * map,const char * name){
	if(map == NULL || name == NULL) return;//invalid parameters
	if(map->n_buildings == 0) return;//array is empty
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_buildings;i++){
		building_t * current_building = map->all_buildings[i];
		
		for(size_t j = 0;j < current_building->n_possible_names;j++){
			const char * possible_name = current_building->possible_names[j];
			
			if(strcmp(possible_name,name) == 0){
				found = true;
				matching_index = i;
				break;
			}
		}
		
		if(found) break;
	}
	
	if(!found) return;
	
	remove_building_from_map_by_index(map,matching_index);
}

void add_node_to_map(map_t * map,map_node_t * node){
	if(map == NULL || node == NULL) return;
	
	if(map->all_nodes == NULL){
		map->node_capacity = DEFAULT_NODES_CAPACITY;
		map->all_nodes = (map_node_t**) malloc(sizeof(map_node_t*)*map->node_capacity);
	}
	
	if(map->node_capacity == map->n_nodes){
		map->node_capacity *= 2;
		map->all_nodes = (map_node_t**) realloc(map->all_nodes,sizeof(map_node_t*)*map->node_capacity);
	}
	
	map->all_nodes[map->n_nodes] = node;
	map->n_nodes++;
}

static void remove_edge_from_map_by_index(map_t * map,size_t index){
	if(map == NULL) return;
	if(!(index < map->n_edges)) return;
	
	delete_map_edge(map->all_edges[index]);
	
	//shift over data
	for(size_t i = index;i < map->n_edges-1;i++){
		map->all_edges[i] = map->all_edges[i+1];
	}
	map->n_edges--;//shrink array
}

static void add_edge_to_map(map_t * map,map_edge_t * edge){
	if(map == NULL || edge == NULL) return;
	
	if(map->all_edges == NULL){
		map->edge_capacity = DEFAULT_EDGES_CAPACITY;
		map->all_edges = (map_edge_t**) malloc(sizeof(map_edge_t*)*map->edge_capacity);
	}
	
	if(map->edge_capacity == map->n_edges){
		map->edge_capacity *= 2;
		map->all_edges = (map_edge_t**) realloc(map->all_edges,sizeof(map_edge_t*)*map->edge_capacity);
	}
	
	map->all_edges[map->n_edges] = edge;
	map->n_edges++;
}

static void remove_edge_from_map(map_t * map,map_edge_t * edge){
	if(map == NULL || edge == NULL) return;
	if(map->n_edges == 0) return;
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_edges;i++){
		map_edge_t * current_edge = map->all_edges[i];
		
		if(current_edge == edge){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found) return;
	
	remove_edge_from_map_by_index(map,matching_index);
}

void remove_node_from_map_by_index(map_t * map,size_t index){
	if(map == NULL) return;//invalid parameter
	if(!(index < map->n_nodes)) return;//out of bounds
	
	map_node_t * node_in_question = map->all_nodes[index];
	
	//remove all connections to the node
	for(size_t i = 0;i < node_in_question->n_outgoing_edges;i++){
		map_edge_t * outgoing_edge = node_in_question->outgoing_edges[i];
		
		if(outgoing_edge->a == node_in_question){
			remove_outgoing_edge_from_node(outgoing_edge->b,outgoing_edge);
		}else if(outgoing_edge->b == node_in_question){
			remove_outgoing_edge_from_node(outgoing_edge->a,outgoing_edge);
		}
		
		remove_edge_from_map(map,outgoing_edge);
	}
	
	//delete the node
	delete_map_node(node_in_question,NULL);//TODO add error context
	
	//shift over data
	for(size_t i = index;i < map->n_nodes-1;i++){
		map->all_nodes[i] = map->all_nodes[i+1];
	}
	map->n_nodes--;//shrink array
}

static size_t find_node_in_map_by_pointer(map_t * map,map_node_t * node,bool * found){
	if(map == NULL || node == NULL) return 0;
	
	*found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_nodes;i++){
		map_node_t * current_node = map->all_nodes[i];
		
		if(current_node == node){
			*found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!(*found)) return 0;
	
	return matching_index;
}

void remove_node_from_map(map_t * map,map_node_t * node){
	if(map == NULL || node == NULL) return;
	
	bool found = false;
	size_t matching_index = find_node_in_map_by_pointer(map,node,&found);
	if(!found) return;
	
	remove_node_from_map_by_index(map,matching_index);
}

static size_t find_node_in_map_by_name(map_t * map,const char * node_name,bool * found){
	if(map == NULL || node_name == NULL) return 0;
	
	*found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_nodes;i++){
		map_node_t * current_node = map->all_nodes[i];
		if(current_node->name == NULL) continue;
		
		if(strcmp(node_name,current_node->name) == 0){
			*found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!(*found)) return 0;
	
	return matching_index;
}

void remove_node_by_name_from_map(map_t * map,const char * node_name){
	if(map == NULL || node_name == NULL) return;//invalid parameters
	if(map->n_nodes == 0) return;//array is empty
	
	bool found = false;
	size_t matching_index = find_node_in_map_by_name(map,node_name,&found);
	if(!found) return;
	
	remove_node_from_map_by_index(map,matching_index);
}

void connect_nodes_in_map_by_indices(map_t * map,size_t index_a,size_t index_b,uint8_t edge_type){
	if(map == NULL) return;//invalid parameters
	if( (!(index_a < map->n_nodes)) || (!(index_b < map->n_nodes))) return;//out of bounds
	if(index_a == index_b) return;//dont connect nodes to themselves
	
	map_node_t * node_a = map->all_nodes[index_a];
	map_node_t * node_b = map->all_nodes[index_b];
	
	map_edge_t * new_edge = create_map_edge(edge_type,node_a,node_b);
	
	add_edge_to_map(map,new_edge);
}

void connect_nodes_in_map(map_t * map,map_node_t * node_a,map_node_t * node_b,uint8_t edge_type){
	if(map == NULL || node_a == NULL || node_b == NULL) return;
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_pointer(map,node_a,&found);
	if(!found) return;
	found = false;
	size_t node_b_index = find_node_in_map_by_pointer(map,node_b,&found);
	if(!found) return;
	
	connect_nodes_in_map_by_indices(map,node_a_index,node_b_index,edge_type);
}

void connect_nodes_in_map_by_names(map_t * map,const char * node_a,const char * node_b,uint8_t edge_type){
	if(map == NULL || node_a == NULL || node_b == NULL) return;
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_name(map,node_a,&found);
	if(!found) return;
	found = false;
	size_t node_b_index = find_node_in_map_by_name(map,node_b,&found);
	if(!found) return;
	
	connect_nodes_in_map_by_indices(map,node_a_index,node_b_index,edge_type);
}

void disconnect_nodes_in_map_by_indices(map_t * map,size_t index_a,size_t index_b){
	if(map == NULL) return;
	if( (!(index_a < map->n_nodes)) || (!(index_b < map->n_nodes))) return;//out of bounds
	if(index_a == index_b) return;//cant disconnet from ourselves
	
	map_node_t * node_a = map->all_nodes[index_a];
	map_node_t * node_b = map->all_nodes[index_b];
	
	for(size_t i = 0;i < node_a->n_outgoing_edges;i++){
		map_edge_t * current_edge = node_a->outgoing_edges[i];
		
		if(current_edge->a == node_b || current_edge->b == node_b){
			remove_outgoing_edge_from_node_by_index(node_a,i);
			remove_outgoing_edge_from_node(node_b,current_edge);
			remove_edge_from_map(map,current_edge);
			return;
		}
	}
}

void disconnect_nodes_in_map(map_t * map,map_node_t * node_a,map_node_t * node_b){
	if(map == NULL || node_a == NULL || node_b == NULL) return;
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_pointer(map,node_a,&found);
	if(!found) return;
	found = false;
	size_t node_b_index = find_node_in_map_by_pointer(map,node_b,&found);
	if(!found) return;
	
	disconnect_nodes_in_map_by_indices(map,node_a_index,node_b_index);
}

void disconnect_nodes_in_map_by_names(map_t * map,const char * node_a,const char * node_b){
	if(map == NULL || node_a == NULL || node_b == NULL) return;
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_name(map,node_a,&found);
	if(!found) return;
	found = false;
	size_t node_b_index = find_node_in_map_by_name(map,node_b,&found);
	if(!found) return;
	
	disconnect_nodes_in_map_by_indices(map,node_a_index,node_b_index);
}

void set_connection_type_for_nodes_by_indices(map_t * map,size_t index_a,size_t index_b,uint8_t new_edge_type){
	if(map == NULL) return;
	if( (!(index_a < map->n_nodes)) || (!(index_b < map->n_nodes))) return;//out of bounds
	if(index_a == index_b) return;
	
	map_node_t * node_a = map->all_nodes[index_a];
	map_node_t * node_b = map->all_nodes[index_b];
	
	for(size_t i = 0;i < node_a->n_outgoing_edges;i++){
		map_edge_t * current_edge = node_a->outgoing_edges[i];
		
		if(current_edge->a == node_b || current_edge->b == node_b){
			current_edge->type = new_edge_type;
			return;
		}
	}
}

void set_connection_type_for_nodes(map_t * map,map_node_t * node_a,map_node_t * node_b,uint8_t new_edge_type){
	if(map == NULL || node_a == NULL || node_b == NULL) return;
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_pointer(map,node_a,&found);
	if(!found) return;
	found = false;
	size_t node_b_index = find_node_in_map_by_pointer(map,node_b,&found);
	if(!found) return;
	
	set_connection_type_for_nodes_by_indices(map,node_a_index,node_b_index,new_edge_type);
}

void set_connection_type_for_nodes_by_name(map_t * map,const char * node_a,const char * node_b,uint8_t new_edge_type){
	if(map == NULL || node_a == NULL || node_b == NULL) return;
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_name(map,node_a,&found);
	if(!found) return;
	found = false;
	size_t node_b_index = find_node_in_map_by_name(map,node_b,&found);
	if(!found) return;
	
	set_connection_type_for_nodes_by_indices(map,node_a_index,node_b_index,new_edge_type);
}

void add_mpo_to_map(map_t * map,mpo_t * mpo){
	if(map == NULL || mpo == NULL) return;
	
	if(map->all_mpos == NULL){
		map->mpo_capacity = DEFAULT_MPO_CAPACITY;
		map->all_mpos = (mpo_t**) malloc(sizeof(mpo_t*)*map->mpo_capacity);
	}
	
	if(map->mpo_capacity == map->n_mpos){
		map->mpo_capacity *= 2;
		map->all_mpos = (mpo_t**) realloc(map->all_mpos,sizeof(mpo_t*)*map->mpo_capacity);
	}
	
	map->all_mpos[map->n_mpos] = mpo;
	map->n_mpos++;
}

void remove_mpo_from_map_by_index(map_t * map,size_t mpo_index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;//invalid parameter
	}
	if(!(mpo_index < map->n_mpos)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;//out of bounds
	}
	
	mpo_t * mpo_in_question = map->all_mpos[mpo_index];
	
	//delete the mpo
	delete_mpo(mpo_in_question,ctx);
	
	//shift over data
	for(size_t i = mpo_index;i < map->n_mpos-1;i++){
		map->all_mpos[i] = map->all_mpos[i+1];
	}
	map->n_mpos--;//shrink array
}

void remove_mpo_from_map_by_name(map_t * map,const char * mpo_name,err_ctx_t * ctx){
	if(map == NULL || mpo_name == NULL) {
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_mpos;i++){
		mpo_t * current_mpo = map->all_mpos[i];
		if(current_mpo->name == NULL) continue;
		
		if(strcmp(mpo_name,current_mpo->name) == 0){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_mpo_from_map_by_index(map,matching_index,ctx);
}

void remove_mpo_from_map(map_t * map,mpo_t * mpo,err_ctx_t * ctx){
	if(map == NULL || mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool found = false;
	size_t matching_index = 0;
	
	//find it in the array
	for(size_t i = 0;i < map->n_mpos;i++){
		mpo_t * current_mpo = map->all_mpos[i];
		
		if(current_mpo == mpo){
			found = true;
			matching_index = i;
			break;
		}
	}
	
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_mpo_from_map_by_index(map,matching_index,ctx);
}

void map_to_output_stream(map_t map,size_t tabs,FILE * stream){
	if(stream == NULL) return;
	
	put_multitab(tabs,stream);
	fputs("Map:\n",stream);
	
	put_multitab(tabs,stream);
	fputs("\tN nodes:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lu\n",map.n_nodes);
	
	
	put_multitab(tabs,stream);
	fputs("\tNodes Array:\n",stream);
	for(size_t i = 0;i < map.n_nodes;i++){
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lu\n",i);
		map_node_to_output_stream(map.all_nodes[i],tabs+2,stream);
	}
	
	put_multitab(tabs,stream);
	fputs("\tN edges:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lu\n",map.n_edges);
	
	put_multitab(tabs,stream);
	fputs("\tEdges Array:\n",stream);
	for(size_t i = 0;i < map.n_edges;i++){
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lu\n",i);
		map_edge_to_output_stream(map.all_edges[i],tabs+2,stream);
	}
	
	put_multitab(tabs,stream);
	fputs("\tN Buildings:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lu\n",map.n_buildings);
	
	put_multitab(tabs,stream);
	fputs("\tBuildings Array:\n",stream);
	for(size_t i = 0;i < map.n_buildings;i++){
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lu\n",i);
		building_to_output_stream(map.all_buildings[i],tabs+2,stream,NULL);//TODO add error context
	}
	
	put_multitab(tabs,stream);
	fputs("\tN Map Polygon Objects:\n",stream);
	put_multitab(tabs,stream);
	fprintf(stream,"\t\t%lu\n",map.n_mpos);
	
	put_multitab(tabs,stream);
	fputs("\tMap Polygon Object Array:\n",stream);
	for(size_t i = 0;i < map.n_mpos;i++){
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lu\n",i);
		mpo_to_output_stream(map.all_mpos[i],tabs+2,stream,NULL);//TODO add error context
	}
}

void do_thing(){
	err_ctx_t ctx = create_err_ctx();
	
	map_t map = init_map();
	
	building_t * building_a = create_building(
		"Building A",//primary building name
		create_map_rect(create_cord(-5,-5),create_cord(5,5)),//bounding box of building
		5,//n floors
		&ctx
	);
	add_building_alias_name(building_a,"Build B",&ctx);
	
	cord_t cord_arr[4] = {create_cord(0,0), create_cord(0,1), create_cord(1,1), create_cord(1,0) };
	mpo_t * square_mpo = create_mpo(cord_arr,4,MPO_TYPE_WATER,&ctx);
	set_mpo_name(square_mpo,"Square Lake",&ctx);
	add_mpo_to_map(&map,square_mpo);
	
	add_building_to_map(&map,building_a);
	
	map_node_t * a = create_map_node(create_cord(2,3));
	set_map_node_name(a,"Node A");
	set_map_node_building(a,building_a);
	add_node_to_map(&map,a);
	
	map_node_t * b = create_map_node(create_cord(4,6));
	set_map_node_name(b,"Node B");
	add_node_to_map(&map,b);
	
	connect_nodes_in_map(&map,a,b,EDGE_TYPE_CROSSWALK);
	
	map_node_t * c = create_map_node(create_cord(-1,-3));
	set_map_node_name(c,"Node C");
	add_node_to_map(&map,c);
	
	connect_nodes_in_map_by_names(&map,"Node A","Node C",EDGE_TYPE_STAIRS);
	
	map_to_output_stream(map,0,stdout);
	
	remove_node_from_map(&map,a);
	
	map_to_output_stream(map,0,stdout);
	clear_map(&map,&ctx);
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
