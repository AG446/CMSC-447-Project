#include "../includes/tester.h"
#include <string.h> 
#include <stdio.h> // Keep for your map_to_output_stream calls

// ======================================================================
// Helper Functions for Setup
// ======================================================================

// Utility function to create a basic building for setup
building_t* create_test_building(const char* name) {
    cord_t bl = create_cord(-10.0, -10.0);
    cord_t tr = create_cord(10.0, 10.0);
    map_rect_t bbox = create_map_rect(bl, tr);
    return create_building(name, bbox, 3);
}

// Utility function to create a basic node for setup
map_node_t* create_test_node(double lon, double lat) {
    cord_t cord = create_cord(lon, lat);
    return create_map_node(cord);
}

// Helper to clean up map_path_t (as defined in your map.cpp code)
void delete_map_path(map_path_t * map_path_ref) {
	if(map_path_ref == NULL) return;
	free(map_path_ref->nodes);
	free(map_path_ref->name);
}

// ======================================================================
// A. YOUR ORIGINAL TESTS (Converted to GTest Format)
// ======================================================================

TEST(OriginalTests, BuildingDataStructureTest) {
	size_t n_floors = 4;
	const char * building_name = "Information Technology";
	map_rect_t building_bounding_box = create_map_rect(create_cord(-1,-2),create_cord(3,4));
	
	building_t * building = create_building(building_name,building_bounding_box,n_floors);
	
	
	add_building_alias_name(building,"ITE");
	add_building_alias_name(building,"Computing");
	add_building_alias_name(building,"Servers");
	add_building_alias_name(building,"Dumbo");
	add_building_alias_name(building,"Computer Lab");
	
	// This output is for visual inspection, not GTest assertion
	// building_to_output_stream(building,0,stdout);
	
	remove_building_alias_name(building,"Dumbo");
	
	// building_to_output_stream(building,0,stdout);
    
    // GTest Assertions for logic
    EXPECT_EQ(building->n_possible_names, 5); // 1 primary + 5 aliases - 1 removed
	
	remove_building_alias_name(building,"Computer Lab");
	remove_building_alias_name(building,"Information Technology");
	change_primary_building_name(building,"Computing");
	set_building_floor_count(building,5);
	
	// building_to_output_stream(building,0,stdout);
    
    EXPECT_STREQ(get_primary_building_name(building), "Computing");
    EXPECT_EQ(building->n_floors, 5);
	
	delete_building(building);
}

TEST(OriginalTests, NodeEdgeDataStructureTest) {
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
	
	// Output for visual inspection
	// map_node_to_output_stream(node_a,0,stdout);
	// map_node_to_output_stream(node_b,0,stdout);
	// map_edge_to_output_stream(edge,0,stdout);

    // GTest Assertions
    EXPECT_STREQ(node_a->name, "Alpha Dog");
    EXPECT_EQ(node_a->floor_number, 3);
    EXPECT_EQ(node_a->building, build);
    EXPECT_EQ(edge->type, EDGE_TYPE_RAMP);
	
	delete_map_edge(edge);
	delete_map_node(node_a);
	delete_map_node(node_b);
	delete_building(build);
}

TEST(OriginalTests, MPODataStructureTest) {
	cord_t square[4] = {create_cord(0,0), create_cord(0,1), create_cord(1,1), create_cord(1,0)};
	
	mpo_t * mpo = create_mpo(square,4,MPO_TYPE_BUILDING);
	// mpo_to_output_stream(mpo,0,stdout);

    EXPECT_EQ(mpo->type, MPO_TYPE_BUILDING);
    EXPECT_DOUBLE_EQ(mpo->cords[3].longitude, 1.0);
	
	set_mpo_type(mpo,MPO_TYPE_TREE);
	set_mpo_cord(mpo,3,create_cord(5,5));
	// mpo_to_output_stream(mpo,0,stdout);

    EXPECT_EQ(mpo->type, MPO_TYPE_TREE);
    EXPECT_DOUBLE_EQ(mpo->cords[3].longitude, 5.0);
    EXPECT_DOUBLE_EQ(mpo->cords[3].latitude, 5.0);
	
	delete_map_mpo(mpo);
}

TEST(OriginalTests, MapConstructionTest) {
	map_t map = init_map();
	map_node_t * n_a = create_test_node(1,1); set_map_node_name(n_a,"Node A"); add_node_to_map(&map,n_a);
	map_node_t * n_b = create_test_node(2,2); set_map_node_name(n_b,"Node B"); add_node_to_map(&map,n_b);
	map_node_t * n_c = create_test_node(3,3); set_map_node_name(n_c,"Node C"); add_node_to_map(&map,n_c);
	map_node_t * n_d = create_test_node(4,4); set_map_node_name(n_d,"Node D"); add_node_to_map(&map,n_d);
	
	connect_nodes_in_map_by_names(&map,"Node A","Node B",EDGE_TYPE_STAIRS);
	connect_nodes_in_map_by_names(&map,"Node C","Node B",EDGE_TYPE_STAIRS);
	
	// Map output for visual inspection
	// map_to_output_stream(map,0,stdout);

    EXPECT_EQ(map.n_edges, 2);
	
	map_node_t * a = get_node_from_map_by_name(&map,"Node A");
	
	connect_nodes_in_map_by_names(&map,"Node A","Node C",EDGE_TYPE_STAIRS);
	
	// map_to_output_stream(map,0,stdout);
    EXPECT_EQ(map.n_edges, 3);
	
	remove_node_from_map(&map,a);
	
	// map_to_output_stream(map,0,stdout);
    
    // Removing 'Node A' should remove 2 edges (A-B and A-C)
    EXPECT_EQ(map.n_nodes, 3); // Node A removed
    EXPECT_EQ(map.n_edges, 1); // Only C-B remains
    EXPECT_STREQ(map.all_nodes[0]->name, "Node B"); // Check shift/order
	
	clear_map(&map);
}


