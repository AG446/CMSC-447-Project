#include "tester.h"
#include "map.h"
#include <stdio.h>

int main(){
	//building_data_structure_test();
	//node_edge_data_structure_test();
	mpo_data_structure_test();
	
	do_thing();
	fputs("End of program\n",stdout);
}

void mpo_data_structure_test(){
	cord_t square[4] = {create_cord(0,0), create_cord(0,1), create_cord(1,1), create_cord(1,0)};
	
	mpo_t * mpo = create_mpo(square,4,MPO_TYPE_BUILDING);
	mpo_to_output_stream(mpo,0,stdout);
	set_mpo_type(mpo,MPO_TYPE_TREE);
	set_mpo_cord(mpo,3,create_cord(5,5));
	mpo_to_output_stream(mpo,0,stdout);
	
	delete_map_mpo(mpo);
}

void node_edge_data_structure_test(){
	building_t * build = create_building("ILSB",create_map_rect(create_cord(0,1),create_cord(2,3)),3);
	
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
	delete_map_node(node_a);
	delete_map_node(node_b);
	delete_building(build);
}

void building_data_structure_test(){
	size_t n_floors = 4;
	const char * building_name = "Information Technology";
	map_rect_t building_bounding_box = create_map_rect(create_cord(-1,-2),create_cord(3,4));
	
	building_t * building = create_building(building_name,building_bounding_box,n_floors);
	
	
	add_building_alias_name(building,"ITE");
	add_building_alias_name(building,"Computing");
	add_building_alias_name(building,"Servers");
	add_building_alias_name(building,"Dumbo");
	add_building_alias_name(building,"Computer Lab");
	
	building_to_output_stream(building,0,stdout);
	
	remove_building_alias_name(building,"Dumbo");
	
	building_to_output_stream(building,0,stdout);
	
	remove_building_alias_name(building,"Computer Lab");
	remove_building_alias_name(building,"Information Technology");
	change_primary_building_name(building,"Computing");
	set_building_floor_count(building,5);
	
	building_to_output_stream(building,0,stdout);
	
	delete_building(building);
}