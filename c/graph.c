#include "graph.h"
#define GRAPH_PAGE 10000
#define LINE_PAGE 10000
#define GRAPH_NUM_POOL 5
#define TOLERANCE 1e-9

void * graph_mem_pool2(enum graph_pool_action action){
	
	static graph_pool_slot graph, line;
	int i;
	
	void *ret_ptr = NULL;
	
	/* initialize the graph pool, the first allocation */
	if (graph.size < 1){
		graph.pool[0] = malloc(GRAPH_PAGE * sizeof(graph_obj));
		if (graph.pool){
			graph.size = 1;
			//printf("Init graph\n");
		}
	}
	
	/* initialize the lines pool, the first allocation */
	if (line.size < 1){
		line.pool[0] = malloc(LINE_PAGE * sizeof(line_node));
		if (line.pool){
			line.size = 1;
			//printf("Init line\n");
		}
	}
	
	/* if current page is full */
	if ((graph.pos >= GRAPH_PAGE) && (graph.size > 0)){
		/* try to change to page previuosly allocated */
		if (graph.page < graph.size - 1){
			graph.page++;
			graph.pos = 0;
			//printf("change graph page\n");
		}
		/* or then allocatte a new page */
		else if(graph.page < GRAPH_POOL_PAGES-1){
			graph.pool[graph.page + 1] = malloc(GRAPH_PAGE * sizeof(graph_obj));
			if (graph.pool[graph.page + 1]){
				graph.page++;
				graph.size ++;
				graph.pos = 0;
				//printf("Realloc graph\n");
			}
		}
	}
	
	/* if current page is full */
	if ((line.pos >= LINE_PAGE) && (line.size > 0)){
		/* try to change to page previuosly allocated */
		if (line.page < line.size - 1){
			line.page++;
			line.pos = 0;
			//printf("change line page\n");
		}
		/* or then allocatte a new page */
		else if(line.page < GRAPH_POOL_PAGES-1){
			line.pool[line.page + 1] = malloc(LINE_PAGE * sizeof(line_node));
			if (line.pool[line.page + 1]){
				line.page++;
				line.size ++;
				line.pos = 0;
				//printf("Realloc line\n");
			}
		}
	}
	
	ret_ptr = NULL;
	
	if ((graph.pool[graph.page] != NULL) &&  (line.pool[line.page] != NULL)){
		switch (action){
			case ADD_GRAPH:
				if (graph.pos < GRAPH_PAGE){
					ret_ptr = &(((graph_obj *)graph.pool[graph.page])[graph.pos]);
					graph.pos++;
				}
				break;
			case ADD_LINE:
				if (line.pos < LINE_PAGE){
					ret_ptr = &(((line_node *)line.pool[line.page])[line.pos]);
					line.pos++;
				}
				break;
			case ZERO_GRAPH:
				graph.pos = 0;
				graph.page = 0;
				break;
			case ZERO_LINE:
				line.pos = 0;
				line.page = 0;
				break;
			case FREE_ALL:
				for (i = 0; i < graph.size; i++){
					free(graph.pool[i]);
					graph.pool[i] = NULL;
				}
				graph.pos = 0;
				graph.page = 0;
				graph.size = 0;
				for (i = 0; i < line.size; i++){
					free(line.pool[i]);
					line.pool[i] = NULL;
				}
				line.pos = 0;
				line.page = 0;
				line.size = 0;
				break;
		}
	}
	return ret_ptr;
}

