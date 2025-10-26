#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cl_tool.h"
#include "text_proc.h"
#include "err_ctx.h"

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

gwo_t create_blank_gwo(){
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

void delete_gwo_data(gwo_t * gwo,err_ctx_t * ctx){
	if(gwo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(gwo->type == GWO_MPO){
		delete_mpo(gwo->mpo,ctx);
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
	
	return gws->working_object_arr[index].type == type;
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

void clear_generic_working_set(gws_t * gws,err_ctx_t * ctx){
	if(gws == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	for(size_t i = 0;i < gws->n_working_objects;i++){
		delete_gwo_data(&gws->working_object_arr[i],ctx);
	}
	free(gws->working_object_arr);
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

void gws_to_output_stream(const gws_t gws,FILE * stream,err_ctx_t * ctx){
	if(stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	fputs("### Current Working Set ###\n",stream);
	
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
		}
	}
}

char * read_line(){
	fputs("> ",stdout);
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

static bool is_halting_string(const char * str){
	if(str == NULL) return false;
	
	if(strcmp(str,"stop") == 0) return true;
	if(strcmp(str,"halt") == 0) return true;
	if(strcmp(str,"quit") == 0) return true;
	if(strcmp(str,"exit") == 0) return true;
	if(strcmp(str,"cancel") == 0) return true;
	
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

static cord_t parse_cord(bool * canceled){
	fputs("### Entering a Coordinate ###\n",stdout);
	
	float longitude = parse_double("Longitude",canceled);
	if(*canceled) return create_cord(0.0,0.0);
	
	float latitude = parse_double("Latitude",canceled);
	if(*canceled) return create_cord(0.0,0.0);
	
	return create_cord(longitude,latitude);
}

void start_cli(){
	
	err_ctx_t err_ctx = create_err_ctx();
	
	gws_t working_set = init_generic_working_set();
	
	bool running = true;
	while(running){
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
			
			if((n_tokens == 2) && (strcmp(tokens[0],"create") == 0)){
				if(strcmp(tokens[1],"cord") == 0){
					bool canceled = false;
					cord_t cord = parse_cord(&canceled);
					if(!canceled){
						add_gwo_to_gws(&working_set,create_cord_gwo(cord),&err_ctx);
					}
				}else if(strcmp(tokens[1],"rect") == 0){
					bool canceled = false;
					const char * msg = "Coordinte Object Index";
					size_t first_obj_index,second_obj_index,temp_index;
					gwo_t first_obj,second_obj,temp_obj;
					bool swap_made = false;
					
					first_obj_index = parse_index(msg,&canceled);
					if(canceled || !valid_gws_object(&working_set,first_obj_index,GWO_CORD,&err_ctx)) goto rect_skip;
					second_obj_index = parse_index(msg,&canceled);
					if(canceled || !valid_gws_object(&working_set,second_obj_index,GWO_CORD,&err_ctx)) goto rect_skip;
					
					if(first_obj_index == second_obj_index){
						err_ctx.flags |= ERROR_DUPLICATE_PARAMETER;
						goto rect_skip;
					}
					
					//the first index must be larger
					if(first_obj_index < second_obj_index){
						temp_index = first_obj_index;
						first_obj_index = second_obj_index;
						second_obj_index  = temp_index;
						swap_made = true;
					}
					
					first_obj = remove_gwo_from_gws(&working_set,first_obj_index,&err_ctx);
					second_obj = remove_gwo_from_gws(&working_set,second_obj_index,&err_ctx);
					
					//preserve order
					if(swap_made){
						temp_obj = first_obj;
						first_obj = second_obj;
						second_obj = temp_obj;
					}
					
					add_gwo_to_gws(&working_set,create_map_rect_gwo( create_map_rect(first_obj.cord,second_obj.cord) ),&err_ctx);
					
					rect_skip:;
					if(err_encountered(&err_ctx)) errs_to_output_stream(&err_ctx,stdout);
					reset_err_ctx(&err_ctx);
				}else{
					fputs("Invalid create option\n",stdout);
				}
			}
			
			delete_tokens(tokens,n_tokens);
		}
		free(lowercase_line);
		free(line);
	}
	
	clear_generic_working_set(&working_set,&err_ctx);
	errs_to_output_stream(&err_ctx,stdout);
}