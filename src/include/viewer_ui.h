// #ifndef VIEWER_UI_H
// #define VIEWER_UI_H

// #include "viewer.h"

// /* Screen-level builders */
// void build_toolbar(lv_obj_t *scr);
// void build_bottom_toolbar(lv_obj_t *scr);        // ← Must be here
// void build_canvas_area(lv_obj_t *scr);
// void build_right_panel(lv_obj_t *scr);

// /* Runtime updates */
// void update_panel(void);
// void refresh_all(void);
// void set_mode(ViewMode m);

// /* Widget helpers */
// lv_obj_t *mk_plain(lv_obj_t *par,
//                     int32_t x, int32_t y,
//                     int32_t w, int32_t h,
//                     uint32_t col);

// lv_obj_t *mk_lbl(lv_obj_t *par, const char *txt,
//                   uint32_t col, const lv_font_t *fnt,
//                   int32_t x, int32_t y);

// lv_obj_t *mk_btn(lv_obj_t *par, const char *label,
//                   lv_event_cb_t cb, int32_t x, int32_t w);

// // Add this line:
// lv_obj_t *mk_tool_btn(lv_obj_t *par, const char *symbol, const char *label,
//                       lv_event_cb_t cb, int32_t x, int32_t w);

// #endif /* VIEWER_UI_H */


#ifndef VIEWER_UI_H
#define VIEWER_UI_H

#include "viewer.h"

/* ── Screen-level builders ───────────────────────────────── */
void build_toolbar(lv_obj_t *scr);
void build_bottom_toolbar(lv_obj_t *scr);
void build_canvas_area(lv_obj_t *scr);
void build_right_panel(lv_obj_t *scr);

void show_pattern_fill_menu(void);

void forged_resync_object(DrawObject *o);

/* ── Runtime updates ─────────────────────────────────────── */
void update_panel(void);
void refresh_all(void);
void set_mode(ViewMode m);

void cb_draw_object_menu(lv_event_t *e);
void cb_tool_bar    (lv_event_t *e);
void cb_tool_pipe   (lv_event_t *e);
void cb_tool_forged (lv_event_t *e);
void cb_tool_hide   (lv_event_t *e);

void show_bar_stock_dialog(void);
void show_forged_segment_dialog(void);

/* Palette */
void show_color_palette(void);

/* Show settings modal (Save / Export) */
void show_settings_modal(void);
/* ── Drawing operations  ← THIS BLOCK WAS MISSING ───────── */
void do_undo(void);
void do_redo(void);
void validate_drawing(void);

/* ── Low-level widget helpers ────────────────────────────── */
lv_obj_t *mk_plain(lv_obj_t *par,
                    int32_t x, int32_t y,
                    int32_t w, int32_t h,
                    uint32_t col);

lv_obj_t *mk_lbl(lv_obj_t *par, const char *txt,
                  uint32_t col, const lv_font_t *fnt,
                  int32_t x, int32_t y);

lv_obj_t *mk_btn(lv_obj_t *par, const char *label,
                  lv_event_cb_t cb, int32_t x, int32_t w);

lv_obj_t *mk_tool_btn(lv_obj_t *par, const char *symbol,
                       const char *label, lv_event_cb_t cb,
                       int32_t x, int32_t w);

#endif /* VIEWER_UI_H */