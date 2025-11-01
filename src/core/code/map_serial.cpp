#include "map_serial.h"
#include <string.h>

uint8_t * convert_size_t_to_binary(size_t number,size_t * n_bytes_out){
	uint8_t * out = (uint8_t*) malloc(sizeof(size_t));
	memcpy(out,&number,sizeof(size_t));
	*n_bytes_out = sizeof(size_t);
	return out;
}

size_t convert_binary_to_size_t(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
	if(n_bytes < sizeof(size_t)){
		*bytes_read_out = 0;
		return 0;
	}
	
	size_t dest = 0;
	memcpy(&dest,bytes,sizeof(size_t));
	*bytes_read_out = sizeof(size_t);
	
	return dest;
}

uint8_t * convert_string_to_binary(const char * string,size_t * n_bytes_out){
	size_t string_len = strlen(string);
	
	size_t string_size_n_bytes = 0;
	uint8_t * string_size_bytes = convert_size_t_to_binary(string_len,&string_size_n_bytes);
	
	*n_bytes_out = string_size_n_bytes + string_len;
	
	uint8_t * out = (uint8_t*) malloc(*n_bytes_out);
	
	size_t at = 0;
	
	memcpy(out+at,string_size_bytes,string_size_n_bytes);
	at += string_size_n_bytes;
	free(string_size_bytes);
	
	memcpy(out+at,string,string_len);
	
	return out;
}

static char * create_empty_string(void){
	char * empty_out = (char*) malloc(1);
	empty_out[0] = '\0';
	return empty_out;
}

char * convert_binary_to_string(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
	if(n_bytes < sizeof(size_t)){
		*bytes_read_out = 0;
		return create_empty_string();
	}
	
	size_t at = 0;
	size_t string_len = convert_binary_to_size_t(bytes,sizeof(size_t),&at);
	
	if(n_bytes < at+string_len){
		*bytes_read_out = 0;
		return create_empty_string();
	}
	
	char * out = (char*) malloc(string_len+1);
	
	memcpy(out,bytes+at,string_len);
	out[string_len] = '\0';
	at += string_len;
	
	*bytes_read_out = at;
	
	return out;
}

uint8_t * convert_string_array_to_binary(const char ** strings,size_t n_strings,size_t * n_bytes_out){
	size_t total_byte_count = 0;
	
	size_t n_strings_size_t_n_bytes = 0;
	uint8_t * n_strings_size_t_bytes = convert_size_t_to_binary(n_strings,&n_strings_size_t_n_bytes);
	total_byte_count += n_strings_size_t_n_bytes;
	
	size_t * each_string_n_bytes = (size_t*) malloc(sizeof(size_t)*n_strings);
	uint8_t ** each_string_bytes = (uint8_t**) malloc(sizeof(uint8_t*)*n_strings);
	
	for(size_t i = 0;i < n_strings;i++){
		each_string_bytes[i] = convert_string_to_binary(strings[i],&(each_string_n_bytes[i]));
		total_byte_count += each_string_n_bytes[i];
	}
	
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	memcpy(out+at,n_strings_size_t_bytes,n_strings_size_t_n_bytes);
	at += n_strings_size_t_n_bytes;
	
	free(n_strings_size_t_bytes);
	
	for(size_t i = 0;i < n_strings;i++){		
		size_t current_string_n_bytes = each_string_n_bytes[i];
		uint8_t * current_string_bytes = each_string_bytes[i];
		memcpy(out+at,current_string_bytes,current_string_n_bytes);
		at += current_string_n_bytes;
		
		free(current_string_bytes);
	}
	
	free(each_string_n_bytes);
	free(each_string_bytes);
	
	return out;
}

static char ** create_empty_string_arr(void){
	char ** strings_arr_out = (char**) malloc(0);
	return strings_arr_out;
}