void * graph_mem_pool(enum graph_pool_action action, int idx){
	
	static graph_pool_slot graph[GRAPH_NUM_POOL], line[GRAPH_NUM_POOL];
	int i;
	
	void *ret_ptr = NULL;
	
	if ((idx >= 0) && (idx < GRAPH_NUM_POOL)){ /* check if index is valid */
		
		/* initialize the graph pool, the first allocation */
		if (graph[idx].size < 1){
			graph[idx].pool[0] = malloc(GRAPH_PAGE * sizeof(graph_obj));
			if (graph[idx].pool){
				graph[idx].size = 1;
				//printf("Init graph\n");
			}
		}
		
		/* initialize the lines pool, the first allocation */
		if (line[idx].size < 1){
			line[idx].pool[0] = malloc(LINE_PAGE * sizeof(line_node));
			if (line[idx].pool){
				line[idx].size = 1;
				//printf("Init line\n");
			}
		}
		
		/* if current page is full */
		if ((graph[idx].pos >= GRAPH_PAGE) && (graph[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (graph[idx].page < graph[idx].size - 1){
				graph[idx].page++;
				graph[idx].pos = 0;
				//printf("change graph page\n");
			}
			/* or then allocatte a new page */
			else if(graph[idx].page < GRAPH_POOL_PAGES-1){
				graph[idx].pool[graph[idx].page + 1] = malloc(GRAPH_PAGE * sizeof(graph_obj));
				if (graph[idx].pool[graph[idx].page + 1]){
					graph[idx].page++;
					graph[idx].size ++;
					graph[idx].pos = 0;
					//printf("Realloc graph\n");
				}
			}
		}
		
		/* if current page is full */
		if ((line[idx].pos >= LINE_PAGE) && (line[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (line[idx].page < line[idx].size - 1){
				line[idx].page++;
				line[idx].pos = 0;
				//printf("change line page\n");
			}
			/* or then allocatte a new page */
			else if(line[idx].page < GRAPH_POOL_PAGES-1){
				line[idx].pool[line[idx].page + 1] = malloc(LINE_PAGE * sizeof(line_node));
				if (line[idx].pool[line[idx].page + 1]){
					line[idx].page++;
					line[idx].size ++;
					line[idx].pos = 0;
					//printf("Realloc line\n");
				}
			}
		}
		
		ret_ptr = NULL;
		
		if ((graph[idx].pool[graph[idx].page] != NULL) &&  (line[idx].pool[line[idx].page] != NULL)){
			switch (action){
				case ADD_GRAPH:
					if (graph[idx].pos < GRAPH_PAGE){
						ret_ptr = &(((graph_obj *)graph[idx].pool[graph[idx].page])[graph[idx].pos]);
						graph[idx].pos++;
					}
					break;
				case ADD_LINE:
					if (line[idx].pos < LINE_PAGE){
						ret_ptr = &(((line_node *)line[idx].pool[line[idx].page])[line[idx].pos]);
						line[idx].pos++;
					}
					break;
				case ZERO_GRAPH:
					graph[idx].pos = 0;
					graph[idx].page = 0;
					break;
				case ZERO_LINE:
					line[idx].pos = 0;
					line[idx].page = 0;
					break;
				case FREE_ALL:
					for (i = 0; i < graph[idx].size; i++){
						free(graph[idx].pool[i]);
						graph[idx].pool[i] = NULL;
					}
					graph[idx].pos = 0;
					graph[idx].page = 0;
					graph[idx].size = 0;
					for (i = 0; i < line[idx].size; i++){
						free(line[idx].pool[i]);
						line[idx].pool[i] = NULL;
					}
					line[idx].pos = 0;
					line[idx].page = 0;
					line[idx].size = 0;
					break;
			}
		}
	}
	return ret_ptr;
}

graph_obj * graph_new(int pool_idx){
	/* create new graphics object */
	
	/* allocate the main struct */
	//graph_obj * new_obj = malloc(sizeof(graph_obj));
	graph_obj * new_obj = graph_mem_pool(ADD_GRAPH, pool_idx);
	
	if (new_obj){
		
		/* initialize */
		new_obj->owner = NULL;
		new_obj->pool_idx = pool_idx;
		/* initialize with a black color */
		new_obj->color.r = 0;
		new_obj->color.g = 0;
		new_obj->color.b =0;
		new_obj->color.a = 255;
		
		new_obj->rot = 0;
		new_obj->scale = 1;
		new_obj->ofs_x = 0;
		new_obj->ofs_y = 0;
		new_obj->tick = 0;
		new_obj->thick_const = 0;
		new_obj->fill = 0;
		
		/* initialize with a solid line pattern */
		new_obj->patt_size = 1;
		new_obj->pattern[0] = 1.0;
		
		/* extent init */
		new_obj->ext_ini = 0;
		
		/* allocate the line list */
		//new_obj->list = malloc(sizeof(line_node));
		new_obj->list = graph_mem_pool(ADD_LINE, pool_idx);
		
		if(new_obj->list){
			/* the list is empty */
			new_obj->list->next = NULL;
		}
		else{ /* if allocation fails */
			//free(new_obj); /* clear the main struct */
			new_obj = NULL;
		}
	}
	return new_obj;
}

void line_add(graph_obj * master, double x0, double y0, double z0, double x1, double y1, double z1){
	/* create and add a line object to the graph master�s list */
	
	if (master){
		//line_node *new_line = malloc(sizeof(line_node));
		line_node *new_line = graph_mem_pool(ADD_LINE, master->pool_idx);
		
		if (new_line){
			new_line->x0 = x0;
			new_line->y0 = y0;
			new_line->z0 = z0;
			new_line->x1 = x1;
			new_line->y1 = y1;
			new_line->z1 = z1;
			new_line->next = NULL;
			
			if(master->list->next == NULL){ /* check if list is empty */
				/* then, the new line is the first element */
				master->list->next = new_line;
			}
			else{ /* look for the end of list */
				line_node *tmp = master->list->next;
				while(tmp->next != NULL){
					tmp = tmp->next;
				}
				/* then, the new line is put in end of list */
				tmp->next = new_line;
			}
			/*update the extent of graph */
			/* sort the coordinates of entire line*/
			double min_x = (x0 < x1) ? x0 : x1;
			double min_y = (y0 < y1) ? y0 : y1;
			double max_x = (x0 > x1) ? x0 : x1;
			double max_y = (y0 > y1) ? y0 : y1;
			if (master->ext_ini == 0){
				master->ext_ini = 1;
				master->ext_min_x = min_x;
				master->ext_min_y = min_y;
				master->ext_max_x = max_x;
				master->ext_max_y = max_y;
			}
			else{
				master->ext_min_x = (master->ext_min_x < min_x) ? master->ext_min_x : min_x;
				master->ext_min_y = (master->ext_min_y < min_y) ? master->ext_min_y : min_y;
				master->ext_max_x = (master->ext_max_x > max_x) ? master->ext_max_x : max_x;
				master->ext_max_y = (master->ext_max_y > max_y) ? master->ext_max_y : max_y;
			}
		}
	}
}

void graph_draw(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale){
	if ((master != NULL) && (img != NULL)){
		if(master->list->next){ /* check if list is not empty */
			int x0, y0, x1, y1, ok = 1;
			double xd0, yd0, xd1, yd1;
			line_node *current = master->list->next;
			int corners = 0, prev_x, prev_y; /* for fill */
			int corner_x[1000], corner_y[1000], stroke[1000];
			
			/* set the pattern */
			patt_change(img, master->pattern, master->patt_size);
			/* set the color */
			img->frg = master->color;
			
			/* set the tickness */
			if (master->thick_const) img->tick = (int) round(master->tick);
			else img->tick = (int) round(master->tick * scale);
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				ok = 1;
				
				ok &= !(isnan(xd0 = round((current->x0 - ofs_x) * scale)));
				ok &= !(isnan(yd0 = round((current->y0 - ofs_y) * scale)));
				ok &= !(isnan(xd1 = round((current->x1 - ofs_x) * scale)));
				ok &= !(isnan(yd1 = round((current->y1 - ofs_y) * scale)));
					
				//y0 = (int) round((current->y0 - ofs_y) * scale);
				//x1 = (int) round((current->x1 - ofs_x) * scale);
				//y1 = (int) round((current->y1 - ofs_y) * scale);
				
				if (ok){
					x0 = (int) xd0;
					y0 = (int) yd0;
					x1 = (int) xd1;
					y1 = (int) yd1;
					
					bmp_line(img, xd0, yd0, xd1, yd1);
					//printf("%f %d %d %d %d\n", scale, x0, y0, x1, y1);
					
					if (master->fill && (corners < 1000)){ /* check if object is filled */
						/*build the lists of corners */
						if (((x0 != prev_x)||(y0 != prev_y))||(corners == 0)){
							corner_x[corners] = x0;
							corner_y[corners] = y0;
							stroke[corners] = 0;
							corners++;
						}
						corner_x[corners] = x1;
						corner_y[corners] = y1;
						stroke[corners] = 1;
						corners++;
						
						prev_x = x1;
						prev_y = y1;
					}
				}
				current = current->next; /* go to next */
			}
			
			if (master->fill && corners){ /* check if object is filled */
				/* draw a filled polygon */
				bmp_poly_fill(img, corners, corner_x, corner_y, stroke);
			}
		}
	}
}


void graph_draw2(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale){
	if ((master != NULL) && (img != NULL)){
		if(master->list->next){ /* check if list is not empty */
			int x0, y0, x1, y1, ok = 1;
			double xd0, yd0, xd1, yd1;
			line_node *current = master->list->next;
			int corners = 0, prev_x, prev_y; /* for fill */
			int corner_x[1000], corner_y[1000], stroke[1000];
			int i;
			int patt_i = 0;
			
			
			
			/* set the pattern */
			patt_change(img, (double[]){1.0,}, 1);
			/* set the color */
			img->frg = master->color;
			
			/* set the tickness */
			if (master->thick_const) img->tick = (int) round(master->tick);
			else img->tick = (int) round(master->tick * scale);
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				ok = 1;
				
				ok &= !(isnan(xd0 = round((current->x0 - ofs_x) * scale)));
				ok &= !(isnan(yd0 = round((current->y0 - ofs_y) * scale)));
				ok &= !(isnan(xd1 = round((current->x1 - ofs_x) * scale)));
				ok &= !(isnan(yd1 = round((current->y1 - ofs_y) * scale)));
					
				//y0 = (int) round((current->y0 - ofs_y) * scale);
				//x1 = (int) round((current->x1 - ofs_x) * scale);
				//y1 = (int) round((current->y1 - ofs_y) * scale);
				
				if (ok){
					x0 = (int) xd0;
					y0 = (int) yd0;
					x1 = (int) xd1;
					y1 = (int) yd1;
					
					double patt_len = 0.0, x, y;
					
					/* get polar parameters of line */
					x = xd1 - xd0;
					y = yd1 - yd0;
					double modulus = sqrt(pow(x, 2) + pow(y, 2));
					double ang = atan2(y, x);
					double cosine = cos(ang);
					double sine = sin(ang);
					
					
					
					
					for (i = 0; i <= master->patt_size && i < 20; i++){
						patt_len += fabs(master->pattern[i]);
					}
				
					
					patt_len *= scale;
					
					double patt_int, patt_rem;
					
					patt_rem = fabs(modf(modulus/patt_len, &patt_int));
					patt_rem *= patt_len;
					
					int patt_a_i = 0;
					double patt_acc = 0.0;
					for (i = 0; i <= master->patt_size && i < 20; i++){
						patt_a_i = i;
						if (patt_rem < patt_acc) {
							patt_rem = patt_acc - patt_rem;
							break;
						}
						patt_acc += fabs(master->pattern[i]) * scale;
					}
					
					double patt_start = xd0 * cosine + yd0 * sine;
					double patt_start_i;
					patt_start = fabs(modf(patt_start/patt_len, &patt_start_i) * patt_len);
					patt_acc = 0.0;
					for (i = 0; i <= master->patt_size && i < 20; i++){
						patt_i = i;
						if (patt_start <= patt_acc) break;
						patt_acc += fabs(master->pattern[i]) * scale;
					}
					
					
					double p1x, p1y, p2x, p2y;
					p1x = xd0;
					p1y = yd0;
					
					p2x = patt_rem * cosine + p1x;
					p2y = patt_rem * sine + p1y;
					if (master->pattern[patt_i] >= 0.0){
						bmp_line(img, p1x, p1y, p2x, p2y);
					}
					
					p1x = p2x;
					p1y = p2y;
					patt_i++;
					if (patt_i > master->patt_size) patt_i = 0;
					
					for (i = (int) (patt_int * (master->patt_size + 1)) + patt_a_i - 2; i > 0; i--){
						
						p2x = fabs(master->pattern[patt_i]) * scale * cosine + p1x;
						p2y = fabs(master->pattern[patt_i]) * scale * sine + p1y;
						if (master->pattern[patt_i] >= 0.0){
							bmp_line(img, p1x, p1y, p2x, p2y);
						}
						
						p1x = p2x;
						p1y = p2y;
						patt_i++;
						if (patt_i > master->patt_size) patt_i = 0;
					}
					if ((fabs(p1x - xd1) > TOLERANCE) || (fabs(p1y - yd1) > TOLERANCE)){
						if (master->pattern[patt_i] >= 0.0) bmp_line(img, p1x, p1y, xd1, yd1);
					}
					
					//bmp_line(img, xd0, yd0, xd1, yd1);
					//printf("%f %d %d %d %d\n", scale, x0, y0, x1, y1);
					
					if (master->fill && (corners < 1000)){ /* check if object is filled */
						/*build the lists of corners */
						if (((x0 != prev_x)||(y0 != prev_y))||(corners == 0)){
							corner_x[corners] = x0;
							corner_y[corners] = y0;
							stroke[corners] = 0;
							corners++;
						}
						corner_x[corners] = x1;
						corner_y[corners] = y1;
						stroke[corners] = 1;
						corners++;
						
						prev_x = x1;
						prev_y = y1;
					}
				}
				current = current->next; /* go to next */
			}
			
			if (master->fill && corners){ /* check if object is filled */
				/* draw a filled polygon */
				bmp_poly_fill(img, corners, corner_x, corner_y, stroke);
			}
		}
	}
}

void graph_draw_fix(graph_obj * master, bmp_img * img, double ofs_x, double ofs_y, double scale, bmp_color color){
	if ((master != NULL) && (img != NULL)){
		if(master->list->next){ /* check if list is not empty */
			int x0, y0, x1, y1, ok = 1;
			double xd0, yd0, xd1, yd1;
			line_node *current = master->list->next;
			int corners = 0, prev_x, prev_y; /* for fill */
			int corner_x[1000], corner_y[1000], stroke[1000];
			
			/* set the pattern */
			patt_change(img, master->pattern, master->patt_size);
			/* set the color */
			img->frg = color;
			/* set the tickness */
			if (master->thick_const) img->tick = (int) round(master->tick) + 3;
			else img->tick = (int) round(master->tick * scale) + 3;
			
			/* draw the lines */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				ok = 1;
				
				ok &= !(isnan(xd0 = round((current->x0 - ofs_x) * scale)));
				ok &= !(isnan(yd0 = round((current->y0 - ofs_y) * scale)));
				ok &= !(isnan(xd1 = round((current->x1 - ofs_x) * scale)));
				ok &= !(isnan(yd1 = round((current->y1 - ofs_y) * scale)));
				
				//x0 = (int) round((current->x0 - ofs_x) * scale);
				//y0 = (int) round((current->y0 - ofs_y) * scale);
				//x1 = (int) round((current->x1 - ofs_x) * scale);
				//y1 = (int) round((current->y1 - ofs_y) * scale);
				if (ok){
					x0 = (int) xd0;
					y0 = (int) yd0;
					x1 = (int) xd1;
					y1 = (int) yd1;
					
					bmp_line(img, xd0, yd0, xd1, yd1);
					//bmp_line(img, x0, y0, x1, y1);
					//printf("%f %d %d %d %d\n", scale, x0, y0, x1, y1);
					
					if (master->fill && (corners < 1000)){ /* check if object is filled */
						/*build the lists of corners */
						if (((x0 != prev_x)||(y0 != prev_y))||(corners == 0)){
							corner_x[corners] = x0;
							corner_y[corners] = y0;
							stroke[corners] = 0;
							corners++;
						}
						corner_x[corners] = x1;
						corner_y[corners] = y1;
						stroke[corners] = 1;
						corners++;
						
						prev_x = x1;
						prev_y = y1;
					}
				}
				current = current->next; /* go to next */
			}
			
			if (master->fill && corners){ /* check if object is filled */
				/* draw a filled polygon */
				bmp_poly_fill(img, corners, corner_x, corner_y, stroke);
			}
		}
	}
}

void graph_arc(graph_obj * master, double c_x, double c_y, double c_z, double radius, double ang_start, double ang_end, int sig){
	if (master){
		int n = 64; //numero de vertices do pol�gono regular que aproxima o circulo ->bom numero 
		double ang;
		int steps, i;
		double x0, y0, x1, y1;
		
		ang_start *= M_PI/180;
		ang_end *= M_PI/180;
		
		ang = (ang_end - ang_start) * sig; //angulo do arco
		if (ang <= 0){ ang = ang + 2*M_PI;}
		
		//descobre quantos passos para o la�o a seguir
		steps = (int) floor(fabs(ang*n/(2*M_PI))); //numero de vertices do arco
		
		x0 = c_x + radius * cos(ang_start);
		y0 = c_y + radius * sin(ang_start);
		
		//printf("Arco, stp = %d, r = %0.2f, ang = %0.2f\n pts = )", steps, radius, ang);
		
		//j� come�a do segundo v�rtice
		for (i = 1; i < steps; i++){
			x1 = c_x + radius * cos(2 * M_PI * i * sig/ n + ang_start);
			y1 = c_y + radius * sin(2 * M_PI * i * sig/ n + ang_start);
			
			
			line_add(master, x0, y0, c_z, x1, y1, c_z);
			//printf("(%0.2f,%0.2f),", x1, y1);
			x0=x1;
			y0=y1;
		}
		// o ultimo vertice do arco eh o ponto final, nao calculado no la�o
		x1 = c_x + radius * cos(ang_end);
		y1 = c_y + radius * sin(ang_end);
		line_add(master, x0, y0, c_z, x1, y1, c_z);
		//printf("(%0.2f,%0.2f)\n", x1, y1);
	}
}

void graph_arc_bulge(graph_obj * master, 
			double pt1_x, double pt1_y , double pt1_z,
			double pt2_x, double pt2_y, double pt2_z, 
			double bulge){
	
	double theta, alfa, d, radius, ang_c, ang_start, ang_end, center_x, center_y;
	int sig;
	
	theta = 2 * atan(bulge);
	alfa = atan2(pt2_y-pt1_y, pt2_x-pt1_x);
	d = sqrt((pt2_y-pt1_y)*(pt2_y-pt1_y) + (pt2_x-pt1_x)*(pt2_x-pt1_x)) / 2;
	radius = d*(bulge*bulge + 1)/(2*bulge);
	
	ang_c = M_PI+(alfa - M_PI/2 - theta);
	center_x = radius*cos(ang_c) + pt1_x;
	center_y = radius*sin(ang_c) + pt1_y;
	
	//angulo inicial e final obtidos das coordenadas iniciais
	ang_start = atan2(pt1_y-center_y,pt1_x-center_x);
	ang_end = atan2(pt2_y-center_y,pt2_x-center_x);
	
	sig = 1;
	if (bulge < 0){
		ang_start += M_PI;
		ang_end += M_PI;
		sig = -1;
	}
	//converte para garus
	ang_start *= 180/M_PI;
	ang_end *= 180/M_PI;
	
	//arco(entidade, camada, center, radius, ang_start, ang_end, cor, esp, sig);
	graph_arc(master, center_x, center_y, pt1_z, radius, ang_start, ang_end, sig);
}

void graph_ellipse(graph_obj * master,
		double p1_x, double p1_y, double p1_z,
		double p2_x, double p2_y, double p2_z,
		double minor_ax, double ang_start, double ang_end){
	if (master){
		int n = 64; //numero de vertices do pol�gono regular que aproxima o circulo ->bom numero 
		double ang, major_ax, cosine, sine;
		int steps, i;
		double x0, y0, x1, y1;
		double xx0, yy0, xx1, yy1;
		
		//ang_start *= M_PI/180;
		//ang_end *= M_PI/180;
		
		major_ax = sqrt(pow(p2_x, 2) + pow(p2_y, 2)) ;
		minor_ax *= major_ax;
		
		/* rotation constants */
		cosine = cos(atan2(p2_y, p2_x));
		sine = sin(atan2(p2_y, p2_x));
		
		//printf("major = %0.2f, minor = %0.2f\n", major_ax, minor_ax);
		//printf("ang_start = %0.2f, ang_end = %0.2f\n", ang_start, ang_end);
		
		ang = (ang_end - ang_start); //angulo do arco
		if (ang <= 0){ ang = ang + 2*M_PI;}
		
		//descobre quantos passos para o la�o a seguir
		steps = (int) floor(fabs(ang*n/(2*M_PI))); //numero de vertices do arco
		
		x0 = p1_x + major_ax * cos(ang_start);
		y0 = p1_y + minor_ax * sin(ang_start);
		
		//printf("Arco, stp = %d, r = %0.2f, ang = %0.2f\n pts = )", steps, radius, ang);
		
		//j� come�a do segundo v�rtice
		for (i = 1; i < steps; i++){
			x1 = p1_x + major_ax * cos(2 * M_PI * i / n + ang_start);
			y1 = p1_y + minor_ax * sin(2 * M_PI * i / n + ang_start);
			
			xx0 = cosine*(x0-p1_x) - sine*(y0-p1_y) + p1_x;
			yy0 = sine*(x0-p1_x) + cosine*(y0-p1_y) + p1_y;
			xx1 = cosine*(x1-p1_x) - sine*(y1-p1_y) + p1_x;
			yy1 = sine*(x1-p1_x) + cosine*(y1-p1_y) + p1_y;
			line_add(master, xx0, yy0, p1_z, xx1, yy1, p1_z);
			//printf("(%0.2f,%0.2f),", x1, y1);
			x0=x1;
			y0=y1;
		}
		// o ultimo vertice do arco eh o ponto final, nao calculado no la�o
		x1 = p1_x + major_ax * cos(ang_end);
		y1 = p1_y + minor_ax * sin(ang_end);
		
		xx0 = cosine*(x0-p1_x) - sine*(y0-p1_y) + p1_x;
		yy0 = sine*(x0-p1_x) + cosine*(y0-p1_y) + p1_y;
		xx1 = cosine*(x1-p1_x) - sine*(y1-p1_y) + p1_x;
		yy1 = sine*(x1-p1_x) + cosine*(y1-p1_y) + p1_y;
		line_add(master, xx0, yy0, p1_z, xx1, yy1, p1_z);
		
		//printf("(%0.2f,%0.2f)\n", x1, y1);
	}
}

void graph_ellipse2(graph_obj * master,
		double major_ax, double minor_ax, 
		double ang_start, double ang_end){
	if (master){
		int n = 64; //numero de vertices do pol�gono regular que aproxima o circulo ->bom numero 
		double ang;
		int steps, i;
		double x0, y0, x1, y1;
		
		//ang_start *= M_PI/180;
		//ang_end *= M_PI/180;
		
		//major_ax = sqrt(pow(p2_x, 2) + pow(p2_y, 2)) ;
		minor_ax *= major_ax;
		
		//printf("major = %0.2f, minor = %0.2f\n", major_ax, minor_ax);
		//printf("ang_start = %0.2f, ang_end = %0.2f\n", ang_start, ang_end);
		
		ang = (ang_end - ang_start); //angulo do arco
		if (ang <= 0){ ang = ang + 2*M_PI;}
		
		//descobre quantos passos para o la�o a seguir
		steps = (int) floor(fabs(ang*n/(2*M_PI))); //numero de vertices do arco
		
		x0 = major_ax * cos(ang_start);
		y0 = minor_ax * sin(ang_start);
		
		//printf("Arco, stp = %d, r = %0.2f, ang = %0.2f\n pts = )", steps, radius, ang);
		
		//j� come�a do segundo v�rtice
		for (i = 1; i < steps; i++){
			x1 = major_ax * cos(2 * M_PI * i / n + ang_start);
			y1 = minor_ax * sin(2 * M_PI * i / n + ang_start);
			
			line_add(master, x0, y0, 0.0, x1, y1, 0.0);
			//printf("(%0.2f,%0.2f),", x1, y1);
			x0=x1;
			y0=y1;
		}
		// o ultimo vertice do arco eh o ponto final, nao calculado no la�o
		x1 = major_ax * cos(ang_end);
		y1 = minor_ax * sin(ang_end);
		line_add(master, x0, y0, 0.0, x1, y1, 0.0);
		
		//printf("(%0.2f,%0.2f)\n", x1, y1);
	}
}

void graph_modify(graph_obj * master, double ofs_x, double ofs_y, double scale_x, double scale_y, double rot){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x0, y0, x1, y1;
			double sine = 0, cosine = 1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			master->ext_ini = 0;
			
			/* rotation constants */
			cosine = cos(rot*M_PI/180);
			sine = sin(rot*M_PI/180);
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				current->x0 = current->x0 * scale_x + ofs_x;
				current->y0 = current->y0 * scale_y + ofs_y;
				current->x1 = current->x1 * scale_x + ofs_x;
				current->y1 = current->y1 * scale_y + ofs_y;
				
				x0 = cosine*(current->x0-ofs_x) - sine*(current->y0-ofs_y) + ofs_x;
				y0 = sine*(current->x0-ofs_x) + cosine*(current->y0-ofs_y) + ofs_y;
				x1 = cosine*(current->x1-ofs_x) - sine*(current->y1-ofs_y) + ofs_x;
				y1 = sine*(current->x1-ofs_x) + cosine*(current->y1-ofs_y) + ofs_y;
				
				/* update the graph */
				current->x0 = x0;
				current->y0 = y0;
				current->x1 = x1;
				current->y1 = y1;
				
				/*update the extent of graph */
				/* sort the coordinates of entire line*/
				min_x = (x0 < x1) ? x0 : x1;
				min_y = (y0 < y1) ? y0 : y1;
				max_x = (x0 > x1) ? x0 : x1;
				max_y = (y0 > y1) ? y0 : y1;
				if (master->ext_ini == 0){
					master->ext_ini = 1;
					master->ext_min_x = min_x;
					master->ext_min_y = min_y;
					master->ext_max_x = max_x;
					master->ext_max_y = max_y;
				}
				else{
					master->ext_min_x = (master->ext_min_x < min_x) ? master->ext_min_x : min_x;
					master->ext_min_y = (master->ext_min_y < min_y) ? master->ext_min_y : min_y;
					master->ext_max_x = (master->ext_max_x > max_x) ? master->ext_max_x : max_x;
					master->ext_max_y = (master->ext_max_y > max_y) ? master->ext_max_y : max_y;
				}
				
				current = current->next; /* go to next */
			}
		}
	}
}

