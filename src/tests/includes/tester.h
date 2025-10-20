#ifndef TESTER_H
#define TESTER_H

// This is where your map.h is located relative to the tests/includes directory
#include "../../src/core/includes/map.h" 

// Include the Google Test framework
#include <gtest/gtest.h>

// --- Test Constants (Based on map.cpp usage) ---
// These should ideally be defined in your actual map.h
#define EDGE_TYPE_SIDEWALK 1
#define EDGE_TYPE_ROAD 2
#define EDGE_TYPE_STAIRS 3
#define EDGE_TYPE_RAMP 4
#define EDGE_TYPE_HALLWAY 5
#define EDGE_TYPE_ELEVATOR_SHAFT 6
#define EDGE_TYPE_OVERPASS 7
#define EDGE_TYPE_DOOR 8
#define EDGE_TYPE_AUTO_DOOR 9
#define EDGE_TYPE_CROSSWALK 10
#define NODE_FLOOR_NUMBER_NONE -1
#define MPO_TYPE_WATER 1
#define MPO_TYPE_TREE 2
#define MPO_TYPE_BUILDING 3


// C linkage block for testing static functions (Requires removing 'static' 
// from these functions in map.cpp temporarily for linking)
extern "C" {
    // Utility helpers for string and serialization tests (if needed by your map.cpp)
    size_t count_tokens(const char * input);
    char * get_first_token(const char * input);
    char ** split_into_tokens(const char * input,size_t * n_tokens_out);
    void delete_tokens(char ** tokens,size_t n_tokens);
    float token_similarity_score(const char * a,const char * b);
    float phrase_similarity_score(const char * phrase_1,const char * phrase_2);
    uint8_t * convert_mpo_to_binary(const mpo_t * mpo,size_t * buffer_size);
    mpo_t * convert_binary_to_mpo(const uint8_t * buffer);
    uint8_t * convert_map_edge_to_binary(const map_edge_t * edge,map_node_t ** all_nodes,size_t * buffer_size);
    map_edge_t * convert_binary_to_map_edge(const uint8_t * buffer,map_node_t ** all_nodes);
}

// Utility function to create a basic node for setup
map_node_t* create_test_node(double lon, double lat);

#endif // TESTER_H
