#include "map_obj_arr.h"

#define DEFAULT_GWS_CAPACITY 4

map_obj_t init_null_map_obj(void){
	map_obj_t out;
	out.type = MO_TYPE_NULL;
	out.cord = create_cord(0.0,0.0);
	return out;
}

map_obj_t init_cord_map_obj(cord_t cord){
	map_obj_t out;
	out.type = MO_TYPE_CORD;
	out.cord = cord;
	return out;
}


map_obj_t init_rect_map_obj(map_rect_t rect){
	map_obj_t out;
	out.type = MO_TYPE_RECT;
	out.rect = rect;
	return out;
}

map_obj_t init_mpo_map_obj(mpo_t * mpo,err_ctx_t * ctx){
	if(mpo == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return init_null_map_obj();
	}
	
	map_obj_t out;
	
	out.type = MO_TYPE_MPO;
	out.mpo = mpo;
	
	return out;
}

map_obj_t init_node_map_obj(map_node_t * node,err_ctx_t * ctx){
	if(node == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return init_null_map_obj();
	}
	
	map_obj_t out;
	
	out.type = MO_TYPE_NODE;
	out.node = node;
	
	return out;
}

map_obj_t init_building_map_obj(building_t * building,err_ctx_t * ctx){
	if(building == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return init_null_map_obj();
	}
	
	map_obj_t out;
	
	out.type = MO_TYPE_BUILDING;
	out.building = building;
	
	return out;
}

void deinit_map_obj(map_obj_t * obj,err_ctx_t * ctx){
	if(obj == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(obj->type == MO_TYPE_MPO){
		delete_mpo(obj->mpo,ctx);
	}else if(obj->type == MO_TYPE_NODE){
		delete_map_node(obj->node,ctx);
	}else if(obj->type == MO_TYPE_BUILDING){
		delete_building(obj->building,ctx);
	}
}






map_obj_arr_t init_map_obj_arr(void){
	map_obj_arr_t out;
	
	out.objects_capacity = DEFAULT_GWS_CAPACITY;
	out.n_objects = 0;
	out.objects = (map_obj_t*) malloc(sizeof(map_obj_t) * out.objects_capacity);
	
	return out;
}

void deinit_map_obj_arr(map_obj_arr_t * arr,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	for(size_t i = 0;i < arr->n_objects;i++){
		deinit_map_obj(&arr->objects[i],ctx);
	}
	free(arr->objects);
}

void clear_map_obj_arr(map_obj_arr_t * arr,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	for(size_t i = 0;i < arr->n_objects;i++){
		deinit_map_obj(&arr->objects[i],ctx);
	}
	arr->n_objects = 0;
}

void add_map_obj_to_map_obj_arr(map_obj_arr_t * arr,map_obj_t obj,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	if(arr->n_objects == arr->objects_capacity){
		arr->objects_capacity *= 2;
		arr->objects = (map_obj_t*) realloc(arr->objects,sizeof(map_obj_t) * arr->objects_capacity);
	}
	
	arr->objects[arr->n_objects] = obj;
	arr->n_objects++;
}

map_obj_t remove_map_obj_from_map_obj_arr(map_obj_arr_t * arr,size_t index,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return init_null_map_obj();
	}
	if(!(index < arr->n_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return init_null_map_obj();
	}
	
	map_obj_t out = arr->objects[index];
	
	//shift over data
	for(size_t i = index;i < arr->n_objects-1;i++){
		arr->objects[i] = arr->objects[i+1];
	}
	arr->n_objects--;//shrink array
	
	return out;
}

const map_obj_t get_map_obj_from_map_obj_arr(map_obj_arr_t * arr,size_t index,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return init_null_map_obj();
	}
	if(!(index < arr->n_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return init_null_map_obj();
	}
	
	return arr->objects[index];
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

map_obj_t * remove_map_objs_from_obj_arr(map_obj_arr_t * arr,size_t * indexes,size_t n_indexes,err_ctx_t * ctx){
	if(arr == NULL || indexes == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return NULL;
	}
	for(size_t i = 0;i < n_indexes;i++){
		if(!(indexes[i] < arr->n_objects)){
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
	
	//check for duplicate indexes
	for(size_t i = 1;i < n_indexes;i++){
		if(indexes_sorted[i-1].index == indexes_sorted[i].index){//duplicate index!
			ctx->flags |= ERROR_DUPLICATE_PARAMETER;
			free(indexes_sorted);
			return NULL;
		}
	}
	
	map_obj_t * out = (map_obj_t*) malloc(sizeof(map_obj_t)*n_indexes);
	
	size_t read_index = 0;
	size_t write_index = 0;
	size_t meta_deletion_index = 0;
	
	while(read_index < arr->n_objects){
		struct Index_Dual deletion_index = indexes_sorted[meta_deletion_index];
		
		if(read_index == deletion_index.index){
			out[deletion_index.position] = arr->objects[read_index];
			meta_deletion_index++;
		}else{
			arr->objects[write_index] = arr->objects[read_index];
			write_index++;
		}
		
		read_index++;
	}
	free(indexes_sorted);
	arr->n_objects -= n_indexes;
	return out;
}

void delete_map_obj_from_map_obj_arr(map_obj_arr_t * arr,size_t index,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	if(!(index < arr->n_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return;
	}
	
	map_obj_t captured_gwo = remove_map_obj_from_map_obj_arr(arr,index,ctx);
	
	deinit_map_obj(&captured_gwo,ctx);
}

bool verify_map_obj_in_map_obj_arr(map_obj_arr_t * arr,size_t index,uint8_t expected_type,err_ctx_t * ctx){
	if(arr == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	
	if(!(index < arr->n_objects)){
		ctx->flags |= ERROR_OUT_OF_BOUNDS_INDEX;
		return false;
	}
	
	if(arr->objects[index].type != expected_type){
		ctx->flags |= ERROR_INVALID_PARAM;
		return false;
	}
	
	return true;
}

void map_obj_arr_to_output_stream(const map_obj_arr_t arr,FILE * stream,err_ctx_t * ctx){
	if(stream == NULL){
		ctx->flags |= ERROR_INVALID_PARAM;
		return;
	}
	
	fputs("\033[36mCurrent Working Set\n\033[0m",stream);
	
	if(arr.n_objects == 0){
		fputs("Empty\n",stream);
	}
	
	for(size_t i = 0;i < arr.n_objects;i++){
		map_obj_t current = arr.objects[i];
		fprintf(stream,"Index: %lu\n",i);
		if(current.type == MO_TYPE_CORD){
			cord_to_output_stream(current.cord,1,stream,ctx);
		}else if(current.type == MO_TYPE_RECT){
			map_rect_to_output_stream(current.rect,1,stream,ctx);
		}else if(current.type == MO_TYPE_MPO){
			mpo_to_output_stream(current.mpo,1,stream,ctx);
		}else if(current.type == MO_TYPE_NODE){
			map_node_to_output_stream(current.node,1,stream,ctx);
		}else if(current.type == MO_TYPE_BUILDING){
			building_to_output_stream(current.building,1,stream,ctx);
		}
	}
}