void graph_rot(graph_obj * master, double base_x, double base_y, double rot){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x0, y0, x1, y1;
			double sine = 0, cosine = 1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			master->ext_ini = 0;
			
			/* rotation constants */
			cosine = cos(rot*M_PI/180);
			sine = sin(rot*M_PI/180);
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				
				x0 = cosine*(current->x0-base_x) - sine*(current->y0-base_y) + base_x;
				y0 = sine*(current->x0-base_x) + cosine*(current->y0-base_y) + base_y;
				x1 = cosine*(current->x1-base_x) - sine*(current->y1-base_y) + base_x;
				y1 = sine*(current->x1-base_x) + cosine*(current->y1-base_y) + base_y;
				
				/* update the graph */
				current->x0 = x0;
				current->y0 = y0;
				current->x1 = x1;
				current->y1 = y1;
				
				/*update the extent of graph */
				/* sort the coordinates of entire line*/
				min_x = (x0 < x1) ? x0 : x1;
				min_y = (y0 < y1) ? y0 : y1;
				max_x = (x0 > x1) ? x0 : x1;
				max_y = (y0 > y1) ? y0 : y1;
				if (master->ext_ini == 0){
					master->ext_ini = 1;
					master->ext_min_x = min_x;
					master->ext_min_y = min_y;
					master->ext_max_x = max_x;
					master->ext_max_y = max_y;
				}
				else{
					master->ext_min_x = (master->ext_min_x < min_x) ? master->ext_min_x : min_x;
					master->ext_min_y = (master->ext_min_y < min_y) ? master->ext_min_y : min_y;
					master->ext_max_x = (master->ext_max_x > max_x) ? master->ext_max_x : max_x;
					master->ext_max_y = (master->ext_max_y > max_y) ? master->ext_max_y : max_y;
				}
				
				current = current->next; /* go to next */
			}
		}
	}
}

