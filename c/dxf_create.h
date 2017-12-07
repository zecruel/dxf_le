#ifndef _DXF_CREATE_LIB
#define _DXF_CREATE_LIB
#define ACT_CHARS 32
#define DO_PAGES 1000
#define DO_PAGE 10000

#include "dxf.h"
#include "list.h"

struct do_item {
	struct do_item *prev;
	struct do_item *next;
	dxf_node *old_obj;
	dxf_node *new_obj;
};

struct do_entry {
	struct do_entry *prev;
	struct do_entry *next;
	char text[ACT_CHARS];
	struct do_item *list;
	struct do_item *current;
};

struct do_list {
	int count;
	struct do_entry *list;
	struct do_entry *current;
};

enum do_pool_action{
	ADD_DO_ITEM,
	ZERO_DO_ITEM,
	ADD_DO_ENTRY,
	ZERO_DO_ENTRY,
	FREE_DO_ALL
};

struct do_pool_slot{
	void *pool[DO_PAGES];
	/* the pool is a vector of pages. The size of each page is out of this definition */
	int pos; /* current position in current page vector */
	int page; /* current page index */
	int size; /* number of pages available in slot */
};

int do_add_item(struct do_entry *entry, dxf_node *old_obj, dxf_node *new_obj);

int do_add_entry(struct do_list *list, char *text);

int init_do_list(struct do_list *list);

int do_undo(struct do_list *list);

int do_redo(struct do_list *list);

int dxf_obj_subst(dxf_node *orig, dxf_node *repl);

int dxf_obj_append(dxf_node *master, dxf_node *obj);

int dxf_obj_detach(dxf_node *obj);

int dxf_attr_append(dxf_node *master, int group, void *value);

int dxf_attr_change(dxf_node *master, int group, void *value);

int dxf_attr_change_i(dxf_node *master, int group, void *value, int idx);

int dxf_find_ext_appid(dxf_node *obj, char *appid, dxf_node **start, dxf_node **end);

int dxf_ext_append(dxf_node *master, char *appid, int group, void *value);

void dxf_obj_transverse(dxf_node *source);

dxf_node *dxf_ent_copy(dxf_node *source, int pool_dest);

dxf_node * dxf_new_line (double x0, double y0, double z0,
double x1, double y1, double z1,
double thick, double elev, int color, char *layer, char *ltype, int paper);

dxf_node * dxf_new_lwpolyline (double x0, double y0, double z0,
double bulge, double thick, int color, char *layer, char *ltype, int paper);

int dxf_lwpoly_append (dxf_node * poly,
double x0, double y0, double z0, double bulge);

int dxf_lwpoly_remove (dxf_node * poly, int idx);

dxf_node * dxf_new_circle (double x0, double y0, double z0,
double r, double thick, int color, char *layer, char *ltype, int paper);

dxf_node * dxf_new_text (double x0, double y0, double z0, double h,
char *txt, double thick, int color, char *layer, char *ltype, int paper);

int dxf_edit_move2 (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z);

int dxf_edit_move (dxf_node * obj, double ofs_x, double ofs_y, double ofs_z);

int dxf_edit_scale (dxf_node * obj, double scale_x, double scale_y, double scale_z);

void drawing_ent_append(dxf_drawing *drawing, dxf_node *element);

#endif