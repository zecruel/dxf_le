#include "dxf.h"
#include <math.h>
#include "shape2.h"
#define DXF_NUM_POOL 5
#define DXF_PAGE 500000

void str_upp(char *str) { /* upper case the string */
	while (*str= toupper(*str)) str++;
}

char * trimwhitespace(char *str){
	/* Note: This function returns a pointer to a substring of the original string.
	If the given string was allocated dynamically, the caller must not overwrite
	that pointer with the returned value, since the original pointer must be
	deallocated using the same allocator with which it was allocated.  The return
	value must NOT be deallocated using free() etc. 
	Author = Adam Rosenfield
	http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
	*/
	char *end;

	// Trim leading space
	while(isspace((unsigned char)*str)) str++;

	if(*str == 0)  // All spaces?
	return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

char *get_filename(char *path){
	/* Get the filename with extension from path string.
	This function does not modify the original string,
	but the return string must be used imediatly */
	static char buf[DXF_MAX_CHARS];
	char *ret = NULL;
	
	strncpy(buf, path, DXF_MAX_CHARS);
	ret = strrchr(buf, '/');
	if (ret == NULL) ret = strrchr(buf, '\\');
	
	if (ret) ret++;
	else {
		buf[0] = 0;
		ret = buf;
	}
	
	return ret;
}

char *get_ext(char *path){
	/* Get the extension of file from path string.
	This function does not modify the original string,
	but the return string must be used imediatly.
	Return string is in lower case*/
	static char buf[DXF_MAX_CHARS];
	char *ret = NULL;
	int i;
	
	strncpy(buf, path, DXF_MAX_CHARS);
	ret = strrchr(buf, '.');
	
	if (ret) {
		ret++;
		i = 0;
		while(ret[i]){
			ret[i] = tolower(ret[i]);
			i++;
		}
	}
	else {
		buf[0] = 0;
		ret = buf;
	}
	
	return ret;
}

void strip_ext(char *filename){
	/* Strip extension from filename string.
	Atention: This function modify the original string */
	char *ret = NULL;
	if (filename){
		ret = strrchr(filename, '.');
		if (ret){
			int pos = (int)(ret - filename);
			filename[pos] = 0;
		}
	}
}

char *str_replace(char *orig, char *rep, char *with) {
	/*
	Author: jmucchiello
	http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
	*/
	/* You must free the result if result is non-NULL. */
	char *result; /* the return string */
	char *ins;    /* the next insert point */
	char *tmp;    /* varies */
	int len_rep;  /* length of rep (the string to remove) */
	int len_with; /* length of with (the string to replace rep with) */
	int len_front; /* distance between rep and end of last rep */
	int count;    /* number of replacements */

	/* sanity checks and initialization */
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; /* empty rep causes infinite loop during count */
	if (!with)
		with = "";
	len_with = strlen(with);

	/* count the number of replacements needed */
	ins = orig;
	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	/*
	first time through the loop, all the variable are set correctly
	from here on, tmp points to the end of the result string
	ins points to the next occurrence of rep in orig
	orig points to the remainder of orig after "end of rep"
	*/
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; /* move to next "end of rep" */
	}
	strcpy(tmp, orig);
	return result;
}

void * dxf_mem_pool(enum dxf_pool_action action, int idx){
	
	static dxf_pool_slot mem_pool[DXF_NUM_POOL];
	int i;
	
	void *ret_ptr = NULL;
	
	if ((idx >= 0) && (idx < DXF_NUM_POOL)){ /* check if index is valid */
		
		/* initialize the pool, the first allocation */
		if (mem_pool[idx].size < 1){
			mem_pool[idx].pool[0] = malloc(DXF_PAGE * sizeof(dxf_node));
			if (mem_pool[idx].pool){
				mem_pool[idx].size = 1;
				//printf("Init mem_pool\n");
			}
		}
		
		/* if current page is full */
		if ((mem_pool[idx].pos >= DXF_PAGE) && (mem_pool[idx].size > 0)){
			/* try to change to page previuosly allocated */
			if (mem_pool[idx].page < mem_pool[idx].size - 1){
				mem_pool[idx].page++;
				mem_pool[idx].pos = 0;
				//printf("change mem_pool page\n");
			}
			/* or then allocatte a new page */
			else if(mem_pool[idx].page < DXF_POOL_PAGES-1){
				mem_pool[idx].pool[mem_pool[idx].page + 1] = malloc(DXF_PAGE * sizeof(dxf_node));
				if (mem_pool[idx].pool[mem_pool[idx].page + 1]){
					mem_pool[idx].page++;
					mem_pool[idx].size ++;
					mem_pool[idx].pos = 0;
					//printf("Realloc mem_pool\n");
				}
			}
		}
		
		ret_ptr = NULL;
		
		if ((mem_pool[idx].pool[mem_pool[idx].page] != NULL)){
			switch (action){
				case ADD_DXF:
					if (mem_pool[idx].pos < DXF_PAGE){
						ret_ptr = &(((dxf_node *)mem_pool[idx].pool[mem_pool[idx].page])[mem_pool[idx].pos]);
						mem_pool[idx].pos++;
					}
					break;
				case ZERO_DXF:
					mem_pool[idx].pos = 0;
					mem_pool[idx].page = 0;
					break;
				case FREE_DXF:
					for (i = 0; i < mem_pool[idx].size; i++){
						free(mem_pool[idx].pool[i]);
						//dxf_ent_clear (mem_pool[idx].pool[i]);
						mem_pool[idx].pool[i] = NULL;
					}
					mem_pool[idx].pos = 0;
					mem_pool[idx].page = 0;
					mem_pool[idx].size = 0;
					break;
			}
		}
	}
	return ret_ptr;
}