/*
int main (void){
	bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
	bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
	bmp_color red = {.r = 255, .g = 0, .b =0, .a = 255};
	
	bmp_img * img = bmp_new(200, 200, white, black);
	graph_obj * test = graph_new();
	line_add(test, 0,0,100,100);
	line_add(test, 210,210,100,2);
	test->tick = 5;
	test->color = red;
	
	graph_draw(test, img);
	
	bmp_save("teste2.ppm", img);
	bmp_free(img);
	graph_free(test);
	return 0;
}
*/



void knot(int n, int c, int x[]){
	/*
	Subroutine to generate a B-spline open knot vector with multiplicity
	equal to the order at the ends.

	c            = order of the basis function
	n            = the number of defining polygon vertices
	nplus2       = index of x() for the first occurence of the maximum knot vector value
	nplusc       = maximum value of the knot vector -- $n + c$
	x()          = array containing the knot vector
	*/
	
	int nplusc,nplus2,i;

	nplusc = n + c;
	nplus2 = n + 2;
	
	if (nplusc < MAX_SPLINE_PTS){
		x[1] = 0;
		for (i = 2; i <= nplusc; i++){
			if ( (i > c) && (i < nplus2) )
				x[i] = x[i-1] + 1;
			else
				x[i] = x[i-1];
		}
	}
}	

