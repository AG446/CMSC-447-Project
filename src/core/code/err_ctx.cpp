#include "err_ctx.h"

err_ctx_t create_err_ctx(){
	err_ctx_t out;
	out.flags = 0;
	return out;
}

void reset_err_ctx(err_ctx_t * ctx){
	ctx->flags = 0;
}

void errs_to_output_stream(const err_ctx_t * ctx,FILE * stream){
	if(ctx->flags == 0){
		fputs("No errors encountered!\n",stream);
		return;
	}
	
	fputs("Errors Collected:\n",stream);
	fputs("\033[31m",stream);
	if((ctx->flags & ERROR_INVALID_PARAM) != 0){
		fputs("\tError: Invalid parameter!\n",stream);
	}else if((ctx->flags & ERROR_DUPLICATE_PARAMETER) != 0){
		fputs("\tError: Duplicate parameters received!\n",stream);
	}else if((ctx->flags & ERROR_OUT_OF_BOUNDS_INDEX) != 0){
		fputs("\tError: Index passed is out of bounds!\n",stream);
	}else if((ctx->flags & ERROR_OBJECT_NOT_FOUND) != 0){
		fputs("\tError: Object not found!\n",stream);
	}
	fputs("\033[0m",stream);
}

bool err_encountered(const err_ctx_t * ctx){
	return ctx->flags != 0;
}