void dxf_ent_print2 (dxf_node *ent){ /* print the entity structure */
	/* this function is non recursive */
	int i;
	int indent = 0;
	dxf_node *current, *prev;
	
	current = ent;
	while (current){
		prev = current;
		if (current->type == DXF_ENT){ /* DXF entity */
			if (current->obj.name){
				for (i=0; i<indent; i++){ /* print the indentation spaces */
					printf("    ");
				}
				printf(current->obj.name);  /* print the string of entity's name */
				printf("\n");
			}
			if (current->obj.content){
				/* starts the content sweep */
				prev = current->obj.content;
				current = current->obj.content->next;
				indent++;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			for (i=0; i<indent; i++){ /* print the indentation spaces */
				printf("    ");
			}
			printf ("%d = ", current->value.group); /* print the DFX group */
			/* print the value of atrribute, acording its type */
			switch (current->value.t_data) {
				case DXF_STR:
					if(current->value.s_data){
						printf(current->value.s_data);
					}
					break;
				case DXF_FLOAT:
					printf("%f", current->value.d_data);
					break;
				case DXF_INT:
					printf("%d", current->value.i_data);
			}
			printf("\n");
			current = current->next; /* go to the next in the list */
		}
		while (current == NULL){
			if (prev == ent){
				current = NULL;
				break;
			}
			prev = prev->master;
			if (prev){
				current = prev->next;
				indent --;
				if (prev == ent){
					current = NULL;
					printf("fim loop ");
					break;
				}
			}
			else{
				current = NULL;
				break;
			}
		}
	}
}

void dxf_ent_print_f (dxf_node *ent, char *path){ /* print the entity structure on file*/
	/* this function is non recursive */
	int i;
	int indent = 0;
	dxf_node *current, *prev;
	
	FILE *file;
	file = fopen(path, "w"); /* open the file */
	
	current = ent;
	while ((current != NULL) && (file != NULL)){
		prev = current;
		if (current->type == DXF_ENT){ /* DXF entity */
			if (current->obj.name){
				for (i=0; i<indent; i++){ /* print the indentation spaces */
					fprintf(file, "    ");
				}
				fprintf(file, current->obj.name);  /* print the string of entity's name */
				fprintf(file, "\n");
			}
			if (current->obj.content){
				/* starts the content sweep */
				prev = current->obj.content;
				current = current->obj.content->next;
				indent++;
			}
		}
		else if (current->type == DXF_ATTR){ /* DXF attibute */
			for (i=0; i<indent; i++){ /* print the indentation spaces */
				fprintf(file, "    ");
			}
			fprintf(file, "%d = ", current->value.group); /* print the DFX group */
			/* print the value of atrribute, acording its type */
			switch (current->value.t_data) {
				case DXF_STR:
					if(current->value.s_data){
						fprintf(file, current->value.s_data);
					}
					break;
				case DXF_FLOAT:
					fprintf(file, "%f", current->value.d_data);
					break;
				case DXF_INT:
					fprintf(file, "%d", current->value.i_data);
			}
			fprintf(file, "\n");
			current = current->next; /* go to the next in the list */
		}
		while (current == NULL){
			if (prev == ent){
				current = NULL;
				break;
			}
			prev = prev->master;
			if (prev){
				current = prev->next;
				indent --;
				if (prev == ent){
					current = NULL;
					fprintf(file, "fim loop ");
					break;
				}
			}
			else{
				current = NULL;
				break;
			}
		}
	}
	fclose(file);
}

dxf_node * dxf_obj_new (char *name){
	//char *new_name = NULL;
	
	/* create a new DXF Object */
	//dxf_node *new_obj = (dxf_node *) malloc(sizeof(dxf_node));
	//dxf_mem_pool(enum dxf_pool_action action, int idx)
	dxf_node *new_obj = dxf_mem_pool(ADD_DXF, 0);
	if (new_obj){
		new_obj->obj.name[0] = 0;
		if(name){
			//new_name = malloc(strlen(name)+1);  /* create new string */
			strncpy(new_obj->obj.name, name, DXF_MAX_CHARS); /* and copy the name */
		}
		new_obj->master = NULL;
		new_obj->prev = NULL;
		new_obj->next = NULL;
		new_obj->type = DXF_ENT;
		new_obj->obj.layer = 0;
		new_obj->obj.paper = 0;
		new_obj->obj.graphics = NULL;
		
		/* create head of content's list */
		//new_obj->obj.content = (dxf_node *) malloc(sizeof(dxf_node));
		new_obj->obj.content = (dxf_node *) dxf_mem_pool(ADD_DXF, 0);
		if(new_obj->obj.content){
			new_obj->end = new_obj->obj.content;
			new_obj->obj.content->master = new_obj;
			new_obj->obj.content->prev = NULL;
			new_obj->obj.content->next = NULL;
			new_obj->obj.content->type = DXF_ATTR;
			new_obj->obj.content->value.t_data = DXF_INT;
		}
		else { /* if error, free the memory */
			//free(new_name);
			//free(new_obj);
			new_obj = NULL;
		}
	}
	return new_obj;
}

int dxf_ident_attr_type (int group){
	/* Identifies the data type of the attribute,  */
	/* according to the value of the group. (DXF ranges) */
	
	if ((group >= 0) && (group < 10)){
		return DXF_STR; /* string */
	}
	else if( (group >= 10) && (group < 60)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 60) && (group < 100)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 100) && (group < 106)){
		return DXF_STR; /* string */
	}
	else if( (group >= 110) && (group < 150)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 170) && (group < 180)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 210) && (group < 240)){
		return DXF_FLOAT; /*  float */
	}
	else if( (group >= 270) && (group < 290)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 300) && (group < 370)){
		return DXF_STR; /* string */
	}
	else if( (group >= 370) && (group < 390)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 390) && (group < 400)){
		return DXF_STR; /* string */
	}
	else if( (group >= 400) && (group < 410)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 410) && (group < 420)){
		return DXF_STR; /* string */
	}
	else if( (group >= 420) && (group < 430)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 430) && (group < 440)){
		return DXF_STR; /* string */
	}
	else if( (group >= 440) && (group < 460)){
		return DXF_INT; /* integer */
	}
	else if( (group >= 460) && (group < 470)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 470) && (group < 480)){
		return DXF_STR; /* string */
	}
	else if( (group >= 999) && (group < 1010)){
		return DXF_STR; /* string */
	}
	else if( (group >= 1010) && (group < 1060)){
		return DXF_FLOAT; /* float */
	}
	else if( (group >= 1060) && (group < 1072)){
		return DXF_INT; /* integer */
	}
	else{ /* not defined type */
		return DXF_STR; /* default type is string */
	}
}

