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

bool building_data_structure_test(bool silent);
bool mpo_data_structure_test(bool silent);

void node_edge_data_structure_test();
void map_construction_test();

#endif