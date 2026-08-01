#ifndef CNC_UI_H
#define CNC_UI_H

#include "lvgl/lvgl.h"

/*==============================================================
 * CNC User Interface - Public API
 *
 * This module provides a complete professional UI for CNC
 * drawing and job management, built on top of LVGL.
 *==============================================================*/

/**
 * cnc_create_ui() - Initialize and build the entire CNC drawing interface
 * @parent: Parent LVGL object (typically the screen)
 *
 * Creates:
 *  • Top toolbar with drawing tools
 *  • Main canvas for drawing
 *  • Right panel with job information
 *  • Bottom status bar
 */
void cnc_create_ui(lv_obj_t *parent);

#endif /* CNC_UI_H */
