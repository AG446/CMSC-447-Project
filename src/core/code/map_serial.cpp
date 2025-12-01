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
	size_t string_len = convert_binary_to_size_t(bytes+at,n_bytes-at,&at);
	
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

uint8_t * convert_string_array_to_binary(char ** strings,size_t n_strings,size_t * n_bytes_out){
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

char ** convert_binary_to_string_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_strings_out){
	size_t at = 0;
	
	size_t n_strings_n_bytes_read = 0;
	size_t n_strings = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_strings_n_bytes_read);
	at += n_strings_n_bytes_read;
	
	*n_strings_out = n_strings;
	
	char ** out = (char**) malloc(sizeof(char*) * n_strings);
	for(size_t i = 0;i < n_strings;i++) out[i] = create_empty_string();//set dummy strings
	
	if(n_strings_n_bytes_read == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	for(size_t i = 0;i < n_strings;i++){
		
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

cord_t * convert_binary_to_cord_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_cords_out){
	size_t at = 0;
	
	size_t n_cords_size_t_n_bytes_read = 0;
	size_t n_cords = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_cords_size_t_n_bytes_read);
	at += n_cords_size_t_n_bytes_read;
	
	*n_cords_out = n_cords;
	
	cord_t * out = (cord_t*) malloc(sizeof(cord_t) * n_cords);
	for(size_t i = 0;i < n_cords;i++) out[i] = create_cord(0.0,0.0);//set dummy values
	
	if(n_cords_size_t_n_bytes_read == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	for(size_t i = 0;i < n_cords;i++){
		
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
	
	if(n_bytes_read_for_cords == 0){
		*bytes_read_out = 0;
		free(cord_arr);
		return NULL;
	}
	
	uint8_t type = bytes[at];
	at += 1;
	
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
	uint8_t * names_bytes = convert_string_array_to_binary(building->possible_names,building->n_possible_names,&names_n_bytes);
	
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
	
	if(bytes_read_for_rect == 0){
		*bytes_read_out = 0;
		return NULL;
	}
	
	size_t n_bytes_read_for_names = 0;
	size_t n_names = 0;
	char ** names = convert_binary_to_string_array(bytes+at,n_bytes-at,&n_bytes_read_for_names,&n_names);
	at += n_bytes_read_for_names;
	
	if(n_bytes_read_for_names == 0){
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

struct Edge_Representation convert_binary_to_edge_representation(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,err_ctx_t * ctx){
	struct Edge_Representation out;
	out.index_a = 0;
	out.index_b = 0;
	out.type = 0;
	
	size_t at = 0;
	
	size_t n_bytes_read_for_index_a = 0;
	size_t index_a = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_index_a);
	at += n_bytes_read_for_index_a;
	
	if(n_bytes_read_for_index_a == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	size_t n_bytes_read_for_index_b = 0;
	size_t index_b = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_index_b);
	at += n_bytes_read_for_index_b;
	
	if(n_bytes_read_for_index_b == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	uint8_t type = bytes[at];
	at++;
	
	*bytes_read_out = at;
	
	out.index_a = index_a;
	out.index_b = index_b;
	out.type = type;
	
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
	
	if(n_bytes_read_for_cord == 0){
		*bytes_read_out = 0;
		return NULL;
	}
	
	size_t n_bytes_read_for_name = 0;
	char * name = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_name);
	at += n_bytes_read_for_name;
	
	if(n_bytes_read_for_name == 0){
		*bytes_read_out = 0;
		free(name);
		return NULL;
	}
	
	size_t n_bytes_read_for_file_name = 0;
	char * file_name = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_file_name);
	at += n_bytes_read_for_file_name;
	
	if(n_bytes_read_for_file_name == 0){
		*bytes_read_out = 0;
		free(name);
		free(file_name);
		return NULL;
	}
	
	bool has_building = bytes[at] == 1;
	at++;
	
	if(n_bytes_read_for_file_name == 0){
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

uint8_t * convert_node_array_to_binary(map_node_t ** nodes,size_t n_nodes,size_t * n_bytes_out){
	size_t total_byte_count = 0;
	
	size_t n_nodes_size_t_n_bytes = 0;
	uint8_t * n_nodes_size_t_bytes = convert_size_t_to_binary(n_nodes,&n_nodes_size_t_n_bytes);
	total_byte_count += n_nodes_size_t_n_bytes;
	
	size_t * nodes_n_bytes = (size_t*) malloc(sizeof(size_t) * n_nodes);
	uint8_t ** nodes_bytes = (uint8_t**) malloc(sizeof(uint8_t*) * n_nodes);
	
	for(size_t i = 0;i < n_nodes;i++){
		nodes_bytes[i] = convert_node_to_binary(nodes[i],&(nodes_n_bytes[i]));
		total_byte_count += nodes_n_bytes[i];
	}
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	*n_bytes_out = total_byte_count;
	size_t at = 0;
	
	memcpy(out+at,n_nodes_size_t_bytes,n_nodes_size_t_n_bytes);
	at += n_nodes_size_t_n_bytes;
	
	for(size_t i = 0;i < n_nodes;i++){
		uint8_t * current_node_bytes = nodes_bytes[i];
		size_t current_node_n_bytes = nodes_n_bytes[i];
		memcpy(out+at,current_node_bytes,current_node_n_bytes);
		at += current_node_n_bytes;
		free(current_node_bytes);
	}
	
	free(n_nodes_size_t_bytes);
	free(nodes_n_bytes);
	free(nodes_bytes);
	
	return out;
}

map_node_t ** convert_binary_to_node_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,building_t ** buildings_ref,size_t * n_nodes_out,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_nodes_n_bytes_read = 0;
	size_t n_nodes = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_nodes_n_bytes_read);
	at += n_nodes_n_bytes_read;
	
	*n_nodes_out = n_nodes;
	
	
	map_node_t ** out = (map_node_t**) malloc(sizeof(map_node_t*) * n_nodes);
	for(size_t i = 0;i < n_nodes;i++) out[i] = NULL;
	
	if(n_nodes_n_bytes_read == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	for(size_t i = 0;i < n_nodes;i++){
		
		size_t n_bytes_read_for_node = 0;
		map_node_t * node = convert_binary_to_node(bytes+at,n_bytes-at,&n_bytes_read_for_node,buildings_ref,ctx);
		
		if(n_bytes_read_for_node == 0){
			*bytes_read_out = 0;
			return out;
		}
		
		out[i] = node;
		
		at+=n_bytes_read_for_node;
	}
	
	*bytes_read_out = at;
	
	return out;
}

uint8_t * convert_mpo_array_to_binary(mpo_t ** mpos,size_t n_mpos,size_t * n_bytes_out){
	size_t total_byte_count = 0;
	
	size_t n_bytes_for_size = 0;
	uint8_t * size_bytes = convert_size_t_to_binary(n_mpos,&n_bytes_for_size);
	total_byte_count += n_bytes_for_size;
	
	size_t * mpos_n_bytes = (size_t*) malloc(sizeof(size_t) * n_mpos);
	uint8_t ** mpos_bytes = (uint8_t**) malloc(sizeof(uint8_t*) * n_mpos);
	
	for(size_t i = 0;i < n_mpos;i++){
		mpos_bytes[i] = convert_mpo_to_binary(mpos[i],&(mpos_n_bytes[i]));
		total_byte_count += mpos_n_bytes[i];
	}
	
	*n_bytes_out = total_byte_count;
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	
	memcpy(out+at,size_bytes,n_bytes_for_size);
	at += n_bytes_for_size;
	
	for(size_t i = 0;i < n_mpos;i++){
		size_t current_mpo_n_bytes = mpos_n_bytes[i];
		uint8_t * current_mpo_bytes = mpos_bytes[i];
		memcpy(out+at,current_mpo_bytes,current_mpo_n_bytes);
		at += current_mpo_n_bytes;
		free(current_mpo_bytes);
	}
	
	free(size_bytes);
	free(mpos_n_bytes);
	free(mpos_bytes);
	
	return out;
}

mpo_t ** convert_binary_to_mpo_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_mpos_out,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_bytes_read_for_n_mpos = 0;
	size_t n_mpos = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_n_mpos);
	at += n_bytes_read_for_n_mpos;
	
	*n_mpos_out = n_mpos;
	
	mpo_t ** out = (mpo_t**) malloc(sizeof(mpo_t*) * n_mpos);
	for(size_t i = 0;i < n_mpos;i++) out[i] = NULL;
	
	if(n_bytes_read_for_n_mpos == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	for(size_t i = 0;i < n_mpos;i++){
		
		size_t n_bytes_read_for_mpo = 0;
		mpo_t * mpo = convert_binary_to_mpo(bytes+at,n_bytes-at,&n_bytes_read_for_mpo,ctx);
		
		if(n_bytes_read_for_mpo == 0){
			*bytes_read_out = 0;
			return out;
		}
		
		out[i] = mpo;
		
		at+=n_bytes_read_for_mpo;
	}
	
	*bytes_read_out = at;
	
	return out;
}

uint8_t * convert_building_array_to_binary(building_t ** buildings,size_t n_buildings,size_t * n_bytes_out){
	size_t total_byte_count = 0;
	
	size_t n_bytes_for_size = 0;
	uint8_t * size_bytes = convert_size_t_to_binary(n_buildings,&n_bytes_for_size);
	total_byte_count += n_bytes_for_size;
	
	size_t * buildings_n_bytes = (size_t*) malloc(sizeof(size_t) * n_buildings);
	uint8_t ** buildings_bytes = (uint8_t**) malloc(sizeof(uint8_t*) * n_buildings);
	
	for(size_t i = 0;i < n_buildings;i++){
		buildings_bytes[i] = convert_building_to_binary(buildings[i],&(buildings_n_bytes[i]));
		total_byte_count += buildings_n_bytes[i];
	}
	
	*n_bytes_out = total_byte_count;
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	
	memcpy(out+at,size_bytes,n_bytes_for_size);
	at += n_bytes_for_size;
	
	for(size_t i = 0;i < n_buildings;i++){
		size_t current_building_n_bytes = buildings_n_bytes[i];
		uint8_t * current_building_bytes = buildings_bytes[i];
		memcpy(out+at,current_building_bytes,current_building_n_bytes);
		at += current_building_n_bytes;
		free(current_building_bytes);
	}
	
	free(size_bytes);
	free(buildings_n_bytes);
	free(buildings_bytes);
	
	return out;
}

building_t ** convert_binary_to_building_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_buildings_out,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_bytes_read_for_n_buildings = 0;
	size_t n_buildings = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_n_buildings);
	at += n_bytes_read_for_n_buildings;
	
	*n_buildings_out = n_buildings;
	
	building_t ** out = (building_t**) malloc(sizeof(building_t*) * n_buildings);
	for(size_t i = 0;i < n_buildings;i++) out[i] = NULL;
	
	if(n_bytes_read_for_n_buildings == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	for(size_t i = 0;i < n_buildings;i++){
		
		size_t n_bytes_read_for_building = 0;
		building_t * building = convert_binary_to_building(bytes+at,n_bytes-at,&n_bytes_read_for_building,ctx);
		
		if(n_bytes_read_for_building == 0){
			*bytes_read_out = 0;
			return out;
		}
		
		out[i] = building;
		
		at+=n_bytes_read_for_building;
	}
	
	*bytes_read_out = at;
	
	return out;
}

uint8_t * convert_edge_array_to_binary(map_edge_t ** edges,size_t n_edges,size_t * n_bytes_out){
	size_t total_byte_count = 0;
	
	size_t n_bytes_for_size = 0;
	uint8_t * size_bytes = convert_size_t_to_binary(n_edges,&n_bytes_for_size);
	total_byte_count += n_bytes_for_size;
	
	size_t * edges_n_bytes = (size_t*) malloc(sizeof(size_t) * n_edges);
	uint8_t ** edges_bytes = (uint8_t**) malloc(sizeof(uint8_t*) * n_edges);
	
	for(size_t i = 0;i < n_edges;i++){
		edges_bytes[i] = convert_edge_to_binary(edges[i],&(edges_n_bytes[i]));
		total_byte_count += edges_n_bytes[i];
	}
	
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	
	memcpy(out+at,size_bytes,n_bytes_for_size);
	at += n_bytes_for_size;
	
	for(size_t i = 0;i < n_edges;i++){
		size_t current_edge_n_bytes = edges_n_bytes[i];
		uint8_t * current_edge_bytes = edges_bytes[i];
		memcpy(out+at,current_edge_bytes,current_edge_n_bytes);
		at += current_edge_n_bytes;
		free(current_edge_bytes);
	}
	
	free(size_bytes);
	free(edges_n_bytes);
	free(edges_bytes);
	
	return out;
}

struct Edge_Representation * convert_binary_to_edge_representation_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_edges_out,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_bytes_read_for_n_edges = 0;
	size_t n_edges = convert_binary_to_size_t(bytes+at,n_bytes-at,&n_bytes_read_for_n_edges);
	at += n_bytes_read_for_n_edges;
	
	*n_edges_out = n_edges;
	
	struct Edge_Representation * out = (struct Edge_Representation *) malloc(sizeof(struct Edge_Representation) * n_edges);
	for(size_t i = 0;i < n_edges;i++){
		struct Edge_Representation blank;
		blank.index_a = 0;
		blank.index_b = 0;
		blank.type = 0;
		out[i] = blank;
	}
	
	if(n_bytes_read_for_n_edges == 0){
		*bytes_read_out = 0;
		return out;
	}
	
	for(size_t i = 0;i < n_edges;i++){
		
		size_t n_bytes_read_for_edge = 0;
		struct Edge_Representation edge = convert_binary_to_edge_representation(bytes+at,n_bytes-at,&n_bytes_read_for_edge,ctx);
		
		if(n_bytes_read_for_edge == 0){
			*bytes_read_out = 0;
			return out;
		}
		
		out[i] = edge;
		
		at+=n_bytes_read_for_edge;
	}
	
	*bytes_read_out = at;
	
	return out;
}

