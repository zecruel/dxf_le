#ifndef _GRAPH_LIB
#define _GRAPH_LIB

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>

#include "dxf.h"
#include "bmp.h"
#include "list.h"

#define MAX_SPLINE_PTS 1000
#define GRAPH_POOL_PAGES 1000

enum graph_pool_action{
	ADD_GRAPH,
	ADD_LINE,
	ZERO_GRAPH,
	ZERO_LINE,
	FREE_ALL
};

struct Graph_pool_slot{
	void *pool[GRAPH_POOL_PAGES];
	/* the pool is a vector of pages. The size of each page is out of this definition */
	int pos; /* current position in current page vector */
	int page; /* current page index */
	int size; /* number of pages available in slot */
};
typedef struct Graph_pool_slot graph_pool_slot;

struct Line_node{
	double x0, y0, z0, x1, y1, z1;
	struct Line_node * next;
};
typedef struct Line_node line_node;

struct Graph_obj{
	int pool_idx;
	dxf_node * owner;
	bmp_color color;
	double rot, scale, ofs_x, ofs_y, ofs_z;
	double tick;
	int thick_const;
	/* pattern information */
	double pattern[20];
	int patt_size;
	/* extent information */
	double ext_min_x, ext_min_y;
	double ext_max_x, ext_max_y;
	int ext_ini;
	/*fill flag*/
	int fill;
	
	line_node * list;
};
typedef struct Graph_obj graph_obj;

void * graph_mem_pool2(enum graph_pool_action action);
void * graph_mem_pool(enum graph_pool_action action, int idx);

graph_obj * graph_new(int pool_idx);

void line_add(graph_obj * master, double x0, double y0, double z0, double x1, double y1, double z1);

void graph_draw(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale);

void graph_arc(graph_obj * master, double c_x, double c_y, double c_z, double radius, double ang_start, double ang_end, int sig);

void graph_arc_bulge(graph_obj * master, 
		double pt1_x, double pt1_y , double pt1_z,
		double pt2_x, double pt2_y, double pt2_z, 
		double bulge);
			
void graph_ellipse(graph_obj * master,
		double p1_x, double p1_y, double p1_z,
		double p2_x, double p2_y, double p2_z,
		double minor_ax, double ang_start, double ang_end);
		
void graph_ellipse2(graph_obj * master,
		double major_ax, double minor_ax, 
		double ang_start, double ang_end);

void graph_modify(graph_obj * master, double ofs_x, double ofs_y, double scale_x, double scale_y, double rot);

void graph_rot(graph_obj * master, double base_x, double base_y, double rot);

void graph_mod_axis(graph_obj * master, double normal[3] , double elev);

void knot(int n, int c, int x[]);

void rbasis(int c, double t, int npts, int x[], double h[], double r[]);

void rbspline(int npts, int k, int p1, double b[], double h[], double p[]);

int graph_list_draw(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale);

int graph_list_draw_fix(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale, bmp_color color);

int graph_list_ext(list_node *list, int *init, double * min_x, double * min_y, double * max_x, double * max_y);

int graph_list_modify(list_node *list, double ofs_x, double ofs_y , double scale_x, double scale_y, double rot);

int graph_list_modify_idx(list_node *list, double ofs_x, double ofs_y , double scale_x, double scale_y, double rot, int start_idx, int end_idx);

int graph_list_rot(list_node *list, double base_x, double base_y , double rot);

int graph_list_rot_idx(list_node *list, double base_x, double base_y , double rot, int start_idx, int end_idx);

int graph_list_mod_ax(list_node *list, double normal[3], double elev, int start_idx, int end_idx);

graph_obj * graph_list_isect(list_node *list, double rect_pt1[2], double rect_pt2[2]);

double dot_product(double a[3], double b[3]);

void cross_product(double a[3], double b[3], double c[3]);

void unit_vector(double a[3]);

graph_obj * graph_hatch(graph_obj * ref, double angle, double orig_x, double orig_y, double delta, double skew, int pool_idx);

#endif