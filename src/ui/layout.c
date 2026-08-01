// /*==============================================================
//  * src/ui/layout.c
//  * One-time screen layout builders.
//  *
//  * FIXES vs broken version:
//  *  [1] Added #include "viewer_draw.h"  — canvas_draw_cb etc. are
//  *      registered here; the compiler must see their declarations to
//  *      verify the lv_event_cb_t signature.
//  *  [2] Added #include "viewer_input.h" — canvas_event_cb and
//  *      scroll_zoom_cb are registered here; same reason as [1].
//  *      Without these includes the function pointers are implicitly
//  *      cast from an unknown type, which on some ARM ABIs causes the
//  *      callbacks to be called with a corrupted stack frame → silent
//  *      crash → black screen.
//  *==============================================================*/

// #include "viewer.h"
// #include "viewer_math.h"
// #include "viewer_ui.h"
// #include "viewer_draw.h"    
// #include "viewer_input.h"   


// /* =========================================================
//  * TOOLBAR
//  * ========================================================= */
// void build_toolbar(lv_obj_t *scr)
// {
//     lv_obj_t *bar = mk_plain(scr, 0, 0, SCR_W, TOOLBAR_H, C_TOOLBAR);

//     lv_obj_t *sep = mk_plain(bar, 0, TOOLBAR_H - 1, SCR_W, 1, C_BORDER);
//     (void)sep;

//     typedef struct {
//         const char    *lbl;
//         lv_event_cb_t  cb;
//         int32_t        w;
//         int            is_pan;
//         int            is_crop;
//     } BtnDef;

//     const BtnDef defs[] = {
//         {"Pan",       cb_pan,      48, 1, 0},
//         {"Crop",      cb_crop,     48, 0, 1},
//         {"+Zoom In",  cb_zin,      76, 0, 0},
//         {"-Zoom Out", cb_zout,     84, 0, 0},
//         {"Reset",     cb_reset,    58, 0, 0},
//         {"Fit",       cb_fit,      44, 0, 0},
//         {"10x",       cb_zoom10x,  44, 0, 0},
//         {"Clear",     cb_clear,    50, 0, 0},
//     };
//     int n = (int)(sizeof(defs) / sizeof(defs[0]));

//     int32_t cx = 3;
//     for(int i = 0; i < n; i++) {
//         lv_obj_t *btn = mk_btn(bar, defs[i].lbl, defs[i].cb, cx, defs[i].w);
//         if(defs[i].is_pan)  g.btn_pan  = btn;
//         if(defs[i].is_crop) g.btn_crop = btn;
//         cx += defs[i].w + 3;
//     }

//     g.hint_lbl = mk_lbl(bar, "Scroll/Swipe=Zoom  Drag=Pan",
//                          C_DIM, FONT_SM, 0, 0);
//     lv_obj_align(g.hint_lbl, LV_ALIGN_RIGHT_MID, -6, 0);
// }

// /* =========================================================
//  * CANVAS + RULERS
//  * ========================================================= */
// void build_canvas_area(lv_obj_t *scr)
// {
//     /* Corner fill at ruler intersection */
//     lv_obj_t *corner = mk_plain(scr, 0, TOOLBAR_H, RULER_SZ, RULER_SZ, C_RULER);
//     lv_obj_set_style_border_color(corner, H(C_BORDER), 0);
//     lv_obj_set_style_border_width(corner, 1, 0);

//     lv_obj_set_style_bg_color(corner, lv_color_hex(0xC87941), 0);
//     lv_obj_set_style_bg_opa(corner, LV_OPA_COVER, 0);
//     lv_obj_t *corner_lbl = lv_label_create(corner);
//     lv_label_set_text(corner_lbl, "mm");
//     lv_obj_set_style_text_color(corner_lbl, lv_color_hex(0x000000), 0);
//     lv_obj_set_style_text_font(corner_lbl, FONT_SM, 0);
//     lv_obj_center(corner_lbl);

