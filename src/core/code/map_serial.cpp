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
		return 0;
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

uint8_t * convert_double_to_binary(double number,size_t * n_bytes_out);//TODO

double convert_binary_to_double(uint8_t * bytes,size_t n_bytes);//TODO

uint8_t * convert_cord_to_binary(cord_t cord,size_t * n_bytes_out);//TODO

cord_t convert_binary_to_cord(uint8_t * bytes,size_t n_bytes);//TODO

uint8_t * convert_rect_to_binary(map_rect_t rect,size_t * n_bytes_out);//TODO

map_rect_t convert_binary_to_rect(uint8_t * bytes,size_t n_bytes);//TODO

uint8_t * convert_cord_array_to_binary(const cord_t * cords,size_t n_cords,size_t * n_bytes_out);//TODO

cord_t ** convert_binary_to_cord_array(uint8_t * bytes,size_t n_bytes,size_t * n_cords_out);//TODO