void rbasis(int c, double t, int npts, int x[], double h[], double r[]){
	/*  Subroutine to generate rational B-spline basis functions--open knot vector

	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: rbasis
	Language: C
	Subroutines called: none
	Book reference: Chapter 4, Sec. 4. , p 296

	c        = order of the B-spline basis function
	d        = first term of the basis function recursion relation
	e        = second term of the basis function recursion relation
	h[]	     = array containing the homogeneous weights
	npts     = number of defining polygon vertices
	nplusc   = constant -- npts + c -- maximum number of knot values
	r[]      = array containing the rationalbasis functions
	       r[1] contains the basis function associated with B1 etc.
	t        = parameter value
	temp[]   = temporary array
	x[]      = knot vector
	*/
	
	int nplusc;
	int i,j,k;
	double d,e;
	double sum;
	double temp[MAX_SPLINE_PTS];

	nplusc = npts + c;
	
	if (nplusc < MAX_SPLINE_PTS){

		/*		
		printf("knot vector is \n");
		for (i = 1; i <= nplusc; i++){
			printf(" %d %d \n", i,x[i]);
		}
		printf("t is %f \n", t);
		*/

		/* calculate the first order nonrational basis functions n[i]	*/

		for (i = 1; i<= nplusc-1; i++){
			if (( t >= x[i]) && (t < x[i+1]))
				temp[i] = 1;
			else
				temp[i] = 0;
		}

		/* calculate the higher order nonrational basis functions */

		for (k = 2; k <= c; k++){
			for (i = 1; i <= nplusc-k; i++){
				if (temp[i] != 0)    /* if the lower order basis function is zero skip the calculation */
					d = ((t-x[i])*temp[i])/(x[i+k-1]-x[i]);
				else
					d = 0;

				if (temp[i+1] != 0)     /* if the lower order basis function is zero skip the calculation */
					e = ((x[i+k]-t)*temp[i+1])/(x[i+k]-x[i+1]);
				else
					e = 0;

				temp[i] = d + e;
			}
		}

		if (t == (double)x[nplusc]){		/*    pick up last point	*/
			temp[npts] = 1;
		}
		/*
		printf("Nonrational basis functions are \n");
		for (i=1; i<= npts; i++){
			printf("%f ", temp[i]);
		}
		printf("\n");
		*/
		/* calculate sum for denominator of rational basis functions */

		sum = 0.;
		for (i = 1; i <= npts; i++){
			sum = sum + temp[i]*h[i];
		}

		/* form rational basis functions and put in r vector */

		for (i = 1; i <= npts; i++){
			if (sum != 0){
				r[i] = (temp[i]*h[i])/(sum);}
			else
				r[i] = 0;
		}
	}
}

