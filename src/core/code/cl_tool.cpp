#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cl_tool.h"
#include "text_proc.h"
#include "err_ctx.h"
#include <ctype.h>

#define DEFAULT_GWO_CAPACITY 4

gwo_t create_cord_gwo(cord_t cord){
	gwo_t out;
	out.type = GWO_CORD;
	out.cord = cord;
	return out;
}
gwo_t create_map_rect_gwo(map_rect_t rect){
	gwo_t out;
	out.type = GWO_RECT;
	out.rect = rect;
	return out;
}

gwo_t create_blank_gwo(void){
	gwo_t out;
	out.cord = create_cord(0.0,0.0);
	out.type = 0;
	return out;
}

gwo_t create_mpo_gwo(mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_blank_gwo();
	}
	
	gwo_t out;
	
	out.type = GWO_MPO;
	out.mpo = mpo;
	
	return out;
}

gwo_t create_node_gwo(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_blank_gwo();
	}
	
	gwo_t out;
	
	out.type = GWO_NODE;
	out.node = node;
	
	return out;
}

gwo_t create_building_gwo(building_t * building,err_ctx_t * ctx){
	if(building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_blank_gwo();
	}
	
	gwo_t out;
	
	out.type = GWO_BUILDING;
	out.building = building;
	
	return out;
}

