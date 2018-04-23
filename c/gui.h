
#ifndef _CZ_GUI_LIB
#define _CZ_GUI_LIB

#include "dxf.h"
#include "bmp.h"
#include "graph.h"
#include "shape2.h"
#include "i_svg_media.h"
#include "list.h"
#include "dxf_create.h"
#include "dxf_attract.h"

#include <SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#include "nuklear.h"

#include "nanosvg.h"
#include "nanosvgrast.h"

#define FONT_SCALE 1.4
#define FIXED_MEM 128*1024

enum Action {
	NONE,
	FILE_OPEN,
	FILE_SAVE,
	EXPORT,
	VIEW_ZOOM_EXT,
	VIEW_ZOOM_P,
	VIEW_ZOOM_M,
	VIEW_ZOOM_W,
	VIEW_PAN_U,
	VIEW_PAN_D,
	VIEW_PAN_L,
	VIEW_PAN_R,
	DELETE,
	UNDO,
	REDO,
	LAYER_CHANGE,
	COLOR_CHANGE,
	LTYPE_CHANGE,
	LW_CHANGE,
	EXIT
};

enum Modal {
	SELECT,
	LINE,
	POLYLINE,
	CIRCLE,
	RECT,
	TEXT,
	ARC,
	DUPLI,
	MOVE,
	SCALE,
	NEW_BLK,
	INSERT
};

enum Gui_ev {
	EV_NONE = 0,
	EV_ENTER = 1,
	EV_CANCEL = 2,
	EV_MOTION = 4,
	EV_LOCK_AX = 8
};
	
	

struct sort_by_idx{
	int idx;
	void *data;
};

struct Gui_obj {
	struct nk_context *ctx;
	struct nk_user_font *font;
	void *buf; /*for fixed memory */
	void *last; /* to verify if needs to draw */
	
	dxf_drawing *drawing;
	dxf_node *element, *near_el;
	
	/* background image dimension */
	unsigned int main_w;
	unsigned int main_h;
	
	/* Window dimension */
	unsigned int win_w;
	unsigned int win_h;
	
	/*gui pos variables */
	int next_win_x, next_win_y, next_win_w, next_win_h;
	int mouse_x, mouse_y;
	double zoom, ofs_x, ofs_y;
	double prev_zoom;
	
	double user_x, user_y;
	double step_x[10], step_y[10];
	double near_x, near_y;
	double bulge, scale;
	double txt_h;
	
	int color_idx, lw_idx, t_al_v, t_al_h;
	int layer_idx, ltypes_idx;
	
	int step, user_flag_x, user_flag_y, lock_ax_x, lock_ax_y, user_number;
	int keyEnter;
	int draw, draw_tmp, draw_phanton;
	int near_attr;
	
	
	int en_distance; /* enable distance entry */
	int entry_relative;
	
	enum Action action;
	enum Modal modal, prev_modal;
	enum Gui_ev ev;
	enum attract_type curr_attr_t;
	
	bmp_color background;
	
	NSVGimage **svg_curves;
	bmp_img **svg_bmp;
	bmp_img *preview_img;
	
	struct nk_style_button b_icon;
	
	/* style for toggle buttons (or select buttons) with image */
	struct nk_style_button b_icon_sel, b_icon_unsel;
	
	char log_msg[64];
	char txt[DXF_MAX_CHARS];
	char blk_name[DXF_MAX_CHARS];
	
	list_node * sel_list;
	list_node *phanton;
	struct do_list list_do;
	
	
	
};
typedef struct Gui_obj gui_obj;

struct font_obj{
	shape *shx_font;
	double scale;
};

enum theme {THEME_BLACK, THEME_WHITE, THEME_RED, THEME_BLUE, THEME_DARK, THEME_ZE};

void set_style(struct nk_context *ctx, enum theme theme);

float nk_user_font_get_text_width(nk_handle handle, float height, const char *text, int len);

bmp_color nk_to_bmp_color(struct nk_color color);

int gui_check_draw(gui_obj *gui);

NK_API void nk_sdl_render(gui_obj *gui, bmp_img *img);

static void nk_sdl_clipbard_paste(nk_handle usr, struct nk_text_edit *edit);

static void nk_sdl_clipbard_copy(nk_handle usr, const char *text, int len);

NK_API int nk_sdl_init(gui_obj* gui, struct nk_user_font *font);

NK_API int nk_sdl_handle_event(gui_obj *gui, SDL_Window *win, SDL_Event *evt);

NK_API void nk_sdl_shutdown(gui_obj *gui);

extern int dxf_lw[];
extern const char *dxf_lw_descr[];
extern bmp_color dxf_colors[];
extern const char *text_al_h[];
extern const char *text_al_v[];

#ifndef DXF_LW_LEN
	#define DXF_LW_LEN 24
#endif

#ifndef T_AL_H_LEN	
	#define T_AL_H_LEN 6
#endif

#ifndef T_AL_V_LEN
	#define T_AL_V_LEN 4
#endif

#endif
