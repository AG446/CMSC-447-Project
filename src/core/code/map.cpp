#include "map.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

/*
 * returns a number between 0 and 1 which corresponds to how similar to tokens are
 * 0.0 means the two tokens are not similar
 * 1.0 menas the strings are identical
 */
static float token_similarity_score(const char * a,const char * b){
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);
	
	
	if(a_len > b_len) return token_similarity_score(b,a);
	float score = 0;
	
	if(strcmp(a,b) == 0) return 1.0f;
	
	bool * used = (bool*) malloc(sizeof(bool)*b_len);
	for(size_t i = 0;i < b_len;i++) used[i] = false;
	
	for(size_t i = 0;i < b_len;i++){
		float this_loop_correctness = 0.0f;
		bool last_correct = false;
		for(size_t j = 0;j < a_len;j++){
			size_t first_index = i+j;
			size_t second_index = j;
			
			char b_char;
			if(first_index < b_len){
				b_char = b[first_index];
			}else{
				b_char = '\0';
			}
			char a_char = a[second_index];
			
			//printf("%c %c\n",a_char,b_char);
			
			if(a_char == b_char){
				if(!used[first_index]) this_loop_correctness += last_correct ? 1.0f : 0.5f;
				used[first_index] = true;
				last_correct = true;
			}else{
				last_correct = false;
			}
			
		}
		//printf("\n");
		
		if(this_loop_correctness <= 2.0f) continue;
		float correctness_percentage = this_loop_correctness/a_len;
		score += correctness_percentage;
	}
	free(used);
	
	return score*((float)a_len)/((float)b_len);
}

/*
 * Counts the number of tokens in a string seperated by spaces or tabs
 */
static size_t count_tokens(const char * input){
	size_t token_count = 0;
	bool last_was_space = true;
	for(size_t i = 0; input[i] != '\0';i++){
		char c = input[i];
		bool space = isspace(c);
		
		if(!space && last_was_space){
			token_count++;
		}
		last_was_space = space;
	}
	
	return token_count;
}

/*
 * convert a c-string to lowercase
 */
static void c_str_lowercase(char * str){
	size_t str_len = strlen(str);
	for(size_t i = 0;i < str_len;i++){
		str[i] = tolower(str[i]);
	}
}

/*
 * Returns a new c-string on the heap which is the first token in the input string
 * It also converts the string to lower case
 */
static char * get_first_token(const char * input){
	size_t index_of_first_space;
	size_t index = 0;
	while(!(input[index] == '\0' || isspace(input[index]))){
		index++;
	}
	index_of_first_space = index;
	char * output = (char*) malloc(index_of_first_space+1);
	memcpy(output,input,index_of_first_space);
	output[index_of_first_space] = '\0';
	c_str_lowercase(output);
	return output;
}

/*
 * Splits an input string into its tokens seperated by spaces or tabs
 */
static char ** split_into_tokens(const char * input,size_t * n_tokens_out){
	size_t token_count = count_tokens(input);
	*n_tokens_out = token_count;
	
	char ** tokens = (char**)malloc(sizeof(char*)*token_count);
	
	bool last_was_space = true;
	size_t at = 0;
	for(size_t i = 0; input[i] != '\0';i++){
		char c = input[i];
		bool space = isspace(c);
		
		if(!space && last_was_space){
			tokens[at] = get_first_token(input+i);
			at++;
		}
		last_was_space = space;
	}
	
	return tokens;
}

/*
 * Delete tokens from the heap
 */
static void delete_tokens(char ** tokens,size_t n_tokens){
	for(size_t i = 0;i < n_tokens;i++){
		free(tokens[i]);
	}
	free(tokens);
}

/*
 * Compare phrases by trying to re-order tokens
 */
float phrase_similarity_score(const char * phrase_1,const char * phrase_2){
	size_t n_phrase_1_tokens;
	char ** phrase_1_tokens = split_into_tokens(phrase_1,&n_phrase_1_tokens);
	
	size_t n_phrase_2_tokens;
	char ** phrase_2_tokens = split_into_tokens(phrase_2,&n_phrase_2_tokens);
	
	float score = 0.0f;
	
	for(size_t i = 0;i < n_phrase_1_tokens;i++){
		for(size_t j = 0;j < n_phrase_2_tokens;j++){
			char * phrase_1_token = phrase_1_tokens[i];
			char * phrase_2_token = phrase_2_tokens[j];
			score += token_similarity_score(phrase_1_token,phrase_2_token);
		}
	}
	
	delete_tokens(phrase_2_tokens,n_phrase_2_tokens);
	delete_tokens(phrase_1_tokens,n_phrase_1_tokens);
	
	return score;
}