char ** convert_binary_to_string_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_strings_out){
	if(n_bytes < sizeof(size_t)){
		*bytes_read_out = 0;
		*n_strings_out = 0;
		return create_empty_string_arr();
	}
	
	size_t at = 0;
	
	size_t n_strings_n_bytes_read = 0;
	size_t n_strings = convert_binary_to_size_t(bytes+at,sizeof(size_t),&n_strings_n_bytes_read);
	at += n_strings_n_bytes_read;
	
	*n_strings_out = n_strings;
	
	char ** out = (char**) malloc(sizeof(char*) * n_strings);
	for(size_t i = 0;i < n_strings;i++) out[i] = create_empty_string();//set dummy strings
	
	for(size_t i = 0;i < n_strings;i++){
		if(!(at < n_bytes)){
			*bytes_read_out = 0;
			return out;
		}
		
		size_t n_bytes_read_for_string = 0;
		char * string = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_string);
		
		if(n_bytes_read_for_string == 0){
			free(string);
			*bytes_read_out = 0;
			return out;
		}
		
		free(out[i]);
		out[i] = string;
		
		at+=n_bytes_read_for_string;
	}
	
	*bytes_read_out = at;
	return out;
}


uint8_t * convert_cord_to_binary(cord_t cord,size_t * n_bytes_out){
	uint8_t * out = (uint8_t*) malloc(sizeof(cord_t));
	memcpy(out,&cord,sizeof(cord_t));
	*n_bytes_out = sizeof(cord_t);
	return out;
}

cord_t convert_binary_to_cord(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
	if(n_bytes < sizeof(cord_t)){
		*bytes_read_out = 0;
		return create_cord(0.0,0.0);
	}
	
	cord_t dest = create_cord(0.0,0.0);
	memcpy(&dest,bytes,sizeof(cord_t));
	*bytes_read_out = sizeof(cord_t);
	
	return dest;
}

uint8_t * convert_rect_to_binary(map_rect_t rect,size_t * n_bytes_out){
	uint8_t * out = (uint8_t*) malloc(sizeof(map_rect_t));
	memcpy(out,&rect,sizeof(map_rect_t));
	*n_bytes_out = sizeof(map_rect_t);
	return out;
}

map_rect_t convert_binary_to_rect(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
	if(n_bytes < sizeof(map_rect_t)){
		*bytes_read_out = 0;
		return create_map_rect(create_cord(0.0,0.0),create_cord(1.0,1.0));
	}
	
	map_rect_t dest = create_map_rect(create_cord(0.0,0.0),create_cord(1.0,1.0));
	memcpy(&dest,bytes,sizeof(map_rect_t));
	*bytes_read_out = sizeof(map_rect_t);
	
	return dest;
}

uint8_t * convert_cord_array_to_binary(const cord_t * cords,size_t n_cords,size_t * n_bytes_out){
	size_t n_cords_size_t_n_bytes = 0;
	uint8_t * n_cords_size_t_bytes = convert_size_t_to_binary(n_cords,&n_cords_size_t_n_bytes);
	
	size_t total_byte_count = 0;
	total_byte_count += n_cords_size_t_n_bytes;
	
	uint8_t ** each_cord_bytes = (uint8_t**) malloc(sizeof(uint8_t*)*n_cords);
	size_t * each_cord_n_bytes = (size_t*) malloc(sizeof(size_t)*n_cords);
	
	for(size_t i = 0;i < n_cords;i++){
		each_cord_bytes[i] = convert_cord_to_binary(cords[i],&(each_cord_n_bytes[i]));
		total_byte_count += each_cord_n_bytes[i];
	}
	
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	size_t at = 0;
	
	memcpy(out+at,n_cords_size_t_bytes,n_cords_size_t_n_bytes);
	free(n_cords_size_t_bytes);
	at += n_cords_size_t_n_bytes;
	
	for(size_t i = 0;i < n_cords;i++){
		uint8_t * current_cord_bytes = each_cord_bytes[i];
		size_t current_cord_n_bytes = each_cord_n_bytes[i];
		memcpy(out+at,current_cord_bytes,current_cord_n_bytes);
		free(current_cord_bytes);
		at += current_cord_n_bytes;
	}
	
	free(each_cord_bytes);
	free(each_cord_n_bytes);
	
	return out;
}

static cord_t * create_empty_cord_arr(){
	cord_t * out = (cord_t*) malloc(0);
	return out;
}

