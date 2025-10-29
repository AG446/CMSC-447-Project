#ifndef CL_TOOL_H
#define CL_TOOL_H

#include <stdint.h>
#include "map.h"

typedef struct Generic_Working_Object gwo_t;
typedef struct Generic_Working_Set gws_t;

#define GWO_CORD 1
#define GWO_MPO 2
#define GWO_RECT 3
#define GWO_NODE 4

struct Generic_Working_Object{
	union{
		cord_t cord;
		map_rect_t rect;
		mpo_t * mpo;
		map_node_t * node;
	};
	uint8_t type;
};

gwo_t create_blank_gwo();
gwo_t create_cord_gwo(cord_t cord);
gwo_t create_map_rect_gwo(map_rect_t rect);
gwo_t create_mpo_gwo(mpo_t * mpo,err_ctx_t * ctx);
gwo_t create_node_gwo(map_node_t * node,err_ctx_t * ctx);
void delete_gwo_data(gwo_t * gwo,err_ctx_t * ctx);

struct Generic_Working_Set{
	gwo_t * working_object_arr;
	size_t n_working_objects;
	size_t working_objects_capacity;
};

gws_t init_generic_working_set(void);
void deinit_generic_working_set(gws_t * gws,err_ctx_t * ctx);
void clear_gws_objects(gws_t * gws,err_ctx_t * ctx);
void add_gwo_to_gws(gws_t * gws,gwo_t gwo,err_ctx_t * ctx);
gwo_t remove_gwo_from_gws(gws_t * gws,size_t index,err_ctx_t * ctx);
gwo_t get_gwo_from_gws(gws_t * gws,size_t index,err_ctx_t * ctx);
gwo_t * remove_gwos_from_gws(gws_t * gws,size_t * indexes,size_t n_indexes,err_ctx_t * ctx);
void delete_gwo_from_gws(gws_t * gws,size_t index,err_ctx_t * ctx);
bool valid_gws_object(gws_t * gws,size_t index,uint8_t type,err_ctx_t * ctx);
void gws_to_output_stream(const gws_t gws,FILE * stream,err_ctx_t * ctx);//TODO in progress

char * read_line();
void start_cli();

#endif