//     /* Horizontal ruler */
//     lv_obj_t *hr = lv_obj_create(scr);
//     lv_obj_set_pos(hr, VP_X, TOOLBAR_H);
//     lv_obj_set_size(hr, VP_W, RULER_SZ);
//     lv_obj_set_style_bg_opa(hr, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(hr, 0, 0);
//     lv_obj_set_style_pad_all(hr, 0, 0);
//     lv_obj_set_style_radius(hr, 0, 0);
//     lv_obj_set_scrollbar_mode(hr, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_add_event_cb(hr, h_ruler_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
//     g.h_ruler_obj = hr;

//     /* Vertical ruler */
//     lv_obj_t *vr = lv_obj_create(scr);
//     lv_obj_set_pos(vr, 0, VP_Y);
//     lv_obj_set_size(vr, RULER_SZ, VP_H);
//     lv_obj_set_style_bg_opa(vr, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(vr, 0, 0);
//     lv_obj_set_style_pad_all(vr, 0, 0);
//     lv_obj_set_style_radius(vr, 0, 0);
//     lv_obj_set_scrollbar_mode(vr, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_add_event_cb(vr, v_ruler_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
//     g.v_ruler_obj = vr;

//     /* Main canvas */
//     lv_obj_t *cv = lv_obj_create(scr);
//     lv_obj_set_pos(cv, VP_X, VP_Y);
//     lv_obj_set_size(cv, VP_W, VP_H);
//     lv_obj_set_style_bg_opa(cv, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(cv, 0, 0);
//     lv_obj_set_style_pad_all(cv, 0, 0);
//     lv_obj_set_style_radius(cv, 0, 0);
//     lv_obj_clear_flag(cv, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_add_flag(cv, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_add_event_cb(cv, canvas_draw_cb,  LV_EVENT_DRAW_MAIN, NULL);
//     lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_PRESSED,   NULL);
//     lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_PRESSING,  NULL);
//     lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_RELEASED,  NULL);
//     lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_GESTURE,   NULL);
//     g.canvas_obj = cv;

//     /*
//      * Transparent scrollable overlay — captures LV_EVENT_SCROLL
//      * (mouse wheel). CLICKABLE cleared so touch falls through to canvas.
//      */
//     lv_obj_t *sw = lv_obj_create(scr);
//     lv_obj_set_pos(sw, VP_X, VP_Y);
//     lv_obj_set_size(sw, VP_W, VP_H);
//     lv_obj_set_style_bg_opa(sw, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(sw, 0, 0);
//     lv_obj_set_style_pad_all(sw, 0, 0);
//     lv_obj_set_style_radius(sw, 0, 0);
//     lv_obj_add_flag(sw, LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_set_scrollbar_mode(sw, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_clear_flag(sw, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_add_event_cb(sw, scroll_zoom_cb, LV_EVENT_SCROLL, NULL);
// }

// /* =========================================================
//  * RIGHT INFO PANEL
//  * ========================================================= */
// static lv_obj_t *panel_row(lv_obj_t *par, const char *key,
//                              int32_t *y, int32_t pw)
// {
//     mk_lbl(par, key, C_DIM, FONT_SM, 0, *y);
//     lv_obj_t *val = mk_lbl(par, "---", C_CYAN, FONT_SM, pw / 2, *y);
//     *y += 16;
//     return val;
// }

// void build_right_panel(lv_obj_t *scr)
// {
//     int32_t px = SCR_W - RPANEL_W;
//     int32_t py = TOOLBAR_H;
//     int32_t ph = SCR_H - TOOLBAR_H;
//     int32_t pw = RPANEL_W - 8;

//     lv_obj_t *panel = lv_obj_create(scr);
//     lv_obj_set_pos(panel, px, py);
//     lv_obj_set_size(panel, RPANEL_W, ph);
//     lv_obj_set_style_bg_color(panel, H(C_PANEL), 0);
//     lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
//     lv_obj_set_style_border_color(panel, H(C_BORDER), 0);
//     lv_obj_set_style_border_width(panel, 1, 0);
//     lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_LEFT, 0);
//     lv_obj_set_style_pad_all(panel, 4, 0);
//     lv_obj_set_style_radius(panel, 0, 0);
//     lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_ACTIVE);

