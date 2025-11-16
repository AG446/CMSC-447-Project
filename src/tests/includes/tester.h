#ifndef TESTER_H
#define TESTER_H

typedef struct Testable_Func test_func_t;

#define PASS true
#define FAIL false

#define SILENT true
#define VERBOSE false

struct Testable_Func{//dont re-order this please
	bool (*function)(bool silent);
	const char * function_name;
	bool silent;
};

//check if the fuzzy token matching algorithms work as intended
bool token_matching_test(bool silent);

//check if the fuzzy phrase matching algorithms work as intended
bool phrase_matching_test(bool silent);

//check if the basic serializtion functions work as intended
bool basic_serialization_test(bool silent);

//check if the serializtion functions work with reference objects
bool serialization_test(bool silent);

//test all the functions related to the building object
bool building_data_structure_test(bool silent);

//test all the functions related to the map polygon object
bool mpo_data_structure_test(bool silent);

//test all the functions related to the map node object
bool map_node_data_structure_test(bool silent);

//test all the functions related to the map edge object
bool map_edge_data_structure_test(bool silent);

/*
 * Test all the basic functions related to the map object.
 * Test adding and remmoval of objects within the map and testing 
 * simple property retreival related stuff.
 */
bool basic_map_data_structure_test(bool silent);

//test the A* algorithm
bool find_path_test(bool silent);

// test the output text
bool path_string_test(bool silent);

// test the output text
bool path_string_test2(bool silent);

#endif
