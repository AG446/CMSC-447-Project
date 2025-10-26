#ifndef TEXT_PROC_H
#define TEXT_PROC_H

#include <stdlib.h>

//convert a c-string to lowercase
void c_str_lowercase(char * str);

/*
 * returns a number between 0 and 1 which corresponds to how similar to tokens are
 * 0.0 means the two tokens are not similar
 * 1.0 menas the strings are identical
 */
float token_similarity_score(const char * a,const char * b);

//Counts the number of tokens in a string seperated by spaces or tabs
size_t count_tokens(const char * input);

//Splits an input string into its tokens seperated by spaces or tabs
char ** split_into_tokens(const char * input,size_t * n_tokens_out);

//Delete tokens from the heap
void delete_tokens(char ** tokens,size_t n_tokens);

/*
 * Returns a new c-string on the heap which is the first token in the input string
 * It also converts the string to lower case
 */
char * get_first_token(const char * input);

//Compare phrases by trying to re-order tokens. Note: This number can be greater than 1.0
float phrase_similarity_score(const char * phrase_1,const char * phrase_2);

#endif