void rbspline(int npts, int k, int p1, double b[], double h[], double p[]){
	/*  Subroutine to generate a rational B-spline curve using an uniform open knot vector

	C code for An Introduction to NURBS
	by David F. Rogers. Copyright (C) 2000 David F. Rogers,
	All rights reserved.
	
	Name: rbspline.c
	Language: C
	Subroutines called: knot.c, rbasis.c, fmtmul.c
	Book reference: Chapter 4, Alg. p. 297

	b[]         = array containing the defining polygon vertices
		  b[1] contains the x-component of the vertex
		  b[2] contains the y-component of the vertex
		  b[3] contains the z-component of the vertex
	h[]			= array containing the homogeneous weighting factors 
	k           = order of the B-spline basis function
	nbasis      = array containing the basis functions for a single value of t
	nplusc      = number of knot values
	npts        = number of defining polygon vertices
	p[,]        = array containing the curve points
		  p[1] contains the x-component of the point
		  p[2] contains the y-component of the point
		  p[3] contains the z-component of the point
	p1          = number of points to be calculated on the curve
	t           = parameter value 0 <= t <= npts - k + 1
	x[]         = array containing the knot vector
	*/
	
	int i,j,icount,jcount;
	int i1;
	int x[MAX_SPLINE_PTS];		/* allows for 20 data points with basis function of order 5 */
	int nplusc;

	double step;
	double t;
	double nbasis[MAX_SPLINE_PTS];
	double temp;


	nplusc = npts + k;
	
	if (nplusc < MAX_SPLINE_PTS){

		/*  zero and redimension the knot vector and the basis array */

		for(i = 0; i <= npts; i++){
			nbasis[i] = 0.;
		}

		for(i = 0; i <= nplusc; i++){
			x[i] = 0.;
		}

		/* generate the uniform open knot vector */

		knot(npts,k,x);

		/*
		printf("The knot vector is ");
		for (i = 1; i <= nplusc; i++){
			printf(" %d ", x[i]);
		}
		printf("\n");
		*/

		
		icount = 0;

		/*    calculate the points on the rational B-spline curve */

		t = 0;
		step = ((double)x[nplusc])/((double)(p1-1));

		for (i1 = 1; i1<= p1; i1++){
			if ((double)x[nplusc] - t < 5e-6){
				t = (double)x[nplusc];
			}
			
			rbasis(k,t,npts,x,h,nbasis);      /* generate the basis function for this value of t */
			//printf("cpts=%d, order=%d, t=%0.2f \n",npts,k,t);
			/*
			printf("t = %f \n",t);
			printf("nbasis = ");
			for (i = 1; i <= npts; i++){
				printf("%f  ",nbasis[i]);
			}
			printf("\n");
			*/
			for (j = 1; j <= 3; j++){      /* generate a point on the curve */
				jcount = j;
				p[icount+j] = 0.;

				for (i = 1; i <= npts; i++){ /* Do local matrix multiplication */
					temp = nbasis[i]*b[jcount];
					p[icount + j] = p[icount + j] + temp;
					/*
					printf("jcount,nbasis,b,nbasis*b,p = %d %f %f %f %f\n",jcount,nbasis[i],b[jcount],temp,p[icount+j]);
					*/
					jcount = jcount + 3;
				}
			}
			/*
			printf("icount, p %d %f %f %f \n",icount,p[icount+1],p[icount+2],p[icount+3]);
			*/
			icount = icount + 3;
			t = t + step;
		}
	}
}