int dxf_ident_ent_type (dxf_node *obj){
	enum dxf_graph ent_type = DXF_NONE;
	if (obj){
		if (obj->type == DXF_ENT){
			if (strcmp(obj->obj.name, "LINE") == 0){
				ent_type = DXF_LINE;
			}
			else if (strcmp(obj->obj.name, "POINT") == 0){
				ent_type = DXF_POINT;
			}
			else if (strcmp(obj->obj.name, "CIRCLE") == 0){
				ent_type = DXF_CIRCLE;
			}
			else if (strcmp(obj->obj.name, "ARC") == 0){
				ent_type = DXF_ARC;
			}
			else if (strcmp(obj->obj.name, "TRACE") == 0){
				ent_type = DXF_TRACE;
			}
			else if (strcmp(obj->obj.name, "SOLID") == 0){
				ent_type = DXF_SOLID;
			}
			else if (strcmp(obj->obj.name, "TEXT") == 0){
				ent_type = DXF_TEXT;
			}
			else if (strcmp(obj->obj.name, "SHAPE") == 0){
				ent_type = DXF_SHAPE;
			}
			else if (strcmp(obj->obj.name, "INSERT") == 0){
				ent_type = DXF_INSERT;
			}
			else if (strcmp(obj->obj.name, "ATTRIB") == 0){
				ent_type = DXF_ATTRIB;
			}
			else if (strcmp(obj->obj.name, "POLYLINE") == 0){
				ent_type = DXF_POLYLINE;
			}
			else if (strcmp(obj->obj.name, "VERTEX") == 0){
				ent_type = DXF_VERTEX;
			}
			else if (strcmp(obj->obj.name, "LWPOLYLINE") == 0){
				ent_type = DXF_LWPOLYLINE;
			}
			else if (strcmp(obj->obj.name, "3DFACE") == 0){
				ent_type = DXF_3DFACE;
			}
			else if (strcmp(obj->obj.name, "VIEWPORT") == 0){
				ent_type = DXF_VIEWPORT;
			}
			else if (strcmp(obj->obj.name, "DIMENSION") == 0){
				ent_type = DXF_DIMENSION;
			}
			else if (strcmp(obj->obj.name, "ELLIPSE") == 0){
				ent_type = DXF_ELLIPSE;
			}
			else if (strcmp(obj->obj.name, "MTEXT") == 0){
				ent_type = DXF_MTEXT;
			}
			else if (strcmp(obj->obj.name, "BLOCK") == 0){
				ent_type = DXF_BLK;
			}
		}
	}
	return ent_type;
}

dxf_node * dxf_attr_new (int group, int type, void *value){
	/* create a new DXF attribute */
	//dxf_node *new_attr = (dxf_node *) malloc(sizeof(dxf_node));
	dxf_node *new_attr = (dxf_node *) dxf_mem_pool(ADD_DXF, 0);
	if (new_attr){
		new_attr->master = NULL;
		new_attr->prev = NULL;
		new_attr->next = NULL;
		new_attr->type = DXF_ATTR;
		
		new_attr->value.group = group;
		new_attr->value.t_data = type;
		
		switch(type) {
			case DXF_FLOAT :
				new_attr->value.d_data = *((double *)value);
				break;
			case DXF_INT :
				new_attr->value.i_data = *((int *)value);
				break;
			case DXF_STR :
				//new_attr->value.s_data = malloc(strlen((char *) value)+1);  /* create new string */
				new_attr->value.s_data[0] = 0;
				strncpy(new_attr->value.s_data,(char *) value, DXF_MAX_CHARS); /* and copy the string */
		}
	}
	return new_attr;
}
#if 0
vector_p dxf_find_attr(dxf_node * obj, int attr){
	int size = 0;
	dxf_node **data = NULL;
	vector_p list;
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					if(current->value.group == attr){ /* success */
						size++;
						data = realloc(data, size * sizeof(void *));
						if (data){
							data[size-1] = current;
						}
						else{
							size = 0;
							break;
						}
					}
				}
				current = current->next;
			}
		}
	}
	list.size = size;
	list.data = data;
	return list;
}
#endif

dxf_node * dxf_find_attr2(dxf_node * obj, int attr){
	/* return the first match */
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					if(current->value.group == attr){ /* success */
						return current;
					}
				}
				current = current->next;
			}
		}
	}
	return NULL;
}

dxf_node * dxf_find_attr_i(dxf_node * obj, int attr, int idx){
	/* return the match of  index (idx) */
	/* if the idx is zero, will return the first occurency. If is negative, will return the last occurency.*/
	dxf_node *current;
	dxf_node *found = NULL;
	int i = 0;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ATTR){
					/* verify if matchs */
					if(current->value.group == attr){
						found = current;
						/* and if is the index wanted */
						if (idx == i){
							return found; /* success */
						}
						i++; /* increment and continues the search */
					}
				}
				current = current->next;
			}
		}
	}
	if (idx < 0) return found; /* return the last element found */
	else return NULL;
}