cord_t * convert_binary_to_cord_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_cords_out){
	if(n_bytes < sizeof(size_t)){
		*bytes_read_out = 0;
		*n_cords_out = 0;
		return create_empty_cord_arr();
	}
	
	size_t at = 0;
	
	size_t n_cords_size_t_n_bytes_read = 0;
	size_t n_cords = convert_binary_to_size_t(bytes+at,sizeof(size_t),&n_cords_size_t_n_bytes_read);
	at += n_cords_size_t_n_bytes_read;
	
	*n_cords_out = n_cords;
	
	cord_t * out = (cord_t*) malloc(sizeof(cord_t) * n_cords);
	for(size_t i = 0;i < n_cords;i++) out[i] = create_cord(0.0,0.0);//set dummy values
	
	for(size_t i = 0;i < n_cords;i++){
		if(!(at < n_bytes)){
			*bytes_read_out = 0;
			return out;
		}
		
		size_t n_bytes_read_for_cord = 0;
		cord_t cord = convert_binary_to_cord(bytes+at,n_bytes-at,&n_bytes_read_for_cord);
		
		if(n_bytes_read_for_cord == 0){
			*bytes_read_out = 0;
			return out;
		}
		
		out[i] = cord;
		
		at+=n_bytes_read_for_cord;
	}
	
	*bytes_read_out = at;
	
	return out;
}

uint8_t * convert_mpo_to_binary(const mpo_t * mpo,size_t * n_bytes_out){
	size_t cord_array_n_bytes = 0;
	uint8_t * cord_array_bytes = convert_cord_array_to_binary(mpo->cords,mpo->n_cords,&cord_array_n_bytes);
	
	uint8_t type_byte = mpo->type;
	
	size_t name_n_bytes = 0;
	
	uint8_t * name_bytes = NULL;
	if(mpo->name == NULL){
		name_bytes = convert_string_to_binary("",&name_n_bytes);
	}else{
		name_bytes = convert_string_to_binary(mpo->name,&name_n_bytes);
	}
	
	size_t total_byte_count = cord_array_n_bytes+1+name_n_bytes;
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	
	memcpy(out+at,cord_array_bytes,cord_array_n_bytes);
	at += cord_array_n_bytes;
	
	memcpy(out+at,&type_byte,1);
	at += 1;
	
	memcpy(out+at,name_bytes,name_n_bytes);
	
	free(cord_array_bytes);
	free(name_bytes);
	
	return out;
}

mpo_t * convert_binary_to_mpo(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,err_ctx_t * ctx){
	
	size_t at = 0;
	
	size_t n_cords = 0;
	size_t n_bytes_read_for_cords = 0;
	cord_t * cord_arr = convert_binary_to_cord_array(bytes+at,n_bytes-at,&n_bytes_read_for_cords,&n_cords);
	at += n_bytes_read_for_cords;
	
	if(n_bytes_read_for_cords == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		free(cord_arr);
		return NULL;
	}
	
	uint8_t type = bytes[at];
	at += 1;
	
	if(!(at < n_bytes)){
		*bytes_read_out = 0;
		free(cord_arr);
		return NULL;
	}
	
	size_t n_bytes_read_for_name = 0;
	char * name = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_name);
	at += n_bytes_read_for_name;
	
	if(n_bytes_read_for_name == 0){
		*bytes_read_out = 0;
		free(cord_arr);
		free(name);
		return NULL;
	}
	
	*bytes_read_out = at;
	
	if(strlen(name) == 0){
		free(name);
		name = NULL;
	}
	
	mpo_t * out = create_mpo(cord_arr,n_cords,type,ctx);
	if(name != NULL){
		set_mpo_name(out,name,ctx);
		free(name);
	}
	
	free(cord_arr);
	
	return out;
}

uint8_t * convert_building_to_binary(const building_t * building,size_t * n_bytes_out){
	size_t rect_n_bytes = 0;
	uint8_t * rect_bytes = convert_rect_to_binary(building->building_bounding_box,&rect_n_bytes);
	
	size_t names_n_bytes = 0;
	uint8_t * names_bytes = convert_string_array_to_binary((const char **)building->possible_names,building->n_possible_names,&names_n_bytes);
	
	uint8_t n_floors_byte = building->n_floors;
	
	size_t total_byte_count = 0;
	total_byte_count = rect_n_bytes + names_n_bytes + 1;
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	
	memcpy(out+at,rect_bytes,rect_n_bytes);
	at += rect_n_bytes;
	
	memcpy(out+at,names_bytes,names_n_bytes);
	at += names_n_bytes;
	
	out[at] = n_floors_byte;
	at += 1;
	
	free(rect_bytes);
	free(names_bytes);
	
	return out;
}