// ======================================================================
// B. NEW CORE DATA TESTS
// ======================================================================

TEST(CoreDataTest, CreateCordInitializesCorrectly) {
    cord_t c = create_cord(10.5, 20.7);
    EXPECT_DOUBLE_EQ(c.longitude, 10.5);
}

TEST(CoreDataTest, CreateMapRectInitializesCorrectly) {
    cord_t bl = create_cord(1.0, 2.0);
    cord_t tr = create_cord(3.0, 4.0);
    map_rect_t rect = create_map_rect(bl, tr);
    EXPECT_DOUBLE_EQ(rect.bottom_left.longitude, 1.0);
}

// ======================================================================
// C. PATH & SAVED PATHS TESTS
// ======================================================================

TEST(PathTest, CopyMapPathCopiesData) {
    map_path_t original = {0}; 
    original.n_nodes = 2;
    original.nodes = (map_node_t**)malloc(sizeof(map_node_t*) * 2);
    map_node_t* n1 = create_test_node(1, 1);
    map_node_t* n2 = create_test_node(2, 2);
    original.nodes[0] = n1;
    original.nodes[1] = n2;
    original.name = strdup("Route 1");

    map_path_t* copy = copy_map_path(&original);

    ASSERT_NE(copy->name, original.name); 
    EXPECT_STREQ(copy->name, "Route 1");
    EXPECT_EQ(copy->nodes[0], n1); 

    delete_map_path(copy);
    free(copy);
    delete_map_path(&original);
    free(original.nodes);
    free(original.name);
    delete_map_node(n1);
    delete_map_node(n2);
}

TEST(SavedPathsTest, InitAndClearSavedPaths) {
    saved_paths_t sp = init_saved_paths();
    EXPECT_EQ(sp.n_paths, 0);

    // Create a mock path to be "managed" by saved_paths_t
    map_path_t* p1 = (map_path_t*)malloc(sizeof(map_path_t));
    p1->nodes = NULL; p1->n_nodes = 0; p1->name = strdup("Mock Path");
    
    // Manually insert (simulating an add function)
    sp.paths = (map_path_t**)malloc(sizeof(map_path_t*));
    sp.paths[0] = p1;
    sp.n_paths = 1;
    sp.paths_capacity = 1;

    clear_saved_paths(&sp);
    
    // Test for NULL input (should safely return)
    clear_saved_paths(NULL);
}


// ======================================================================
// D. STRING SIMILARITY TESTS
// ======================================================================

TEST(StringSimilarityTest, TokenSimilarityScore_Identical) {
    EXPECT_FLOAT_EQ(token_similarity_score("library", "library"), 1.0f);
}

TEST(StringSimilarityTest, PhraseSimilarityScore_PartialAndOrder) {
    float score = phrase_similarity_score("info tech", "information technology");
    // Should be non-zero and less than 1.0
    EXPECT_GT(score, 0.0f); 
    EXPECT_LT(score, 1.0f);
}

// ======================================================================
// E. SERIALIZATION TESTS
// ======================================================================

TEST(SerializationTest, MPOToAndFromBinary) {
    cord_t cords[] = { create_cord(10.0, 20.0) };
    mpo_t* original_mpo = create_mpo(cords, 1, MPO_TYPE_WATER);
    size_t buffer_size = 0;
    
    uint8_t *buffer = convert_mpo_to_binary(original_mpo, &buffer_size);
    mpo_t *reconstructed_mpo = convert_binary_to_mpo(buffer);
    
    EXPECT_EQ(reconstructed_mpo->type, MPO_TYPE_WATER);
    EXPECT_DOUBLE_EQ(reconstructed_mpo->cords[0].longitude, 10.0);
    
    delete_map_mpo(original_mpo);
    delete_map_mpo(reconstructed_mpo);
    free(buffer);
}

TEST(SerializationTest, MapEdgeToAndFromBinary) {
    map_node_t* n1 = create_test_node(1, 1);
    map_node_t* n2 = create_test_node(2, 2);
    map_node_t *all_nodes_mock[2] = {n1, n2}; // Mock array for indexing logic
    map_edge_t* original_edge = create_map_edge(EDGE_TYPE_STAIRS, n1, n2);
    size_t buffer_size = 0;
    
    uint8_t *buffer = convert_map_edge_to_binary(original_edge, all_nodes_mock, &buffer_size); 
    map_edge_t *reconstructed_edge = convert_binary_to_map_edge(buffer, all_nodes_mock);
    
    EXPECT_EQ(reconstructed_edge->type, EDGE_TYPE_STAIRS);
    EXPECT_EQ(reconstructed_edge->a, n1); 
    
    // Clean up
    delete_map_edge(original_edge); 
    free(buffer);
    free(reconstructed_edge); 
    free(n1->outgoing_edges); free(n2->outgoing_edges);
    delete_map_node(n1); delete_map_node(n2);
}

// ======================================================================
// GTest MAIN FUNCTION
// ======================================================================

int main(int argc, char **argv) {
    // fputs("hello\n",stdout); // Removed, as GTest handles output
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    fputs("End of program\n",stdout); // Match original program flow
    return result;
}