//     int32_t y = 2;

//     mk_lbl(panel, "ZOOM", C_BLUE, FONT_MD, 0, y); y += 16;
//     g.lbl_zoom_pct = mk_lbl(panel, "100%", C_WHITE, FONT_LG, 0, y); y += 20;

//     lv_obj_t *s1 = mk_plain(panel, 0, y, pw, 1, C_BORDER); (void)s1; y += 4;

//     g.lbl_level   = panel_row(panel, "Level",    &y, pw);
//     g.lbl_px_mm   = panel_row(panel, "px/mm",    &y, pw);
//     g.lbl_mm_px   = panel_row(panel, "mm/px",    &y, pw);
//     g.lbl_eff_dpi = panel_row(panel, "Eff. DPI", &y, pw);

//     lv_obj_t *s2 = mk_plain(panel, 0, y, pw, 1, C_BORDER); (void)s2; y += 4;

//     mk_lbl(panel, "IMAGE", C_BLUE, FONT_MD, 0, y); y += 16;

//     char spx[24];
//     lv_snprintf(spx, sizeof(spx), "%dx%d", IMG_W_PX, IMG_H_PX);
//     mk_lbl(panel, "Size px",  C_DIM,  FONT_SM, 0,      y);
//     mk_lbl(panel, spx,        C_CYAN, FONT_SM, pw / 2, y); y += 16;

//     mk_lbl(panel, "Size mm",  C_DIM,  FONT_SM, 0,      y);
//     g.lbl_size_mm = mk_lbl(panel, "---", C_CYAN, FONT_SM, pw / 2, y); y += 16;

//     mk_lbl(panel, "Mem",      C_DIM,  FONT_SM, 0,      y);
//     g.lbl_mem = mk_lbl(panel, "---", C_CYAN, FONT_SM, pw / 2, y); y += 16;

//     lv_obj_t *s3 = mk_plain(panel, 0, y, pw, 1, C_BORDER); (void)s3; y += 4;

//     mk_lbl(panel, "SCALE BAR - 10mm", C_BLUE, FONT_SM, 0, y); y += 14;
//     lv_obj_t *bar_bg = mk_plain(panel, 0, y, pw, 7, C_BORDER);
//     lv_obj_set_style_radius(bar_bg, 3, 0);
//     g.scale_bar_fill = mk_plain(bar_bg, 0, 0, 40, 7, C_BLUE);
//     lv_obj_set_style_radius(g.scale_bar_fill, 3, 0);
//     y += 11;
//     g.lbl_scale_val = mk_lbl(panel, "10mm = -- px", C_DIM, FONT_SM, 0, y); y += 16;

//     lv_obj_t *s4 = mk_plain(panel, 0, y, pw, 1, C_BORDER); (void)s4; y += 4;

//     mk_lbl(panel, "SELECTION", C_BLUE, FONT_MD, 0, y); y += 16;
//     g.lbl_sel = mk_lbl(panel, "No selection", C_CYAN, FONT_SM, 0, y); y += 36;

//     lv_obj_t *s5 = mk_plain(panel, 0, y, pw, 1, C_BORDER); (void)s5; y += 4;

//     mk_lbl(panel, "MAGNIFIER", C_BLUE, FONT_MD, 0, y); y += 16;

//     int32_t mag_h = ph - y - 10;
//     if(mag_h < 60) mag_h = 60;

//     lv_obj_t *mag = lv_obj_create(panel);
//     lv_obj_set_pos(mag, 0, y);
//     lv_obj_set_size(mag, pw, mag_h);
//     lv_obj_set_style_bg_opa(mag, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_width(mag, 0, 0);
//     lv_obj_set_style_pad_all(mag, 0, 0);
//     lv_obj_set_style_radius(mag, 0, 0);
//     lv_obj_set_scrollbar_mode(mag, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_add_event_cb(mag, mag_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
//     g.mag_obj = mag;
// }


