#include "tester.h"
#include "map.h"
#include "cl_tool.h"
#include "text_proc.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define N_TESTS 7
test_func_t func_table[N_TESTS] = {
	{token_matching_test,"Token matching test",SILENT},
	{phrase_matching_test,"Phrase matching test",SILENT},
	{building_data_structure_test,"Building data structure test",SILENT},
	{mpo_data_structure_test,"Map polygon object data structure test",SILENT},
	{map_node_data_structure_test,"Map node data structure test",SILENT},
	{map_edge_data_structure_test,"Map edge data structure test",SILENT},
	{basic_map_data_structure_test,"Basic map data structure test",VERBOSE}
};

int main(){
	
	for(size_t i = 0;i < N_TESTS;i++){
		test_func_t testable_func = func_table[i];
		fputs(testable_func.function_name,stdout);
		fputs(" : ",stdout);
		if(!testable_func.silent) fputc('\n',stdout);
		bool function_passes = testable_func.function(testable_func.silent);
		if(function_passes){
			fputs("\033[32mPASS\033[0m\n",stdout);
		}else{
			fputs("\033[31mFAIL\033[0m\n",stdout);
		}
	}
	do_thing();
	//start_cli();
}

bool basic_map_data_structure_test(bool silent){
	
	return PASS;
}