double dot_product(double a[3], double b[3]){
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}
 
void cross_product(double a[3], double b[3], double c[3]){
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}

void unit_vector(double a[3]){
	double mod;
	
	mod = sqrt(pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2));
	if (mod > 0.0) {
		a[0] /= mod;
		a[1] /= mod;
		a[2] /= mod;
	}
}

void graph_mod_axis(graph_obj * master, double normal[3] , double elev){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double x_axis[3], y_axis[3], point[3], x_col[3], y_col[3];
			double wy_axis[3] = {0.0, 1.0, 0.0};
			double wz_axis[3] = {0.0, 0.0, 1.0};
			
			
			double x0, y0, x1, y1;
			double min_x, min_y, max_x, max_y;
			line_node *current = master->list->next;
			
			master->ext_ini = 0;
			
			
			if ((fabs(normal[0] < 0.015625)) && (fabs(normal[1] < 0.015625))){
				cross_product(wy_axis, normal, x_axis);
			}
			else{
				cross_product(wz_axis, normal, x_axis);
			}
			cross_product(normal, x_axis, y_axis);
			
			unit_vector(x_axis);
			unit_vector(y_axis);
			unit_vector(normal);
			
			x_col[0] = x_axis[0];
			x_col[1] = y_axis[0];
			x_col[2] = normal[0];
			
			y_col[0] = x_axis[1];
			y_col[1] = y_axis[1];
			y_col[2] = normal[1];
			
			/* apply changes to each point */
			while(current){ /*sweep the list content */
				/* apply the scale and offset */
				point[0] = current->x0;
				point[1] = current->y0;
				point[2] = current->z0;
				if (fabs(point[2]) < TOLERANCE) point[2] = elev;
				x0 = dot_product(point, x_col);
				y0 = dot_product(point, y_col);
				
				point[0] = current->x1;
				point[1] = current->y1;
				point[2] = current->z1;
				if (fabs(point[2]) < TOLERANCE) point[2] = elev;
				x1 = dot_product(point, x_col);
				y1 = dot_product(point, y_col);
				
				
				/* update the graph */
				current->x0 = x0;
				current->y0 = y0;
				current->x1 = x1;
				current->y1 = y1;
				
				/*update the extent of graph */
				/* sort the coordinates of entire line*/
				min_x = (x0 < x1) ? x0 : x1;
				min_y = (y0 < y1) ? y0 : y1;
				max_x = (x0 > x1) ? x0 : x1;
				max_y = (y0 > y1) ? y0 : y1;
				if (master->ext_ini == 0){
					master->ext_ini = 1;
					master->ext_min_x = min_x;
					master->ext_min_y = min_y;
					master->ext_max_x = max_x;
					master->ext_max_y = max_y;
				}
				else{
					master->ext_min_x = (master->ext_min_x < min_x) ? master->ext_min_x : min_x;
					master->ext_min_y = (master->ext_min_y < min_y) ? master->ext_min_y : min_y;
					master->ext_max_x = (master->ext_max_x > max_x) ? master->ext_max_x : max_x;
					master->ext_max_y = (master->ext_max_y > max_y) ? master->ext_max_y : max_y;
				}
				
				current = current->next; /* go to next */
			}
		}
	}
}

