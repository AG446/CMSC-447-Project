#include "map.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "text_proc.h"
#include <float.h>
#include <math.h>

//MEMORY PARAMETERS

#define DEFAULT_POSSIBLE_NAMES_CAPACITY 1
#define DEFAULT_OUTGOING_EDGES_CAPACITY 1
#define DEFAULT_BUILDINGS_CAPACITY 1
#define DEFAULT_NODES_CAPACITY 1
#define DEFAULT_EDGES_CAPACITY 1
#define DEFAULT_MPO_CAPACITY 1


const char * mpo_type_names[N_MPO_TYPES] = {
	"Water",
	"Tree",
	"Building"
};

const char * edge_type_names[N_EDGE_TYPES] = {
	"Sidewalk",
	"Road",
	"Stairs",
	"Ramp",
	"Hallway",
	"Elevator Shaft",
	"Overpass",
	"Door",
	"Automatic Door",
	"Crosswalk"
};

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

bool are_cords_equal(cord_t a,cord_t b){
	return (a.longitude == b.longitude) && (a.latitude == b.latitude);
}

double cord_distance(cord_t a,cord_t b){
	double dlon = b.longitude-a.longitude;
	double dlat = b.latitude-a.latitude;
	return sqrt(dlon*dlon + dlat*dlat );
}

