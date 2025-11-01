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

char ** convert_binary_to_string_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_strings_out);

uint8_t * convert_cord_to_binary(cord_t cord,size_t * n_bytes_out);

cord_t convert_binary_to_cord(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);

uint8_t * convert_rect_to_binary(map_rect_t rect,size_t * n_bytes_out);

map_rect_t convert_binary_to_rect(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out);

uint8_t * convert_cord_array_to_binary(const cord_t * cords,size_t n_cords,size_t * n_bytes_out);

cord_t * convert_binary_to_cord_array(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,size_t * n_cords_out);

uint8_t * convert_mpo_to_binary(const mpo_t * mpo,size_t * n_bytes_out);

mpo_t * convert_binary_to_mpo(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,err_ctx_t * ctx);

uint8_t * convert_building_to_binary(const building_t * building,size_t * n_bytes_out);

building_t * convert_binary_to_building(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,err_ctx_t * ctx);

uint8_t * convert_edge_to_binary(const map_edge_t * edge,size_t * n_bytes_out);

map_edge_t * convert_binary_to_edge(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,map_node_t ** nodes_ref,err_ctx_t * ctx);

uint8_t * convert_node_to_binary(const map_node_t * node,size_t * n_bytes_out);

map_node_t * convert_binary_to_node(const uint8_t * bytes,size_t n_bytes,size_t * bytes_read_out,building_t ** buildings_ref,err_ctx_t * ctx);//TODO

#endif