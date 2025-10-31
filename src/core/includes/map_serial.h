#ifndef MAP_SERIAL_H
#define MAP_SERIAL_H

#include <stdint.h>
#include <stdlib.h>
#include "map.h"

uint8_t * convert_size_t_to_binary(size_t number,size_t * n_bytes_out);

size_t convert_binary_to_size_t(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);

uint8_t * convert_string_to_binary(const char * string,size_t * n_bytes_out);

char * convert_binary_to_string(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);

uint8_t * convert_string_array_to_binary(const char ** strings,size_t n_strings,size_t * n_bytes_out);

char ** convert_binary_to_string_array(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_strings_out);

uint8_t * convert_cord_to_binary(cord_t cord,size_t * n_bytes_out);

cord_t convert_binary_to_cord(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);

uint8_t * convert_rect_to_binary(map_rect_t rect,size_t * n_bytes_out);

map_rect_t convert_binary_to_rect(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);

uint8_t * convert_cord_array_to_binary(const cord_t * cords,size_t n_cords,size_t * n_bytes_out);

cord_t * convert_binary_to_cord_array(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_cords_out);

uint8_t * convert_mpo_to_binary(const mpo_t * mpo,size_t * n_bytes_out);

mpo_t * convert_binary_to_mpo(uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);


uint8_t * convert_building_to_binary(const building_t * building,size_t * n_bytes_out);//TODO

#endif