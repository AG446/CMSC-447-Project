#include "map.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

static float token_similarity_score(const char * a,const char * b){
	size_t a_len = strlen(a);
	size_t b_len = strlen(b);
	
	
	if(a_len > b_len) return token_similarity_score(b,a);
	float score = 0;
	
	if(a_len <= 2){
		return strcmp(a,b) == 0 ? 1.0f : 0.0f;
	}
	
	bool used[64];
	for(int i = 0;i < 64;i++) used[i] = false;
	
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
	return score*((float)a_len)/((float)b_len);
}

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

static char * get_first_token(const char * input){
	size_t index_of_first_space;
	size_t index = 0;
	while(!(input[index] == '\0' || isspace(input[index]))){
		index++;
	}
	index_of_first_space = index;
	char * output = (char*) malloc(index_of_first_space+1);
	memcpy(output,input,index_of_first_space);
	for(size_t i = 0;i < index_of_first_space;i++) output[i] = tolower(output[i]);
	output[index_of_first_space] = '\0';
	return output;
}

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

static void delete_tokens(char ** tokens,size_t n_tokens){
	for(size_t i = 0;i < n_tokens;i++){
		free(tokens[i]);
	}
	free(tokens);
}

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

void do_thing(){
	char input[64];
	const char * locations[6] = {
		"Information Technology Computing ITE",
		"Library",
		"Engineering ENG",
		"Interdisciplinary Life Sciences ILS",
		"Mathematics MATH",
		"Janet and Walter Sondheim"
	};
	
	while(true){
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
}

map_node_t ** filter_locations(const char * location_name,const map_t * map_ref,size_t max_results){
	
}