#if 0
vector_p dxf_find_obj(dxf_node * obj, char *name){
	int size = 0;
	dxf_node **data = NULL;
	vector_p list;
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
					if(strcmp(current->obj.name, name) == 0){ /* success */
						size++;
						data = realloc(data, size * sizeof(void *));
						if (data){
							data[size-1] = current;
						}
						else{
							size = 0;
							break;
						}
					}
				}
				current = current->next;
			}
		}
	}
	list.size = size;
	list.data = data;
	return list;
}
#endif

dxf_node * dxf_find_obj2(dxf_node * obj, char *name){
	dxf_node *current;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
					if(strcmp(current->obj.name, name) == 0){ /* success */
						return current;
					}
				}
				current = current->next;
			}
		}
	}
	return NULL;
}

dxf_node * dxf_find_obj_i(dxf_node * obj, char *name, int idx){
	/* return the match of  index (idx) */
	/* if the idx is zero, will return the first occurency. If is negative, will return the last occurency.*/
	dxf_node *current;
	dxf_node *found = NULL;
	int i = 0;
	
	if(obj != NULL){ /* check if exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){
				if (current->type == DXF_ENT){
					/* verify if matchs */
					if(strcmp(current->obj.name, name) == 0){
						found = current;
						/* and if is the index wanted */
						if (idx == i){
							return found; /* success */
						}
						i++; /* increment and continues the search */
					}
				}
				current = current->next;
			}
		}
	}
	if (idx < 0) return found; /* return the last element found */
	else return NULL;
}

#if 0
vector_p dxf_find_obj_descr(dxf_node * obj, char *name, char *descr){
	int size = 0;
	dxf_node **data = NULL;
	vector_p list, descr_attr;
	dxf_node *current;
	
	if(obj != NULL){ /* check if obj exist */
		if (obj->type == DXF_ENT){
			current = obj->obj.content->next;
			while (current){ /* sweep master content */
				if (current->type == DXF_ENT){ /* look for dxf entities */
					if(strcmp(current->obj.name, name) == 0){ /* match obj's name */
						descr_attr = dxf_find_attr(current, 2); /* look for descriptor in group 2 attribute */
						if (descr_attr.data){ /* found attribute */
							/* match descriptor */
							if(strcmp(((dxf_node **) descr_attr.data)[0]->value.s_data, descr) == 0){
								/* success */
								size++;
								data = realloc(data, size * sizeof(void *));
								if (data){
									data[size-1] = current;
								}
								else{
									size = 0;
									break;
								}
							}
							free (descr_attr.data);
						}
					}
				}
				current = current->next;
			}
		}
	}
	list.size = size;
	list.data = data;
	return list;
}
#endif

dxf_node * dxf_find_obj_descr2(dxf_node * obj, char *name, char *descr){
	dxf_node *current, *descr_attr;
	
	if(obj != NULL){ /* check if obj exist */
		if (obj->type == DXF_ENT){
			
			char name_cpy[DXF_MAX_CHARS], *new_name;
			char descr_cpy[DXF_MAX_CHARS], *new_descr;
			
			/* copy strings for secure manipulation */
			strncpy(name_cpy, name, DXF_MAX_CHARS);
			strncpy(descr_cpy, descr, DXF_MAX_CHARS);
			/* remove trailing spaces */
			new_name = trimwhitespace(name_cpy);
			new_descr = trimwhitespace(descr_cpy);
			/* change to upper case */
			str_upp(new_name);
			str_upp(new_descr);
			
			current = obj->obj.content->next;
			while (current){ /* sweep master content */
				if (current->type == DXF_ENT){ /* look for dxf entities */
					if(strcmp(current->obj.name, new_name) == 0){ /* match obj's name */
						descr_attr = dxf_find_attr2(current, 2); /* look for descriptor in group 2 attribute */
						if (descr_attr){ /* found attribute */
							/* copy strings for secure manipulation */
							char test_descr[DXF_MAX_CHARS];
							strncpy(test_descr, descr_attr->value.s_data, DXF_MAX_CHARS);
							/* change to upper case */
							str_upp(test_descr);
							
							/* match descriptor */
							if(strcmp(test_descr, new_descr) == 0){
								/* success */
								return current;
							}
						}
					}
				}
				current = current->next;
			}
		}
	}
	return NULL;
}

void dxf_layer_assemb (dxf_drawing *drawing){
	int i, flags;
	dxf_node *current = NULL, *curr_layer = NULL;
	
	char name[DXF_MAX_CHARS];
	int color;
	char ltype[DXF_MAX_CHARS];
	int line_w;
	int frozen;
	int lock;
	int off;
	
	/* always set the index 0 as the default layer*/
	drawing->num_layers = 1;
	drawing->layers[0].name[0] = 0;
	drawing->layers[0].ltype[0] = 0;
	drawing->layers[0].color = 0;
	drawing->layers[0].line_w = 0;
	drawing->layers[0].frozen = 0;
	drawing->layers[0].lock = 0;
	drawing->layers[0].off = 0;
	
	drawing->layers[0].num_el = 0;
	drawing->layers[0].obj = NULL;
	
	i = 0;
	while (curr_layer = dxf_find_obj_i(drawing->t_layer, "LAYER", i)){/* get the next layer */
	
		name[0] = 0;
		color = 0;
		ltype[0] = 0;
		line_w = 0;
		frozen = 0;
		lock = 0;
		off = 0;
		
		/* and sweep its content */
		if (curr_layer->obj.content) current = curr_layer->obj.content->next;
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 2: /* layer name */
						strcpy(name, current->value.s_data);
						break;
					case 6: /* layer line type name */
						strcpy(ltype, current->value.s_data);
						break;
					case 62: /* layer color */
						color = current->value.i_data;
						if (color < 0) {
							off = 1;
							color = abs(color);
						}
						break;
					case 70: /* flags */
						flags = current->value.i_data;
						if (flags & 1) frozen = 1;
						if (flags & 4) lock = 1;
						break;
					case 370:
						line_w = current->value.i_data;
				}
			}
			current = current->next;
		}
		if ((i + 1) < DXF_MAX_LAYERS){
			/* set the variables on the current layer in drawing structure */
			strcpy(drawing->layers[i+1].name, name);
			strcpy(drawing->layers[i+1].ltype, ltype);
			drawing->layers[i+1].color = color;
			drawing->layers[i+1].line_w = line_w;
			drawing->layers[i+1].frozen = frozen;
			drawing->layers[i+1].lock = lock;
			drawing->layers[i+1].off = off;
			drawing->layers[i+1].num_el = 0;
			drawing->layers[i+1].obj = curr_layer;
		}
		
		i++;
	}
	if ((i + 1) < DXF_MAX_LAYERS) drawing->num_layers += i;
	else drawing->num_layers = DXF_MAX_LAYERS;
	//printf("Num Layers = %d\n", drawing->num_layers);
}

