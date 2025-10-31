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

char ** convert_binary_to_string_array(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_strings_out){
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

cord_t convert_binary_to_cord(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
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

map_rect_t convert_binary_to_rect(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
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

cord_t * convert_binary_to_cord_array(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_cords_out){
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

mpo_t * convert_binary_to_mpo(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out){
	mpo_t * out = (mpo_t*) malloc(sizeof(mpo_t));
	
	size_t at = 0;
	
	size_t n_cords = 0;
	size_t n_bytes_read_for_cords = 0;
	cord_t * cord_arr = convert_binary_to_cord_array(bytes+at,n_bytes-at,&n_bytes_read_for_cords,&n_cords);
	at += n_bytes_read_for_cords;
	
	if(n_bytes_read_for_cords == 0 || !(at < n_bytes)){
		free(out);
		return NULL;
	}
	
	uint8_t type = bytes[at];
	at += 1;
	
	if(!(at < n_bytes)){
		free(out);
		return NULL;
	}
	
	out->type = type;
	
	size_t n_bytes_read_for_name = 0;
	char * name = convert_binary_to_string(bytes+at,n_bytes-at,&n_bytes_read_for_name);
	at += n_bytes_read_for_name;
	
	if(n_bytes_read_for_name == 0){
		free(out);
		return NULL;
	}
	
	out->cords = cord_arr;
	out->n_cords = n_cords;
	
	if(strlen(name) == 0){
		out->name = NULL;
		free(name);
	}else{
		out->name = name;
	}
	
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
	
	uint8_t * out = (uint8_t*) malloc(total_byte_count);
	
	size_t at = 0;
	memcpy(out+at,rect_bytes,rect_n_bytes);
	
	//TODO
	
	return out;
}