building_t * convert_binary_to_building(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t bytes_read_for_rect = 0;
	map_rect_t building_rect = convert_binary_to_rect(bytes+at,n_bytes-at,&bytes_read_for_rect);
	at += bytes_read_for_rect;
	
	if(bytes_read_for_rect == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		return NULL;
	}
	
	size_t n_bytes_read_for_names = 0;
	size_t n_names = 0;
	char ** names = convert_binary_to_string_array(bytes+at,n_bytes-at,&n_bytes_read_for_names,&n_names);
	at += n_bytes_read_for_names;
	
	if(n_bytes_read_for_names == 0 || !(at < n_bytes)){
		for(size_t i = 0;i < n_names;i++) free(names[i]);
		free(names);
		*bytes_read_out = 0;
		return NULL;
	}
	
	uint8_t n_floors = bytes[at];
	at++;
	
	*bytes_read_out = at;
	
	building_t * out = create_building(names[0],building_rect,n_floors,ctx);
	for(size_t i = 1;i < n_names;i++){
		add_building_alias_name(out,names[i],ctx);
	}
	
	for(size_t i = 0;i < n_names;i++) free(names[i]);
	free(names);
	
	return out;
}

uint8_t * convert_edge_to_binary(const map_edge_t * edge,size_t * n_bytes_out){
	size_t node_a_index = edge->a->index_temp;
	size_t node_b_index = edge->b->index_temp;
	
	size_t a_index_size_t_n_bytes = 0;
	uint8_t * a_index_size_t_bytes = convert_size_t_to_binary(node_a_index,&a_index_size_t_n_bytes);
	
	size_t b_index_size_t_n_bytes = 0;
	uint8_t * b_index_size_t_bytes = convert_size_t_to_binary(node_b_index,&b_index_size_t_n_bytes);
	
	uint8_t type_byte = edge->type;
	
	size_t total_byte_count = a_index_size_t_n_bytes + b_index_size_t_n_bytes + 1;
	
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	size_t at = 0;
	
	memcpy(out+at,a_index_size_t_bytes,a_index_size_t_n_bytes);
	at += a_index_size_t_n_bytes;
	
	memcpy(out+at,b_index_size_t_bytes,b_index_size_t_n_bytes);
	at += b_index_size_t_n_bytes;
	
	out[at] = type_byte;
	
	free(a_index_size_t_bytes);
	free(b_index_size_t_bytes);
	
	return out;
}

map_edge_t * convert_binary_to_edge(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,map_node_t ** nodes_ref,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_bytes_read_for_index_a = 0;
	size_t index_a = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_index_a);
	at += n_bytes_read_for_index_a;
	
	if(n_bytes_read_for_index_a == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		return NULL;
	}
	
	size_t n_bytes_read_for_index_b = 0;
	size_t index_b = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_index_b);
	at += n_bytes_read_for_index_b;
	
	if(n_bytes_read_for_index_b == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		return NULL;
	}
	
	uint8_t type = bytes[at];
	at++;
	
	*bytes_read_out = at;
	
	map_node_t * node_a = nodes_ref[index_a];
	map_node_t * node_b = nodes_ref[index_b];
	
	map_edge_t * out = create_map_edge(type,node_a,node_b,ctx);
	
	return out;
}