void delete_gwo_data(gwo_t * gwo,err_ctx_t * ctx){
	if(gwo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(gwo->type == GWO_MPO){
		delete_mpo(gwo->mpo,ctx);
	}else if(gwo->type == GWO_NODE){
		delete_map_node(gwo->node,ctx);
	}else if(gwo->type == GWO_BUILDING){
		delete_building(gwo->building,ctx);
	}
}

gws_t init_generic_working_set(void){
	gws_t out;
	
	out.working_objects_capacity = DEFAULT_GWO_CAPACITY;
	out.n_working_objects = 0;
	out.working_object_arr = (gwo_t*) malloc(sizeof(gwo_t) * out.working_objects_capacity);
	
	return out;
}

bool valid_gws_object(gws_t * gws,size_t index,uint8_t type,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	
	if(!(index < gws->n_working_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return false;
	}
	
	if(gws->working_object_arr[index].type != type){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	
	return true;
}

void add_gwo_to_gws(gws_t * gws,gwo_t gwo,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(gws->n_working_objects == gws->working_objects_capacity){
		gws->working_objects_capacity *= 2;
		gws->working_object_arr = (gwo_t*) realloc(gws->working_object_arr,sizeof(gwo_t) * gws->working_objects_capacity);
	}
	
	gws->working_object_arr[gws->n_working_objects] = gwo;
	gws->n_working_objects++;
}

void deinit_generic_working_set(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	for(size_t i = 0;i < gws->n_working_objects;i++){
		delete_gwo_data(&gws->working_object_arr[i],ctx);
	}
	free(gws->working_object_arr);
}

void clear_gws_objects(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	for(size_t i = 0;i < gws->n_working_objects;i++){
		delete_gwo_data(&gws->working_object_arr[i],ctx);
	}
	gws->n_working_objects = 0;
}

gwo_t remove_gwo_from_gws(gws_t * gws,size_t index,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_blank_gwo();
	}
	if(!(index < gws->n_working_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return create_blank_gwo();
	}
	
	gwo_t out = gws->working_object_arr[index];
	
	//shift over data
	for(size_t i = index;i < gws->n_working_objects-1;i++){
		gws->working_object_arr[i] = gws->working_object_arr[i+1];
	}
	gws->n_working_objects--;//shrink array
	
	return out;
}

gwo_t get_gwo_from_gws(gws_t * gws,size_t index,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return create_blank_gwo();
	}
	if(!(index < gws->n_working_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return create_blank_gwo();
	}
	
	return gws->working_object_arr[index];
}

struct Index_Dual{
	size_t index;
	size_t position;
};

static int increasing_index(const void * a, const void * b){
	const struct Index_Dual * a_index = (struct Index_Dual *)a;
	const struct Index_Dual * b_index = (struct Index_Dual *)b;
	
	if (a_index->index > b_index->index) return 1;
	if (a_index->index < b_index->index) return -1;
	return 0;
}

gwo_t * remove_gwos_from_gws(gws_t * gws,size_t * indexes,size_t n_indexes,err_ctx_t * ctx){
	if(gws == NULL || indexes == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	for(size_t i = 0;i < n_indexes;i++){
		if(!(indexes[i] < gws->n_working_objects)){
			ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
			return NULL;
		}
	}
	
	struct Index_Dual * indexes_sorted = (struct Index_Dual *) malloc(sizeof(struct Index_Dual)*n_indexes);
	for(size_t i = 0;i < n_indexes;i++){
		indexes_sorted[i].index = indexes[i];
		indexes_sorted[i].position = i;
	}
	qsort(indexes_sorted,n_indexes,sizeof(struct Index_Dual),increasing_index);
	
	gwo_t * out = (gwo_t*) malloc(sizeof(gwo_t)*n_indexes);
	
	size_t read_index = 0;
	size_t write_index = 0;
	size_t meta_deletion_index = 0;
	
	while(read_index < gws->n_working_objects){
		struct Index_Dual deletion_index = indexes_sorted[meta_deletion_index];
		
		if(read_index == deletion_index.index){
			out[deletion_index.position] = gws->working_object_arr[read_index];
			meta_deletion_index++;
		}else{
			gws->working_object_arr[write_index] = gws->working_object_arr[read_index];
			write_index++;
		}
		
		read_index++;
	}
	free(indexes_sorted);
	gws->n_working_objects -= n_indexes;
	return out;
}

void delete_gwo_from_gws(gws_t * gws,size_t index,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(!(index < gws->n_working_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	
	gwo_t captured_gwo = remove_gwo_from_gws(gws,index,ctx);
	
	delete_gwo_data(&captured_gwo,ctx);
}

void gws_to_output_stream(const gws_t gws,FILE * stream,err_ctx_t * ctx){
	if(stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	fputs("\033[36mCurrent Working Set\n\033[0m",stream);
	
	if(gws.n_working_objects == 0){
		fputs("Empty\n",stream);
	}
	
	for(size_t i = 0;i < gws.n_working_objects;i++){
		gwo_t current = gws.working_object_arr[i];
		fprintf(stream,"Index: %lu\n",i);
		if(current.type == GWO_CORD){
			cord_to_output_stream(current.cord,1,stream,ctx);
		}else if(current.type == GWO_RECT){
			map_rect_to_output_stream(current.rect,1,stream,ctx);
		}else if(current.type == GWO_MPO){
			mpo_to_output_stream(current.mpo,1,stream,ctx);
		}else if(current.type == GWO_NODE){
			map_node_to_output_stream(current.node,1,stream,ctx);
		}else if(current.type == GWO_BUILDING){
			building_to_output_stream(current.building,1,stream,ctx);
		}
	}
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

static bool is_halting_string(const char * str){
	if(str == NULL) return false;
	
	if(strcmp(str,"stop") == 0) return true;
	if(strcmp(str,"halt") == 0) return true;
	if(strcmp(str,"quit") == 0) return true;
	if(strcmp(str,"exit") == 0) return true;
	if(strcmp(str,"cancel") == 0) return true;
	
	return false;
}

static bool is_deleting_string(const char * str){
	if(str == NULL) return false;
	
	if(strcmp(str,"pop") == 0) return true;
	if(strcmp(str,"delete") == 0) return true;
	if(strcmp(str,"del") == 0) return true;
	if(strcmp(str,"remove") == 0) return true;
	if(strcmp(str,"rem") == 0) return true;
	
	return false;
}

static double parse_double(const char * title,bool * canceled){
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

static size_t parse_index(const char * title,bool * canceled){
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

static size_t parse_index_in_range(const char * title,size_t min,size_t max,bool * canceled){
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

static bool parse_confirmation(const char * title){
	fputs(title,stdout);
	fputc(' ',stdout);
	char * choice_string = read_line();
	
	bool out = strlen(choice_string) == 1 && tolower(choice_string[0]) == 'y';
	
	free(choice_string);
	
	return out;
}

static bool parse_bool(const char * title,bool * canceled){
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

static size_t parse_among_options(const char * title,const char ** options,size_t n_options,bool * canceled){
	fputs(title,stdout);
	fputc('\n',stdout);
	for(size_t i = 1;i <= n_options;i++){
		printf("%lu. %s\n",i,options[i-1]);
	}
	
	return parse_index_in_range("Choose",1,n_options,canceled);
}

static uint8_t parse_mpo_type(bool * canceled){
	return (uint8_t) parse_among_options("### Choose an MPO type ###",mpo_type_names,N_MPO_TYPES,canceled);
}

static uint8_t parse_edge_type(bool * canceled){
	return (uint8_t) parse_among_options("### Choose an Edge type ###",edge_type_names,N_EDGE_TYPES,canceled);
}

#define WORKING_MAP 1
#define WORKING_SET 2
static size_t parse_working_location(bool * canceled){
	const char * options[2] = {"Working Map","Working Set"};
	
	return parse_among_options("### Choose Where to Find ###",options,2,canceled);
}

static cord_t parse_cord(bool * canceled){
	fputs("### Entering a Coordinate ###\n",stdout);
	
	float longitude = parse_double("Longitude",canceled);
	if(*canceled) return create_cord(0.0,0.0);
	
	float latitude = parse_double("Latitude",canceled);
	if(*canceled) return create_cord(0.0,0.0);
	
	return create_cord(longitude,latitude);
}

static void create_cord_command(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	cord_t cord = parse_cord(&canceled);
	if(!canceled){
		add_gwo_to_gws(gws,create_cord_gwo(cord),ctx);
	}
}

static void create_rect_command(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	
	size_t first_obj_index = parse_index("Coordinte Object Index (Bottom Left)",&canceled);
	if(canceled || !valid_gws_object(gws,first_obj_index,GWO_CORD,ctx)) return;
	size_t second_obj_index = parse_index("Coordinte Object Index (Top Right)",&canceled);
	if(canceled || !valid_gws_object(gws,second_obj_index,GWO_CORD,ctx)) return;
	
	if(first_obj_index == second_obj_index){
		ctx->flags |= ERROR_DUPLICATE_PARAMETER;
		return;
	}
	
	size_t indexes[2] = {first_obj_index,second_obj_index};
	gwo_t * removed = remove_gwos_from_gws(gws,indexes,2,ctx);
	gwo_t first_obj = removed[0];
	gwo_t second_obj = removed[1];
	free(removed);
	
	add_gwo_to_gws(gws,create_map_rect_gwo( create_map_rect(first_obj.cord,second_obj.cord) ),ctx);
}

static void create_mpo_command(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	size_t n_cords = parse_index_in_range("Number of Coordinates",3,512,&canceled);
	if(canceled) return;
	
	size_t * obj_indexes = (size_t*) malloc(sizeof(size_t)*n_cords);
	
	for(size_t i = 0; i < n_cords;i++){
		printf("%lu ",i);
		size_t obj_index = parse_index("Coordinte Object Index",&canceled);
		
		if(canceled || !valid_gws_object(gws,obj_index,GWO_CORD,ctx)) {
			free(obj_indexes);
			return;
		}
		
		obj_indexes[i] = obj_index;
	}
	
	gwo_t * removed = remove_gwos_from_gws(gws,obj_indexes,n_cords,ctx);
	free(obj_indexes);
	
	cord_t * cord_arr = (cord_t*) malloc(sizeof(cord_t)*n_cords);
	for(size_t i = 0;i < n_cords;i++){
		cord_arr[i] = removed[i].cord;
	}
	free(removed);
	
	uint8_t chosen_mpo_type = parse_mpo_type(&canceled);
	if(canceled) free(cord_arr);
	
	add_gwo_to_gws(gws,create_mpo_gwo( create_mpo(cord_arr,n_cords,chosen_mpo_type,ctx),ctx ),ctx);
	free(cord_arr);
}

static void create_node_command(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	size_t obj_index = parse_index("Coordinte Object Index",&canceled);
	
	if(canceled || !valid_gws_object(gws,obj_index,GWO_CORD,ctx)) return;
	
	gwo_t removed = remove_gwo_from_gws(gws,obj_index,ctx);
	
	add_gwo_to_gws(gws,create_node_gwo( create_map_node(removed.cord),ctx),ctx);
}

static void delete_command(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	size_t deletion_index = parse_index("Object Index",&canceled);
		
	if(canceled) return;
	
	delete_gwo_from_gws(gws,deletion_index,ctx);
}

static void show_map_command(map_t working_map,err_ctx_t * ctx){
	fputs("\033[36m### Current Working Map ###\n\033[0m",stdout);
	map_to_output_stream(working_map,0,stdout,ctx);
}

static void add_node_to_map_command(map_t * map,gws_t * gws,err_ctx_t * ctx){
	if(map == NULL || gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	
	size_t obj_index = parse_index("Node index",&canceled);
	if(canceled || !valid_gws_object(gws,obj_index,GWO_NODE,ctx)) return;
	
	gwo_t removed = remove_gwo_from_gws(gws,obj_index,ctx);
	
	add_node_to_map(map,removed.node,ctx);
}

static map_node_t * fetch_node(map_t * map,gws_t * gws,err_ctx_t * ctx){
	if(map == NULL || gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	bool canceled = false;
	size_t choice = parse_working_location(&canceled);
	if(canceled) return NULL;
	
	if(choice == WORKING_MAP){
		//TODO
	}else if(choice == WORKING_SET){
		size_t obj_index = parse_index("Node index",&canceled);
		if(canceled || !valid_gws_object(gws,obj_index,GWO_NODE,ctx)) return NULL;
		
		gwo_t obj = get_gwo_from_gws(gws,obj_index,ctx);
		return obj.node;
	}
	
	return NULL;
}

static mpo_t * fetch_mpo(map_t * map,gws_t * gws,err_ctx_t * ctx){
	if(map == NULL || gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	bool canceled = false;
	size_t choice = parse_working_location(&canceled);
	if(canceled) return NULL;
	
	if(choice == WORKING_MAP){
		//TODO
	}else if(choice == WORKING_SET){
		size_t obj_index = parse_index("MPO index",&canceled);
		if(canceled || !valid_gws_object(gws,obj_index,GWO_MPO,ctx)) return NULL;
		
		gwo_t obj = get_gwo_from_gws(gws,obj_index,ctx);
		return obj.mpo;
	}
	
	return NULL;
}

static void set_node_property_command(map_t * map,gws_t * gws,err_ctx_t * ctx){
	if(map == NULL || gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	
	map_node_t * node = fetch_node(map,gws,ctx);
	if(node == NULL) return;
	
	map_node_to_output_stream(node,0,stdout,ctx);
	
	const size_t n_options = 10;
	const char * options[n_options] = {
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
	size_t chosen_option = parse_among_options("Choose which property to edit",options,n_options,&canceled);
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
		//TODO
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
	fputc('\n',stdout);
}

static void create_building_command(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	fputs("Building name ",stdout);
	char * building_name = read_line();
	
	size_t rect_index = parse_index("Bounding Box Rectangle index",&canceled);
	if(canceled || !valid_gws_object(gws,rect_index,GWO_RECT,ctx)){
		free(building_name);
		return;
	}
	
	size_t n_floors = parse_index_in_range("Number of Floors (0,64)",0,64,&canceled);
	if(canceled){
		free(building_name);
		return;
	}
	
	gwo_t removed = remove_gwo_from_gws(gws,rect_index,ctx);
	map_rect_t building_rect = removed.rect;
	
	add_gwo_to_gws(gws,create_building_gwo( create_building(building_name,building_rect,n_floors,ctx),ctx),ctx);
	
	free(building_name);
}

static void set_mpo_property_command(map_t * map,gws_t * gws,err_ctx_t * ctx){
	if(map == NULL || gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	bool canceled = false;
	
	mpo_t * mpo = fetch_mpo(map,gws,ctx);
	if(mpo == NULL) return;
	
	mpo_to_output_stream(mpo,0,stdout,ctx);
	
	const size_t n_options = 4;
	const char * options[n_options] = {
		"Edit Coordinate",
		"Set Name",
		"Set Type",
		"Clear Name"
	};
	
	size_t chosen_option = parse_among_options("Choose which property to edit",options,n_options,&canceled);
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
	fputc('\n',stdout);
}

void start_cli(){
	
	err_ctx_t err_ctx = create_err_ctx();
	map_t working_map = init_map();
	
	gws_t working_set = init_generic_working_set();
	
	bool running = true;
	while(running){
		//clear_terminal_screen();
		gws_to_output_stream(working_set,stdout,&err_ctx);
		
		char * line = read_line();
		char * lowercase_line = (char*) malloc(strlen(line)+1);
		strcpy(lowercase_line,line);
		c_str_lowercase(lowercase_line);
		
		if(is_halting_string(lowercase_line)){
			running = false;
		}else{
			size_t n_tokens = 0;
			char ** tokens = split_into_tokens(lowercase_line,&n_tokens);
			
			if(n_tokens == 3){
				if(strcmp(tokens[0],"set") == 0){
					if(strcmp(tokens[1],"node") == 0 && (strcmp(tokens[2],"property") == 0 || strcmp(tokens[2],"prop") == 0)){
						set_node_property_command(&working_map,&working_set,&err_ctx);
					}else if(strcmp(tokens[1],"mpo") == 0 && (strcmp(tokens[2],"property") == 0 || strcmp(tokens[2],"prop") == 0)){
						set_mpo_property_command(&working_map,&working_set,&err_ctx);
					}
				}
			}else if(n_tokens == 2){
				
				if(strcmp(tokens[0],"create") == 0){
					if(strcmp(tokens[1],"cord") == 0){
						create_cord_command(&working_set,&err_ctx);
					}else if(strcmp(tokens[1],"rect") == 0){
						create_rect_command(&working_set,&err_ctx);
					}else if(strcmp(tokens[1],"mpo") == 0){
						create_mpo_command(&working_set,&err_ctx);
					}else if(strcmp(tokens[1],"node") == 0){
						create_node_command(&working_set,&err_ctx);
					}else if(strcmp(tokens[1],"building") == 0 || strcmp(tokens[1],"build") == 0){
						create_building_command(&working_set,&err_ctx);
					}
				}else if(strcmp(tokens[0],"show") == 0){
					if(strcmp(tokens[1],"map") == 0){
						show_map_command(working_map,&err_ctx);
					}
				}else if(strcmp(tokens[0],"add") == 0){
					if(strcmp(tokens[1],"node") == 0){
						add_node_to_map_command(&working_map,&working_set,&err_ctx);
					}
				}
			}else if(n_tokens == 1){
				if(is_deleting_string(tokens[0])){
					delete_command(&working_set,&err_ctx);
				}else if(strcmp(tokens[0],"clear") == 0){
					if(parse_confirmation("Are you Sure?")) clear_gws_objects(&working_set,&err_ctx);
					else fputs("Not cleared.\n",stdout);
				}
			}
			
			delete_tokens(tokens,n_tokens);
		}
		free(lowercase_line);
		free(line);
		
		errs_to_output_stream(&err_ctx,stdout);
		reset_err_ctx(&err_ctx);
		
		if(running){
			fputs("Press ENTER to continue\n",stdout);
			getc(stdin);
		}
	}
	
	deinit_generic_working_set(&working_set,&err_ctx);
	deinit_map(&working_map,&err_ctx);
}