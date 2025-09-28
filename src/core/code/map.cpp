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
static uint8_t * convert_mpo_to_binary(mpo_t * mpo,size_t * buffer_size){
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
static mpo_t * convert_binary_to_mpo(uint8_t * buffer){
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

void file_save_test(){
	FILE *write_ptr;
	
	mpo_t mpo_test;
	mpo_test.type = 4;
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
	
	printf("%d %d\n",mpo_test->type,mpo_test->n_cords);
	for(size_t i = 0;i < mpo_test->n_cords;i++){
		printf("(%lf %lf),\n",mpo_test->cords[i].longitude,mpo_test->cords[i].latitude);
	}
	
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
	file_save_test();
	file_open_test();
	best_match_test();
}

map_node_t ** filter_locations(const char * location_name,const map_t * map_ref,size_t max_results){
	
}