int l_r_isect(double lin_pt1[2], double lin_pt2[2], double rect_pt1[2], double rect_pt2[2]){
	/* check if a line instersect a rectangle */
	
	double r_bl_x, r_bl_y, r_tr_x,r_tr_y;
	double a, b, c, pol1, pol2, pol3, pol4;
	
	/* sort the rectangle corners */
	r_bl_x = (rect_pt1[0] < rect_pt2[0]) ? rect_pt1[0] : rect_pt2[0];
	r_bl_y = (rect_pt1[1] < rect_pt2[1]) ? rect_pt1[1] : rect_pt2[1];
	r_tr_x = (rect_pt1[0] > rect_pt2[0]) ? rect_pt1[0] : rect_pt2[0];
	r_tr_y = (rect_pt1[1] > rect_pt2[1]) ? rect_pt1[1] : rect_pt2[1];
	
	/* check if line is out of bounds of rectangle */
	if (((lin_pt1[0] > r_tr_x) && (lin_pt2[0] > r_tr_x)) || ( /* line in right side */
	(lin_pt1[1] > r_tr_y) && (lin_pt2[1] > r_tr_y)) || (  /* line in up side */
	(lin_pt1[0] < r_bl_x) && (lin_pt2[0] < r_bl_x)) || ( /* line in left side */
	(lin_pt1[1] < r_bl_y) && (lin_pt2[1] < r_bl_y))){ /* line in down side */
		return 0;}
	else{ /* compute the triangle polarizations in each corner of rectangle,
		relative to line*/
		a = lin_pt2[1]-lin_pt1[1];
		b = lin_pt1[0]-lin_pt2[0];
		c = lin_pt2[0]*lin_pt1[1] - lin_pt1[0]*lin_pt2[1];
		
		pol1 = a*rect_pt1[0] + b*rect_pt1[1] + c;
		pol2 = a*rect_pt1[0] + b*rect_pt2[1] + c;
		pol3 = a*rect_pt2[0] + b*rect_pt1[1] + c;
		pol4 = a*rect_pt2[0] + b*rect_pt2[1] + c;
		if (((pol1>=0) && (pol2>=0) && (pol3>=0) && (pol4>=0)) ||(
		(pol1<0) && (pol2<0) && (pol3<0) && (pol4<0))){
			return 0;}
		return 1;
	}
}

int graph_isect(graph_obj * master, double rect_pt1[2], double rect_pt2[2]){
	if ((master != NULL)){
		if(master->list->next){ /* check if list is not empty */
			double lin_pt1[2], lin_pt2[2];
			line_node *current = master->list->next;
			
			while(current){ /*sweep the list content */
				/* update the graph */
				lin_pt1[0] = current->x0;
				lin_pt1[1] = current->y0;
				lin_pt2[0] = current->x1;
				lin_pt2[1] = current->y1;
				if(l_r_isect(lin_pt1, lin_pt2, rect_pt1, rect_pt2)){
					return 1;
				}
				
				current = current->next; /* go to next */
			}
		}
	}
	return 0;
}

int graph_list_draw(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_draw2(curr_graph, img, ofs_x, ofs_y, scale);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_draw_fix(list_node *list, bmp_img * img, double ofs_x, double ofs_y, double scale, bmp_color color){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_draw_fix(curr_graph, img, ofs_x, ofs_y, scale, color);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_ext(list_node *list, int *init, double * min_x, double * min_y, double * max_x, double * max_y){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if (*init == 0){
					*init = 1;
					*min_x = curr_graph->ext_min_x;
					*min_y = curr_graph->ext_min_y;
					*max_x = curr_graph->ext_max_x;
					*max_y = curr_graph->ext_max_y;
				}
				else{
					*min_x = (*min_x < curr_graph->ext_min_x) ? *min_x : curr_graph->ext_min_x;
					*min_y = (*min_y < curr_graph->ext_min_y) ? *min_y : curr_graph->ext_min_y;
					*max_x = (*max_x > curr_graph->ext_max_x) ? *max_x : curr_graph->ext_max_x;
					*max_y = (*max_y > curr_graph->ext_max_y) ? *max_y : curr_graph->ext_max_y;
				}
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_modify(list_node *list, double ofs_x, double ofs_y , double scale_x, double scale_y, double rot){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_modify(curr_graph, ofs_x, ofs_y, scale_x, scale_y, rot);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_modify_idx(list_node *list, double ofs_x, double ofs_y , double scale_x, double scale_y, double rot, int start_idx, int end_idx){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0, i = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(i >= start_idx){
					graph_modify(curr_graph, ofs_x, ofs_y, scale_x, scale_y, rot);
				}
			}
			if(i >= end_idx) break;
			i++;
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_rot(list_node *list, double base_x, double base_y , double rot){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				graph_rot(curr_graph, base_x, base_y, rot);
			}
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_rot_idx(list_node *list, double base_x, double base_y , double rot, int start_idx, int end_idx){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0, i = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(i >= start_idx){
					graph_rot(curr_graph, base_x, base_y, rot);
				}
			}
			if(i >= end_idx) break;
			i++;
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

int graph_list_mod_ax(list_node *list, double normal[3], double elev, int start_idx, int end_idx){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
	int ok = 0, i = 0;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(i >= start_idx){
					graph_mod_axis(curr_graph, normal, elev);
				}
			}
			if(i >= end_idx) break;
			i++;
			current = current->next;
		}
		ok = 1;
	}
	return ok;
}

graph_obj * graph_list_isect(list_node *list, double rect_pt1[2], double rect_pt2[2]){
	list_node *current = NULL;
	graph_obj *curr_graph = NULL;
		
	if (list != NULL){
		current = list->next;
		
		/* sweep the main list */
		while (current != NULL){
			if (current->data){
				curr_graph = (graph_obj *)current->data;
				if(graph_isect(curr_graph, rect_pt1, rect_pt2)){
					return curr_graph;
				}
			}
			current = current->next;
		}
	}
	return NULL;
}