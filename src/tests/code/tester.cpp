#include "tester.h"
#include "map.h"
#include <stdio.h>
#include <string.h>

#define N_TESTS 2
test_func_t func_table[N_TESTS] = { 
	{building_data_structure_test,"building data structure test",SILENT},
	{mpo_data_structure_test,"map polygon object data structure test",SILENT}
};

int main(){
	
	for(size_t i = 0;i < N_TESTS;i++){
		test_func_t testable_func = func_table[i];
		fputs(testable_func.function_name,stdout);
		fputs(" : ",stdout);
		if(!testable_func.silent) fputc('\n',stdout);
		bool function_passes = testable_func.function(testable_func.silent);
		if(function_passes){
			fputs("PASS\n",stdout);
		}else{
			fputs("FAIL\n",stdout);
		}
	}
}

void map_construction_test(){
	fputs("hello\n",stdout);
}

void node_edge_data_structure_test(){
	err_ctx_t err_ctx = create_err_ctx();
	
	building_t * build = create_building("ILSB",create_map_rect(create_cord(0,1),create_cord(2,3)),3,&err_ctx);
	
	map_node_t * node_a = create_map_node(create_cord(2.0,3.0));
	map_node_t * node_b = create_map_node(create_cord(5.0,7.0));
	
	map_edge_t * edge = create_map_edge(EDGE_TYPE_SIDEWALK,node_a,node_b);
	set_map_edge_type(edge,EDGE_TYPE_RAMP);
	
	set_map_node_name(node_a,"Alpha Dog");
	set_map_node_picture(node_a,"Jacked_Dude.jpeg");
	set_map_node_floor_number(node_a,3);
	set_map_node_building(node_a,build);
	set_map_node_selectable(node_a,true);
	
	map_node_to_output_stream(node_a,0,stdout);
	map_node_to_output_stream(node_b,0,stdout);
	
	map_edge_to_output_stream(edge,0,stdout);
	
	delete_map_edge(edge);
	delete_map_node(node_a,&err_ctx);
	delete_map_node(node_b,&err_ctx);
	delete_building(build,&err_ctx);
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
	
	set_mpo_cord(NULL,3,create_cord(5,5),&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_mpo_cord(mpo,3,create_cord(5,5),&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	set_mpo_type(NULL,MPO_TYPE_TREE,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	set_mpo_type(mpo,MPO_TYPE_TREE,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
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