/*
 * convert a map polygon object into a stream of bytes
 */
static uint8_t * convert_mpo_to_binary(const mpo_t * mpo,size_t * buffer_size){
	*buffer_size = sizeof(uint8_t)+sizeof(size_t)+sizeof(coord_t)*mpo->n_cords;
	uint8_t * buffer = (uint8_t *) malloc(*buffer_size);
	
	size_t current_offset = 0;
	
	memcpy(buffer+current_offset,&(mpo->type),sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(buffer+current_offset,&(mpo->n_cords),sizeof(size_t));
	current_offset += sizeof(size_t);
	
	memcpy(buffer+current_offset,mpo->cords,sizeof(coord_t)*mpo->n_cords);
	
	return buffer;
}

/*
 * convert a stream of bytes into a map polygon object
 */
static mpo_t * convert_binary_to_mpo(const uint8_t * buffer){
	mpo_t * out = (mpo_t*) malloc(sizeof(mpo_t));
	
	size_t current_offset = 0;
	
	memcpy(&(out->type),buffer+current_offset,sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(&(out->n_cords),buffer+current_offset,sizeof(size_t));
	current_offset += sizeof(size_t);
	
	out->cords = (coord_t*) malloc(sizeof(coord_t)*out->n_cords);
	
	memcpy(out->cords,buffer+current_offset,sizeof(coord_t)*out->n_cords);
	
	return out;
}


/*
 * get index of node
 */
size_t get_node_index(map_node_t * node,map_node_t ** all_nodes){
	return ((size_t)node-((size_t)&(all_nodes[0])))/sizeof(map_node_t);
}

/*
 * convert a map edge object into a stream of bytes
 */
static uint8_t * convert_map_edge_to_binary(const map_edge_t * edge,map_node_t ** all_nodes,size_t * buffer_size){
	*buffer_size = sizeof(uint8_t)+sizeof(size_t)+sizeof(size_t);
	uint8_t * buffer = (uint8_t *) malloc(*buffer_size);
	
	size_t current_offset = 0;
	
	memcpy(buffer+current_offset,&(edge->type),sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	size_t node_a_index = get_node_index(edge->a,all_nodes);
	memcpy(buffer+current_offset,&(node_a_index),sizeof(size_t));
	current_offset += sizeof(size_t);
	
	size_t node_b_index = get_node_index(edge->b,all_nodes);
	memcpy(buffer+current_offset,&(node_b_index),sizeof(size_t));
	
	return buffer;
}

/*
 * convert a stream of bytes into an edge object
 */
static map_edge_t * convert_binary_to_map_edge(const uint8_t * buffer,map_node_t ** all_nodes){
	map_edge_t * out = (map_edge_t*) malloc(sizeof(map_edge_t));
	
	size_t current_offset = 0;
	
	memcpy(&(out->type),buffer+current_offset,sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	size_t index;
	memcpy(&(index),buffer+current_offset,sizeof(size_t));
	out->a = all_nodes[index];
	current_offset += sizeof(size_t);
	
	memcpy(&(index),buffer+current_offset,sizeof(size_t));
	out->b = all_nodes[index];
	
	return out;
}

/*
 * convert a map node object into a stream of bytes
 */
static uint8_t * convert_map_node_to_binary(const map_node_t * node,size_t * buffer_size){
	size_t total_alias_name_length = 0;
	for(size_t i = 0;i < node->n_possible_names;i++){
		char * string = node->possible_names[i];
		size_t string_len = strlen(string);
		total_alias_name_length += string_len+1;
	}
	
	size_t picture_file_path_len = 0;
	if(node->picture_file_path != NULL) picture_file_path_len = strlen(node->picture_file_path);
	
	*buffer_size = 
		sizeof(uint8_t) + //type
		sizeof(coord_t) + //coordinate
		sizeof(size_t) + //n_possible_names
		total_alias_name_length + //array of strings
		picture_file_path_len+1 + //picture file path name
		sizeof(size_t);//n_outgoing_edges
	
	uint8_t * buffer = (uint8_t *) malloc(*buffer_size);
	
	size_t current_offset = 0;
	
	memcpy(buffer+current_offset,&(node->type),sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(buffer+current_offset,&(node->coordinate),sizeof(coord_t));
	current_offset += sizeof(coord_t);
	
	memcpy(buffer+current_offset,&(node->n_possible_names),sizeof(size_t));
	current_offset += sizeof(size_t);
	
	
	for(size_t i = 0;i < node->n_possible_names;i++){
		char * string = node->possible_names[i];
		size_t string_len = strlen(string);
		memcpy(buffer+current_offset,string,string_len+1);
		current_offset += string_len+1;
	}
	
	if(node->picture_file_path != NULL){
		memcpy(buffer+current_offset,node->picture_file_path,picture_file_path_len+1);
		current_offset += picture_file_path_len+1;
	}else{
		memcpy(buffer+current_offset,"\0",1);
		current_offset++;
	}
	
	memcpy(buffer+current_offset,&(node->n_outgoing_edges),sizeof(size_t));
	
	return buffer;
}

/*
 * convert a stream of bytes into an node object
 */
static map_node_t * convert_binary_to_map_node(const uint8_t * buffer){
	map_node_t * out = (map_node_t*) malloc(sizeof(map_node_t));
	
	size_t current_offset = 0;
	
	memcpy(&(out->type),buffer+current_offset,sizeof(uint8_t));
	current_offset += sizeof(uint8_t);
	
	memcpy(&(out->coordinate),buffer+current_offset,sizeof(coord_t));
	current_offset += sizeof(coord_t);
	
	memcpy(&(out->n_possible_names),buffer+current_offset,sizeof(size_t));
	current_offset += sizeof(size_t);
	
	out->possible_names = (char**) malloc(sizeof(char*)*out->n_possible_names);
	
	size_t next_null_char_index;
	for(size_t i = 0; i < out->n_possible_names;i++){
		next_null_char_index = 0;
		while((char)buffer[current_offset+next_null_char_index] != '\0'){
			next_null_char_index++;
		}
		char * possible_name = (char*) malloc(next_null_char_index+1);
		memcpy(possible_name,buffer+current_offset,next_null_char_index+1);
		current_offset+=next_null_char_index+1;
		out->possible_names[i] = possible_name;
	}
	next_null_char_index = 0;
	while((char)buffer[current_offset+next_null_char_index] != '\0'){
		next_null_char_index++;
	}
	if(next_null_char_index == 0){
		out->picture_file_path = NULL;
		current_offset++;
	}else{
		out->picture_file_path = (char*) malloc(next_null_char_index+1);
		memcpy(out->picture_file_path,buffer+current_offset,next_null_char_index+1);
		current_offset += next_null_char_index+1;
	}
	
	memcpy(&(out->n_outgoing_edges),buffer+current_offset,sizeof(size_t));
	
	out->outgoing_edges = (map_edge_t**) malloc(sizeof(map_edge_t*)*out->n_outgoing_edges);
	for(size_t i = 0;i < out->n_outgoing_edges;i++) out->outgoing_edges[i] = NULL;
	
	return out;
}

void coordinate_to_string(coord_t coordinate,FILE * stream){
	fprintf(stream,"lon = %lf, lat = %lf",coordinate.longitude,coordinate.latitude);
}

void mpo_to_string(const mpo_t * mpo_ref,FILE * stream){
	fprintf(stream,"Map-Polygon-Object: %p\n",mpo_ref);
	fprintf(stream,"\tpoints:\n");
	for(size_t i = 0;i < mpo_ref->n_cords;i++){
		fputs("\t\t",stream);
		coordinate_to_string(mpo_ref->cords[i],stream);
		fputc('\n',stream);
	}
	fprintf(stream,"\ttype:\n\t\t");
	if(mpo_ref->type == 1){
		fputs("WATER",stream);
	}else if(mpo_ref->type == 2){
		fputs("TREE",stream);
	}else if(mpo_ref->type == 3){
		fputs("BUILDING",stream);
	}
	fputc('\n',stream);
}

void map_node_to_string(const map_node_t * node_ref,FILE * stream){
	fprintf(stream,"Map-Node: %p\n",node_ref);
	fputs("\tlocation:\n",stream);
	fputs("\t\t",stream);
	coordinate_to_string(node_ref->coordinate,stream);
	fprintf(stream,"\n\tnames:\n");
	for(size_t i = 0;i < node_ref->n_possible_names;i++){
		fputs("\t\t",stream);
		fputs(node_ref->possible_names[i],stream);
		fputc('\n',stream);
	}
	fputs("\tpicture file path:\n",stream);
	if(node_ref->picture_file_path == NULL){
		fputs("\t\tNULL\n",stream);
	}else{
		fprintf(stream,"\t\t%s\n",node_ref->picture_file_path);
	}
	fputs("\ttype:\n\t\t",stream);
	if(node_ref->type == 1){
		fputs("NOTABLE LOCATION",stream);
	}else if(node_ref->type == 2){
		fputs("BASIC",stream);
	}
	fputc('\n',stream);
	fprintf(stream,"\tn outgoing edges:\n\t\t%ld\n",node_ref->n_outgoing_edges);
	fprintf(stream,"\toutgoing edges:\n");
	for(size_t i = 0;i < node_ref->n_outgoing_edges;i++){
		fprintf(stream,"\t\t%p\n",node_ref->outgoing_edges[i]);
	}
}

void file_save_test(){
	FILE *write_ptr;
	
	mpo_t mpo_test;
	mpo_test.type = MPO_TYPE_TREE;
	mpo_test.n_cords = 4;
	mpo_test.cords = (coord_t*) malloc(sizeof(coord_t)*mpo_test.n_cords);
	mpo_test.cords[0] = {0.0,0.0};
	mpo_test.cords[1] = {0.0,1.0};
	mpo_test.cords[2] = {1.0,1.0};
	mpo_test.cords[3] = {1.0,0.0};
	
	size_t buffer_size;
	uint8_t * buffer = convert_mpo_to_binary(&mpo_test,&buffer_size);
	
	write_ptr = fopen("test.bin","wb");  // w for write, b for binary

	fwrite(buffer,buffer_size,1,write_ptr); // write 10 bytes from our buffer
	
	free(mpo_test.cords);
	
	fclose(write_ptr);
}

void file_open_test(){
	FILE *read_ptr;
	
	mpo_t * mpo_test;
	
	read_ptr = fopen("test.bin","rb");

	uint8_t buffer[256];
	for(int i = 0;i < 256;i++) buffer[i] = 0;
	
	size_t file_size;
	
	fseek(read_ptr, 0, SEEK_END); // seek to end of file
	file_size = ftell(read_ptr); // get current file pointer
	fseek(read_ptr, 0, SEEK_SET);
	
	fread(buffer,file_size,1,read_ptr);
	
	mpo_test = convert_binary_to_mpo(buffer);
	
	/*
	printf("%d %ld\n",mpo_test->type,mpo_test->n_cords);
	for(size_t i = 0;i < mpo_test->n_cords;i++){
		printf("(%lf %lf),\n",mpo_test->cords[i].longitude,mpo_test->cords[i].latitude);
	}
	*/
	mpo_to_string(mpo_test, stdout);
	
	fclose(read_ptr);
}

void best_match_test(){
	char input[64];
	const char * locations[6] = {
		"Information Technology Computing ITE",
		"Library",
		"Engineering ENG",
		"Interdisciplinary Life Sciences ILS",
		"Mathematics MATH",
		"Janet and Walter Sondheim"
	};

	input[0] = '\0';
	scanf(" %[^\n]",input);
	const char * best = NULL;
	
	float max = 0.0f;
	for(size_t i = 0;i < 6;i++){
		const char * loc = locations[i];
	float current_score = phrase_similarity_score(input,loc);
		if(current_score > max){
			max = current_score;
			best = loc;
		}
	}
	
	printf("Best Guess is : %s\n",best);
}

void do_thing(){
	map_node_t * node = (map_node_t *) malloc(sizeof(map_node_t));
	node->coordinate.longitude = 2.5;
	node->coordinate.latitude = -3.1;
	node->n_possible_names = 3;
	
	char * temp;
	
	node->possible_names = (char**) malloc(sizeof(char*)*3);
	temp = (char*) malloc(5);
	strcpy(temp,"UMBC");
	node->possible_names[0] = temp;
	temp = (char*) malloc(3);
	strcpy(temp,"hi");
	node->possible_names[1] = temp;
	temp = (char*) malloc(4);
	strcpy(temp,"mum");
	node->possible_names[2] = temp;
	temp = (char*) malloc(9);
	strcpy(temp,"test.png");
	node->picture_file_path = temp;
	node->type = NODE_TYPE_NOTABLE_LOCATION;
	node->n_outgoing_edges = 2;
	node->outgoing_edges = (map_edge_t**)malloc(sizeof(map_edge_t*)*node->n_outgoing_edges);
	for(size_t i = 0;i < 2;i++) node->outgoing_edges[i] = NULL;
	map_node_to_string(node,stdout);
	
	size_t buffer_size;
	uint8_t * buffer = convert_map_node_to_binary(node,&buffer_size);
	for(size_t i = 0;i < buffer_size;i++){
		printf("%x ",buffer[i]);
	}
	printf("\n");
	
	map_node_t * regen = convert_binary_to_map_node(buffer);
	map_node_to_string(regen,stdout);
	
	//file_save_test();
	file_open_test();
	//best_match_test();
}

/*
map_node_t ** filter_locations(const char * location_name,const map_t * map_ref,size_t max_results){
	
}
*/