uint8_t * convert_node_to_binary(const map_node_t * node,size_t * n_bytes_out){
	//outgoing edges are not serialized, they are reconstructed during map load
	
	size_t n_bytes_for_cord = 0;
	uint8_t * cord_bytes = convert_cord_to_binary(node->coordinate,&n_bytes_for_cord);
	
	size_t n_bytes_for_name = 0;
	uint8_t * name_bytes = NULL;
	
	if(node->name == NULL){
		name_bytes = convert_string_to_binary("",&n_bytes_for_name);
	}else{
		name_bytes = convert_string_to_binary(node->name,&n_bytes_for_name);
	}
	
	size_t n_bytes_for_file_name = 0;
	uint8_t * file_name_bytes = NULL;
	
	if(node->picture_file_path == NULL){
		file_name_bytes = convert_string_to_binary("",&n_bytes_for_file_name);
	}else{
		file_name_bytes = convert_string_to_binary(node->picture_file_path,&n_bytes_for_file_name);
	}
	
	uint8_t has_building_byte = (node->associated_building == NULL) ? 0 : 1;
	
	size_t n_bytes_for_building_index = 0;
	uint8_t * bytes_for_building_index = NULL;
	if(node->associated_building != NULL){
		bytes_for_building_index = convert_size_t_to_binary(node->associated_building->index_temp,&n_bytes_for_building_index);
	}
	
	uint8_t is_selectable_byte = node->selectable ? 1 : 0;
	
	uint8_t floor_number_byte = node->floor_number;
	
	size_t total_byte_count = n_bytes_for_cord + n_bytes_for_name + n_bytes_for_file_name + 1 + n_bytes_for_building_index + 1 + 1;
	*n_bytes_out = total_byte_count;
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	size_t at = 0;
	
	memcpy(out+at,cord_bytes,n_bytes_for_cord);
	at += n_bytes_for_cord;
	
	memcpy(out+at,name_bytes,n_bytes_for_name);
	at += n_bytes_for_name;
	
	memcpy(out+at,file_name_bytes,n_bytes_for_file_name);
	at += n_bytes_for_file_name;
	
	out[at] = has_building_byte;
	at++;
	
	if(node->associated_building != NULL){
		memcpy(out+at,bytes_for_building_index,n_bytes_for_building_index);
		at += n_bytes_for_building_index;
	}
	
	out[at] = is_selectable_byte;
	at++;
	
	out[at] = floor_number_byte;
	
	free(cord_bytes);
	free(name_bytes);
	free(file_name_bytes);
	free(bytes_for_building_index);
	
	return out;
}

map_node_t * convert_binary_to_node(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,building_t ** buildings_ref,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_bytes_read_for_cord = 0;
	cord_t coordinate = convert_binary_to_cord(bytes+at,n_bytes-at,&n_bytes_read_for_cord);
	at += n_bytes_read_for_cord;
	
	if(n_bytes_read_for_cord == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		return NULL;
	}
	
	size_t n_bytes_read_for_name = 0;
	char * name = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_name);
	at += n_bytes_read_for_name;
	
	if(n_bytes_read_for_name == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		free(name);
		return NULL;
	}
	
	size_t n_bytes_read_for_file_name = 0;
	char * file_name = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_file_name);
	at += n_bytes_read_for_file_name;
	
	if(n_bytes_read_for_file_name == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		free(name);
		free(file_name);
		return NULL;
	}
	
	bool has_building = bytes[at] == 1;
	at++;
	
	if(n_bytes_read_for_file_name == 0 || !(at < n_bytes)){
		*bytes_read_out = 0;
		free(name);
		free(file_name);
		return NULL;
	}
	
	building_t * build = NULL;
	
	if(has_building){
		size_t n_bytes_read_for_building_index = 0;
		size_t building_index = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_building_index);
		at += n_bytes_read_for_building_index;
		build = buildings_ref[building_index];
		
		if(n_bytes_read_for_building_index == 0){
			*bytes_read_out = 0;
			free(name);
			free(file_name);
			return NULL;
		}
	}
	
	if(!(at+1 < n_bytes)){
		*bytes_read_out = 0;
		free(name);
		free(file_name);
		return NULL;
	}
	
	bool is_selectable = bytes[at] == 1;
	at++;
	
	uint8_t floor_number = bytes[at];
	at++;
	
	*bytes_read_out = at;
	
	map_node_t * out = create_map_node(coordinate);
	
	if(strlen(name) > 0){
		set_map_node_name(out,name,ctx);
	}
	free(name);
	
	if(strlen(file_name) > 0){
		set_map_node_picture(out,file_name,ctx);
	}
	free(file_name);
	
	if(build != NULL){
		set_map_node_building(out,build,ctx);
	}
	
	set_map_node_selectable(out,is_selectable,ctx);
	set_map_node_floor_number(out,floor_number,ctx);
	
	return out;
}