/*==============================================================
 * src/ui/layout.c
 * One-time screen layout builders for the new CNC UI design.
 *==============================================================*/

#include "viewer.h"
#include "viewer_math.h"
#include "viewer_ui.h"
#include "viewer_draw.h"
#include "viewer_input.h"

/* =========================================================
 * TOP TOOLBAR
 * ========================================================= */
void build_toolbar(lv_obj_t *scr)
{
    lv_obj_t *bar = mk_plain(scr, 0, 0, SCR_W, TOOLBAR_H, C_TOOLBAR);

    /* Bottom separator */
    mk_plain(bar, 0, TOOLBAR_H - 1, SCR_W, 1, C_BORDER);

    typedef struct {
        const char    *lbl;
        lv_event_cb_t  cb;
        int32_t        w;
        int            is_pan;
        int            is_crop;
    } BtnDef;

    /* Fixed: LV_SYMBOL_CROP → LV_SYMBOL_CUT (standard LVGL symbol) */
    const BtnDef defs[] = {
        {"PAN",          cb_pan,      52, 1, 0},
        {LV_SYMBOL_CUT    "\nCROP",     cb_crop,     52, 0, 1},
        {LV_SYMBOL_PLUS   "\nZOOM+",    cb_zin,      52, 0, 0},
        {LV_SYMBOL_MINUS  "\nZOOM-",    cb_zout,     52, 0, 0},
        {LV_SYMBOL_REFRESH"\nRESET",    cb_reset,    52, 0, 0},
        {LV_SYMBOL_HOME   "\nFIT",      cb_fit,      44, 0, 0},
        {LV_SYMBOL_TRASH  "\nCLEAR",    cb_clear,    52, 0, 0},
        {LV_SYMBOL_SETTINGS"\nSETTINGS",cb_settings, 68, 0, 0},
    };

    int n = (int)(sizeof(defs) / sizeof(defs[0]));

    int32_t cx = 4;
    for (int i = 0; i < n; i++) {
        lv_obj_t *btn = mk_btn(bar, defs[i].lbl, defs[i].cb, cx, defs[i].w);
        if (defs[i].is_pan)  g.btn_pan  = btn;
        if (defs[i].is_crop) g.btn_crop = btn;
        cx += defs[i].w + 3;
    }

    g.hint_lbl = mk_lbl(bar, "Pan mode | Scroll = Zoom  Drag = Pan",
                         C_DIM, FONT_SM, 0, 0);
    lv_obj_align(g.hint_lbl, LV_ALIGN_RIGHT_MID, -6, 0);
}

/* =========================================================
 * BOTTOM DRAWING TOOLS TOOLBAR
 * ========================================================= */