void cord_to_output_stream(cord_t cord,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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

void map_rect_to_output_stream(map_rect_t rect,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	put_multitab(tabs,stream);
	fputs("Map-Rect:\n",stream);
	
	put_multitab(tabs,stream);
	fputs("\tBottom left:\n",stream);
	cord_to_output_stream(rect.bottom_left,tabs+2,stream,ctx);
	
	put_multitab(tabs,stream);
	fputs("\tTop Right:\n",stream);
	cord_to_output_stream(rect.top_right,tabs+2,stream,ctx);
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
	map_rect_to_output_stream(building->building_bounding_box,tabs+2,stream,ctx);
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

void set_map_node_cord(map_node_t * node,cord_t new_cord,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	node->coordinate = new_cord;
}

cord_t get_map_node_cord(const map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_cord(0.0,0.0);
	}
	
	return node->coordinate;
}

void set_map_node_name(map_node_t * node,const char * name,err_ctx_t * ctx){
	if(node == NULL || name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(node->name != NULL) free(node->name);
	
	size_t name_length = strlen(name);
	char * name_cpy = (char*) malloc(name_length+1);
	strcpy(name_cpy,name);
	
	node->name = name_cpy;
}

void clear_map_node_name(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(node->name == NULL) return;
	
	free(node->name);
	node->name = NULL;
}

const char * get_map_node_name(const map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	return node->name;
}

void set_map_node_picture(map_node_t * node,const char * file_path,err_ctx_t * ctx){
	if(node == NULL || file_path == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(node->picture_file_path != NULL) free(node->picture_file_path);
	
	size_t path_length = strlen(file_path);
	char * path_cpy = (char*) malloc(path_length+1);
	strcpy(path_cpy,file_path);
	
	node->picture_file_path = path_cpy;
}

void clear_map_node_picture(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(node->picture_file_path == NULL) return;
	
	free(node->picture_file_path);
	node->picture_file_path = NULL;
}

const char * get_map_node_picture(const map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	return node->picture_file_path;
}

bool node_adjacent_to_auto_door(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	if(node->n_outgoing_edges == 0) return false;
	
	for(size_t i = 0;i < node->n_outgoing_edges;i++){
		map_edge_t * connected_edge = node->outgoing_edges[i];
		
		if(connected_edge->type == EDGE_TYPE_AUTO_DOOR){
			return true;
		}
	}
	
	return false;
}

void set_map_node_floor_number(map_node_t * node,int8_t floor_number,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	node->floor_number = floor_number;
}

int8_t get_map_node_floor_number(const map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NODE_FLOOR_NUMBER_NONE;
	}
	
	return node->floor_number;
}

void clear_map_node_floor_number(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	node->floor_number = NODE_FLOOR_NUMBER_NONE;
}

void set_map_node_selectable(map_node_t * node,bool selectable,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	node->selectable = selectable;
}

bool get_map_node_selectable(const map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	
	return node->selectable;
}

void set_map_node_building(map_node_t * node,building_t * building,err_ctx_t * ctx){
	if(node == NULL || building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	node->associated_building = building;
}

void clear_map_node_building(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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

void map_node_to_output_stream(const map_node_t * node,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(node == NULL || stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	put_multitab(tabs,stream);
	fprintf(stream,"Map-Node %p:\n",node);
	cord_to_output_stream(node->coordinate,tabs+1,stream,ctx);
	
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
		fprintf(stream,"\t\t%s\n",get_primary_building_name(node->associated_building,ctx));
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

map_edge_t * create_map_edge(uint8_t type,map_node_t * a,map_node_t * b,err_ctx_t * ctx) {
	if(a == NULL || b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	map_edge_t * output = (map_edge_t *)malloc(sizeof(map_edge_t));
	output->a = a;
	output->b = b;
	output->type = type;
	
	add_outgoing_edge_to_node(a,output);
	add_outgoing_edge_to_node(b,output);
	
	return output;
}

void delete_map_edge(map_edge_t * edge,err_ctx_t * ctx){
	if(edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	free(edge);
}

void set_map_edge_type(map_edge_t * edge,uint8_t type,err_ctx_t * ctx){
	if(edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	edge->type = type;
}

uint8_t get_map_edge_type(const map_edge_t * edge,err_ctx_t * ctx){
	if(edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return edge->type;
}

double get_edge_length(const map_edge_t * edge,err_ctx_t * ctx){
	if(edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0.0;
	}
	
	if(edge->a == NULL || edge->b == NULL){
		return FLT_MAX;
	}
	
	return cord_distance(edge->a->coordinate,edge->b->coordinate);
}

const map_node_t * get_edge_node_a(const map_edge_t * edge,err_ctx_t * ctx){
	if(edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	return edge->a;
}

const map_node_t * get_edge_node_b(const map_edge_t * edge,err_ctx_t * ctx){
	if(edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	return edge->b;
}

void map_edge_to_output_stream(const map_edge_t * edge,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(edge == NULL || stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	put_multitab(tabs,stream);
	fprintf(stream,"Edge %p:\n",edge);
	
	put_multitab(tabs,stream);
	fputs("\tType:\n",stream);
	put_multitab(tabs,stream);
	fputs("\t\t",stream);
	if(edge->type >= 1 && edge->type <= N_EDGE_TYPES){
		fputs(edge_type_names[edge->type-1],stream);
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
	
	if(edge->a != NULL && edge->b != NULL){
		put_multitab(tabs,stream);
		fputs("\tEuclidian Length:\n",stream);
		put_multitab(tabs,stream);
		fprintf(stream,"\t\t%lf\n",get_edge_length(edge,ctx));
	}
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
	if(mpo->type >= 1 && mpo->type <= N_MPO_TYPES){
		fputs(mpo_type_names[mpo->type-1],stream);
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
		cord_to_output_stream(mpo->cords[i],tabs+2,stream,ctx);
	}
}

void set_mpo_type(mpo_t * mpo,uint8_t new_type,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	mpo->type = new_type;
}

uint8_t get_mpo_type(const mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return mpo->type;
}

size_t get_mpo_size(const mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return mpo->n_cords;
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

cord_t get_mpo_cord(const mpo_t * mpo,size_t index,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_cord(0.0,0.0);
	}
	if(!(index < mpo->n_cords)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return create_cord(0.0,0.0);
	}
	
	return mpo->cords[index];
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

void deinit_map(map_t * map,err_ctx_t * ctx){
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
			delete_map_edge(map->all_edges[i],ctx);
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

void add_building_to_map(map_t * map,building_t * building,err_ctx_t * ctx){
	if(map == NULL || building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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

building_t * get_building_by_index_from_map(map_t * map,size_t index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	if(!(index < map->n_buildings)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return NULL;
	}
	
	return map->all_buildings[index];
}

building_t * get_building_by_name_from_map(map_t * map, const char * name,err_ctx_t * ctx){
	if(map == NULL || name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	for(size_t i = 0;i < map->n_buildings;i++){
		building_t * current_building = map->all_buildings[i];
		
		for(size_t j = 0;j < current_building->n_possible_names;j++){
			const char * possible_name = current_building->possible_names[j];
			
			if(strcmp(possible_name,name) == 0){
				return current_building;
			}
		}
	}
	
	ctx->flags |= ERROR_OBJECT_NOT_FOUND;
	
	return NULL;
}

mpo_t * get_mpo_by_index_from_map(map_t * map,size_t index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	if(!(index < map->n_mpos)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return NULL;
	}
	
	return map->all_mpos[index];
}

mpo_t * get_mpo_by_name_from_map(map_t * map,const char * name,err_ctx_t * ctx){
	if(map == NULL || name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	
	for(size_t i = 0;i < map->n_mpos;i++){
		mpo_t * current_mpo = map->all_mpos[i];
		
		if(current_mpo->name != NULL){
			if(strcmp(current_mpo->name,name) == 0){
				return current_mpo;
			}
		}
	}
	
	ctx->flags |= ERROR_OBJECT_NOT_FOUND;
	
	return NULL;
}

void remove_building_from_map_by_index(map_t * map,size_t index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(!(index < map->n_buildings)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	
	building_t * building_in_question = map->all_buildings[index];
	
	//remove all references to that building within the map
	for(size_t i = 0;i < map->n_nodes;i++){
		map_node_t * current_node = map->all_nodes[i];
		if(current_node->associated_building == building_in_question){
			clear_map_node_building(current_node,ctx);
		}
	}
	
	//delete the building
	delete_building(building_in_question,ctx);
	
	//shift over data
	for(size_t i = index;i < map->n_buildings-1;i++){
		map->all_buildings[i] = map->all_buildings[i+1];
	}
	map->n_buildings--;//shrink array
}

void remove_building_from_map(map_t * map,building_t * building,err_ctx_t * ctx){
	if(map == NULL || building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(map->n_buildings == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
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
	
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_building_from_map_by_index(map,matching_index,ctx);
}

void remove_building_by_name_from_map(map_t * map,const char * name,err_ctx_t * ctx){
	if(map == NULL || name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(map->n_buildings == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
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
	
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_building_from_map_by_index(map,matching_index,ctx);
}

void add_node_to_map(map_t * map,map_node_t * node,err_ctx_t * ctx){
	if(map == NULL || node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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

static void remove_edge_from_map_by_index(map_t * map,size_t index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(!(index < map->n_edges)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	
	delete_map_edge(map->all_edges[index],ctx);
	
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

static void remove_edge_from_map(map_t * map,map_edge_t * edge,err_ctx_t * ctx){
	if(map == NULL || edge == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(map->n_edges == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
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
	
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_edge_from_map_by_index(map,matching_index,ctx);
}

void remove_node_from_map_by_index(map_t * map,size_t index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(!(index < map->n_nodes)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	
	map_node_t * node_in_question = map->all_nodes[index];
	
	//remove all connections to the node
	for(size_t i = 0;i < node_in_question->n_outgoing_edges;i++){
		map_edge_t * outgoing_edge = node_in_question->outgoing_edges[i];
		
		if(outgoing_edge->a == node_in_question){
			remove_outgoing_edge_from_node(outgoing_edge->b,outgoing_edge);
		}else if(outgoing_edge->b == node_in_question){
			remove_outgoing_edge_from_node(outgoing_edge->a,outgoing_edge);
		}
		
		remove_edge_from_map(map,outgoing_edge,ctx);
	}
	
	//delete the node
	delete_map_node(node_in_question,ctx);
	
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

void remove_node_from_map(map_t * map,map_node_t * node,err_ctx_t * ctx){
	if(map == NULL || node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool found = false;
	size_t matching_index = find_node_in_map_by_pointer(map,node,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_node_from_map_by_index(map,matching_index,ctx);
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

map_node_t * get_node_by_index_from_map(map_t * map,size_t index,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	if(!(index < map->n_nodes)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return NULL;
	}
	
	return map->all_nodes[index];
}

map_node_t * get_node_by_name_from_map(map_t * map, const char * name,err_ctx_t * ctx){
	if(map == NULL || name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	if(map->n_nodes == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return NULL;
	}
	
	bool found = false;
	size_t matching_index = find_node_in_map_by_name(map,name,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return NULL;
	}
	
	return map->all_nodes[matching_index];
}

void remove_node_by_name_from_map(map_t * map,const char * node_name,err_ctx_t * ctx){
	if(map == NULL || node_name == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(map->n_nodes == 0){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	bool found = false;
	size_t matching_index = find_node_in_map_by_name(map,node_name,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	remove_node_from_map_by_index(map,matching_index,ctx);
}

void connect_nodes_in_map_by_indices(map_t * map,size_t index_a,size_t index_b,uint8_t edge_type,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if( (!(index_a < map->n_nodes)) || (!(index_b < map->n_nodes))){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	if(index_a == index_b){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;//dont connect nodes to themselves
	}
	
	map_node_t * node_a = map->all_nodes[index_a];
	map_node_t * node_b = map->all_nodes[index_b];
	
	map_edge_t * new_edge = create_map_edge(edge_type,node_a,node_b,ctx);
	
	add_edge_to_map(map,new_edge);
}

void connect_nodes_in_map(map_t * map,map_node_t * node_a,map_node_t * node_b,uint8_t edge_type,err_ctx_t * ctx){
	if(map == NULL || node_a == NULL || node_b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(node_a == node_b){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_pointer(map,node_a,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	found = false;
	size_t node_b_index = find_node_in_map_by_pointer(map,node_b,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	connect_nodes_in_map_by_indices(map,node_a_index,node_b_index,edge_type,ctx);
}

void connect_nodes_in_map_by_names(map_t * map,const char * node_a,const char * node_b,uint8_t edge_type,err_ctx_t * ctx){
	if(map == NULL || node_a == NULL || node_b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(strcmp(node_a,node_b) == 0){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_name(map,node_a,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	found = false;
	size_t node_b_index = find_node_in_map_by_name(map,node_b,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	connect_nodes_in_map_by_indices(map,node_a_index,node_b_index,edge_type,ctx);
}

void disconnect_nodes_in_map_by_indices(map_t * map,size_t index_a,size_t index_b,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if( (!(index_a < map->n_nodes)) || (!(index_b < map->n_nodes))){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	if(index_a == index_b){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	map_node_t * node_a = map->all_nodes[index_a];
	map_node_t * node_b = map->all_nodes[index_b];
	
	for(size_t i = 0;i < node_a->n_outgoing_edges;i++){
		map_edge_t * current_edge = node_a->outgoing_edges[i];
		
		if(current_edge->a == node_b || current_edge->b == node_b){
			remove_outgoing_edge_from_node_by_index(node_a,i);
			remove_outgoing_edge_from_node(node_b,current_edge);
			remove_edge_from_map(map,current_edge,ctx);
			return;
		}
	}
}

void disconnect_nodes_in_map(map_t * map,map_node_t * node_a,map_node_t * node_b,err_ctx_t * ctx){
	if(map == NULL || node_a == NULL || node_b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(node_a == node_b){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_pointer(map,node_a,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	found = false;
	size_t node_b_index = find_node_in_map_by_pointer(map,node_b,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	disconnect_nodes_in_map_by_indices(map,node_a_index,node_b_index,ctx);
}

void disconnect_nodes_in_map_by_names(map_t * map,const char * node_a,const char * node_b,err_ctx_t * ctx){
	if(map == NULL || node_a == NULL || node_b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(strcmp(node_a,node_b) == 0){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_name(map,node_a,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	found = false;
	size_t node_b_index = find_node_in_map_by_name(map,node_b,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	disconnect_nodes_in_map_by_indices(map,node_a_index,node_b_index,ctx);
}

void set_connection_type_for_nodes_by_indices(map_t * map,size_t index_a,size_t index_b,uint8_t new_edge_type,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if( (!(index_a < map->n_nodes)) || (!(index_b < map->n_nodes))){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	if(index_a == index_b){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
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

void set_connection_type_for_nodes(map_t * map,map_node_t * node_a,map_node_t * node_b,uint8_t new_edge_type,err_ctx_t * ctx){
	if(map == NULL || node_a == NULL || node_b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(node_a == node_b){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_pointer(map,node_a,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	found = false;
	size_t node_b_index = find_node_in_map_by_pointer(map,node_b,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	set_connection_type_for_nodes_by_indices(map,node_a_index,node_b_index,new_edge_type,ctx);
}

void set_connection_type_for_nodes_by_name(map_t * map,const char * node_a,const char * node_b,uint8_t new_edge_type,err_ctx_t * ctx){
	if(map == NULL || node_a == NULL || node_b == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(strcmp(node_a,node_b) == 0){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	bool found = false;
	size_t node_a_index = find_node_in_map_by_name(map,node_a,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	found = false;
	size_t node_b_index = find_node_in_map_by_name(map,node_b,&found);
	if(!found){
		ctx->flags |= ERROR_OBJECT_NOT_FOUND;
		return;
	}
	
	set_connection_type_for_nodes_by_indices(map,node_a_index,node_b_index,new_edge_type,ctx);
}

void add_mpo_to_map(map_t * map,mpo_t * mpo,err_ctx_t * ctx){
	if(map == NULL || mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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

size_t get_map_node_count(map_t * map,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return map->n_nodes;
}

size_t get_map_mpo_count(map_t * map,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return map->n_mpos;
}

size_t get_map_edge_count(map_t * map,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return map->n_edges;
}

size_t get_map_building_count(map_t * map,err_ctx_t * ctx){
	if(map == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return 0;
	}
	
	return map->n_buildings;
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

void map_to_output_stream(map_t map,size_t tabs,FILE * stream,err_ctx_t * ctx){
	if(stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
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
		map_node_to_output_stream(map.all_nodes[i],tabs+2,stream,ctx);
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
		map_edge_to_output_stream(map.all_edges[i],tabs+2,stream,ctx);
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
		building_to_output_stream(map.all_buildings[i],tabs+2,stream,ctx);
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
		mpo_to_output_stream(map.all_mpos[i],tabs+2,stream,ctx);
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
	add_mpo_to_map(&map,square_mpo,&ctx);
	
	add_building_to_map(&map,building_a,&ctx);
	
	map_node_t * a = create_map_node(create_cord(2,3));
	set_map_node_name(a,"Node A",&ctx);
	set_map_node_building(a,building_a,&ctx);
	add_node_to_map(&map,a,&ctx);
	
	map_node_t * b = create_map_node(create_cord(4,6));
	set_map_node_name(b,"Node B",&ctx);
	add_node_to_map(&map,b,&ctx);
	
	connect_nodes_in_map(&map,a,b,EDGE_TYPE_CROSSWALK,&ctx);
	
	map_node_t * c = create_map_node(create_cord(-1,-3));
	set_map_node_name(c,"Node C",&ctx);
	add_node_to_map(&map,c,&ctx);
	
	connect_nodes_in_map_by_names(&map,"Node A","Node C",EDGE_TYPE_STAIRS,&ctx);
	
	map_to_output_stream(map,0,stdout,&ctx);
	
	remove_node_from_map(&map,a,&ctx);
	
	map_to_output_stream(map,0,stdout,&ctx);
	deinit_map(&map,&ctx);
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
/*
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
	
	
	//printf("%d %ld\n",mpo_test->type,mpo_test->n_cords);
	//for(size_t i = 0;i < mpo_test->n_cords;i++){
	//	printf("(%lf %lf),\n",mpo_test->cords[i].longitude,mpo_test->cords[i].latitude);
	//}
	
	//mpo_to_string(mpo_test, stdout);
	
	fclose(read_ptr);
}
*/

/*
map_node_t ** filter_locations(const char * location_name,const map_t * map_ref,size_t max_results){
	
}
*/