void dxf_ltype_assemb (dxf_drawing *drawing){
	int i, j, pat_idx;
	dxf_node *current = NULL, *curr_ltype = NULL;
	
	char name[DXF_MAX_CHARS], descr[DXF_MAX_CHARS];
	int size;
	double pat[DXF_MAX_PAT];
	double length, max;
	
	/* always set the index 0 as the default ltype*/
	drawing->num_ltypes = 1;
	drawing->ltypes[0].name[0] = 0;
	drawing->ltypes[0].descr[0] = 0;
	drawing->ltypes[0].size = 1;
	drawing->ltypes[0].pat[0] = 0;
	drawing->ltypes[0].length = 0;
	drawing->ltypes[0].num_el = 0;
	drawing->ltypes[0].obj = NULL;
	
	i = 0;
	while (curr_ltype = dxf_find_obj_i(drawing->t_ltype, "LTYPE", i)){/* get the next layer */
	
		name[0] = 0;
		descr[0] = 0;
		size = 0;
		pat[0] = 0;
		pat_idx = 0;
		length = 0;
		
		if (curr_ltype->obj.content) current = curr_ltype->obj.content->next;
		
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 2: /* ltype name */
						strcpy(name, current->value.s_data);
						break;
					case 3: /* ltype descriptive text */
						strcpy(descr, current->value.s_data);
						break;
					case 40: /* pattern length */
						length = current->value.d_data;
						break;
					case 49: /* pattern element */
						if (pat_idx < DXF_MAX_PAT) {
							pat[pat_idx] = current->value.d_data;
							pat_idx++;
						}
						break;
					case 73: /* num of pattern elements */
						size = current->value.i_data;
						if (size > DXF_MAX_PAT) {
							size < DXF_MAX_PAT;}
				}
			}
			current = current->next;
		}
		
		/* adjust pattern to pixel units 
		/* first, find the max patt length
		max = 0.0;
		for(j = 0; j < size; j++){
			if (max < fabs(pat[j])){
				max = fabs(pat[j]);
			}
		}
		if (max == 0.0) max = 1.0;
		/* then normalize each value in pattern 
		for(j = 0; j < size; j++){
			pat[j] = pat[j]/max;
		}
		*/
		
		/* set the variables on the current ltype in drawing structure */
		if ((i + 1) < DXF_MAX_LTYPES){
			strcpy(drawing->ltypes[i+1].name, name);
			strcpy(drawing->ltypes[i+1].descr, descr);
			memcpy(drawing->ltypes[i+1].pat, pat, size * sizeof(double));
			drawing->ltypes[i+1].size = size;
			drawing->ltypes[i+1].length = length;
			drawing->ltypes[i+1].num_el = 0;
			drawing->ltypes[i+1].obj = curr_ltype;
		}
		
		i++;
	}
	if ((i + 1) < DXF_MAX_LTYPES) drawing->num_ltypes += i;
	else drawing->num_ltypes = DXF_MAX_LTYPES;
}

void dxf_fonts_assemb (dxf_drawing *drawing){
	int i, flags;
	dxf_node *current = NULL, *curr_font = NULL;
	
	char name[DXF_MAX_CHARS];
	char file_name[DXF_MAX_CHARS];
	
	drawing->num_fonts = 0;
	
	/* open default font */
	shape *shx_font = shx_font_open("txt.shx");
	
	if (shx_font){
		/* always set the index 0 as the default font*/
		drawing->num_fonts = 1;
		drawing->text_fonts[0].name[0] = 0;
		drawing->text_fonts[0].shx_font = shx_font;
		drawing->text_fonts[0].num_el = 0;
		drawing->text_fonts[0].obj = NULL;
	}
	
	i = 0;
	while (curr_font = dxf_find_obj_i(drawing->t_style, "STYLE", i)){/* get the next layer */
	
		name[0] = 0;
		file_name[0] = 0;
			
		if (curr_font->obj.content) current = curr_font->obj.content->next;
		
		while (current){
			if (current->type == DXF_ATTR){
				switch (current->value.group){
					case 2: /* font name */
						strcpy(name, current->value.s_data);
						break;
					case 3: /* file name */
						strcpy(file_name, current->value.s_data);
						break;
					case 70: /* flags */
						flags = current->value.i_data;
				}
			}
			current = current->next;
		}
		if ((i + 1) < DXF_MAX_FONTS){
			/* set the variables on the current font in drawing structure */
			strcpy(drawing->text_fonts[i+1].name, name);
			shx_font = shx_font_open(file_name);
			drawing->text_fonts[i+1].shx_font = shx_font;
			drawing->text_fonts[i+1].num_el = 0;
			drawing->text_fonts[i+1].obj = curr_font;
		}
		i++;
	}
	if ((i + 1) < DXF_MAX_FONTS) drawing->num_fonts += i;
	else drawing->num_fonts = DXF_MAX_FONTS;
}

