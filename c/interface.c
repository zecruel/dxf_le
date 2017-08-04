#include <SDL.h>

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "shape2.h"
#include "dxf_graph.h"
#include "list.h"

#include "dxf_colors.h"
#include "dxf_seed.h"

#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
//#include <locale.h>

#include "tinyfiledialogs.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_ZERO_COMMAND_MEMORY
#include "nuklear.h"
#include "gui.h"

void toolbox_get_imgs(bmp_img *img, int w, int h, bmp_img *vec[], int num){
	if (vec){
		int i, col = 0, lin = 0;
		for (i = 0; i < num; i++){
			vec[i] = NULL;
			vec[i] = bmp_sub_img(img, col * w, lin * h, w, h);
			col++;
			if ((col * w) + w > (int) img->width){
				col = 0;
				lin++;
			}
		}
	}
}

void draw_cursor(bmp_img *img, int x, int y, bmp_color color){
	/* draw cursor */
	/* set the pattern */
	double pat = 1;
	patt_change(img, &pat, 1);
	/* set the color */
	img->frg = color;
	/* set the tickness */
	img->tick = 3;
	bmp_line(img, 0, y, img->width, y);
	bmp_line(img, x, 0, x, img->height);
	img->tick = 1;
	bmp_line(img, x-5, y+5, x+5, y+5);
	bmp_line(img, x-5, y-5, x+5, y-5);
	bmp_line(img, x+5, y-5, x+5, y+5);
	bmp_line(img, x-5, y-5, x-5, y+5);
}

void zoom_ext(dxf_drawing *drawing, bmp_img *img, double *zoom, double *ofs_x, double *ofs_y){
	double min_x, min_y, max_x, max_y;
	double zoom_x, zoom_y;
	dxf_ents_ext(drawing, &min_x, &min_y, &max_x, &max_y);
	zoom_x = (max_x - min_x)/img->width;
	zoom_y = (max_y - min_y)/img->height;
	*zoom = (zoom_x > zoom_y) ? zoom_x : zoom_y;
	if (*zoom <= 0){ *zoom =1;}
	else{ *zoom = 1/(1.1 * (*zoom));}
	
	*ofs_x = min_x - (fabs((max_x - min_x)*(*zoom) - img->width)/2)/(*zoom);
	*ofs_y = min_y - (fabs((max_y - min_y)*(*zoom) - img->height)/2)/(*zoom);
}

void sel_list_append(list_node *list, dxf_node *ent){
	if (list && ent){
		list_node * new_el = (list_node *)list_new(ent, 0);
		if (new_el){
			if (list_find_data(list, ent)){
				//printf ("ja existe!\n");
			}
			else{
				list_push(list, new_el);
			}
		}
	}
}

