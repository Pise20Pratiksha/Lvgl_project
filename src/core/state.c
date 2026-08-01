/*==============================================================
 * src/core/state.c
 *
 * Defines:
 *   1. The single global ViewerState instance (declared in
 *      viewer.h as:  extern ViewerState g;)
 *
 *   2. The screen / viewport dimension variables (declared in
 *      viewer.h as:  extern int32_t SCR_W; etc.)
 *      Without these definitions the linker produces:
 *        "undefined reference to SCR_W / SCR_H / VP_X ..."
 *      because every other .c file only sees the extern
 *      declaration from the header — no storage is allocated
 *      anywhere.
 *
 * Rule of thumb:
 *   viewer.h  →  extern int32_t SCR_W;   (declaration, no storage)
 *   THIS file →  int32_t SCR_W = 0;      (definition,  with storage)
 *==============================================================*/

#include "viewer.h"

/* ── Single global viewer state instance ── */
/* Zero-initialised at program start by the C runtime.        */
ViewerState g = {
    .fill_color = 0x38BDF8,  /* Default cyan color for fill */
    .fill_tolerance = 8,
};

/* ── Screen dimensions ── */
/* Filled in by build_ui() after the display driver reports   */
/* the real framebuffer resolution via:                        */
/*   SCR_W = lv_display_get_horizontal_resolution(disp);      */
/*   SCR_H = lv_display_get_vertical_resolution(disp);        */
int32_t SCR_W = 0;
int32_t SCR_H = 0;

/* ── Viewport geometry ── */
/* Computed by build_ui() once SCR_W / SCR_H are known:       */
/*   VP_X = RULER_SZ;                                         */
/*   VP_Y = TOOLBAR_H + RULER_SZ;                             */
/*   VP_W = SCR_W - RULER_SZ - RPANEL_W;                     */
/*   VP_H = SCR_H - TOOLBAR_H - RULER_SZ;                    */
int32_t VP_X  = 0;   /* left edge of the canvas viewport      */
int32_t VP_Y  = 0;   /* top  edge of the canvas viewport      */
int32_t VP_W  = 0;   /* width  of the canvas viewport         */
int32_t VP_H  = 0;   /* height of the canvas viewport         */