int dxf_lay_idx (dxf_drawing *drawing, char *name){
	int i;
	if (drawing){
		for (i=1; i < drawing->num_layers; i++){
			
			char name_cpy[DXF_MAX_CHARS], *new_name;
			char lay_cpy[DXF_MAX_CHARS], *new_lay;
			
			/* copy strings for secure manipulation */
			strncpy(name_cpy, name, DXF_MAX_CHARS);
			strncpy(lay_cpy, drawing->layers[i].name, DXF_MAX_CHARS);
			/* remove trailing spaces */
			new_name = trimwhitespace(name_cpy);
			new_lay = trimwhitespace(lay_cpy);
			/* change to upper case */
			str_upp(new_name);
			str_upp(new_lay);
			
			if (strcmp(new_lay, new_name) == 0){
				return i;
			}
		}
	}
	
	return 0; /*if search fails, return the standard layer */
}

int dxf_layer_get(dxf_drawing *drawing, dxf_node * obj){
	/* Return the layer index of drawing's layer vector */
	int ok = 0; /*if search fails, return the standard layer */
	
	if ((obj) && (drawing)){
		dxf_node *current = NULL;
		char layer[DXF_MAX_CHARS];
		layer[0] = 0;
		
		if (obj->type == DXF_ENT){
			if (obj->obj.content){
				current = obj->obj.content->next;
			}
		}
		while (current){
			if (current->type == DXF_ATTR){ /* DXF attibute */
				if (current->value.group == 8){
					strcpy(layer, current->value.s_data);
					break;
				}
			}
			current = current->next; /* go to the next in the list */
		}
		if (strlen(layer) > 0) ok = dxf_lay_idx (drawing, layer);
	}
	return ok;
}

int dxf_ltype_idx (dxf_drawing *drawing, char *name){
	int i;
	if (drawing){
		for (i=1; i < drawing->num_ltypes; i++){
			
			char name_cpy[DXF_MAX_CHARS], *new_name;
			char ltp_cpy[DXF_MAX_CHARS], *new_ltp;
			
			/* copy strings for secure manipulation */
			strncpy(name_cpy, name, DXF_MAX_CHARS);
			strncpy(ltp_cpy, drawing->ltypes[i].name, DXF_MAX_CHARS);
			/* remove trailing spaces */
			new_name = trimwhitespace(name_cpy);
			new_ltp = trimwhitespace(ltp_cpy);
			/* change to upper case */
			str_upp(new_name);
			str_upp(new_ltp);
			
			if (strcmp(new_ltp, new_name) == 0){
				return i;
			}
		}
	}
	
	return 0; /*if search fails, return the standard layer */
}

int dxf_font_idx (dxf_drawing *drawing, char *name){
	int i;
	if (drawing){
		for (i=1; i < drawing->num_fonts; i++){
			if (strcmp(drawing->text_fonts[i].name, name) == 0){
				return i;
			}
		}
	}
	
	return 0; /*if search fails, return the standard font */
}

int dxf_save (char *path, dxf_drawing *drawing){
	
	FILE *file;
	list_node *stack, *item ;
	dxf_node *current;
	int ret_success;
	
	/* initialize the stack */
	int stack_size = 0;
	stack = list_new(NULL, ONE_TIME);
	
	ret_success = 0;
	if (drawing){
		file = fopen(path, "w"); /* open the file */
		
		
		if ((file != NULL) && (drawing->main_struct != NULL)){
			current = drawing->main_struct->obj.content->next;
			while ((current != NULL) || (stack_size > 0)){
				if (current == NULL){ /* end of list sweeping */
					/* try to up in the structure hierarchy */
					item = list_pop(stack);
					if (item){
						stack_size--;
						current = (dxf_node *)item->data;
					}
					//current = stack_pop (&stack);
					if (current){
						/* write the end of complex entities, acording its type */
						if(strcmp(current->obj.name, "SECTION") == 0){
							fprintf(file, "0\nENDSEC\n");
						}
						else if(strcmp(current->obj.name, "TABLE") == 0){
							fprintf(file, "0\nENDTAB\n");
						}
						/*
						else if(strcmp(current->obj.name, "BLOCK") == 0){
							fprintf(file, "0\nENDBLK\n");
						}
						else{
							attr = dxf_find_attr(current, 66); /* look for entities folow flag
							if (attr.data){ /* if flag is found and 
								if(((dxf_node **) attr.data)[0]->value.i_data != 0){ /* its value is non zero 
									fprintf(file, "0\nSEQEND\n");
								}
								free(attr.data);
							}
						}*/
						current = current->next; /* go to the next in the list */
					}
				}
				else if (current->type == DXF_ENT){ /* DXF entity */
					/* down in the structure hierarchy */
					list_push(stack, list_new((void *)current, ONE_TIME));
					stack_size++;
					//stack_push (&stack, current);
					if (current->obj.name){
						fprintf(file, "0\n%s\n", current->obj.name); /* write the start of entity */
					}
					if (current->obj.content){
						/* starts the content sweep */
						current = current->obj.content->next;
					}
				}
				else if (current->type == DXF_ATTR){ /* DXF attribute */
					fprintf(file, "%d\n",  current->value.group); /* write the DFX group */
					/* write the value of attribute, acording its type */
					switch (current->value.t_data) {
						case DXF_STR:
							fprintf(file, "%s\n", current->value.s_data);
							break;
						case DXF_FLOAT:
							fprintf(file, "%f\n", current->value.d_data);
							break;
						case DXF_INT:
							fprintf(file, "%d\n",  current->value.i_data);
					}
					current = current->next; /* go to the next in the list */
				}
			}
			ret_success = 1; /* return success */
		}
		list_mem_pool(ZERO_LIST, ONE_TIME);
		fclose(file);
	}
	return ret_success;
}