void build_bottom_toolbar(lv_obj_t *scr)
{
    int32_t by = SCR_H - BTOOLBAR_H;
    lv_obj_t *bar = mk_plain(scr, 0, by, SCR_W - RPANEL_W, BTOOLBAR_H, C_BTOOLBAR);

    /* Top separator */
    mk_plain(bar, 0, 0, SCR_W - RPANEL_W, 1, C_BORDER);

    typedef struct {
        const char    *symbol;
        const char    *label;
        lv_event_cb_t   cb;           
        int32_t        w;
        lv_obj_t     **out;
    } ToolDef;

    /*const ToolDef tools[] = {
        {LV_SYMBOL_EDIT,   "STRAIGHT LINE", cb_tool_line, 110, &g.btn_line},
        {LV_SYMBOL_LOOP,   "CURVE LINE",    cb_tool_arc,   94,  &g.btn_arc},
        {LV_SYMBOL_STOP,   "BOX CREATION",  cb_tool_box,  106, &g.btn_box},
        {LV_SYMBOL_EDIT,   "EDIT",          cb_tool_edit,  64,  &g.btn_edit},
        {LV_SYMBOL_TINT,   "FILL",          cb_tool_fill,  52,  &g.btn_fill},
        {LV_SYMBOL_OK,     "CHECK",         cb_tool_check, 52,  &g.btn_check},
        {LV_SYMBOL_LEFT,   "UNDO",          cb_tool_undo,  64,  &g.btn_undo},
        {LV_SYMBOL_RIGHT,  "REDO",          cb_tool_redo,  64,  &g.btn_redo},
    };*/

    const ToolDef tools[] = {
        {LV_SYMBOL_LIST,   "DRAW OBJECT",   cb_draw_object_menu, 124, &g.btn_draw_object},
        {LV_SYMBOL_IMAGE,   "PATTERN",       cb_tool_pattern_fill, 76, &g.btn_pattern},
        {LV_SYMBOL_EDIT,   "EDIT",          cb_tool_edit,  64,  &g.btn_edit},
        {LV_SYMBOL_TINT,   "FILL",          cb_tool_fill,  52,  &g.btn_fill},
        {LV_SYMBOL_OK,     "CHECK",         cb_tool_check, 52,  &g.btn_check},
        {LV_SYMBOL_LEFT,   "UNDO",          cb_tool_undo,  64,  &g.btn_undo},
        {LV_SYMBOL_RIGHT,  "REDO",          cb_tool_redo,  64,  &g.btn_redo},
        {LV_SYMBOL_MINUS,  "HIDE",          cb_tool_hide,  52,  NULL},
    };

    int n = (int)(sizeof(tools) / sizeof(tools[0]));

    int32_t cx = 4;
    for (int i = 0; i < n; i++) {
        lv_obj_t *btn = mk_tool_btn(bar, tools[i].symbol, tools[i].label,
                                    tools[i].cb, cx, tools[i].w);
        if (tools[i].out) *tools[i].out = btn;
        cx += tools[i].w + 4;
    }
}

/* =========================================================
 * CANVAS + RULERS
 * ========================================================= */