bool map_edge_data_structure_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	map_node_t * node_a = create_map_node(create_cord(2.0,3.0));
	map_node_t * node_b = create_map_node(create_cord(4.0,5.0));
	
	map_edge_t * edge = create_map_edge(EDGE_TYPE_HALLWAY,node_a,node_b,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	if(get_edge_node_a(edge,&err_ctx) != node_a) return FAIL;
	if(get_edge_node_b(edge,&err_ctx) != node_b) return FAIL;
	if(get_edge_node_a(NULL,&err_ctx) != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	if(get_edge_node_b(NULL,&err_ctx) != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	map_edge_t * bad_edge = create_map_edge(EDGE_TYPE_HALLWAY,NULL,node_b,&err_ctx);
	if(bad_edge != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	if(!silent) map_edge_to_output_stream(edge,0,stdout,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	map_edge_to_output_stream(edge,0,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_map_edge_type(NULL,EDGE_TYPE_ELEVATOR_SHAFT,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_edge_type(edge,EDGE_TYPE_ELEVATOR_SHAFT,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	get_map_edge_type(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	if(get_map_edge_type(edge,&err_ctx) != EDGE_TYPE_ELEVATOR_SHAFT) return NULL;
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_edge_to_output_stream(edge,0,stdout,&err_ctx);
	
	double length = get_edge_length(edge,&err_ctx);
	if(abs(length - 2*sqrt(2)) > 0.0001) return FAIL;
	if(!silent) printf("length: %lf\n",length);
	
	delete_map_edge(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	delete_map_edge(edge,&err_ctx);
	delete_map_node(node_a,&err_ctx);
	delete_map_node(node_b,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

bool token_matching_test(bool silent){
	const char * token_1 = "Computing";
	const char * token_2 = "Comting";
	const char * token_3 = "Staples";
	const char * token_4 = "Computing";
	
	float comparison_1 = token_similarity_score(token_1,token_2);
	if(!silent) printf("%s %s %f\n",token_1,token_2,comparison_1);
	
	if(comparison_1 == 0.0f || comparison_1 == 1.0f) return FAIL;
	
	float comparison_2 = token_similarity_score(token_1,token_3);
	if(!silent) printf("%s %s %f\n",token_1,token_3,comparison_2);
	
	if(comparison_2 > 0.5f) return FAIL;
	if(comparison_2 > comparison_1) return FAIL;
	
	float comparison_3 = token_similarity_score(token_1,token_4);
	if(!silent) printf("%s %s %f\n",token_1,token_4,comparison_3);
	
	if(comparison_3 != 1.0f) return FAIL;
	if(comparison_1 > comparison_3) return FAIL;
	
	return PASS;
}

bool phrase_matching_test(bool silent){
	
	const char * locations[6] = {
		"Information Technology Computing ITE",
		"Library",
		"Engineering ENG",
		"Interdisciplinary Life Sciences ILS",
		"Mathematics MATH",
		"Janet and Walter Sondheim"
	};
	
	const char * mistypes[6] = {
		"sondhime janet",
		"Info Tech",
		"libaryy",
		"inter sience",
		"eng",
		"maths bulding"
	};
	
	const size_t correct_solution[6] = {
		5,
		0,
		1,
		3,
		2,
		4
	};
	
	for(size_t i = 0;i < 6;i++){
		const char * mistyped = mistypes[i];
		
		size_t best_index = 0;
		float best_score = -1.0f;
		
		for(size_t j = 0;j < 6;j++){
			const char * location = locations[j];
			float score = phrase_similarity_score(location,mistyped);
			if(score > best_score){
				best_score = score;
				best_index = j;
			}
			if(!silent) printf("\"%s\" \"%s\" %f\n",mistyped,location,score);
		}
		
		if(correct_solution[i] != best_index) return FAIL;
		if(!silent) fputs("\n\n",stdout);
	}
	
	return PASS;
}

bool map_node_data_structure_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	map_node_t * node = create_map_node(create_cord(2.0,3.0));
	if(node == NULL) return FAIL;
	
	set_map_node_name(NULL,"Node A",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_name(node,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_name(node,"Node A",&err_ctx);
	set_map_node_name(node,"Node B",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(strcmp(get_map_node_name(node,&err_ctx),"Node B") != 0) return FAIL;
	get_map_node_name(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_map_node_picture(NULL,"image2.jpeg",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_picture(node,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_picture(node,"image.jpeg",&err_ctx);
	set_map_node_picture(node,"image2.jpeg",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(strcmp(get_map_node_picture(node,&err_ctx),"image2.jpeg") != 0) return FAIL;
	get_map_node_picture(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_map_node_floor_number(NULL,5,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_floor_number(node,5,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(get_map_node_floor_number(node,&err_ctx) != 5) return FAIL;
	get_map_node_floor_number(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_map_node_selectable(NULL,true,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_selectable(node,true,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	if(get_map_node_selectable(node,&err_ctx) != true) return FAIL;
	
	set_map_node_building(node,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	building_t * building = create_building("Random building",create_map_rect(create_cord(-1.0,-1.0),create_cord(1.0,1.0)),3,&err_ctx);
	set_map_node_building(NULL,building,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_building(node,building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_node_to_output_stream(node,0,stdout,&err_ctx);
	
	clear_map_node_building(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	clear_map_node_building(node,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	set_map_node_selectable(node,false,&err_ctx);
	if(get_map_node_selectable(node,&err_ctx) != false) return FAIL;
	get_map_node_selectable(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	clear_map_node_floor_number(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	clear_map_node_floor_number(node,&err_ctx);
	if(get_map_node_floor_number(node,&err_ctx) != NODE_FLOOR_NUMBER_NONE) return FAIL;
	
	clear_map_node_picture(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	clear_map_node_picture(node,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	clear_map_node_name(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	clear_map_node_name(node,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	set_map_node_cord(NULL,create_cord(5.0,8.0),&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_map_node_cord(node,create_cord(5.0,8.0),&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	if(!are_cords_equal(get_map_node_cord(node,&err_ctx),create_cord(5.0,8.0))) return FAIL;
	if(err_encountered(&err_ctx)) return FAIL;
	get_map_node_cord(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	if(!silent) map_node_to_output_stream(node,0,stdout,&err_ctx);
	
	map_node_to_output_stream(node,0,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	map_node_to_output_stream(NULL,0,stdout,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	delete_map_node(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	delete_map_node(node,&err_ctx);
	delete_building(building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

bool mpo_data_structure_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	cord_t square[4] = {create_cord(0,0), create_cord(0,1), create_cord(1,1), create_cord(1,0)};
	
	mpo_t * mpo = create_mpo(square,4,MPO_TYPE_BUILDING,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	mpo_t * invalid_mpo = create_mpo(NULL,4,MPO_TYPE_BUILDING,&err_ctx);
	if(invalid_mpo != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_mpo_name(mpo,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_mpo_name(NULL,"square",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_mpo_name(mpo,"square",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) mpo_to_output_stream(mpo,0,stdout,&err_ctx);
	
	set_mpo_cord(NULL,3,create_cord(5,6),&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_mpo_cord(mpo,3,create_cord(5,6),&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	get_mpo_cord(NULL,3,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	if(!are_cords_equal(get_mpo_cord(mpo,3,&err_ctx),create_cord(5,6))) return FAIL;
	if(err_encountered(&err_ctx)) return FAIL;
	
	set_mpo_type(NULL,MPO_TYPE_TREE,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_mpo_type(mpo,MPO_TYPE_TREE,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(get_mpo_type(mpo,&err_ctx) != MPO_TYPE_TREE) return FAIL;
	if(err_encountered(&err_ctx)) return FAIL;
	get_mpo_type(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_mpo_name(mpo,"squid",&err_ctx);
	if(strcmp(get_mpo_name(mpo,&err_ctx),"squid") != 0) return FAIL;
	get_mpo_name(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	if(!silent) mpo_to_output_stream(mpo,0,stdout,&err_ctx);
	
	mpo_to_output_stream(NULL,0,stdout,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	clear_mpo_name(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	clear_mpo_name(mpo,&err_ctx);
	if(get_mpo_name(mpo,&err_ctx) != NULL) return FAIL;
	
	if(!silent) mpo_to_output_stream(mpo,0,stdout,&err_ctx);
	
	delete_mpo(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	delete_mpo(mpo,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

bool building_data_structure_test(bool silent){
	
	err_ctx_t err_ctx = create_err_ctx();
	
	size_t n_floors = 4;
	const char * building_name = "Information Technology";
	map_rect_t building_bounding_box = create_map_rect(create_cord(-1,-2),create_cord(3,4));
	
	building_t * invalid_create_test = create_building(NULL,building_bounding_box,n_floors,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	if(invalid_create_test != NULL) return FAIL;
	reset_err_ctx(&err_ctx);
	
	building_t * building = create_building(building_name,building_bounding_box,n_floors,&err_ctx);
	
	add_building_alias_name(building,"ITE",&err_ctx);
	add_building_alias_name(building,"Computing",&err_ctx);
	add_building_alias_name(building,"Servers",&err_ctx);
	add_building_alias_name(building,"Dumbo",&err_ctx);
	add_building_alias_name(building,"Computer Lab",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	add_building_alias_name(building,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_building_alias_name(NULL,"uh oh",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	if(!silent) building_to_output_stream(building,0,stdout,&err_ctx);
	
	remove_building_alias_name(building,"Dumbo",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	remove_building_alias_name(building,"Lumbi",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_alias_name(NULL,"Computer Lab",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_alias_name(building,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	if(!silent) building_to_output_stream(building,0,stdout,&err_ctx);
	
	remove_building_alias_name(building,"Computer Lab",&err_ctx);
	remove_building_alias_name(building,"Information Technology",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	change_primary_building_name(building,"Computi",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	change_primary_building_name(building,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	change_primary_building_name(NULL,"Computing",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	change_primary_building_name(building,"Computing",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	const char * primary_name = get_primary_building_name(building,&err_ctx);
	if(!silent){
		fputs(primary_name,stdout);
		fputc('\n',stdout);
	}
	if(primary_name == NULL) return FAIL;
	if(strcmp(primary_name,"Computing") != 0) return FAIL;
	if(err_encountered(&err_ctx)) return FAIL;
	const char * primary_name_2 = get_primary_building_name(NULL,&err_ctx);
	if(primary_name_2 != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	set_building_floor_count(NULL,5,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_building_floor_count(building,5,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	building_bounding_box = create_map_rect(create_cord(-10,-20),create_cord(30,40));
	set_building_bounding_box(NULL,building_bounding_box,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_building_bounding_box(building,building_bounding_box,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) building_to_output_stream(building,0,stdout,&err_ctx);
	
	building_to_output_stream(building,0,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	delete_building(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	delete_building(building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}