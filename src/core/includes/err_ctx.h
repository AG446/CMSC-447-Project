#ifndef ERR_CTX_H
#define ERR_CTX_H

#include <stdint.h>
#include <stdio.h>

#define ERROR_INVALID_PARAM 1
#define ERROR_DUPLICATE_PARAMETER 2
#define ERROR_OUT_OF_BOUNDS_INDEX 4
#define ERROR_OBJECT_NOT_FOUND 8

typedef struct Error_Context err_ctx_t;

struct Error_Context{
	uint8_t flags;
};

//create a new error context. Not on heap
err_ctx_t create_err_ctx();

//was an error encountered during a function call
bool err_encountered(const err_ctx_t * ctx);

//remove all errors
void reset_err_ctx(err_ctx_t * ctx);

//print the errors in human readable form
void errs_to_output_stream(const err_ctx_t * ctx,FILE * stream);

#endif