int main(int argc, char** argv){
	//setlocale(LC_ALL,""); //seta a localidade como a current do computador para aceitar acentuacao
	
	double zoom = 20.0 , ofs_x = 0.0, ofs_y = 0.0;
	double prev_zoom;
	int color_idx = 256;
	int layer_idx = 0, ltypes_idx = 0;
	dxf_node *element = NULL, *prev_el = NULL, *new_el = NULL;
	double pos_x, pos_y, x0, y0, x1, y1, bulge = 0.0, txt_h = 1.0;
	double thick = 0.0;
	char txt[DXF_MAX_CHARS];
	dxf_node *x0_attr = NULL, *y0_attr = NULL, *x1_attr = NULL, *y1_attr = NULL;
	
	unsigned int width = 1024;
	unsigned int height = 600;
	int open_prg = 0;
	int progress = 0;
	int progr_win = 0;
	int progr_end = 0;
	unsigned int quit = 0;
	unsigned int wait_open = 0;
	int i;
	int ev_type, draw = 0, draw_tmp = 0;
	struct nk_color background;
	
	int leftMouseButtonDown = 0;
	int rightMouseButtonDown = 0;
	int leftMouseButtonClick = 0;
	int rightMouseButtonClick = 0;
	int MouseMotion = 0;
	
	int step = 0;
	//graph_obj *tmp_graph = NULL;
	
	SDL_Event event;
	int mouse_x, mouse_y;
	
	double rect_pt1[2], rect_pt2[2];
	int low_proc = 1;
	
	enum Action {
		NONE,
		FILE_OPEN,
		FILE_SAVE,
		EXPORT,
		VIEW_ZOOM_EXT,
		DELETE,
		EXIT
	} action = NONE;
	
	enum Modal {
		SELECT,
		LINE,
		POLYLINE,
		CIRCLE,
		RECT,
		TEXT,
		ARC
	}modal = SELECT;
	
	char recv_comm[64];
	int recv_comm_flag = 0;
	
	char *url = NULL;
	char const * lFilterPatterns[2] = { "*.dxf", "*.txt" };
	
	char *file_buf = NULL;
	long file_size = 0;
	
	char* dropped_filedir;                  /* Pointer for directory of dropped file */
	
	/* Colors in use */
	bmp_color white = {.r = 255, .g = 255, .b =255, .a = 255};
	bmp_color black = {.r = 0, .g = 0, .b =0, .a = 255};
	bmp_color blue = {.r = 0, .g = 0, .b =255, .a = 255};
	bmp_color red = {.r = 255, .g = 0, .b =0, .a = 255};
	bmp_color green = {.r = 0, .g = 255, .b =0, .a = 255};
	bmp_color grey = {.r = 100, .g = 100, .b = 100, .a = 255};
	bmp_color hilite = {.r = 255, .g = 0, .b = 255, .a = 150};
	bmp_color transp = {.r = 255, .g = 255, .b = 255, .a = 0};
	bmp_color cursor = {.r = 255, .g = 255, .b = 255, .a = 100};
	background = nk_rgb(28,48,62);
	
	/* line types in use */
	double center [] = {12, -6, 2 , -6};
	double dash [] = {8, -8};
	double continuous[] = {1};
	
	/* initialize the selection list */
	list_node * sel_list = (list_node *)list_new(NULL, 0);
	
	/* init the main image */
	bmp_img * img = bmp_new(width, height, grey, red);
	
	/* init Nuklear GUI */
	shape *shx_font = shx_font_open("txt.shx");
	gui_obj *gui = nk_sdl_init(shx_font);
	
	gui->ctx->style.edit.padding = nk_vec2(4, -6);
	
	/* init the toolbox image */
	bmp_img * tool_img = bmp_load_img("tool2.png");
	bmp_img * tool_vec[40];
	toolbox_get_imgs(tool_img, 16, 16, tool_vec, 40);
	struct nk_image i_new = nk_image_ptr(tool_vec[0]);
	struct nk_image i_open = nk_image_ptr(tool_vec[1]);
	struct nk_image i_save = nk_image_ptr(tool_vec[2]);
	struct nk_image i_close = nk_image_ptr(tool_vec[3]);
	struct nk_image i_export = nk_image_ptr(tool_vec[4]);
	struct nk_image i_import = nk_image_ptr(tool_vec[5]);
	struct nk_image i_print = nk_image_ptr(tool_vec[6]);
	struct nk_image i_help = nk_image_ptr(tool_vec[7]);
	struct nk_image i_copy = nk_image_ptr(tool_vec[8]);
	struct nk_image i_cut = nk_image_ptr(tool_vec[9]);
	struct nk_image i_paste = nk_image_ptr(tool_vec[10]);
	struct nk_image i_undo = nk_image_ptr(tool_vec[11]);
	struct nk_image i_redo = nk_image_ptr(tool_vec[12]);
	struct nk_image i_view = nk_image_ptr(tool_vec[13]);
	struct nk_image i_unview = nk_image_ptr(tool_vec[14]);
	struct nk_image i_unlock = nk_image_ptr(tool_vec[15]);
	struct nk_image i_lock = nk_image_ptr(tool_vec[16]);
	struct nk_image i_select = nk_image_ptr(tool_vec[17]);
	struct nk_image i_move = nk_image_ptr(tool_vec[18]);
	struct nk_image i_dupli = nk_image_ptr(tool_vec[19]);
	struct nk_image i_rotate = nk_image_ptr(tool_vec[20]);
	struct nk_image i_mirror = nk_image_ptr(tool_vec[21]);
	struct nk_image i_group = nk_image_ptr(tool_vec[22]);
	struct nk_image i_ungroup = nk_image_ptr(tool_vec[23]);
	struct nk_image i_delete = nk_image_ptr(tool_vec[24]);
	struct nk_image i_line = nk_image_ptr(tool_vec[25]);
	struct nk_image i_poly = nk_image_ptr(tool_vec[26]);
	struct nk_image i_block = nk_image_ptr(tool_vec[27]);
	struct nk_image i_circle = nk_image_ptr(tool_vec[28]);
	struct nk_image i_arc = nk_image_ptr(tool_vec[29]);
	struct nk_image i_text = nk_image_ptr(tool_vec[30]);
	struct nk_image i_spline = nk_image_ptr(tool_vec[31]);
	struct nk_image i_z_plus = nk_image_ptr(tool_vec[32]);
	struct nk_image i_z_minus = nk_image_ptr(tool_vec[33]);
	struct nk_image i_z_win = nk_image_ptr(tool_vec[34]);
	struct nk_image i_z_all = nk_image_ptr(tool_vec[35]);
	struct nk_image i_meas = nk_image_ptr(tool_vec[36]);
	struct nk_image i_layer = nk_image_ptr(tool_vec[37]);
	struct nk_image i_image = nk_image_ptr(tool_vec[38]);
	struct nk_image i_lib = nk_image_ptr(tool_vec[39]);
	
	struct nk_style_button b_icon_style;
	if (gui){
		b_icon_style = gui->ctx->style.button;
	}
	b_icon_style.image_padding.x = -4;
	b_icon_style.image_padding.y = -4;
	
	/* init comands */
	recv_comm[0] = 0;
	txt[0] = 0;
	
	/* init the drawing */
	dxf_drawing *drawing = malloc(sizeof(dxf_drawing));
	url = NULL; /* pass a null file only for initialize the drawing structure */
	//dxf_open(drawing, url);
	while (dxf_read (drawing, (char *)dxf_seed_2007, strlen(dxf_seed_2007), &progress) > 0){
		
	}
	//printf(dxf_seed_r12);
	dxf_ents_parse(drawing);

	/* init the SDL2 */
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window * window = SDL_CreateWindow(
		"DXF Viewer with SDL2", /* title */
		SDL_WINDOWPOS_UNDEFINED, /* x position */
		SDL_WINDOWPOS_UNDEFINED, /* y position */
		width, /* width */
		height, /* height */
		0); /* flags */
	
	SDL_Surface *surface;     // Declare an SDL_Surface to be filled in with pixel data from an image file
	Uint16 pixels[16*16] = {  // ...or with raw pixel data:
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0aab, 0x0789, 0x0bcc, 0x0eee, 0x09aa, 0x099a, 0x0ddd,
		0x0fff, 0x0eee, 0x0899, 0x0fff, 0x0fff, 0x1fff, 0x0dde, 0x0dee,
		0x0fff, 0xabbc, 0xf779, 0x8cdd, 0x3fff, 0x9bbc, 0xaaab, 0x6fff,
		0x0fff, 0x3fff, 0xbaab, 0x0fff, 0x0fff, 0x6689, 0x6fff, 0x0dee,
		0xe678, 0xf134, 0x8abb, 0xf235, 0xf678, 0xf013, 0xf568, 0xf001,
		0xd889, 0x7abc, 0xf001, 0x0fff, 0x0fff, 0x0bcc, 0x9124, 0x5fff,
		0xf124, 0xf356, 0x3eee, 0x0fff, 0x7bbc, 0xf124, 0x0789, 0x2fff,
		0xf002, 0xd789, 0xf024, 0x0fff, 0x0fff, 0x0002, 0x0134, 0xd79a,
		0x1fff, 0xf023, 0xf000, 0xf124, 0xc99a, 0xf024, 0x0567, 0x0fff,
		0xf002, 0xe678, 0xf013, 0x0fff, 0x0ddd, 0x0fff, 0x0fff, 0xb689,
		0x8abb, 0x0fff, 0x0fff, 0xf001, 0xf235, 0xf013, 0x0fff, 0xd789,
		0xf002, 0x9899, 0xf001, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0xe789,
		0xf023, 0xf000, 0xf001, 0xe456, 0x8bcc, 0xf013, 0xf002, 0xf012,
		0x1767, 0x5aaa, 0xf013, 0xf001, 0xf000, 0x0fff, 0x7fff, 0xf124,
		0x0fff, 0x089a, 0x0578, 0x0fff, 0x089a, 0x0013, 0x0245, 0x0eff,
		0x0223, 0x0dde, 0x0135, 0x0789, 0x0ddd, 0xbbbc, 0xf346, 0x0467,
		0x0fff, 0x4eee, 0x3ddd, 0x0edd, 0x0dee, 0x0fff, 0x0fff, 0x0dee,
		0x0def, 0x08ab, 0x0fff, 0x7fff, 0xfabc, 0xf356, 0x0457, 0x0467,
		0x0fff, 0x0bcd, 0x4bde, 0x9bcc, 0x8dee, 0x8eff, 0x8fff, 0x9fff,
		0xadee, 0xeccd, 0xf689, 0xc357, 0x2356, 0x0356, 0x0467, 0x0467,
		0x0fff, 0x0ccd, 0x0bdd, 0x0cdd, 0x0aaa, 0x2234, 0x4135, 0x4346,
		0x5356, 0x2246, 0x0346, 0x0356, 0x0467, 0x0356, 0x0467, 0x0467,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff,
		0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff, 0x0fff
	};
	surface = SDL_CreateRGBSurfaceFrom(pixels,16,16,16,16*2,0x0f00,0x00f0,0x000f,0xf000);

	// The icon is attached to the window pointer
	SDL_SetWindowIcon(window, surface);

	// ...and the surface containing the icon pixel data is no longer required.
	SDL_FreeSurface(surface);

	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
	/*
	SDL_RendererInfo info;
	if (SDL_GetRenderDriverInfo(0, &info) == 0){
		//printf("Driver num pix format = %d\n", info.num_texture_formats);
		for (i = 0; i < info.num_texture_formats; i++){
			switch (info.texture_formats[i]){
				case SDL_PIXELFORMAT_ARGB8888:
					//printf("pix format ARGB8888\n");
					//img->r_i = 2;
					//img->g_i = 1;
					//img->b_i = 0;
					//img->a_i = 3;
					break;
				case SDL_PIXELFORMAT_RGBA8888:
					//printf("pix format RGBA8888\n");
					break;
				case SDL_PIXELFORMAT_ABGR8888:
					//printf("pix format ABGR8888\n");
					break;
				case SDL_PIXELFORMAT_BGRA8888:
					//printf("pix format BGRA8888\n");
					break;
			}
		}
	}
	*/
	
	SDL_Texture * canvas = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STATIC, 
		width, /* width */
		height); /* height */
		
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	
	//SDL_SetTextureBlendMode(canvas, SDL_BLENDMODE_BLEND);
	
	/* main loop */
	while (quit == 0){
		ev_type = 0;
		low_proc = 1;
		
		//SDL_ShowCursor(SDL_DISABLE);
		
		/* get events for Nuklear GUI input */
		nk_input_begin(gui->ctx);
		if(SDL_PollEvent(&event)){
			if (event.type == SDL_QUIT) quit = 1;
			nk_sdl_handle_event(gui, window, &event);
			ev_type = event.type;
		}
		nk_input_end(gui->ctx);
		
		/* ===============================*/
		if (nk_window_is_any_hovered(gui->ctx)) {
			SDL_ShowCursor(SDL_ENABLE);
			//printf("show\n");
		}
		else{
			SDL_ShowCursor(SDL_DISABLE);
			if (ev_type != 0){
				
				switch (event.type){
					case SDL_MOUSEBUTTONUP:
						mouse_x = event.button.x;
						mouse_y = event.button.y;
						mouse_y = height - mouse_y;
						if (event.button.button == SDL_BUTTON_LEFT){
							leftMouseButtonDown = 0;
						}
						else if(event.button.button == SDL_BUTTON_RIGHT){
							rightMouseButtonDown = 0;
						}
						break;
					case SDL_MOUSEBUTTONDOWN:
						mouse_x = event.button.x;
						mouse_y = event.button.y;
						mouse_y = height - mouse_y;
						if (event.button.button == SDL_BUTTON_LEFT){
							leftMouseButtonDown = 1;
							leftMouseButtonClick = 1;
						}
						else if(event.button.button == SDL_BUTTON_RIGHT){
							rightMouseButtonDown = 1;
							rightMouseButtonClick = 1;
						}
						break;
					case SDL_MOUSEMOTION:
						MouseMotion = 1;
						mouse_x = event.motion.x;
						mouse_y = event.motion.y;
						mouse_y = height - mouse_y;
						pos_x = (double) mouse_x/zoom + ofs_x;
						pos_y = (double) mouse_y/zoom + ofs_y;
						draw = 1;
						break;
					case SDL_MOUSEWHEEL:
						prev_zoom = zoom;
						zoom = zoom + event.wheel.y * 0.2 * zoom;
						
						SDL_GetMouseState(&mouse_x, &mouse_y);
						mouse_y = height - mouse_y;
						ofs_x += ((double) mouse_x)*(1/prev_zoom - 1/zoom);
						ofs_y += ((double) mouse_y)*(1/prev_zoom - 1/zoom);
						draw = 1;
						break;
					
					case (SDL_DROPFILE): {      // In case if dropped file
						dropped_filedir = event.drop.file;
						// Shows directory of dropped file
						//SDL_ShowSimpleMessageBox(
						//	SDL_MESSAGEBOX_INFORMATION,
						//	"File dropped on window",
						//	dropped_filedir,
						//	window);
						printf(dropped_filedir);
						printf("\n----------------------------\n");
						SDL_free(dropped_filedir);    // Free dropped_filedir memory
						break;
					}
				}
			}
		}
		
		/* GUI */
		/* main toolbox, for open files, save or exit */
		/*if (nk_begin(gui->ctx, "Main", nk_rect(5, 5, 200, 40),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE)){
			/*
			nk_layout_row_dynamic(gui->ctx, 30, 1);
			if (nk_button_label(gui->ctx, "Open")){
				action = FILE_OPEN;
			}
			if (nk_button_label(gui->ctx, "Save")){
				action = FILE_SAVE;
			}
			if (nk_button_label(gui->ctx, "Exit")){
				action = EXIT;
				quit = 1;
			}*/
			/*main menu 
			nk_menubar_begin(gui->ctx);
			nk_layout_row_dynamic(gui->ctx, 15, 3);
			/* new menu
			if (nk_menu_begin_label(gui->ctx, "FILE", NK_TEXT_LEFT, nk_vec2(120, 200))){
				nk_layout_row_dynamic(gui->ctx, 15, 1);
				if (nk_menu_item_label(gui->ctx, "Open", NK_TEXT_LEFT)){
					action = FILE_OPEN;
				}
				if (nk_menu_item_label(gui->ctx, "Save", NK_TEXT_LEFT)){
					action = FILE_SAVE;
				}
				if (nk_menu_item_label(gui->ctx, "Exit", NK_TEXT_LEFT)){
					action = EXIT;
					quit = 1;
				}
				
				nk_menu_end(gui->ctx);
			}
			/* new menu
			if (nk_menu_begin_label(gui->ctx, "VIEW", NK_TEXT_LEFT, nk_vec2(120, 200))){
				nk_layout_row_dynamic(gui->ctx, 15, 1);
				if (nk_menu_item_label(gui->ctx, "Extend", NK_TEXT_LEFT)){
					action = VIEW_ZOOM_EXT;
				}
				if (nk_menu_item_label(gui->ctx, "Zoom in", NK_TEXT_LEFT)){
					//action = FILE_OPEN;
				}
				nk_menu_end(gui->ctx);
			}
			/* new menu
			if (nk_menu_begin_label(gui->ctx, "HELP", NK_TEXT_LEFT, nk_vec2(120, 200))){
				nk_layout_row_dynamic(gui->ctx, 15, 1);
				if (nk_menu_item_label(gui->ctx, "About", NK_TEXT_LEFT)){
					//action = FILE_OPEN;
				}
				
				nk_menu_end(gui->ctx);
			}
			
			nk_menubar_end(gui->ctx);
		}
		nk_end(gui->ctx);*/
		
		if (nk_begin(gui->ctx, "Main", nk_rect(0, 0, 200, 32),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR)){
			nk_layout_row_static(gui->ctx, 20, 20, 10);
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_new)){
				printf("NEW\n");
			}
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_open)){
				action = FILE_OPEN;
			}
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_save)){
				action = FILE_SAVE;
			}
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_export)){
				action = EXPORT;
			}
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_close)){
				printf("CLOSE\n");
			}
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_help)){
				printf("HELP\n");
			}
			
		}
		nk_end(gui->ctx);
		
		if (nk_begin(gui->ctx, "Toolbox", nk_rect(2, 50, 100, 300),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)){
			
			/*
			struct nk_image tool;
			for (i = 0; i < 40; i++){
				tool = nk_image_ptr(tool_vec[i]);
				tool.w = tool_vec[0]->width;
				tool.h = tool_vec[0]->height;
				tool.region[2] = (unsigned short)tool_vec[0]->width;
				tool.region[3] = (unsigned short)tool_vec[0]->height;
				
				if (nk_button_image_styled(gui->ctx, &b_icon_style, tool)){
					printf("i = %d\n", i);
				}
			}*/
			if (nk_tree_push(gui->ctx, NK_TREE_TAB, "Place", NK_MAXIMIZED)) {
				nk_layout_row_static(gui->ctx, 20, 20, 2);
				
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_select)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","SELECT");
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_line)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","LINE");
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_poly)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","POLYLINE");
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_block)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","RECT");
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_text)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","TEXT");
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_circle)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","CIRCLE");
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_arc)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_spline)){
					
				}
				nk_tree_pop(gui->ctx);
			}
			if (nk_tree_push(gui->ctx, NK_TREE_TAB, "Modify", NK_MAXIMIZED)) {
				nk_layout_row_static(gui->ctx, 20, 20, 2);
				
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_move)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_dupli)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_rotate)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_mirror)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_group)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_ungroup)){
					
				}
				if (nk_button_image_styled(gui->ctx, &b_icon_style, i_delete)){
					recv_comm_flag = 1;
					snprintf(recv_comm, 64, "%s","DELETE");
				}
				nk_tree_pop(gui->ctx);
			}
		}
		nk_end(gui->ctx);
		
		if (nk_begin(gui->ctx, "Tool", nk_rect(110, 50, 200, 200),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
		NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)){
			switch (modal) {
				case SELECT:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Select an object", NK_TEXT_LEFT);
					break;
				case LINE:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Place a single line", NK_TEXT_LEFT);
					if (step == 0){
						nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
					} else {
						nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
					}
					break;
				case POLYLINE:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Place a poly line", NK_TEXT_LEFT);
					if (step == 0){
						nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
					} else {
						nk_label(gui->ctx, "Enter next point", NK_TEXT_LEFT);
					}
					bulge = nk_propertyd(gui->ctx, "Bulge", 0.0d, bulge, 1.0d, 0.1d, 0.1d);
					break;
				case CIRCLE:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Place a circle", NK_TEXT_LEFT);
					if (step == 0){
						nk_label(gui->ctx, "Enter center point", NK_TEXT_LEFT);
					} else {
						nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
					}
					break;
				case RECT:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Place a rectangle", NK_TEXT_LEFT);
					if (step == 0){
						nk_label(gui->ctx, "Enter first point", NK_TEXT_LEFT);
					} else {
						nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
					}
					break;
				case TEXT:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Place an text", NK_TEXT_LEFT);
					//nk_edit_string(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER, comm, &comm_len, 64, nk_filter_default);
					nk_edit_string_zero_terminated(gui->ctx, NK_EDIT_SIMPLE, txt, DXF_MAX_CHARS, nk_filter_default);
					txt_h = nk_propertyd(gui->ctx, "Text Height", 0.0d, txt_h, 100.0d, 0.1d, 0.1d);
					break;
				case ARC:
					nk_layout_row_dynamic(gui->ctx, 20, 1);
					nk_label(gui->ctx, "Place an arc", NK_TEXT_LEFT);
					break;
			}
		}
		nk_end(gui->ctx);
		
		if (progr_win){
			/* opening */
			if (nk_begin(gui->ctx, "Progress", nk_rect(200, 200, 400, 40),
			NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR))
			//if (nk_popup_begin(gui->ctx, NK_POPUP_STATIC, "opening", 0, nk_rect(200, 200, 400, 40)))
			{
				static char text[64];
				static int text_len;
				nk_layout_row_dynamic(gui->ctx, 20, 2);
				text_len = snprintf(text, 63, "Opening...");
				nk_label(gui->ctx, text, NK_TEXT_LEFT);
				nk_progress(gui->ctx, &progress, 100, NK_FIXED);
				//nk_popup_end(gui->ctx);
				nk_end(gui->ctx);
			}
			
			if (progr_end){
				progr_win = 0;
				progr_end = 0;
				nk_window_close(gui->ctx, "Progress");
			}
		}
		
		/* status */
		if (nk_begin(gui->ctx, "status", nk_rect(415, height - 32, 600, 32),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR))
		{
			static char comm[64];
			static int comm_len;
			
			static char text[64];
			static int text_len;
			
			nk_layout_row_dynamic(gui->ctx, 20, 4);
			
			nk_flags res = nk_edit_string(gui->ctx, NK_EDIT_SIMPLE|NK_EDIT_SIG_ENTER, comm, &comm_len, 64, nk_filter_default);
			//NK_EDIT_ACTIVE
			if (res & NK_EDIT_COMMITED){
				comm[comm_len] = 0;
				recv_comm_flag = 1;
				snprintf(recv_comm, 64, "%s", comm);
				//printf("%s\n", comm);
				comm_len = 0;
			}
			
			text_len = snprintf(text, 63, "Layers=%d", drawing->num_layers);
			nk_label(gui->ctx, text, NK_TEXT_LEFT);
			
			/*if (wait_open != 0){
				text_len = snprintf(text, 63, "Opening...");
				nk_label(gui->ctx, text, NK_TEXT_LEFT);
				nk_progress(gui->ctx, &progress, 100, NK_FIXED);
			}*/
		}
		nk_end(gui->ctx);
		
		/* view coordinates of mouse in drawing units */
		if (nk_begin(gui->ctx, "POS", nk_rect(0, height - 32, 400, 32),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR))
		{
			static char text[64];
			static int text_len;
			float ratio[] = {0.1f, 0.4f, 0.1f, 0.4f};
			nk_layout_row(gui->ctx, NK_DYNAMIC, 20, 4, ratio);
			nk_label(gui->ctx, "X=", NK_TEXT_RIGHT);
			text_len = snprintf(text, 63, "%f", pos_x);
			nk_edit_string(gui->ctx, NK_EDIT_SIMPLE, text, &text_len, 64, nk_filter_float);
			nk_label(gui->ctx, "Y=", NK_TEXT_RIGHT);
			text_len = snprintf(text, 63, "%f", pos_y);
			nk_edit_string(gui->ctx, NK_EDIT_SIMPLE, text, &text_len, 64, nk_filter_float);
		}
		nk_end(gui->ctx);
		
		/* */
		if (nk_begin(gui->ctx, "Prop", nk_rect(200, 0, 750, 32),
		NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR))
		{
			static char text[64];
			int text_len;
			nk_layout_row_begin(gui->ctx, NK_STATIC, 20, 9);
			
			/*layer*/
			nk_layout_row_push(gui->ctx, 200);
			if (nk_combo_begin_label(gui->ctx, drawing->layers[layer_idx].name, nk_vec2(200,200))){
				float wid[] = {120, 20, 20};
				nk_layout_row(gui->ctx, NK_STATIC, 20, 3, wid);
				int num_layers = drawing->num_layers;
				for (i = 0; i < num_layers; i++){
					//strcpy(layer_nam[i], drawing->layers[i].name);
					if (nk_button_label(gui->ctx, drawing->layers[i].name)){
						layer_idx = i;
						nk_combo_close(gui->ctx);
					}
					if (drawing->layers[i].off){
						if (nk_button_image_styled(gui->ctx, &b_icon_style, i_unview)){
							drawing->layers[i].off = 0;
						}
					}
					else{
						if (nk_button_image_styled(gui->ctx, &b_icon_style, i_view)){
							drawing->layers[i].off = 1;
						}
					}
					if (drawing->layers[i].lock){
						if (nk_button_image_styled(gui->ctx, &b_icon_style, i_lock)){
							drawing->layers[i].lock = 0;
						}
					}
					else{
						if (nk_button_image_styled(gui->ctx, &b_icon_style, i_unlock)){
							drawing->layers[i].lock = 1;
						}
					}
				}
				
				nk_combo_end(gui->ctx);
			}
			
			/*color picker */
			int c_idx = color_idx;
			if (c_idx >255){
				c_idx = drawing->layers[layer_idx].color;
				if (c_idx >255){
					c_idx = 0;
				}
			}
			struct nk_color combo_color = {
				.r = dxf_colors[c_idx].r,
				.g = dxf_colors[c_idx].g,
				.b = dxf_colors[c_idx].b,
				.a = dxf_colors[c_idx].a
			};
			nk_layout_row_push(gui->ctx, 40);
			struct nk_style_button b_style = gui->ctx->style.button;
			b_style.normal.data.color = combo_color;
			combo_color.a = 120;
			b_style.hover.data.color = combo_color;
			
			if (color_idx <256){
				text_len = snprintf(text, 63, "%d", color_idx);
			}
			else{
				text_len = snprintf(text, 63, "%s", "ByL");
			}
			
			nk_button_text_styled(gui->ctx, &b_style, text, text_len);
			//if(nk_button_color(gui->ctx, combo_color)){
				//
			//}
			nk_layout_row_push(gui->ctx, 25);
			if (nk_combo_begin_color(gui->ctx, combo_color, nk_vec2(215,320))) {
				nk_layout_row_static(gui->ctx, 15, 15, 10);
				for (i = 0; i < 256; i++){
					struct nk_color b_color = {
						.r = dxf_colors[i].r,
						.g = dxf_colors[i].g,
						.b = dxf_colors[i].b,
						.a = dxf_colors[i].a
					};
					if(nk_button_color(gui->ctx, b_color)){
						color_idx = i;
						nk_combo_close(gui->ctx);
					}
				}
				nk_layout_row_dynamic(gui->ctx, 20, 1);
					if (nk_button_label(gui->ctx, "By Layer")){
						color_idx = 256;
						nk_combo_close(gui->ctx);
					}
				nk_combo_end(gui->ctx);
			}
			
			/*line type*/
			nk_layout_row_push(gui->ctx, 200);
			if (nk_combo_begin_label(gui->ctx, drawing->ltypes[ltypes_idx].name, nk_vec2(300,200))){
				nk_layout_row_dynamic(gui->ctx, 25, 2);
				int num_ltypes = drawing->num_ltypes;
				
				for (i = 0; i < num_ltypes; i++){
					
					if (nk_button_label(gui->ctx, drawing->ltypes[i].name)){
						ltypes_idx = i;
						nk_combo_close(gui->ctx);
					}
					nk_label(gui->ctx, drawing->ltypes[i].descr, NK_TEXT_LEFT);
				}
				
				nk_combo_end(gui->ctx);
			}
			
			/* thickness */
			nk_layout_row_push(gui->ctx, 150);
			//nk_property_float(struct nk_context*, const char *name, float min, float *val, float max, float step, float inc_per_pixel);
			//nk_property_float(gui->ctx, "Thick:", 0.0, &thick, 20.0, 0.1, 0.1);
			
			//double nk_propertyd(struct nk_context*, const char *name, double min, double val, double max, double step, float inc_per_pixel);
			thick = nk_propertyd(gui->ctx, "Thickness", 0.0d, thick, 20.0d, 0.1d, 0.1d);
			
			
			/* zoom*/
			nk_layout_row_push(gui->ctx, 20);
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_z_plus)){
				printf("ZOOM +\n");
			}
			nk_layout_row_push(gui->ctx, 20);
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_z_minus)){
				printf("ZOOM -\n");
			}
			nk_layout_row_push(gui->ctx, 20);
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_z_win)){
				printf("ZOOM win\n");
			}
			nk_layout_row_push(gui->ctx, 20);
			if (nk_button_image_styled(gui->ctx, &b_icon_style, i_z_all)){
				printf("ZOOM all\n");
				action = VIEW_ZOOM_EXT;
			}
			
			nk_layout_row_end(gui->ctx);
			nk_end(gui->ctx);
		}
		
		
		
		if (wait_open != 0){
			low_proc = 0;
			draw = 1;
			
			open_prg = dxf_read(drawing, file_buf, file_size, &progress);
			
			if(open_prg <= 0){
				free(file_buf);
				file_buf = NULL;
				file_size = 0;
				
				dxf_ents_parse(drawing);				
				action = VIEW_ZOOM_EXT;
				layer_idx = 0;
				ltypes_idx = 0;
				color_idx = 256;
				wait_open = 0;
				progr_end = 1;
				list_clear(sel_list);
			}
			
		}
		
		
		/*===============================*/
		
		if(action == FILE_OPEN) {
			action = NONE;
			url = (char *) tinyfd_openFileDialog(
			"Open a Drawing",
			"",
			2,
			lFilterPatterns,
			NULL,
			0);
			if (url){
				dxf_mem_pool(ZERO_DXF, 0);
				graph_mem_pool(ZERO_GRAPH, 0);
				graph_mem_pool(ZERO_LINE, 0);
				
				wait_open = 1;
				progress = 0;
				
				file_buf = dxf_load_file(url, &file_size);
				open_prg = dxf_read(drawing, file_buf, file_size, &progress);
				
				low_proc = 0;
				progr_win = 1;
			}
			SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
		}
		else if(action == FILE_SAVE) {
			action = NONE;
			url = (char *) tinyfd_saveFileDialog(
			"Save Drawing",
			"save.txt",
			2,
			lFilterPatterns,
			NULL);
			if ((url != NULL) && (drawing->main_struct != NULL)){
				//dxf_ent_print_f (drawing->main_struct, url);
				dxf_save (url, drawing);
			}
		}
		else if(action == EXPORT) {
			action = NONE;
			url = (char *) tinyfd_saveFileDialog(
			"Save Drawing",
			"save.txt",
			2,
			lFilterPatterns,
			NULL);
			if ((url != NULL) && (drawing->main_struct != NULL)){
				dxf_ent_print_f (drawing->main_struct, url);
				//dxf_save (url, drawing);
			}
		}
		else if(action == VIEW_ZOOM_EXT){
			action = NONE;
			zoom_ext(drawing, img, &zoom, &ofs_x, &ofs_y);
			draw = 1;
		}
		else if(action == DELETE){
			action = NONE;
		
			if (sel_list != NULL){
				list_node *current = sel_list->next;
				
				// starts the content sweep 
				while (current != NULL){
					if (current->data){
						if (((dxf_node *)current->data)->type == DXF_ENT){ // DXF entity 
							
							// -------------------------------------------
							dxf_obj_detach((dxf_node *)current->data);
							
							//---------------------------------------
						}
					}
					current = current->next;
				}
				list_clear(sel_list);
			}
			draw = 1;
		}
		
		if(recv_comm_flag){
			recv_comm_flag =0;
			str_upp(recv_comm);
			if (strcmp(recv_comm, "SELECT") == 0){
				modal = SELECT;
			}
			else if (strcmp(recv_comm, "LINE") == 0){
				modal = LINE;
			}
			else if (strcmp(recv_comm, "POLYLINE") == 0){
				modal = POLYLINE;
			}
			else if (strcmp(recv_comm, "LINE") == 0){
				modal = LINE;
			}
			else if (strcmp(recv_comm, "DELETE") == 0){
				action = DELETE;
			}
			else if (strcmp(recv_comm, "CIRCLE") == 0){
				modal = CIRCLE;
			}
			else if (strcmp(recv_comm, "RECT") == 0){
				modal = RECT;
			}
			else if (strcmp(recv_comm, "TEXT") == 0){
				modal = TEXT;
			}
		}
		
		if (modal == SELECT){
			if (leftMouseButtonClick){
				sel_list_append(sel_list, element);
				/*-------------------------------test-------------- 
				dxf_node *current, *start, *end;
				if(dxf_find_ext_appid(element, "ZECRUEL", &start, &end)){
					printf("ext data Zecruel, start = %d, end = %d\n", start, end);
					current = start;
					while (current != NULL){
						printf ("%d = ", current->value.group); 
						
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
						//printf ("%x\n", current);
						if (current == end) break;
						current = current->next;
					}
				}*/
			}
			if (rightMouseButtonClick){
				list_clear(sel_list);
				draw = 1;
			}
			if (MouseMotion){
				rect_pt1[0] = (double) (mouse_x - 5)/zoom + ofs_x;
				rect_pt1[1] = (double) (mouse_y - 5)/zoom + ofs_y;
				rect_pt2[0] = (double) (mouse_x + 5)/zoom + ofs_x;
				rect_pt2[1] = (double) (mouse_y + 5)/zoom + ofs_y;
				
				/* for hilite test */
				element = (dxf_node *)dxf_ents_isect(drawing, rect_pt1, rect_pt2);
				draw = 1;
			}
		}
		if (modal == LINE){
			if (step == 0){
				if (leftMouseButtonClick){
					step = 1;
					x0 = (double) mouse_x/zoom + ofs_x;
					y0 = (double) mouse_y/zoom + ofs_y;
					x1 = x0;
					y1 = y0;
					draw_tmp = 1;
					/* create a new DXF line */
					new_el = (dxf_node *) dxf_new_line (
						x0, y0, 0.0, x1, y1, 0.0, /* pt1, pt2 */
						(double) thick, 0.0, /* thickness, elevation */
						color_idx, drawing->layers[layer_idx].name, /* color, layer */
						drawing->ltypes[ltypes_idx].name, 0); /* line type, paper space */
					//new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
					//x1_attr = dxf_find_attr2(new_el, 11);
					//y1_attr = dxf_find_attr2(new_el, 21);
					element = new_el;
				}
				else if (rightMouseButtonClick){
					modal = SELECT;
					draw_tmp = 0;
					element = NULL;
				}
			}
			else{
				if (leftMouseButtonClick){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					
					/*
					if(x1_attr){
						x1_attr->value.d_data = x1;
					}
					if(y1_attr){
						y1_attr->value.d_data = y1;
					}*/
					
					dxf_attr_change(new_el, 11, &x1);
					dxf_attr_change(new_el, 21, &y1);
					
					//printf("line (%.2f,%.2f)-(%.2f,%.2f)\n", x0, y0, x1, y1);
					
					//dxf_ent_print2(new_el);
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 0);
					drawing_ent_append(drawing, new_el);
					
					x0 = x1;
					y0 = y1;
					
					new_el = (dxf_node *) dxf_new_line (
						x0, y0, 0.0, x1, y1, 0.0, /* pt1, pt2 */
						(double) thick, 0.0, /* thickness, elevation */
						color_idx, drawing->layers[layer_idx].name, /* color, layer */
						drawing->ltypes[ltypes_idx].name, 0); /* line type, paper space */
					//new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
					//x1_attr = dxf_find_attr2(new_el, 11);
					//y1_attr = dxf_find_attr2(new_el, 21);
					element = new_el;
				}
				else if (rightMouseButtonClick){
					step = 0;
					draw_tmp = 0;
					element = NULL;
					draw = 1;
				}
				if (MouseMotion){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					
					
					dxf_attr_change(new_el, 6, drawing->ltypes[ltypes_idx].name);
					dxf_attr_change(new_el, 8, drawing->layers[layer_idx].name);
					dxf_attr_change(new_el, 11, &x1);
					dxf_attr_change(new_el, 21, &y1);
					dxf_attr_change(new_el, 39, &thick);
					dxf_attr_change(new_el, 62, &color_idx);
					
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
				}
			}
		}
		if (modal == POLYLINE){
			if (step == 0){
				if (leftMouseButtonClick){
					step = 1;
					x0 = (double) mouse_x/zoom + ofs_x;
					y0 = (double) mouse_y/zoom + ofs_y;
					x1 = x0;
					y1 = y0;
					draw_tmp = 1;
					/* create a new DXF lwpolyline */
					new_el = (dxf_node *) dxf_new_lwpolyline (
						x0, y0, 0.0, /* pt1, */
						bulge, (double) thick, /* bulge, thickness, */
						color_idx, drawing->layers[layer_idx].name, /* color, layer */
						drawing->ltypes[ltypes_idx].name, 0); /* line type, paper space */
					//new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
					//x1_attr = dxf_find_attr2(new_el, 11);
					//y1_attr = dxf_find_attr2(new_el, 21);
					dxf_lwpoly_append (new_el, x1, y1, 0.0, bulge);
					element = new_el;
					
				}
				else if (rightMouseButtonClick){
					modal = SELECT;
					draw_tmp = 0;
					element = NULL;
				}
			}
			else{
				if (leftMouseButtonClick){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					step = 2;
					
					/*
					if(x1_attr){
						x1_attr->value.d_data = x1;
					}
					if(y1_attr){
						y1_attr->value.d_data = y1;
					}*/
					
					dxf_attr_change_i(new_el, 10, &x1, -1);
					dxf_attr_change_i(new_el, 20, &y1, -1);
					dxf_attr_change_i(new_el, 42, &bulge, -1);
					
					//printf("line (%.2f,%.2f)-(%.2f,%.2f)\n", x0, y0, x1, y1);
					
					//dxf_ent_print2(new_el);
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
					
					
					x0 = x1;
					y0 = y1;
					
					dxf_lwpoly_append (new_el, x1, y1, 0.0, bulge);
				}
				else if (rightMouseButtonClick){
					//step = 0;
					draw_tmp = 0;
					if (step == 2){
						dxf_lwpoly_remove (new_el, -1);
						new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 0);
						drawing_ent_append(drawing, new_el);
						step = 0;
					}
					element = NULL;
					draw = 1;
				}
				if (MouseMotion){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					
					
					dxf_attr_change(new_el, 6, drawing->ltypes[ltypes_idx].name);
					dxf_attr_change(new_el, 8, drawing->layers[layer_idx].name);
					dxf_attr_change_i(new_el, 10, &x1, -1);
					dxf_attr_change_i(new_el, 20, &y1, -1);
					dxf_attr_change_i(new_el, 42, &bulge, -1);
					dxf_attr_change(new_el, 39, &thick);
					dxf_attr_change(new_el, 62, &color_idx);
					
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
				}
			}
		}
		if (modal == CIRCLE){
			if (step == 0){
				if (leftMouseButtonClick){
					step = 1;
					x0 = (double) mouse_x/zoom + ofs_x;
					y0 = (double) mouse_y/zoom + ofs_y;
					x1 = x0;
					y1 = y0;
					draw_tmp = 1;
					/* create a new DXF circle */
					new_el = (dxf_node *) dxf_new_circle (
						x0, y0, 0.0, 0.0, /* pt1, radius */
						(double) thick, /* thickness, elevation */
						color_idx, drawing->layers[layer_idx].name, /* color, layer */
						drawing->ltypes[ltypes_idx].name, 0); /* line type, paper space */
					element = new_el;
				}
				else if (rightMouseButtonClick){
					modal = SELECT;
					draw_tmp = 0;
					element = NULL;
				}
			}
			else{
				if (leftMouseButtonClick){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					double radius = sqrt(pow((x1 - x0), 2) + pow((y1 - y0), 2));
					
					dxf_attr_change(new_el, 40, &radius);
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 0);
					drawing_ent_append(drawing, new_el);
					
					step = 0;
					draw_tmp = 0;
					element = NULL;
					draw = 1;
				}
				else if (rightMouseButtonClick){
					step = 0;
					draw_tmp = 0;
					element = NULL;
					draw = 1;
				}
				if (MouseMotion){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					double radius = sqrt(pow((x1 - x0), 2) + pow((y1 - y0), 2));
					
					dxf_attr_change(new_el, 40, &radius);
					dxf_attr_change(new_el, 6, drawing->ltypes[ltypes_idx].name);
					dxf_attr_change(new_el, 8, drawing->layers[layer_idx].name);
					dxf_attr_change(new_el, 39, &thick);
					dxf_attr_change(new_el, 62, &color_idx);
					
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
				}
			}
		}
		if (modal == RECT){
			if (step == 0){
				if (leftMouseButtonClick){
					step = 1;
					x0 = (double) mouse_x/zoom + ofs_x;
					y0 = (double) mouse_y/zoom + ofs_y;
					x1 = x0;
					y1 = y0;
					draw_tmp = 1;
					int closed = 1;
					/* create a new DXF lwpolyline */
					new_el = (dxf_node *) dxf_new_lwpolyline (
						x0, y0, 0.0, /* pt1, */
						0.0, (double) thick, /* bulge, thickness, */
						color_idx, drawing->layers[layer_idx].name, /* color, layer */
						drawing->ltypes[ltypes_idx].name, 0); /* line type, paper space */
					dxf_lwpoly_append (new_el, x1, y1, 0.0, 0.0);
					dxf_lwpoly_append (new_el, x1, y1, 0.0, 0.0);
					dxf_lwpoly_append (new_el, x1, y1, 0.0, 0.0);
					dxf_attr_change_i(new_el, 70, (void *) &closed, 0);
					element = new_el;
				}
				else if (rightMouseButtonClick){
					modal = SELECT;
					draw_tmp = 0;
					element = NULL;
				}
			}
			else{
				if (leftMouseButtonClick){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					
					dxf_attr_change_i(new_el, 10, (void *) &x1, 1);
					dxf_attr_change_i(new_el, 10, (void *) &x1, 2);
					dxf_attr_change_i(new_el, 20, (void *) &y1, 2);
					dxf_attr_change_i(new_el, 20, (void *) &y1, 3);
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 0);
					drawing_ent_append(drawing, new_el);
					
					step = 0;
					draw_tmp = 0;
					element = NULL;
					draw = 1;
				}
				else if (rightMouseButtonClick){
					step = 0;
					draw_tmp = 0;
					element = NULL;
					draw = 1;
				}
				if (MouseMotion){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					dxf_attr_change_i(new_el, 10, (void *) &x1, 1);
					dxf_attr_change_i(new_el, 10, (void *) &x1, 2);
					dxf_attr_change_i(new_el, 20, (void *) &y1, 2);
					dxf_attr_change_i(new_el, 20, (void *) &y1, 3);
					dxf_attr_change(new_el, 6, drawing->ltypes[ltypes_idx].name);
					dxf_attr_change(new_el, 8, drawing->layers[layer_idx].name);
					dxf_attr_change(new_el, 39, &thick);
					dxf_attr_change(new_el, 62, &color_idx);
					
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
				}
			}
		}
		if (modal == TEXT){
			if (step == 0){
				step = 1;
				x0 = (double) mouse_x/zoom + ofs_x;
				y0 = (double) mouse_y/zoom + ofs_y;
				x1 = x0;
				y1 = y0;
				draw_tmp = 1;
				/* create a new DXF text */
				//dxf_new_text (double x0, double y0, double z0, double h, char *txt, double thick, int color, char *layer, char *ltype, int paper)
				new_el = (dxf_node *) dxf_new_text (
					x0, y0, 0.0, txt_h, /* pt1, height */
					txt, (double) thick, /* text, thickness */
					color_idx, drawing->layers[layer_idx].name, /* color, layer */
					drawing->ltypes[ltypes_idx].name, 0); /* line type, paper space */
				element = new_el;
				
				
			}
			else{
				if (leftMouseButtonClick){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					
					dxf_attr_change_i(new_el, 10, &x1, -1);
					dxf_attr_change_i(new_el, 20, &y1, -1);
					dxf_attr_change(new_el, 40, &txt_h);
					dxf_attr_change(new_el, 1, txt);
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 0);
					drawing_ent_append(drawing, new_el);
					
					step = 0;
					draw_tmp = 0;
					element = NULL;
					draw = 1;
				}
				else if (rightMouseButtonClick){
					step = 0;
					modal = SELECT;
					draw_tmp = 0;
					element = NULL;
				}
				if (MouseMotion){
					x1 = (double) mouse_x/zoom + ofs_x;
					y1 = (double) mouse_y/zoom + ofs_y;
					dxf_attr_change_i(new_el, 10, &x1, -1);
					dxf_attr_change_i(new_el, 20, &y1, -1);
					dxf_attr_change(new_el, 40, &txt_h);
					dxf_attr_change(new_el, 1, txt);
					dxf_attr_change(new_el, 6, drawing->ltypes[ltypes_idx].name);
					dxf_attr_change(new_el, 8, drawing->layers[layer_idx].name);
					dxf_attr_change(new_el, 39, &thick);
					dxf_attr_change(new_el, 62, &color_idx);
					
					new_el->obj.graphics = dxf_graph_parse(drawing, new_el, 0 , 1);
				}
				
			}
		}
		
		if (gui_check_draw(gui) != 0){
			draw = 1;
		}
		
		if (draw != 0){
		
			bmp_fill(img, img->bkg); /* clear bitmap */
			dxf_ents_draw(drawing, img, ofs_x, ofs_y, zoom); /* redraw */
			
			draw_cursor(img, mouse_x, mouse_y, cursor);
			
			
			
			/*hilite test */
			if(draw_tmp){
				element->obj.graphics = dxf_graph_parse(drawing, element, 0 , 1);
			}
			if(element != NULL){
				vec_graph_draw_fix(element->obj.graphics, img, ofs_x, ofs_y, zoom, hilite);
			}
			dxf_list_draw(sel_list, img, ofs_x, ofs_y, zoom, hilite);
			
			nk_sdl_render(gui, img);
			
			SDL_UpdateTexture(canvas, NULL, img->buf, width * 4);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, canvas, NULL, NULL);
			SDL_RenderPresent(renderer);
			
			SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
			draw = 0;
		}
		
		leftMouseButtonClick = 0;
		rightMouseButtonClick = 0;
		MouseMotion = 0;
		
		//graph_mem_pool(ZERO_GRAPH, 2);
		//graph_mem_pool(ZERO_LINE, 2);
		
		graph_mem_pool(ZERO_GRAPH, 1);
		graph_mem_pool(ZERO_LINE, 1);
		nk_clear(gui->ctx); /*IMPORTANT */
		if (low_proc){
			SDL_Delay(60);
		}
	}
	
	/* safe quit */
	SDL_DestroyTexture(canvas);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	list_mem_pool(FREE_LIST, 0);
	dxf_mem_pool(FREE_DXF, 0);
	graph_mem_pool(FREE_ALL, 0);
	graph_mem_pool(FREE_ALL, 1);
	graph_mem_pool(FREE_ALL, 2);
	
	bmp_free(img);
	bmp_free(tool_img);
	for(i = 0; i < 40; i++){
		bmp_free(tool_vec[i]);
	}
	for (i = 0; i<drawing->num_fonts; i++){
		shx_font_free(drawing->text_fonts[i].shx_font);
	}
	nk_sdl_shutdown(gui);
	shx_font_free(shx_font);
	free(drawing);
		
	return 0;
}