// #ifndef VIEWER_DRAW_H
// #define VIEWER_DRAW_H

/*==============================================================
 * src/include/viewer_draw.h
 * LV_EVENT_DRAW_MAIN callback declarations.
 * Implemented in src/ui/draw.c
 *==============================================================*/

// #include "viewer.h"

// void canvas_draw_cb(lv_event_t *e);
// void h_ruler_draw_cb(lv_event_t *e);
// void v_ruler_draw_cb(lv_event_t *e);
// void mag_draw_cb(lv_event_t *e);

// #endif /* VIEWER_DRAW_H */


/*==============================================================
 * src/include/viewer_draw.h
 *
 * PURPOSE:
 * This header declares custom drawing callback functions
 * used by LVGL during the rendering phase.
 *
 * These functions are triggered using LV_EVENT_DRAW_MAIN,
 * meaning they are responsible for drawing custom graphics
 * on widgets (like canvas, rulers, magnifier, etc.).
 *
 * IMPLEMENTATION:
 * Actual drawing logic is written in:
 *   src/ui/draw.c
 *==============================================================*/

#ifndef VIEWER_DRAW_H
#define VIEWER_DRAW_H

#include "viewer.h"   // Includes global variables, colors, macros, etc.

/*--------------------------------------------------------------
 * canvas_draw_cb()
 *
 * PURPOSE:
 * Draws the main image viewing area (canvas).
 *
 * WHAT IT HANDLES:
 *  - Image rendering
 *  - Zoom (scale)
 *  - Pan (offset)
 *  - Crosshair / cursor
 *  - Overlays (if any)
 *
 * PARAMETER:
 *  e → LVGL event object
 *      Used to access:
 *        - target object (canvas)
 *        - drawing layer
 *        - drawing area
 *--------------------------------------------------------------*/
void canvas_draw_cb(lv_event_t *e);


/*--------------------------------------------------------------
 * h_ruler_draw_cb()
 *
 * PURPOSE:
 * Draws the horizontal ruler (top side).
 *
 * WHAT IT HANDLES:
 *  - Scale markings (ticks)
 *  - Measurement numbers (X-axis)
 *  - Adjusts based on zoom level
 *
 * USE CASE:
 * Helps user measure width / horizontal position
 *--------------------------------------------------------------*/
void h_ruler_draw_cb(lv_event_t *e);


/*--------------------------------------------------------------
 * v_ruler_draw_cb()
 *
 * PURPOSE:
 * Draws the vertical ruler (left side).
 *
 * WHAT IT HANDLES:
 *  - Vertical scale markings
 *  - Y-axis measurement values
 *  - Updates based on zoom
 *
 * USE CASE:
 * Helps user measure height / vertical position
 *--------------------------------------------------------------*/
void v_ruler_draw_cb(lv_event_t *e);


/*--------------------------------------------------------------
 * mag_draw_cb()
 *
 * PURPOSE:
 * Draws the magnifier (zoomed-in preview).
 *
 * WHAT IT HANDLES:
 *  - Small zoom window
 *  - Shows detailed view near cursor
 *  - Used for precision inspection
 *
 * USE CASE:
 * Your project → "zoom up to 10x to see cutting details"
 *--------------------------------------------------------------*/
void mag_draw_cb(lv_event_t *e);


#endif /* VIEWER_DRAW_H */