char * dxf_load_file(char *path, long *fsize){
	FILE *file;
	
	*fsize = 0;
	file = fopen(path, "rb");
	if(file == NULL){
		return NULL;
	}
	
	fseek(file, 0, SEEK_END);
	*fsize = ftell(file); /* size of file*/
	fseek(file, 0, SEEK_SET);  //same as rewind(f);
	//printf("file size = %d\n", fsize);
	
	char *buf = malloc(*fsize + 1);
	if (!buf){
		*fsize = 0;
		fclose(file);
		return NULL;
	}
	fread(buf, *fsize, 1, file);
	fclose(file);
	buf[*fsize] = 0;
	return buf;
}

int dxf_read (dxf_drawing *drawing, char *buf, long fsize, int *prog){
	static enum {NONE, INIT, READ, FINISH} state = INIT;
	static char *line, *cur_line, *next_line, line_buf[DXF_MAX_CHARS];
	static dxf_node *main_struct = NULL;
	static dxf_node *master, *prev, *next, *tmp, *last_obj;
	
	static long f_index = 0;  /*  indexes the file�s lines */
	int line_size = 0;

	static dxf_node *new_node = NULL, *blk = NULL, *cplx = NULL, *part = NULL;
	
	static int group = 0; /* current group */
	static int t_data = DXF_STR; /* current type of data */
	static double d_data;
	static int i_data;
	//static vector_p part;
	static int blk_end = 0, cplx_end = 0; /* close block and include ENDBLOCK in structure*/
	static int ins_flag = 0; /* inside INSERT entity - for cascadind ATTRIB ents */
	
	if (state == INIT){
		if ((!drawing)||(!buf)){
			goto quit_error;
		}

		/* init the drawing */
		drawing->head = NULL;
		drawing->tabs = NULL;
		drawing->blks = NULL;
		drawing->ents = NULL; 
		drawing->t_ltype = NULL;
		drawing->t_layer = NULL;
		drawing->t_style = NULL;
		drawing->t_view = NULL;
		drawing->t_ucs = NULL;
		drawing->t_vport = NULL;
		drawing->t_dimst = NULL;
		drawing->t_appid = NULL;
		drawing->main_struct = NULL;
		
		/* create a new main_struct */
		main_struct = dxf_obj_new(NULL);
		if (!main_struct){
			goto quit_error;
		}
		
		cur_line = buf;
		
		master = main_struct; /* current master is the main struct */
		last_obj = main_struct;
		prev = main_struct->obj.content; /* the list starts on main_struct's content */
		next = NULL; /* end of list (empty list) */
		f_index = 0;
		state = READ;
	}
	
	if (state == READ){
		int i = 0;
		while((i++ < 10001) && (state == READ)){
			next_line = strchr(cur_line, '\n');
			if (next_line) {
				line_size = next_line - cur_line;
				if (line_size > DXF_MAX_CHARS - 1) line_size = DXF_MAX_CHARS - 1;
				memcpy(line_buf, cur_line, line_size);
				line_buf[line_size] = '\0';  /*terminate the current line*/
			}
			else{
				strncpy(line_buf, cur_line, DXF_MAX_CHARS);
			}
			
			/* trim leading/trailing whitespace of line */
			line = trimwhitespace(line_buf);
			//printf("%s\n", line);
			
			if((f_index % 2) == 0){ /* check if the line is even */
				/* in DXF files, the even lines identify the groups */
				group = atoi(line);
				
				/* Identifies the data type of the next line,  */
				/* according to the value of the group. (DXF ranges) */
				
				t_data = dxf_ident_attr_type(group);
				
			}
			else {
				/* strips \n of line string */
				//line[strcspn(line, "\r\n")] = 0; /* strips \n or \r of line string */
				
				switch(group){
					case 0:
						if (blk_end) { /* End Block after ENDBLK entity */
							blk_end = 0;
							/* back to the previous level on hierarchy */
							master = blk->master;
							last_obj = master;
							prev = blk;
						} 
						else if (cplx_end) { /* End Complex after SEQEND entity */
							cplx_end = 0;
							/* back to the previous level on hierarchy */
							master = cplx->master;
							last_obj = master;
							prev = cplx;
						} 
						else prev = master->end;
					
						str_upp(line); /* upper case the line */
					
						/* Find the end of the master's content list */
						//tmp = master->obj.content;
						//while(tmp->next != NULL)
						//	tmp = tmp->next;
						//prev = tmp; /* new objs will append here */
						
						ins_flag = 0;
					
						/* new level of hierarchy  */
						if((strcmp(line, "SECTION") == 0) ||
							(strcmp(line, "TABLE") == 0) ||
							(strcmp(line, "BLOCK") == 0) ||
							(strcmp(line, "POLYLINE") == 0)){
							new_node = dxf_obj_new (line); /* new object */
							if (new_node){
								/*  append new to master's list */
								new_node->master = master;
								new_node->prev = prev;
								if (prev){
									prev->next = new_node;
								}
								new_node->next = NULL; /* at end of list */
								master->end = new_node;
								last_obj = new_node;
								
								if (strcmp(line, "BLOCK") == 0) blk = new_node;
								if (strcmp(line, "POLYLINE") == 0) cplx = new_node;
								
								/* the new becomes the master */
								master = new_node;
								prev = new_node->obj.content; /* new objs will append here */
							}
						}
						
						/* back to the previous level on hierarchy */
						else if((strcmp(line, "ENDSEC") == 0) ||
							(strcmp(line, "ENDTAB") == 0) ){//||
							//(strcmp(line, "ENDBLK") == 0)||
							//(strcmp(line, "SEQEND") == 0)){
							if(master->master){
								/* back to master's master ;) */
								master = master->master;
								last_obj = master;
								/* Find the end of the master's content list */
								//tmp = master->obj.content;
								//while(tmp->next != NULL)
								//	tmp = tmp->next;
								//prev = tmp; /* new items will append here */
								prev = master->end;
							}
						}
						
						/*  new ordinary DXF object */
						else {
							if (strcmp(line, "INSERT") == 0) ins_flag = 1;
							
							new_node = dxf_obj_new (line); /* new object */
							if (new_node){
								/*  append new to master's list */
								new_node->master = master;
								new_node->prev = prev;
								if (prev){
									prev->next = new_node;
								}
								new_node->next = NULL; /* append at end of list */
								master->end = new_node;
								prev = new_node->obj.content; /* new attrs will append here */
								new_node->end = new_node->obj.content;
								last_obj = new_node;
								
								if (strcmp(line, "ENDBLK") == 0){
									blk_end = 1;
									//blk = master;
								}
								if (strcmp(line, "SEQEND") == 0) cplx_end = 1;
							}
							
						}
						//new_node = NULL;
						break;
						
					case 66: /* Entities follow flag -  for INSERT ents only*/
						
						i_data = atoi(line);
						new_node = dxf_attr_new (group, t_data, (void *) &i_data);
						if (new_node){
							/*  append new to last obj's list */
							new_node->master = last_obj;
							new_node->prev = prev;
							if (prev){
								prev->next = new_node;
							}
							new_node->next = NULL; /* append at end of list */
							last_obj->end = new_node;
						}
					
						if ((i_data != 0) && (ins_flag != 0)){
							/* new level of hierarchy */
							cplx = last_obj;
							master = last_obj; /*  the last obj becomes the master */
							
							/* Find the end of the master's content list */
							//tmp = master->obj.content;
							//while(tmp->next != NULL)
							//	tmp = tmp->next;
							//prev = tmp; /* new objs will append here */
							prev = master->end;
						}
						else prev = new_node;
						break;
					default:
						switch(t_data) {
							case DXF_FLOAT :
								d_data = atof(line);
								new_node = dxf_attr_new (group, t_data, (void *) &d_data);
								break;
							case DXF_INT :
								i_data = atoi(line);
								new_node = dxf_attr_new (group, t_data, (void *) &i_data);
								break;
							case DXF_STR :
								new_node = dxf_attr_new (group, t_data, (void *) line);
						}
						
						if (new_node){
							/*  append new to last obj's list */
							new_node->master = last_obj;
							new_node->prev = prev;
							if (prev){
								prev->next = new_node;
							}
							new_node->next = NULL; /* append at end of list */
							last_obj->end = new_node;
							prev = new_node;
						}
					
				}
				new_node = NULL;
			}
			
			f_index++; /* next line index */
			cur_line = next_line ? (next_line+1) : NULL;
			if(cur_line == NULL){
				//free(buf);
				//buf = NULL;
				state = FINISH;
			}
		}
		if (cur_line){
			*prog = (int)((double)(cur_line - buf)*100/fsize);
		}
		return 1;
	}
	if (state == FINISH){
			
		drawing->main_struct = main_struct;
		
		/* disassembly the drawing structure */
		/* the main sections */
		part = dxf_find_obj_descr2(main_struct, "SECTION", "HEADER");
		if (part) drawing->head = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "TABLES");
		if (part) drawing->tabs = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "BLOCKS");
		if (part) drawing->blks = part;
		part = dxf_find_obj_descr2(main_struct, "SECTION", "ENTITIES");
		if (part) drawing->ents = part;
		
		/* the tables */
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "LTYPE");
		if (part) drawing->t_ltype = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "LAYER");
		if (part) drawing->t_layer = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "STYLE");
		if (part) drawing->t_style = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "VIEW");
		if (part) drawing->t_view = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "UCS");
		if (part) drawing->t_ucs = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "VPORT");
		if (part) drawing->t_vport = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "DIMSTYLE");
		if (part) drawing->t_dimst = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "BLOCK_RECORD");
		if (part) drawing->blks_rec = part;
		
		part = dxf_find_obj_descr2(drawing->tabs, "TABLE", "APPID");
		if (part) drawing->t_appid = part;
		
		/* assemble the layers list */
		dxf_layer_assemb (drawing);
		
		/* assemble the ltypes list */
		dxf_ltype_assemb (drawing);
		
		/* assemble the fonts list */
		dxf_fonts_assemb(drawing);
		
		/*find the handle seed */
		drawing->hand_seed = dxf_find_attr2(drawing->head, 5);
		
		//return drawing;
		state = INIT;
		*prog = 100;
	}
	return 0;
	
quit_error:
	state = INIT;
	return -1;
}

void dxf_append(dxf_node *master, dxf_node *new_node){
	if (master && new_node){
		dxf_node *prev = master->end;
		/*  append new to master's list */
		new_node->master = master;
		new_node->prev = prev;
		if (prev){
			prev->next = new_node;
		}
		new_node->next = NULL; /* append at end of list */
		master->end = new_node;
	}
}

void dxf_list_clear (dxf_node *list){
	if (list){
		list->obj.name[0] = 0;
		list->master = NULL;
		list->prev = NULL;
		list->next = NULL;
		list->type = DXF_ENT;
		list->obj.graphics = NULL;
		if(list->obj.content){
			list->end = list->obj.content;
			list->obj.content->master = list;
			list->obj.content->prev = NULL;
			list->obj.content->next = NULL;
			list->obj.content->type = DXF_ATTR;
			list->obj.content->value.t_data = DXF_INT;
		}
	}
}