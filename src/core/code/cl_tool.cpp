#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cl_tool.h"
#include "text_proc.h"

char * read_line(){
	fputs("> ",stdout);
	fflush(stdout);
	
	size_t allocated_space = 16;
	char * output = (char*) malloc(allocated_space);
	size_t current_length = 0;
	
	int val;
	while((val = getc(stdin)) != '\n'){
		if(allocated_space == current_length){
			allocated_space *= 2;
			output = (char*) realloc(output,allocated_space);
		}
		output[current_length] = (char)val;
		current_length++;
	}
	
	if(allocated_space == current_length){
		allocated_space *= 2;
		output = (char*) realloc(output,allocated_space);
	}
	output[current_length] = '\0';
	
	return output;
}

static bool is_halting_string(const char * str){
	if(str == NULL) return false;
	
	if(strcmp(str,"stop") == 0) return true;
	if(strcmp(str,"halt") == 0) return true;
	if(strcmp(str,"quit") == 0) return true;
	if(strcmp(str,"exit") == 0) return true;
	
	return false;
}

void start_cli(){
	bool running = true;
	while(running){
		char * line = read_line();
		char * lowercase_line = (char*) malloc(strlen(line)+1);
		strcpy(lowercase_line,line);
		c_str_lowercase(lowercase_line);
		
		if(is_halting_string(lowercase_line)){
			running = false;
		}
		
		free(lowercase_line);
		free(line);
	}
}