uint8_t * convert_map_to_binary(map_t * map,size_t * n_bytes_out){
	for(size_t i = 0;i < map->n_nodes;i++) map->all_nodes[i]->index_temp = i;
	for(size_t i = 0;i < map->n_buildings;i++) map->all_buildings[i]->index_temp = i;
	
	size_t total_byte_count = 0;
	
	size_t n_bytes_for_buildings = 0;
	uint8_t * buildings_bytes = convert_building_array_to_binary(map->all_buildings,map->n_buildings,&n_bytes_for_buildings);
	total_byte_count += n_bytes_for_buildings;
	
	size_t n_bytes_for_nodes = 0;
	uint8_t * nodes_bytes = convert_node_array_to_binary(map->all_nodes,map->n_nodes,&n_bytes_for_nodes);
	total_byte_count += n_bytes_for_nodes;
	
	size_t n_bytes_for_edges = 0;
	uint8_t * edges_bytes = convert_edge_array_to_binary(map->all_edges,map->n_edges,&n_bytes_for_edges);
	total_byte_count += n_bytes_for_edges;
	
	size_t n_bytes_for_mpos = 0;
	uint8_t * mpos_bytes = convert_mpo_array_to_binary(map->all_mpos,map->n_mpos,&n_bytes_for_mpos);
	total_byte_count += n_bytes_for_mpos;
	
	*n_bytes_out = total_byte_count;
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	size_t at = 0;
	
	memcpy(out+at,buildings_bytes,n_bytes_for_buildings);
	at += n_bytes_for_buildings;
	
	memcpy(out+at,nodes_bytes,n_bytes_for_nodes);
	at += n_bytes_for_nodes;
	
	memcpy(out+at,edges_bytes,n_bytes_for_edges);
	at += n_bytes_for_edges;
	
	memcpy(out+at,mpos_bytes,n_bytes_for_mpos);
	at += n_bytes_for_mpos;
	
	free(buildings_bytes);
	free(nodes_bytes);
	free(edges_bytes);
	free(mpos_bytes);
	
	return out;
}

