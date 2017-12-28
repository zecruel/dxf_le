#ifndef _DXF_LIB
#define _DXF_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define DXF_MAX_LAYERS 50
#define DXF_MAX_LTYPES 50
#define DXF_MAX_FONTS 50
#define DXF_MAX_CHARS 250
#define DXF_MAX_PAT 10
#define DXF_POOL_PAGES 1000

/* supportable graphic entities */
enum dxf_graph {
	DXF_NONE,
	DXF_LINE,
	DXF_POINT,
	DXF_CIRCLE,
	DXF_ARC,
	DXF_TRACE,
	DXF_SOLID,
	DXF_TEXT,
	DXF_SHAPE,
	DXF_INSERT,
	DXF_ATTRIB,
	DXF_POLYLINE,
	DXF_VERTEX,
	DXF_LWPOLYLINE,
	DXF_3DFACE,
	DXF_VIEWPORT,
	DXF_DIMENSION,
	DXF_ELLIPSE,
	DXF_MTEXT,
	DXF_BLK,
	DXF_ENDBLK
};

enum dxf_pool_action{
	ADD_DXF,
	ZERO_DXF,
	FREE_DXF
};

enum dxf_pool_life{
	DWG_LIFE = 0,
	FRAME_LIFE = 1,
	ONE_TIME = 2
};

struct Dxf_pool_slot{
	void *pool[DXF_POOL_PAGES];
	/* the pool is a vector of pages. The size of each page is out of this definition */
	int pos; /* current position in current page vector */
	int page; /* current page index */
	int size; /* number of pages available in slot */
};
typedef struct Dxf_pool_slot dxf_pool_slot;

struct Dxf_node{
	struct Dxf_node *master; /* entity to which it is contained */
	struct Dxf_node *next;    /* next node (double linked list) */
	struct Dxf_node *prev;    /* previous node (double linked list) */
	struct Dxf_node *end; /*last object in list*/
	
	/* defines whether it is an DXF entity (obj) or an attribute (value) */
	enum {DXF_ENT, DXF_ATTR} type;
	
	union{
		struct {
			/* == entity dxf especific */
			char name[DXF_MAX_CHARS]; /* standardized DXF name of entity */
			
			void * graphics; /* graphics information */
			
			struct Dxf_node *content; /* the content is a list */
		} obj;
		
		struct {
			/* ==group dxf especific */
			int group; /* standardized DXF group */
			/* the group defines the type of data, which can be: */
			enum {DXF_FLOAT, DXF_INT, DXF_STR} t_data;
			union {
				double d_data; /* a float number, */
				int i_data; /* a integer number, */
				char s_data[DXF_MAX_CHARS]; /* or a string. */
			};
		} value;
	};
}; 
typedef struct Dxf_node dxf_node;

struct Dxf_layer{
	char name[DXF_MAX_CHARS];
	int color;
	char ltype[DXF_MAX_CHARS];
	int line_w;
	int frozen;
	int lock;
	int off;
};
typedef struct Dxf_layer dxf_layer;

struct Dxf_ltype{
	char name[DXF_MAX_CHARS];
	char descr[DXF_MAX_CHARS];
	int size;
	double pat[DXF_MAX_PAT];
	double length;
};
typedef struct Dxf_ltype dxf_ltype;

struct Dxf_tfont{
	char name[DXF_MAX_CHARS];
	void *shx_font;
};
typedef struct Dxf_tfont dxf_tfont;

struct Dxf_drawing{
	/* DXF main sections */
	dxf_node 	
	*head,  /* header with drawing variables */
	*tabs,  /* tables - see next */
	*blks, /* blocks definitions */
	*ents; /* entities - grahics elements */
	
	/* DXF tables */
	dxf_node 
	*t_ltype, /* line types */ 
	*t_layer,  /* layers */
	*t_style,  /* text styles, text fonts */
	*t_view,  /* views */
	*t_ucs,   /* UCS - coordinate systems */
	*t_vport,  /* viewports (useful in layout mode) */
	*t_dimst, /* dimension styles*/
	*blks_rec, /* blocks records */
	*t_appid; /* third part application indentifier */
	
	/* complete structure access */
	dxf_node *main_struct;
	
	dxf_node *hand_seed; /* handle generator */
	
	dxf_layer layers[DXF_MAX_LAYERS];
	int num_layers;
	
	dxf_ltype ltypes[DXF_MAX_LTYPES];
	int num_ltypes;
	
	dxf_tfont text_fonts[DXF_MAX_FONTS];
	int num_fonts;
	
};
typedef struct Dxf_drawing dxf_drawing;

/* structure to store vectors (arrays) */
struct Vector_p{
	int size;
	void *data;
};
typedef struct Vector_p vector_p;

/* functions*/
void str_upp(char *str);

char * trimwhitespace(char *str);

vector_p * vect_new (void);

int stack_push (vector_p *stack, void *new_ptr);

void * stack_pop (vector_p *stack);

void dxf_ent_print2 (dxf_node *ent);

void dxf_ent_print_f (dxf_node *ent, char *path);

dxf_node * dxf_obj_new (char *name);

int dxf_ident_attr_type (int group);

int dxf_ident_ent_type (dxf_node *obj);

dxf_node * dxf_attr_new (int group, int type, void *value);

vector_p dxf_find_attr(dxf_node * obj, int attr);

dxf_node * dxf_find_attr2(dxf_node * obj, int attr);

dxf_node * dxf_find_attr_i(dxf_node * obj, int attr, int idx);

vector_p dxf_find_obj(dxf_node * obj, char *name);

dxf_node * dxf_find_obj_i(dxf_node * obj, char *name, int idx);

vector_p dxf_find_obj_descr(dxf_node * obj, char *name, char *descr);

dxf_node * dxf_find_obj_descr2(dxf_node * obj, char *name, char *descr);

void dxf_layer_assemb (dxf_drawing *drawing);

void dxf_ltype_assemb (dxf_drawing *drawing);

int dxf_lay_idx (dxf_drawing *drawing, char *name);

int dxf_ltype_idx (dxf_drawing *drawing, char *name);

int dxf_save (char *path, dxf_drawing *drawing);

char * dxf_load_file(char *path, long *fsize);

int dxf_read (dxf_drawing *drawing, char *buf, long fsize, int *prog);



#endif