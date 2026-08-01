// #ifndef VIEWER_INPUT_H
// #define VIEWER_INPUT_H

// /*==============================================================
//  * src/include/viewer_input.h
//  * Touch, scroll, and toolbar event callback declarations.
//  * Implemented in src/input/events.c
//  *==============================================================*/

// #include "viewer.h"

// // Handles touch interactions like pan, tap, and cursor movement on the canvas
// void canvas_event_cb(lv_event_t *e);
// // Handles zooming using scroll or gesture input
// void scroll_zoom_cb(lv_event_t *e);

// // Switches interaction mode to pan (move image)
// void cb_pan    (lv_event_t *e);
// // Enables crop mode for selecting a region of the image
// void cb_crop   (lv_event_t *e);
// // Increases zoom level (zoom in)
// void cb_zin    (lv_event_t *e);
// // Decreases zoom level (zoom out)
// void cb_zout   (lv_event_t *e);
// // Resets zoom, pan, and cursor to default state
// void cb_reset  (lv_event_t *e);
// // Fits the image to the screen automatically
// void cb_fit    (lv_event_t *e);
// // Sets zoom level directly to 10x for detailed view
// void cb_zoom10x(lv_event_t *e);
// // Clears crop selection or any overlays
// void cb_clear  (lv_event_t *e);

// #endif /* VIEWER_INPUT_H */


#ifndef VIEWER_INPUT_H
#define VIEWER_INPUT_H

#include "viewer.h"

/* Canvas & scroll handlers */
void canvas_event_cb(lv_event_t *e);
void scroll_zoom_cb(lv_event_t *e);

/* Top toolbar callbacks */
void cb_pan     (lv_event_t *e);
void cb_crop    (lv_event_t *e);
void cb_zin     (lv_event_t *e);
void cb_zout    (lv_event_t *e);
void cb_reset   (lv_event_t *e);
void cb_fit     (lv_event_t *e);
void cb_zoom10x (lv_event_t *e);
void cb_clear   (lv_event_t *e);
void cb_settings(lv_event_t *e);      

/* Bottom toolbar tool callbacks */
void cb_tool_line (lv_event_t *e);
void cb_tool_arc  (lv_event_t *e);
void cb_tool_box  (lv_event_t *e);
void cb_tool_edit (lv_event_t *e);
void cb_tool_fill (lv_event_t *e);
void cb_tool_check(lv_event_t *e);
void cb_tool_undo (lv_event_t *e);
void cb_tool_redo (lv_event_t *e);

void cb_tool_pattern_fill(lv_event_t *e);

void cb_draw_object_menu(lv_event_t *e);
void cb_tool_bar    (lv_event_t *e);
void cb_tool_pipe   (lv_event_t *e);
void cb_tool_forged (lv_event_t *e);

void show_bar_stock_dialog(void);

void cb_draw_object_menu(lv_event_t *e);   /* opens/closes the popup              */
void cb_submode_line    (lv_event_t *e);   /* picks Straight Line  -> MODE_LINE   */
void cb_submode_arc     (lv_event_t *e);   /* picks Curve Line     -> MODE_ARC    */
void cb_submode_box     (lv_event_t *e);   /* picks Box Creation   -> MODE_BOX    */
void cb_submode_bar     (lv_event_t *e);   /* picks Bar            -> MODE_BAR    */
void cb_submode_pipe    (lv_event_t *e);   /* picks Pipes          -> MODE_PIPE   */
void cb_submode_forged  (lv_event_t *e);   /* picks Forged         -> MODE_FORGED */


#endif /* VIEWER_INPUT_H */