map_t init_map_from_binary(const uint8_t * bytes,size_t n_bytes,err_ctx_t * ctx){
	size_t at = 0;
	
	size_t n_bytes_read_for_buildings = 0;
	size_t n_buildings = 0;
	building_t ** buildings = convert_binary_to_building_array(bytes+at,n_bytes-at,&n_bytes_read_for_buildings,&n_buildings,ctx);
	at += n_bytes_read_for_buildings;
	
	for(size_t i = 0;i < n_buildings;i++){
		buildings[i]->index_temp = i;
	}
	
	size_t n_bytes_read_for_nodes = 0;
	size_t n_nodes = 0;
	map_node_t ** nodes = convert_binary_to_node_array(bytes+at,n_bytes-at,&n_bytes_read_for_nodes,buildings,&n_nodes,ctx);
	at += n_bytes_read_for_nodes;
	
	for(size_t i = 0;i < n_nodes;i++){
		nodes[i]->index_temp = i;
	}
	
	size_t n_bytes_read_for_edges = 0;
	size_t n_edges = 0;
	struct Edge_Representation * edges = convert_binary_to_edge_representation_array(bytes+at,n_bytes-at,&n_bytes_read_for_edges,&n_edges,ctx);
	at += n_bytes_read_for_edges;
	
	size_t n_bytes_read_for_mpos = 0;
	size_t n_mpos = 0;
	mpo_t ** mpos = convert_binary_to_mpo_array(bytes+at,n_bytes-at,&n_bytes_read_for_mpos,&n_mpos,ctx);
	
	map_t out = init_map();
	
	for(size_t i = 0;i < n_buildings;i++){
		add_building_to_map(&out,buildings[i],ctx);
	}
	free(buildings);
	
	for(size_t i = 0;i < n_nodes;i++){
		add_node_to_map(&out,nodes[i],ctx);
	}
	free(nodes);
	
	for(size_t i = 0;i < n_edges;i++){
		connect_nodes_in_map_by_indices(&out,edges[i].index_a,edges[i].index_b,edges[i].type,ctx);
	}
	free(edges);
	
	for(size_t i = 0;i < n_mpos;i++){
		add_mpo_to_map(&out,mpos[i],ctx);
	}
	free(mpos);
	
	if(n_nodes > 0){
		out.scaling_y_factor = calculate_scale_y_factor(out.all_nodes[0]->coordinate);
	}else{
		out.scaling_y_factor = 1.0;
	}
	
	return out;
}

void save_map_to_file(map_t * map,const char * file_name){
	FILE * write_ptr = fopen(file_name,"wb");
	if(write_ptr == NULL) return;
	
	size_t map_buffer_size = 0;
	uint8_t * map_buffer = convert_map_to_binary(map,&map_buffer_size);
	
	fwrite(map_buffer,map_buffer_size,1,write_ptr);
	
	free(map_buffer);
	fclose(write_ptr);
}

map_t load_map_from_file(const char * file_name,err_ctx_t * ctx){
	FILE * read_ptr = fopen(file_name,"rb");
	if(read_ptr == NULL) return init_map();
	
	size_t file_size =  0;
	fseek(read_ptr, 0, SEEK_END);
	file_size = ftell(read_ptr);
	fseek(read_ptr, 0, SEEK_SET);
	
	uint8_t * map_buffer = (uint8_t*) malloc(file_size);
	
	fread(map_buffer,file_size,1,read_ptr);
	
	map_t out = init_map_from_binary(map_buffer,file_size,ctx);
	free(map_buffer);
	fclose(read_ptr);
	
	return out;
}