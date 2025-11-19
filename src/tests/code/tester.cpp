#include "tester.h"
#include "map.h"
#include "text_proc.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "map_serial.h"
#include <time.h>

#define N_TESTS 12
test_func_t func_table[N_TESTS] = {
	{token_matching_test,"Token matching test",SILENT},
	{phrase_matching_test,"Phrase matching test",SILENT},
	{basic_serialization_test,"Basic serialization test",SILENT},
	{serialization_test,"Serialization test",SILENT},
	{building_data_structure_test,"Building data structure test",SILENT},
	{mpo_data_structure_test,"Map polygon object data structure test",SILENT},
	{map_node_data_structure_test,"Map node data structure test",SILENT},
	{map_edge_data_structure_test,"Map edge data structure test",SILENT},
	{basic_map_data_structure_test,"Basic map data structure test",SILENT},
	{find_path_test,"Find path test",SILENT},
    {path_string_test,"Output text test",VERBOSE},
    {path_string_test2,"Output text test 2", VERBOSE}
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
}

bool find_path_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	map_sys_t sys = init_map_sys();
	
	map_node_t * node_a = create_map_node(create_cord(0,0));
	set_map_node_name(node_a,"A",&err_ctx);
	
	map_node_t * node_b = create_map_node(create_cord(1,2));
	set_map_node_name(node_b,"B",&err_ctx);
	
	map_node_t * node_c = create_map_node(create_cord(1,-1));
	set_map_node_name(node_c,"C",&err_ctx);
	
	map_node_t * node_d = create_map_node(create_cord(2,0));
	set_map_node_name(node_d,"D",&err_ctx);
	
	map_node_t * node_e = create_map_node(create_cord(3,1));
	set_map_node_name(node_e,"E",&err_ctx);
	
	map_node_t * node_f = create_map_node(create_cord(3,-2));
	set_map_node_name(node_f,"F",&err_ctx);
	
	map_node_t * node_g = create_map_node(create_cord(4,0));
	set_map_node_name(node_g,"G",&err_ctx);
	
	add_node_to_map(&sys.map,node_a,&err_ctx);
	add_node_to_map(&sys.map,node_b,&err_ctx);
	add_node_to_map(&sys.map,node_c,&err_ctx);
	add_node_to_map(&sys.map,node_d,&err_ctx);
	add_node_to_map(&sys.map,node_e,&err_ctx);
	add_node_to_map(&sys.map,node_f,&err_ctx);
	add_node_to_map(&sys.map,node_g,&err_ctx);
	
	connect_nodes_in_map(&sys.map,node_a,node_b,EDGE_TYPE_HALLWAY,&err_ctx);//1
	connect_nodes_in_map(&sys.map,node_a,node_c,EDGE_TYPE_STAIRS,&err_ctx);//2
	connect_nodes_in_map(&sys.map,node_b,node_d,EDGE_TYPE_HALLWAY,&err_ctx);//3
	connect_nodes_in_map(&sys.map,node_c,node_d,EDGE_TYPE_HALLWAY,&err_ctx);//4
	connect_nodes_in_map(&sys.map,node_d,node_e,EDGE_TYPE_HALLWAY,&err_ctx);//5
	connect_nodes_in_map(&sys.map,node_d,node_f,EDGE_TYPE_HALLWAY,&err_ctx);//6
	connect_nodes_in_map(&sys.map,node_e,node_g,EDGE_TYPE_HALLWAY,&err_ctx);//7
	connect_nodes_in_map(&sys.map,node_f,node_g,EDGE_TYPE_HALLWAY,&err_ctx);//8
	
	if(!silent) map_to_output_stream(sys.map,0,stdout,&err_ctx);
	
	sys.active_start = node_a;
	sys.active_end = node_g;
	sys.active_edge_cost_function = calculate_wheelchair_edge_cost;
	
	clock_t start_time = clock();
	find_best_path(&sys,&err_ctx);
	clock_t end_time = clock();
	
	if(!silent){
		if(sys.active_path != NULL){
			for(size_t i = 0; i < sys.active_path->n_nodes;i++){
				map_node_to_output_stream(sys.active_path->nodes[i],0,stdout,&err_ctx);
			}
		}
	}
	
	if(!silent) printf("Path Finder CPU time: %f ms\n",(float)(end_time-start_time) / CLOCKS_PER_SEC*1000.0f);
	
	deinit_map_sys(&sys,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

static void print_bytes(uint8_t * bytes,size_t n_bytes){
	for(size_t i = 0;i < n_bytes;i++){
		printf("%X ",bytes[i]);
	}
	fputc('\n',stdout);
}

bool serialization_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	{
		map_node_t * nodes[3] = {create_map_node(create_cord(2.0,3.0)), create_map_node(create_cord(5.0,6.0)), create_map_node(create_cord(8.0,-2.0))};
		for(size_t i = 0;i < 3;i++) nodes[i]->index_temp = i;
		
		
		map_edge_t * edge = create_map_edge(EDGE_TYPE_DOOR,nodes[1],nodes[2],&err_ctx);
		
		if(!silent) map_edge_to_output_stream(edge,0,stdout,&err_ctx);
		
		size_t edge_n_bytes = 0;
		uint8_t * edge_bytes = convert_edge_to_binary(edge,&edge_n_bytes);
		if(!silent) print_bytes(edge_bytes,edge_n_bytes);
		delete_map_edge(edge,&err_ctx);
		
		size_t n_bytes_read_for_edge = 0;
		struct Edge_Representation read_edge = convert_binary_to_edge_representation(edge_bytes,edge_n_bytes,&n_bytes_read_for_edge,&err_ctx);
		if(n_bytes_read_for_edge != edge_n_bytes) return FAIL;
		
		if(!silent){
			printf("Edge Representation %d %lu %lu\n",read_edge.type,read_edge.index_a,read_edge.index_b);
		}
		
		free(edge_bytes);
		
		for(size_t i = 0;i < 3;i++) delete_map_node(nodes[i],&err_ctx);
	}
	
	{
		building_t * building_arr[3] = {
			create_building("Math",create_map_rect(create_cord(2.0,3.0),create_cord(5.0,8.0)),7,&err_ctx ),
			create_building("Art",create_map_rect(create_cord(-2.0,-3.0),create_cord(6.0,2.0)),3,&err_ctx ),
			create_building("Music",create_map_rect(create_cord(-3.0,-8.0),create_cord(9.0,13.0)),4,&err_ctx )
		};
		for(size_t i = 0;i < 3;i++) building_arr[i]->index_temp = i;
		map_node_t * node_a = create_map_node(create_cord(3.278,7.893));
		set_map_node_name(node_a,"Node A",&err_ctx);
		set_map_node_picture(node_a,"Node_A.jpeg",&err_ctx);
		set_map_node_floor_number(node_a,3,&err_ctx);
		set_map_node_selectable(node_a,true,&err_ctx);
		set_map_node_building(node_a,building_arr[2],&err_ctx);
		
		if(!silent) map_node_to_output_stream(node_a,0,stdout,NULL);
		
		size_t n_node_a_bytes = 0;
		uint8_t * node_a_bytes = convert_node_to_binary(node_a,&n_node_a_bytes);
		if(!silent) print_bytes(node_a_bytes,n_node_a_bytes);
		delete_map_node(node_a,&err_ctx);
		
		size_t n_bytes_read_for_node_a = 0;
		map_node_t * read_node_a = convert_binary_to_node(node_a_bytes,n_node_a_bytes,&n_bytes_read_for_node_a,building_arr,&err_ctx);
		if(read_node_a == NULL) return FAIL;
		if(n_bytes_read_for_node_a != n_node_a_bytes) return FAIL;
		
		if(!silent) map_node_to_output_stream(read_node_a,0,stdout,NULL);
		
		delete_map_node(read_node_a,&err_ctx);
		
		free(node_a_bytes);
		for(size_t i = 0;i < 3;i++) delete_building(building_arr[i],&err_ctx);
	}
	
	
	{
		const size_t n_nodes = 5;
		map_node_t ** nodes = (map_node_t**) malloc(sizeof(map_node_t*) * n_nodes);
		for(size_t i = 0;i < n_nodes;i++){
			nodes[i] = create_map_node(create_cord(i,i*2));
			if(i == 3){
				set_map_node_name(nodes[i],"Node 3!",&err_ctx);
			}
			if(!silent) map_node_to_output_stream(nodes[i],0,stdout,NULL);
		}
		
		size_t n_nodes_bytes = 0;
		uint8_t * nodes_bytes = convert_node_array_to_binary(nodes,n_nodes,&n_nodes_bytes);
		if(!silent) print_bytes(nodes_bytes,n_nodes_bytes);
		
		size_t n_bytes_read_for_nodes = 0;
		size_t n_read_nodes = 0;
		map_node_t ** read_nodes = convert_binary_to_node_array(nodes_bytes,n_nodes_bytes,&n_bytes_read_for_nodes,NULL,&n_read_nodes,&err_ctx);
		if(read_nodes == NULL) return FAIL;
		if(n_bytes_read_for_nodes != n_nodes_bytes) return FAIL;
		if(n_read_nodes != n_nodes) return FAIL;
		if(!silent){
			for(size_t i = 0;i < n_read_nodes;i++){
				map_node_to_output_stream(read_nodes[i],0,stdout,NULL);
			}
		}
		
		free(nodes_bytes);
		for(size_t i = 0;i < n_nodes;i++){
			delete_map_node(nodes[i],&err_ctx);
			delete_map_node(read_nodes[i],&err_ctx);
		}
		free(nodes);
		free(read_nodes);
	}
	
	{
		cord_t square_cords[4] = {create_cord(0,0), create_cord(0,1), create_cord(1,1), create_cord(1,0)};
		mpo_t * square_mpo = create_mpo(square_cords,4,MPO_TYPE_WATER,&err_ctx);
		
		cord_t triangle_cords[3] = {create_cord(0,0), create_cord(0.5,1), create_cord(1,0)};
		mpo_t * triangle_mpo = create_mpo(triangle_cords,3,MPO_TYPE_TREE,&err_ctx);
		set_mpo_name(triangle_mpo,"Triangle Tree!",&err_ctx);
		
		cord_t diamond_cords[4] = {create_cord(0.5,0), create_cord(1,0.5), create_cord(0.5,1), create_cord(0,0.5)};
		mpo_t * diamond_mpo = create_mpo(diamond_cords,4,MPO_TYPE_BUILDING,&err_ctx);
		
		mpo_t * mpos[3] = {square_mpo, triangle_mpo, diamond_mpo};
		if(!silent){
			for(size_t i = 0;i < 3;i++){
				mpo_to_output_stream(mpos[i],0,stdout,&err_ctx);
			}
		}
		
		size_t mpos_n_bytes = 0;
		uint8_t * mpos_bytes = convert_mpo_array_to_binary(mpos,3,&mpos_n_bytes);
		for(size_t i = 0;i < 3;i++) delete_mpo(mpos[i],&err_ctx);
		
		if(!silent) print_bytes(mpos_bytes,mpos_n_bytes);
		
		size_t n_bytes_read_for_mpos = 0;
		size_t n_read_mpos = 0;
		mpo_t ** read_mpos = convert_binary_to_mpo_array(mpos_bytes,mpos_n_bytes,&n_bytes_read_for_mpos,&n_read_mpos,&err_ctx);
		if(read_mpos == NULL) return FAIL;
		if(n_read_mpos != 3) return FAIL;
		if(n_bytes_read_for_mpos != mpos_n_bytes) return FAIL;
		
		if(!silent){
			for(size_t i = 0;i < n_read_mpos;i++){
				mpo_to_output_stream(read_mpos[i],0,stdout,&err_ctx);
			}
		}
		
		for(size_t i = 0;i < n_read_mpos;i++) delete_mpo(read_mpos[i],&err_ctx);
		free(read_mpos);
		
		free(mpos_bytes);
	}
	
	{
		building_t * math_building = create_building("Math",create_map_rect(create_cord(2.0,3.0),create_cord(5.0,8.0)),7,&err_ctx );
		
		building_t * art_building = create_building("Art",create_map_rect(create_cord(-2.0,-3.0),create_cord(6.0,2.0)),3,&err_ctx );
		add_building_alias_name(art_building,"painting",&err_ctx);
		add_building_alias_name(art_building,"drawing",&err_ctx);
		
		building_t * music_building = create_building("Music",create_map_rect(create_cord(-3.0,-8.0),create_cord(9.0,13.0)),4,&err_ctx );
		
		building_t * buildings[3] = {math_building,art_building,music_building};
		
		if(!silent){
			for(size_t i = 0;i < 3;i++){
				building_to_output_stream(buildings[i],0,stdout,&err_ctx);
			}
		}
		
		size_t n_buildings_bytes = 0;
		uint8_t * buildings_bytes = convert_building_array_to_binary(buildings,3,&n_buildings_bytes);
		
		if(!silent) print_bytes(buildings_bytes,n_buildings_bytes);
		for(size_t i = 0;i < 3;i++) delete_building(buildings[i],&err_ctx);
		
		
		size_t n_bytes_read_for_buildings = 0;
		size_t n_buildings_read = 0;
		building_t ** read_buildings = convert_binary_to_building_array(buildings_bytes,n_buildings_bytes,&n_bytes_read_for_buildings,&n_buildings_read,&err_ctx);
		if(read_buildings == NULL) return FAIL;
		if(n_buildings_read != 3) return FAIL;
		if(n_bytes_read_for_buildings != n_buildings_bytes) return FAIL;
		
		if(!silent){
			for(size_t i = 0;i < n_buildings_read;i++){
				building_to_output_stream(read_buildings[i],0,stdout,&err_ctx);
			}
		}
		
		for(size_t i = 0;i < n_buildings_read;i++) delete_building(read_buildings[i],&err_ctx);
		free(read_buildings);
		
		free(buildings_bytes);
	}
	
	{
		/*
		map_node_t * nodes[5];
		for(size_t i = 0;i < 5;i++){
			nodes[i] = create_map_node(create_cord(i,2*i));
			char name[2] = {(char)('A'+i),'\0'};
			set_map_node_name(nodes[i],name,&err_ctx);
			nodes[i]->index_temp = i;
		}
		if(!silent){
			for(size_t i = 0;i < 5;i++) map_node_to_output_stream(nodes[i],0,stdout,&err_ctx);
		}
		
		map_edge_t * edges[3] = {
			create_map_edge(EDGE_TYPE_DOOR,nodes[0],nodes[3],&err_ctx),
			create_map_edge(EDGE_TYPE_HALLWAY,nodes[2],nodes[4],&err_ctx),
			create_map_edge(EDGE_TYPE_RAMP,nodes[3],nodes[4],&err_ctx)
		};
		if(!silent){
			for(size_t i = 0;i < 3;i++) map_edge_to_output_stream(edges[i],0,stdout,&err_ctx);
		}
		
		size_t total_edges_bytes = 0;
		uint8_t * edges_bytes = convert_edge_array_to_binary(edges,3,&total_edges_bytes);
		if(!silent) print_bytes(edges_bytes,total_edges_bytes);
		for(size_t i = 0;i < 3;i++) delete_map_edge(edges[i],&err_ctx);
		
		size_t bytes_read_for_edges = 0;
		size_t n_edges_read = 0;
		map_edge_t ** read_edges = convert_binary_to_edge_array(edges_bytes,total_edges_bytes,&bytes_read_for_edges,nodes,&n_edges_read,&err_ctx);
		if(read_edges == NULL) return FAIL;
		if(n_edges_read != 3) return FAIL;
		if(bytes_read_for_edges != total_edges_bytes) return FAIL;
		
		if(!silent){
			for(size_t i = 0;i < n_edges_read;i++) map_edge_to_output_stream(read_edges[i],0,stdout,&err_ctx);
		}
		
		for(size_t i = 0;i < n_edges_read;i++) delete_map_edge(read_edges[i],&err_ctx);
		free(read_edges);
		free(edges_bytes);
		for(size_t i = 0;i < 5;i++) delete_map_node(nodes[i],&err_ctx);
		*/
	}
	
	{
		map_t map = init_map();
		building_t * building_a = create_building("Building A",create_map_rect(create_cord(2,3),create_cord(5,6)),3,&err_ctx);
		building_t * building_b = create_building("Building B",create_map_rect(create_cord(3,5),create_cord(9,10)),6,&err_ctx);
		add_building_to_map(&map,building_a,&err_ctx);
		add_building_to_map(&map,building_b,&err_ctx);
		
		map_node_t * node_a = create_map_node(create_cord(2,3));
		set_map_node_name(node_a,"Node A",&err_ctx);
		
		map_node_t * node_b = create_map_node(create_cord(3,8));
		set_map_node_name(node_b,"Node B",&err_ctx);
		
		map_node_t * node_c = create_map_node(create_cord(1,4));
		set_map_node_name(node_c,"Node C",&err_ctx);
		
		map_node_t * node_d = create_map_node(create_cord(-3,8));
		set_map_node_name(node_d,"Node D",&err_ctx);
		set_map_node_building(node_c,building_a,&err_ctx);
		
		add_node_to_map(&map,node_a,&err_ctx);
		add_node_to_map(&map,node_b,&err_ctx);
		add_node_to_map(&map,node_c,&err_ctx);
		add_node_to_map(&map,node_d,&err_ctx);
		
		connect_nodes_in_map(&map,node_a,node_b,EDGE_TYPE_OVERPASS,&err_ctx);
		connect_nodes_in_map(&map,node_c,node_d,EDGE_TYPE_HALLWAY,&err_ctx);
		connect_nodes_in_map(&map,node_a,node_d,EDGE_TYPE_AUTO_DOOR,&err_ctx);
		
		cord_t square[4] = {create_cord(0,0),create_cord(0,1),create_cord(1,1),create_cord(1,0)};
		mpo_t * square_lake = create_mpo(square,4,MPO_TYPE_WATER,&err_ctx);
		set_mpo_name(square_lake,"Square Lake",&err_ctx);
		add_mpo_to_map(&map,square_lake,&err_ctx);
		
		if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
		
		save_map_to_file(&map,"map_test.hmap");
		
		size_t n_map_bytes = 0;
		uint8_t * map_bytes = convert_map_to_binary(&map,&n_map_bytes);
		
		deinit_map(&map,&err_ctx);
		
		if(!silent) print_bytes(map_bytes,n_map_bytes);
		
		map = init_map_from_binary(map_bytes,n_map_bytes,&err_ctx);
		
		if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
		
		deinit_map(&map,&err_ctx);
		
		free(map_bytes);
		
		map = load_map_from_file("map_test.map",&err_ctx);
		
		if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
		
		deinit_map(&map,&err_ctx);
	}
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

bool basic_serialization_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	size_t number = 16897;
	size_t n_bytes_for_size_t = 0;
	uint8_t * bytes_for_size_t = convert_size_t_to_binary(number,&n_bytes_for_size_t);
	
	if(!silent) print_bytes(bytes_for_size_t,n_bytes_for_size_t);
	
	size_t bytes_read = 0;
	size_t number_read = convert_binary_to_size_t(bytes_for_size_t,n_bytes_for_size_t,&bytes_read);
	if(bytes_read != sizeof(size_t)) return FAIL;
	
	free(bytes_for_size_t);
	
	if(!silent) printf("got: %lu\n",number_read);
	if(number_read != number) return FAIL;
	
	
	
	const char * string = "Hello There!";
	size_t n_bytes_for_string = 0;
	uint8_t * bytes_for_string = convert_string_to_binary(string,&n_bytes_for_string);
	
	if(!silent) print_bytes(bytes_for_string,n_bytes_for_string);
	
	bytes_read = 0;
	char * string_read = convert_binary_to_string(bytes_for_string,n_bytes_for_string,&bytes_read);
	if(bytes_read != sizeof(size_t)+strlen(string_read)) return FAIL;
	
	if(!silent) printf("%s\n",string_read);
	
	if(strcmp(string,string_read) != 0) return FAIL;
	
	free(string_read);
	free(bytes_for_string);
	
	
	
	const char * strings_arr[3] = {"Hi","Hello!","Computer"};
	size_t n_bytes_for_string_arr = 0;
	uint8_t * bytes_for_string_arr = convert_string_array_to_binary((char**)strings_arr,3,&n_bytes_for_string_arr);
	
	if(!silent){
		print_bytes(bytes_for_string_arr,n_bytes_for_string_arr);
		printf("%lu\n",n_bytes_for_string_arr);
	}
	
	size_t n_bytes_read_for_string_arr = 0;
	size_t n_strings = 0;
	char ** strings_arr_read = convert_binary_to_string_array(bytes_for_string_arr,n_bytes_for_string_arr,&n_bytes_read_for_string_arr,&n_strings);
	
	free(bytes_for_string_arr);
	
	if(n_strings != 3) return FAIL;
	
	for(size_t i = 0;i < n_strings;i++){
		if(strcmp(strings_arr[i],strings_arr_read[i]) != 0) return FAIL;
	}
	
	if(!silent){
		for(size_t i = 0;i < n_strings;i++){
			printf("%s\n",strings_arr_read[i]);
		}
		printf("%lu\n",n_bytes_read_for_string_arr);
	}
	
	for(size_t i = 0;i < n_strings;i++) free(strings_arr_read[i]);
	free(strings_arr_read);
	
	
	
	cord_t cord = create_cord(78.873,92.7865);
	
	size_t n_cord_bytes;
	uint8_t * cord_bytes = convert_cord_to_binary(cord,&n_cord_bytes);
	
	if(!silent){
		print_bytes(cord_bytes,n_cord_bytes);
		printf("%lu\n",n_cord_bytes);
	}
	
	size_t n_bytes_read_for_cord = 0;
	cord_t read_cord = convert_binary_to_cord(cord_bytes,n_cord_bytes,&n_bytes_read_for_cord);
	
	if(!are_cords_equal(cord,read_cord)) return FAIL;
	
	if(!silent) cord_to_output_stream(read_cord,0,stdout,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	free(cord_bytes);
	
	
	
	map_rect_t rect = create_map_rect(create_cord(78.873,92.7865),create_cord(89.873,103.982));
	
	size_t n_rect_bytes;
	uint8_t * rect_bytes = convert_rect_to_binary(rect,&n_rect_bytes);
	
	if(!silent){
		print_bytes(rect_bytes,n_rect_bytes);
		printf("%lu\n",n_rect_bytes);
	}
	
	size_t n_bytes_read_for_rect = 0;
	map_rect_t read_rect = convert_binary_to_rect(rect_bytes,n_rect_bytes,&n_bytes_read_for_rect);
	
	if(memcmp(&rect,&read_rect,sizeof(map_rect_t)) != 0) return FAIL;
	
	if(!silent) map_rect_to_output_stream(read_rect,0,stdout,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	free(rect_bytes);
	
	
	
	const cord_t cords[5] = {
		create_cord(1.0,2.0),
		create_cord(3.0,4.0),
		create_cord(5.0,6.0),
		create_cord(7.0,8.0),
		create_cord(7.0,6.0)
	};
	
	size_t cord_arr_n_bytes = 0;
	uint8_t * cord_arr_bytes = convert_cord_array_to_binary(cords,5,&cord_arr_n_bytes);
	
	if(!silent){
		print_bytes(cord_arr_bytes,cord_arr_n_bytes);
		printf("%lu\n",cord_arr_n_bytes);
	}
	
	size_t cord_arr_n_bytes_read = 0;
	size_t n_cords = 0;
	cord_t * read_cord_arr = convert_binary_to_cord_array(cord_arr_bytes,cord_arr_n_bytes,&cord_arr_n_bytes_read,&n_cords);
	if(read_cord_arr == NULL) return FAIL;
	
	if(n_cords != 5) return FAIL;
	
	for(size_t i = 0;i < n_cords;i++){
		if(!are_cords_equal(cords[i],read_cord_arr[i])) return FAIL;
		if(!silent) cord_to_output_stream(read_cord_arr[i],0,stdout,&err_ctx);
	}
	
	free(read_cord_arr);
	free(cord_arr_bytes);
	
	
	
	mpo_t * mpo = create_mpo(cords,5,MPO_TYPE_BUILDING,&err_ctx);
	set_mpo_name(mpo,"Bob",&err_ctx);
	if(!silent) mpo_to_output_stream(mpo,0,stdout,&err_ctx);
	
	size_t mpo_n_bytes = 0;
	uint8_t * mpo_bytes = convert_mpo_to_binary(mpo,&mpo_n_bytes);
	
	delete_mpo(mpo,&err_ctx);
	
	if(!silent) print_bytes(mpo_bytes,mpo_n_bytes);
	
	size_t bytes_read_for_mpo = 0;
	mpo_t * read_mpo = convert_binary_to_mpo(mpo_bytes,mpo_n_bytes,&bytes_read_for_mpo,&err_ctx);
	free(mpo_bytes);
	if(read_mpo == NULL) return FAIL;
	
	if(!silent) mpo_to_output_stream(read_mpo,0,stdout,&err_ctx);
	
	delete_mpo(read_mpo,&err_ctx);
	
	
	
	building_t * build = create_building("Math Building",create_map_rect(create_cord(2.3,6.5),create_cord(1.1,0.3)),3,&err_ctx);
	add_building_alias_name(build,"MATH",&err_ctx);
	
	if(!silent) building_to_output_stream(build,0,stdout,&err_ctx);
	
	size_t n_building_bytes = 0;
	uint8_t * building_bytes = convert_building_to_binary(build,&n_building_bytes);
	
	if(!silent) print_bytes(building_bytes,n_building_bytes);
	delete_building(build,&err_ctx);
	
	size_t building_bytes_read = 0;
	building_t * building_read = convert_binary_to_building(building_bytes,n_building_bytes,&building_bytes_read,&err_ctx);
	if(building_read == NULL) return FAIL;
	if(building_bytes_read != n_building_bytes) return FAIL;
	
	free(building_bytes);
	
	if(!silent) building_to_output_stream(building_read,0,stdout,&err_ctx);
	
	delete_building(building_read,&err_ctx);
	
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

bool basic_map_data_structure_test(bool silent){
	err_ctx_t err_ctx = create_err_ctx();
	
	map_t map = init_map();
	
	building_t * math_building = create_building("MATH",create_map_rect(create_cord(1,-1),create_cord(5,3)),3,&err_ctx);
	add_building_to_map(NULL,math_building,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_building_to_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_building_to_map(&map,math_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	building_t * lib_building = create_building("LIB",create_map_rect(create_cord(5,7),create_cord(6,9)),3,&err_ctx);
	add_building_to_map(&map,lib_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	building_t * cmsc_building = create_building("CMSC",create_map_rect(create_cord(5,7),create_cord(6,9)),3,&err_ctx);
	add_building_alias_name(cmsc_building,"Computer Science",&err_ctx);
	add_building_to_map(&map,cmsc_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	remove_building_from_map(NULL,lib_building,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_from_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_from_map(&map,lib_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_building_by_name_from_map(NULL,"Computer Science",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_by_name_from_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_by_name_from_map(&map,"Computer Science",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	
	cmsc_building = create_building("CMSC",create_map_rect(create_cord(5,7),create_cord(6,9)),3,&err_ctx);
	add_building_to_map(&map,cmsc_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	lib_building = create_building("LIB",create_map_rect(create_cord(5,7),create_cord(6,9)),3,&err_ctx);
	add_building_to_map(&map,lib_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_building_from_map_by_index(&map,5,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_from_map_by_index(NULL,0,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_building_from_map_by_index(&map,0,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	building_t * retr;
	retr =  get_building_by_index_from_map(&map,5,&err_ctx);
	if(retr != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	retr =  get_building_by_index_from_map(NULL,0,&err_ctx);
	if(retr != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	retr =  get_building_by_index_from_map(&map,0,&err_ctx);
	if(retr == NULL) return FAIL;
	if(retr != cmsc_building) return FAIL;
	retr =  get_building_by_index_from_map(&map,1,&err_ctx);
	if(retr == NULL) return FAIL;
	if(retr != lib_building) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	retr = get_building_by_name_from_map(&map,NULL,&err_ctx);
	if(retr != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	retr = get_building_by_name_from_map(NULL,"CMSC",&err_ctx);
	if(retr != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	retr = get_building_by_name_from_map(&map,"NOT CMSC",&err_ctx);
	if(retr != NULL) return FAIL;
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	retr = get_building_by_name_from_map(&map,"CMSC",&err_ctx);
	if(retr == NULL) return FAIL;
	if(err_encountered(&err_ctx)) return FAIL;
	if(retr != cmsc_building) return FAIL;
	
	map_node_t * node_a = create_map_node(create_cord(2,3));
	set_map_node_name(node_a,"A",&err_ctx);
	set_map_node_building(node_a,lib_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	add_node_to_map(NULL,node_a,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_node_to_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_node_to_map(&map,node_a,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	map_node_t * node_b = create_map_node(create_cord(5,6));
	set_map_node_name(node_b,"B",&err_ctx);
	add_node_to_map(&map,node_b,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	map_node_t * node_c = create_map_node(create_cord(5,6));
	set_map_node_name(node_c,"C",&err_ctx);
	add_node_to_map(&map,node_c,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	remove_node_from_map(NULL,node_b,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_node_from_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_node_from_map(&map,node_b,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_node_by_name_from_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_node_by_name_from_map(NULL,"C",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_node_by_name_from_map(&map,"C",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_building_from_map(&map,lib_building,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_node_from_map_by_index(&map,5,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_node_from_map_by_index(NULL,0,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_node_from_map_by_index(&map,0,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	const cord_t square[4] = {create_cord(0,0),create_cord(0,1),create_cord(1,1),create_cord(1,0)};
	mpo_t * mpo_a = create_mpo(square,4,MPO_TYPE_TREE,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	add_mpo_to_map(NULL,mpo_a,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_mpo_to_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	add_mpo_to_map(&map,mpo_a,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	const cord_t triangle[3] = {create_cord(0,0),create_cord(1,0),create_cord(0.5,1)};
	mpo_t * mpo_b = create_mpo(triangle,3,MPO_TYPE_WATER,&err_ctx);
	set_mpo_name(mpo_b,"B",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	add_mpo_to_map(&map,mpo_b,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	const cord_t diaomond[4] = {create_cord(0.5,0),create_cord(1,0.5),create_cord(0.5,1),create_cord(0,0.5)};
	mpo_t * mpo_c = create_mpo(diaomond,4,MPO_TYPE_BUILDING,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	add_mpo_to_map(&map,mpo_c,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	remove_mpo_from_map(NULL,mpo_a,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_mpo_from_map(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_mpo_from_map(&map,mpo_a,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_mpo_from_map_by_name(NULL,"B",&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_mpo_from_map_by_name(&map,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_mpo_from_map_by_name(&map,"B",&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	remove_mpo_from_map_by_index(NULL,0,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_mpo_from_map_by_index(&map,5,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	remove_mpo_from_map_by_index(&map,0,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
	if(!silent) map_to_output_stream(map,0,stdout,&err_ctx);
	
	map_to_output_stream(map,0,NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	
	deinit_map(NULL,&err_ctx);
	if(!err_encountered(&err_ctx)) return FAIL;
	reset_err_ctx(&err_ctx);
	deinit_map(&map,&err_ctx);
	if(err_encountered(&err_ctx)) return FAIL;
	
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
	if(abs(length - 3.269829) > 0.0001) return FAIL;
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

bool path_string_test(bool silent) {
	err_ctx_t err_ctx = create_err_ctx();
	
	map_sys_t sys = init_map_sys();
	
	// Use realistic Lat/Lon coordinates to fix distance calculations
	const double BASE_LAT = 39.33000;
	const double BASE_LON = -76.62000;
	// Define scale for realistic movement and aggregation distance
	const double SCALE_UNIT = 0.0001; 

	// --- Create mock buildings ---
	map_rect_t bbox_A = create_map_rect(
		create_cord(BASE_LAT - 0.002, BASE_LON - 0.002), 
		create_cord(BASE_LAT + 0.002, BASE_LON + 0.002)
	);
	building_t * building_A = create_building("Library", bbox_A, 2, &err_ctx);

	map_rect_t bbox_B = create_map_rect(
		create_cord(BASE_LAT - 0.002, BASE_LON + 0.008), 
		create_cord(BASE_LAT + 0.002, BASE_LON + 0.012)
	);
	building_t * building_B = create_building("Science Center", bbox_B, 5, &err_ctx);

	if (err_encountered(&err_ctx)) {
		if (!silent) fprintf(stderr, "Error creating buildings for test.\n");
		if (building_A) delete_building(building_A, &err_ctx);
		if (building_B) delete_building(building_B, &err_ctx);
		deinit_map_sys(&sys,&err_ctx);
		return FAIL;
	}
	
	// --- Define Nodes for a Long Path with Sequential Elevator ---
	
	// 1. Starting Node (Library, Floor 1)
	map_node_t * node_a = create_map_node(create_cord(BASE_LAT + 0.5 * SCALE_UNIT, BASE_LON + 0.5 * SCALE_UNIT));
	set_map_node_name(node_a, "Start: Library Desk", &err_ctx);
	node_a->associated_building = building_A;
	node_a->floor_number = 1;
	
	// 2. Library Exit Door (Floor 1)
	map_node_t * node_j = create_map_node(create_cord(BASE_LAT + 1.0 * SCALE_UNIT, BASE_LON + 1.0 * SCALE_UNIT));
	set_map_node_name(node_j, "Library Exit Door", &err_ctx);
	node_j->associated_building = building_A;
	node_j->floor_number = 1;

	// 3-7. Outdoor Path Nodes (J -> K -> L -> M -> N) - Used for Aggregation & Turns
	map_node_t * node_k = create_map_node(create_cord(BASE_LAT + 1.0 * SCALE_UNIT, BASE_LON + 3.0 * SCALE_UNIT));
	set_map_node_name(node_k, "Corner 1 (Straight)", &err_ctx);
	map_node_t * node_l = create_map_node(create_cord(BASE_LAT + 3.0 * SCALE_UNIT, BASE_LON + 3.0 * SCALE_UNIT));
	set_map_node_name(node_l, "Corner 2 (Left Turn)", &err_ctx);
	map_node_t * node_m = create_map_node(create_cord(BASE_LAT + 3.0 * SCALE_UNIT, BASE_LON + 6.0 * SCALE_UNIT));
	set_map_node_name(node_m, "Corner 3 (Right Turn)", &err_ctx);
	map_node_t * node_n = create_map_node(create_cord(BASE_LAT + 5.0 * SCALE_UNIT, BASE_LON + 6.0 * SCALE_UNIT));
	set_map_node_name(node_n, "Science Ctr Approach", &err_ctx);
	
	// 8. Science Center Entrance (Floor 1)
	map_node_t * node_g = create_map_node(create_cord(BASE_LAT + 5.0 * SCALE_UNIT, BASE_LON + 8.0 * SCALE_UNIT));
	set_map_node_name(node_g, "Science Ctr Entrance", &err_ctx);
	node_g->associated_building = building_B;
	node_g->floor_number = 1;
	
	// 9. Elevator Lobby (Floor 1)
	map_node_t * node_h = create_map_node(create_cord(BASE_LAT + 5.1 * SCALE_UNIT, BASE_LON + 8.1 * SCALE_UNIT));
	set_map_node_name(node_h, "Elevator Lobby F1", &err_ctx);
	node_h->associated_building = building_B;
	node_h->floor_number = 1;

	// 10-11. Intermediate Elevator Nodes (Same coordinates, different floors)
	map_node_t * node_p = create_map_node(create_cord(BASE_LAT + 5.1 * SCALE_UNIT, BASE_LON + 8.1 * SCALE_UNIT));
	set_map_node_name(node_p, "Elevator Shaft F2", &err_ctx);
	node_p->associated_building = building_B;
	node_p->floor_number = 2;

	map_node_t * node_o = create_map_node(create_cord(BASE_LAT + 5.1 * SCALE_UNIT, BASE_LON + 8.1 * SCALE_UNIT));
	set_map_node_name(node_o, "Elevator Shaft F3", &err_ctx);
	node_o->associated_building = building_B;
	node_o->floor_number = 3;
	
	// 12. Elevator Exit Lobby (Floor 4)
	map_node_t * node_q = create_map_node(create_cord(BASE_LAT + 5.1 * SCALE_UNIT, BASE_LON + 8.1 * SCALE_UNIT));
	set_map_node_name(node_q, "Elevator Exit F4", &err_ctx);
	node_q->associated_building = building_B;
	node_q->floor_number = 4;

	// 13. Final Destination (Floor 4)
	map_node_t * node_i = create_map_node(create_cord(BASE_LAT + 5.0 * SCALE_UNIT, BASE_LON + 8.5 * SCALE_UNIT));
	set_map_node_name(node_i, "Destination: Lab 401", &err_ctx);
	node_i->associated_building = building_B;
	node_i->floor_number = 4;
	
	// --- Add Nodes to Map ---
	add_node_to_map(&sys.map, node_a, &err_ctx);
	add_node_to_map(&sys.map, node_j, &err_ctx);
	add_node_to_map(&sys.map, node_k, &err_ctx);
	add_node_to_map(&sys.map, node_l, &err_ctx);
	add_node_to_map(&sys.map, node_m, &err_ctx);
	add_node_to_map(&sys.map, node_n, &err_ctx);
	add_node_to_map(&sys.map, node_g, &err_ctx);
	add_node_to_map(&sys.map, node_h, &err_ctx);
	add_node_to_map(&sys.map, node_p, &err_ctx); // F2
	add_node_to_map(&sys.map, node_o, &err_ctx); // F3
	add_node_to_map(&sys.map, node_q, &err_ctx); // F4
	add_node_to_map(&sys.map, node_i, &err_ctx);

	// --- Connect Nodes (Path: A -> J -> K -> L -> M -> N -> G -> H -> P -> O -> Q -> I) ---
	
	// Inside Library (Hallway & Exit)
	connect_nodes_in_map(&sys.map, node_a, node_j, EDGE_TYPE_HALLWAY, &err_ctx); // 1. Start -> Door
	
	// Outside Travel (J -> K -> L -> M -> N)
	connect_nodes_in_map(&sys.map, node_j, node_k, EDGE_TYPE_SIDEWALK, &err_ctx); // 2. Exit Door -> K (Long straight)
	connect_nodes_in_map(&sys.map, node_k, node_l, EDGE_TYPE_SIDEWALK, &err_ctx); // 3. K -> L (Straight turn)
	connect_nodes_in_map(&sys.map, node_l, node_m, EDGE_TYPE_SIDEWALK, &err_ctx); // 4. L -> M (Left turn)
	connect_nodes_in_map(&sys.map, node_m, node_n, EDGE_TYPE_SIDEWALK, &err_ctx); // 5. M -> N (Right turn)
	
	// Enter Science Center
	connect_nodes_in_map(&sys.map, node_n, node_g, EDGE_TYPE_DOOR, &err_ctx);    // 6. N -> Entrance (Door)
	
	// Inside Science Center (F1)
	connect_nodes_in_map(&sys.map, node_g, node_h, EDGE_TYPE_HALLWAY, &err_ctx); // 7. Entrance -> Elevator F1

	// *** ELEVATOR SHAFT CHAIN TEST *** (H -> P -> O -> Q)
	connect_nodes_in_map(&sys.map, node_h, node_p, EDGE_TYPE_ELEVATOR_SHAFT, &err_ctx); // 8. F1 -> F2
	connect_nodes_in_map(&sys.map, node_p, node_o, EDGE_TYPE_ELEVATOR_SHAFT, &err_ctx); // 9. F2 -> F3
	connect_nodes_in_map(&sys.map, node_o, node_q, EDGE_TYPE_ELEVATOR_SHAFT, &err_ctx); // 10. F3 -> F4
	
	// Inside Science Center (F4)
	connect_nodes_in_map(&sys.map, node_q, node_i, EDGE_TYPE_HALLWAY, &err_ctx); // 11. Elevator Exit F4 -> Destination F4
	
	// --- Find the path ---
	sys.active_start = node_a;
	sys.active_end = node_i;
	sys.active_edge_cost_function = calculate_wheelchair_edge_cost;
	
	find_best_path(&sys, &err_ctx);

	if (err_encountered(&err_ctx)) {
		if (!silent) fprintf(stderr, "Error finding path.\n");
		delete_building(building_A, &err_ctx);
		delete_building(building_B, &err_ctx);
		deinit_map_sys(&sys, &err_ctx);
		return FAIL;
	}

	if (sys.active_path == NULL) {
		if (!silent) printf("No path found.\n");
		delete_building(building_A, &err_ctx);
		delete_building(building_B, &err_ctx);
		deinit_map_sys(&sys, &err_ctx);
		return FAIL;
	}

	// --- Convert path to string (Test Execution) ---
	if (!silent) {
		printf("\n--- Path String Test (Elevator Skip) ---\n");
		printf("Path found from %s (F%d) to %s (F%d) with %zu nodes.\n", 
			sys.active_start->name, sys.active_start->floor_number, 
			sys.active_end->name, sys.active_end->floor_number, 
			sys.active_path->n_nodes);
		
		printf("Path nodes (F=Floor): ");
		for(size_t i = 0; i < sys.active_path->n_nodes; i++) {
			printf("%s(F%d) ", sys.active_path->nodes[i]->name, (int)sys.active_path->nodes[i]->floor_number);
		}
		printf("\n");
	}
	
	char * directions = convert_path_to_directions_str(sys.active_path, &err_ctx);

	if (err_encountered(&err_ctx) || directions == NULL) {
		if (!silent) fprintf(stderr, "Error converting path to string.\n");
		free(directions);
		delete_building(building_A, &err_ctx);
		delete_building(building_B, &err_ctx);
		deinit_map_sys(&sys, &err_ctx);
		return FAIL;
	}

	if (!silent) {
		printf("\n--- GENERATED DIRECTIONS (EXPECT 1 ELEVATOR INSTRUCTION) ---\n");
		printf("%s", directions);
		printf("-----------------------------------------------------------\n\n");
	}
	
	// NOTE: The resulting directions string should contain "Take the elevator to floor 4." exactly once.
	
	free(directions);
	delete_building(building_A, &err_ctx);
	delete_building(building_B, &err_ctx);
	deinit_map_sys(&sys, &err_ctx);
	
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}

bool path_string_test2(bool silent) {
	err_ctx_t err_ctx = create_err_ctx();
	
	map_sys_t sys = init_map_sys();
	
	// --- Create a more robust map for testing ---
	
	const double BASE_LAT = 39.33000;
	const double BASE_LON = -76.62000;
	const double SCALE_LAT = 0.00010; // ~36.5 ft
	const double SCALE_LON = 0.00012; // ~36.5 ft at this latitude

	// --- Create mock buildings ---
	map_rect_t bbox_A = create_map_rect(
		create_cord(BASE_LAT - 0.001, BASE_LON - 0.001), 
		create_cord(BASE_LAT + 0.001, BASE_LON + 0.001)
	);
	building_t * building_A = create_building("Library", bbox_A, 2, &err_ctx);

	map_rect_t bbox_B = create_map_rect(
		create_cord(BASE_LAT - 0.001, BASE_LON + 0.003), 
		create_cord(BASE_LAT + 0.001, BASE_LON + 0.005)
	);
	building_t * building_B = create_building("Science Center", bbox_B, 5, &err_ctx);

	if (err_encountered(&err_ctx)) {
		if (!silent) fprintf(stderr, "Error creating buildings for test.\n");
		if (building_A) delete_building(building_A, &err_ctx);
		if (building_B) delete_building(building_B, &err_ctx);
		deinit_map_sys(&sys,&err_ctx);
		return FAIL;
	}
	
	// A(0,0) - Inside Building A, Floor 1
	map_node_t * node_a = create_map_node(create_cord(BASE_LAT, BASE_LON));
	set_map_node_name(node_a,"A (Library Entrance)",&err_ctx);
	node_a->associated_building = building_A; 
	node_a->floor_number = 1;
	
	// B(1,2) - Outside, Sidewalk
	map_node_t * node_b = create_map_node(create_cord(BASE_LAT + 1*SCALE_LAT, BASE_LON + 2*SCALE_LON));
	set_map_node_name(node_b,"B (Sidewalk Corner)",&err_ctx);
	
	// D(2,0) - Outside, Corner
	map_node_t * node_d = create_map_node(create_cord(BASE_LAT + 2*SCALE_LAT, BASE_LON));
	set_map_node_name(node_d,"D (Path Start)",&err_ctx);
	
	// M(3,0) - Outside, Road start
	map_node_t * node_m = create_map_node(create_cord(BASE_LAT + 3*SCALE_LAT, BASE_LON));
	set_map_node_name(node_m,"M (Road Start)",&err_ctx);
	
	// N(3,1) - Outside, Road End (Turn)
	map_node_t * node_n = create_map_node(create_cord(BASE_LAT + 3*SCALE_LAT, BASE_LON + 1*SCALE_LON));
	set_map_node_name(node_n,"N (Road End)",&err_ctx);
	
	// G(4,1.5) - Inside Building B, Floor 1 (Entrance)
	map_node_t * node_g = create_map_node(create_cord(BASE_LAT + 4*SCALE_LAT, BASE_LON + 1.5*SCALE_LON));
	set_map_node_name(node_g,"G (Science Ctr Entrance)",&err_ctx);
	node_g->associated_building = building_B; 
	node_g->floor_number = 1;

	// J(5,2) - Inside Building B, Floor 1 (Office)
	map_node_t * node_j = create_map_node(create_cord(BASE_LAT + 5*SCALE_LAT, BASE_LON + 2*SCALE_LON));
	set_map_node_name(node_j,"J (Office F1)",&err_ctx);
	node_j->associated_building = building_B; 
	node_j->floor_number = 1;


	
	add_node_to_map(&sys.map,node_a,&err_ctx);
	add_node_to_map(&sys.map,node_b,&err_ctx);
	add_node_to_map(&sys.map,node_d,&err_ctx);
	add_node_to_map(&sys.map,node_m,&err_ctx);
	add_node_to_map(&sys.map,node_n,&err_ctx);
	add_node_to_map(&sys.map,node_g,&err_ctx);
	add_node_to_map(&sys.map,node_j,&err_ctx);
	
	// Connect nodes (Path: A -> B -> D -> M -> N -> G -> J)
	// These values must match your map.h and edge_type_names array
	connect_nodes_in_map(&sys.map,node_a,node_b,EDGE_TYPE_SIDEWALK,&err_ctx); 
	connect_nodes_in_map(&sys.map,node_b,node_d,EDGE_TYPE_SIDEWALK,&err_ctx); 
	connect_nodes_in_map(&sys.map,node_d,node_m,EDGE_TYPE_ROAD,&err_ctx); // Change in type
	connect_nodes_in_map(&sys.map,node_m,node_n,EDGE_TYPE_ROAD,&err_ctx); // Turn at M->N
	connect_nodes_in_map(&sys.map,node_n,node_g,EDGE_TYPE_DOOR,&err_ctx); // Door (Enter Bldg B)
	connect_nodes_in_map(&sys.map,node_g,node_j,EDGE_TYPE_HALLWAY,&err_ctx); // Interior Hallway
	
	// --- Find the path ---
	sys.active_start = node_a;
	sys.active_end = node_j; // Go all the way to Floor 1 Office
	// Set cost function (even though we're just testing path string conversion)
	sys.active_edge_cost_function = calculate_wheelchair_edge_cost; 
	
	find_best_path(&sys,&err_ctx);

	if (err_encountered(&err_ctx)) {
		if (!silent) fprintf(stderr, "Error finding path.\n");
		delete_building(building_A, &err_ctx); // Clean up buildings
		delete_building(building_B, &err_ctx); // Clean up buildings
		deinit_map_sys(&sys,&err_ctx);
		return FAIL;
	}

	if (sys.active_path == NULL) {
		if (!silent) printf("No path found.\n");
		delete_building(building_A, &err_ctx); 
		delete_building(building_B, &err_ctx); 
		deinit_map_sys(&sys,&err_ctx);
		return FAIL; 
	}

	// --- Convert path to string (the new part) ---
	if (!silent) {
		printf("\n--- Path String Test 2 ---\n");
		printf("Path found from %s to %s with %zu nodes.\n", 
			sys.active_start->name, sys.active_end->name, sys.active_path->n_nodes);
		
		printf("Path nodes: ");
		for(size_t i = 0; i < sys.active_path->n_nodes; i++) {
			printf("%s(%d) ", sys.active_path->nodes[i]->name, (int)sys.active_path->nodes[i]->floor_number);
		}
		printf("\n");
	}
	
	char * directions = convert_path_to_directions_str(sys.active_path, &err_ctx);

	if (err_encountered(&err_ctx) || directions == NULL) {
		if (!silent) fprintf(stderr, "Error converting path to string.\n");
		free(directions); 
		delete_building(building_A, &err_ctx); 
		delete_building(building_B, &err_ctx); 
		deinit_map_sys(&sys,&err_ctx);
		return FAIL;
	}

	if (!silent) {
		printf("\n--- GENERATED DIRECTIONS (Test 2) ---\n");
		printf("%s", directions);
		printf("-------------------------------------\n\n");
	}
	
	free(directions);
	delete_building(building_A, &err_ctx); 
	delete_building(building_B, &err_ctx); 
	deinit_map_sys(&sys,&err_ctx);
	
	if(err_encountered(&err_ctx)) return FAIL;
	
	return PASS;
}
