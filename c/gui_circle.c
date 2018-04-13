#include "gui_use.h"

int gui_circle_interactive(gui_obj *gui){
	
	if (gui->modal == CIRCLE){
		static dxf_node *new_el;
		if (gui->step == 0){
			if (gui->ev & EV_ENTER){
				gui->draw_tmp = 1;
				/* create a new DXF circle */
				new_el = (dxf_node *) dxf_new_circle (
					gui->step_x[gui->step], gui->step_y[gui->step], 0.0, 0.0, /* pt1, radius */
					gui->color_idx, gui->drawing->layers[gui->layer_idx].name, /* color, layer */
					gui->drawing->ltypes[gui->ltypes_idx].name, dxf_lw[gui->lw_idx], /* line type, line weight */
					0); /* paper space */
				gui->element = new_el;
				gui->step = 1;
				gui->en_distance = 1;
				goto next_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto default_modal;
			}
		}
		else{
			if (gui->ev & EV_ENTER){
				double radius = sqrt(pow((gui->step_x[gui->step] - gui->step_x[gui->step - 1]), 2) + pow((gui->step_y[gui->step] - gui->step_y[gui->step - 1]), 2));
				dxf_attr_change(new_el, 40, &radius);
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 0);
				drawing_ent_append(gui->drawing, new_el);
				
				do_add_entry(&gui->list_do, "CIRCLE");
				do_add_item(gui->list_do.current, NULL, new_el);
				
				goto first_step;
			}
			else if (gui->ev & EV_CANCEL){
				goto first_step;
			}
			if (gui->ev & EV_MOTION){
				double x1 = (double) gui->mouse_x/gui->zoom + gui->ofs_x;
				double y1 = (double) gui->mouse_y/gui->zoom + gui->ofs_y;
				double radius = sqrt(pow((gui->step_x[gui->step] - gui->step_x[gui->step - 1]), 2) + pow((gui->step_y[gui->step] - gui->step_y[gui->step - 1]), 2));
				
				dxf_attr_change(new_el, 40, &radius);
				dxf_attr_change(new_el, 6, gui->drawing->ltypes[gui->ltypes_idx].name);
				dxf_attr_change(new_el, 8, gui->drawing->layers[gui->layer_idx].name);
				dxf_attr_change(new_el, 370, &dxf_lw[gui->lw_idx]);
				dxf_attr_change(new_el, 62, &gui->color_idx);
				
				new_el->obj.graphics = dxf_graph_parse(gui->drawing, new_el, 0 , 1);
			}
		}
	}
	goto end_step;
	default_modal:
		gui->modal = SELECT;
	first_step:
		gui->en_distance = 0;
		gui->draw_tmp = 0;
		gui->element = NULL;
		gui->draw = 1;
		gui->step = 0;
		gui->draw_phanton = 0;
		//if (gui->phanton){
		//	gui->phanton = NULL;
		//}
	next_step:
		
		gui->lock_ax_x = 0;
		gui->lock_ax_y = 0;
		gui->user_flag_x = 0;
		gui->user_flag_y = 0;

		gui->draw = 1;
	end_step:
		return 1;
}

int gui_circle_info (gui_obj *gui){
	if (gui->modal == CIRCLE) {
		nk_layout_row_dynamic(gui->ctx, 20, 1);
		nk_label(gui->ctx, "Place a circle", NK_TEXT_LEFT);
		if (gui->step == 0){
			nk_label(gui->ctx, "Enter center point", NK_TEXT_LEFT);
		} else {
			nk_label(gui->ctx, "Enter end point", NK_TEXT_LEFT);
		}
	}
	return 1;
}