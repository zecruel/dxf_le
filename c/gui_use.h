#ifndef _CZ_GUI_USE_LIB
#define _CZ_GUI_USE_LIB

#include "gui.h"

int gui_select_interactive(gui_obj *gui);

int gui_select_info (gui_obj *gui);

int gui_line_interactive(gui_obj *gui);

int gui_line_info (gui_obj *gui);

int gui_pline_interactive(gui_obj *gui);

int gui_pline_info (gui_obj *gui);

int gui_circle_interactive(gui_obj *gui);

int gui_circle_info (gui_obj *gui);

int gui_rect_interactive(gui_obj *gui);

int gui_rect_info (gui_obj *gui);

int gui_text_interactive(gui_obj *gui);

int gui_text_info (gui_obj *gui);

int gui_move_interactive(gui_obj *gui);

int gui_move_info (gui_obj *gui);

int gui_dupli_interactive(gui_obj *gui);

int gui_dupli_info (gui_obj *gui);

int gui_scale_interactive(gui_obj *gui);

int gui_scale_info (gui_obj *gui);


int gui_insert_interactive(gui_obj *gui);

int gui_insert_info (gui_obj *gui);

int gui_block_interactive(gui_obj *gui);

int gui_block_info (gui_obj *gui);

#endif