void build_canvas_area(lv_obj_t *scr)
{
    /* Corner "mm" label */
    lv_obj_t *corner = mk_plain(scr, 0, TOOLBAR_H, RULER_SZ, RULER_SZ, C_RULER_BG);
    lv_obj_set_style_border_color(corner, H(C_RULER_BORDER), 0);
    lv_obj_set_style_border_width(corner, 1, 0);
    lv_obj_set_style_bg_color(corner, H(C_RULER_BG), 0);
    lv_obj_set_style_bg_opa(corner, LV_OPA_COVER, 0);

    lv_obj_t *corner_lbl = lv_label_create(corner);
    lv_label_set_text(corner_lbl, "mm");
    lv_obj_set_style_text_color(corner_lbl, H(C_RULER_MJ), 0);
    lv_obj_set_style_text_font(corner_lbl, FONT_SM, 0);
    lv_obj_center(corner_lbl);

    /* Horizontal ruler */
    lv_obj_t *hr = lv_obj_create(scr);
    lv_obj_set_pos(hr, VP_X, TOOLBAR_H);
    lv_obj_set_size(hr, VP_W, RULER_SZ);
    lv_obj_set_style_bg_opa(hr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hr, 0, 0);
    lv_obj_set_style_pad_all(hr, 0, 0);
    lv_obj_set_style_radius(hr, 0, 0);
    lv_obj_set_scrollbar_mode(hr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(hr, h_ruler_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    g.h_ruler_obj = hr;

    /* Vertical ruler */
    lv_obj_t *vr = lv_obj_create(scr);
    lv_obj_set_pos(vr, 0, VP_Y);
    lv_obj_set_size(vr, RULER_SZ, VP_H);
    lv_obj_set_style_bg_opa(vr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vr, 0, 0);
    lv_obj_set_style_pad_all(vr, 0, 0);
    lv_obj_set_style_radius(vr, 0, 0);
    lv_obj_set_scrollbar_mode(vr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(vr, v_ruler_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    g.v_ruler_obj = vr;

    /* Main canvas */
    lv_obj_t *cv = lv_obj_create(scr);
    lv_obj_set_pos(cv, VP_X, VP_Y);
    lv_obj_set_size(cv, VP_W, VP_H);
    lv_obj_set_style_bg_opa(cv, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cv, 0, 0);
    lv_obj_set_style_pad_all(cv, 0, 0);
    lv_obj_set_style_radius(cv, 0, 0);
    lv_obj_clear_flag(cv, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(cv, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(cv, canvas_draw_cb,  LV_EVENT_DRAW_MAIN, NULL);
    lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_PRESSED,   NULL);
    lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_PRESSING,  NULL);
    lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_RELEASED,  NULL);
    lv_obj_add_event_cb(cv, canvas_event_cb, LV_EVENT_GESTURE,   NULL);
    g.canvas_obj = cv;

    /* Scroll overlay for mouse wheel zoom */
    lv_obj_t *sw = lv_obj_create(scr);
    lv_obj_set_pos(sw, VP_X, VP_Y);
    lv_obj_set_size(sw, VP_W, VP_H);
    lv_obj_set_style_bg_opa(sw, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sw, 0, 0);
    lv_obj_set_style_pad_all(sw, 0, 0);
    lv_obj_set_style_radius(sw, 0, 0);
    lv_obj_add_flag(sw, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(sw, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(sw, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(sw, scroll_zoom_cb, LV_EVENT_SCROLL, NULL);
}

/* =========================================================
 * RIGHT PANEL — CNC JOB DETAILS
 * ========================================================= */
static lv_obj_t *hsep(lv_obj_t *par, int32_t y, int32_t pw)
{
    return mk_plain(par, 0, y, pw, 1, C_BORDER);
}

static lv_obj_t *kv_row(lv_obj_t *par, const char *key,
                        const char *init, int32_t *y, int32_t pw)
{
    mk_lbl(par, key, C_PNL_KEY, FONT_SM, 4, *y);
    lv_obj_t *val = mk_lbl(par, init, C_PNL_VAL, FONT_SM, pw / 2 + 4, *y);
    *y += 15;
    return val;
}

void build_right_panel(lv_obj_t *scr)
{
    int32_t px = SCR_W - RPANEL_W;
    int32_t py = TOOLBAR_H;
    int32_t ph = SCR_H - TOOLBAR_H;
    int32_t pw = RPANEL_W;

    lv_obj_t *panel = lv_obj_create(scr);
    lv_obj_set_pos(panel, px, py);
    lv_obj_set_size(panel, pw, ph);
    lv_obj_set_style_bg_color(panel, H(C_PANEL), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(panel, H(C_BORDER), 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_side(panel, LV_BORDER_SIDE_LEFT, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_set_style_radius(panel, 0, 0);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_ACTIVE);

    /* Header */
    lv_obj_t *hdr = mk_plain(panel, 0, 0, pw, 36, C_PANEL_HDR);
    lv_obj_t *hdr_title = mk_lbl(hdr, "CNC JOB DETAILS", C_PNL_TITLE, FONT_MD, 0, 3);
    lv_obj_set_width(hdr_title, pw);
    lv_obj_set_style_text_align(hdr_title, LV_TEXT_ALIGN_CENTER, 0);

    char res_buf[24];
    lv_snprintf(res_buf, sizeof(res_buf), "(%dx%d)", IMG_W_PX, IMG_H_PX);
    lv_obj_t *hdr_sub = mk_lbl(hdr, res_buf, C_PNL_KEY, FONT_SM, 0, 20);
    lv_obj_set_width(hdr_sub, pw);
    lv_obj_set_style_text_align(hdr_sub, LV_TEXT_ALIGN_CENTER, 0);

    int32_t y = 40;

    /* Zoom Level */
    {
        lv_obj_t *zrow = mk_plain(panel, 0, y, pw, 22, C_PANEL_HDR);
        mk_lbl(zrow, "Zoom Level:", C_PNL_KEY, FONT_SM, 4, 4);
        g.lbl_zoom_pct = mk_lbl(zrow, "1.0x (Max 10x)", C_PNL_VAL, FONT_SM, 68, 4);
        y += 22;
    }

    hsep(panel, y, pw); y += 5;

    /* Image Control */
    {
        lv_obj_t *sec = mk_plain(panel, 0, y, pw, 16, C_PANEL_HDR);
        mk_lbl(sec, "Image Control", C_PNL_SEC, FONT_MD, 4, 1);
        y += 18;
    }

    g.lbl_array_idx  = kv_row(panel, "Array Index:",  "(0,0) (1920, 1200)", &y, pw);
    g.lbl_array_sz   = kv_row(panel, "Array Size:",   "20005 X 12005",      &y, pw);
    g.lbl_resolution = kv_row(panel, "Resolution:",   "1920x1200",          &y, pw);

    mk_lbl(panel, "Coordinate Data:", C_PNL_KEY, FONT_SM, 4, y); y += 13;
    g.lbl_coord = mk_lbl(panel, "X: 0.000mm | Z: 0.000mm", C_PNL_VAL, FONT_SM, 8, y);
    y += 15;

    hsep(panel, y, pw); y += 5;

    /* Tool Precision */
    {
        lv_obj_t *sec = mk_plain(panel, 0, y, pw, 16, C_PANEL_HDR);
        mk_lbl(sec, "Tool Precision", C_PNL_SEC, FONT_MD, 4, 1);
        y += 18;
    }
    g.lbl_precision = mk_lbl(panel, "X-Axis: 0.005mm | Z-Axis: 0.002mm",
                             C_PNL_KEY, FONT_SM, 4, y);
    y += 16;

    hsep(panel, y, pw); y += 5;

    /* Scale Bar */
    mk_lbl(panel, "SCALE BAR — 10mm", C_BLUE, FONT_SM, 4, y); y += 14;
    lv_obj_t *bar_bg = mk_plain(panel, 4, y, pw - 8, 7, C_BORDER);
    lv_obj_set_style_radius(bar_bg, 3, 0);
    g.scale_bar_fill = mk_plain(bar_bg, 0, 0, 40, 7, C_BLUE);
    lv_obj_set_style_radius(g.scale_bar_fill, 3, 0);
    y += 11;
    g.lbl_scale_val = mk_lbl(panel, "10mm = -- px", C_DIM, FONT_SM, 4, y); y += 15;

    hsep(panel, y, pw); y += 5;

    /* Selection */
    {
        lv_obj_t *sec = mk_plain(panel, 0, y, pw, 16, C_PANEL_HDR);
        mk_lbl(sec, "SELECTION", C_BLUE, FONT_MD, 4, 1);
        y += 18;
    }
    g.lbl_sel = mk_lbl(panel, "No selection", C_PNL_VAL, FONT_SM, 4, y); y += 30;

    hsep(panel, y, pw); y += 5;

    /* Draw Objects */
    {
        lv_obj_t *sec = mk_plain(panel, 0, y, pw, 16, C_PANEL_HDR);
        mk_lbl(sec, "DRAW OBJECTS", C_BLUE, FONT_MD, 4, 1);
        y += 18;
    }
    g.lbl_draw_count = mk_lbl(panel, "Lines: 0  Arcs: 0  Boxes: 0",
                               C_PNL_VAL, FONT_SM, 4, y); y += 15;

    hsep(panel, y, pw); y += 5;

    /* Memory */
    mk_lbl(panel, "Mem:", C_PNL_KEY, FONT_SM, 4, y);
    g.lbl_mem = mk_lbl(panel, "---", C_PNL_VAL, FONT_SM, pw / 2, y);
}