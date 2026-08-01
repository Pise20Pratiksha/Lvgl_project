// /*==============================================================
//  * src/ui/widgets.c
//  * Low-level LVGL widget factory helpers.
//  *==============================================================*/

// #include "viewer.h"
// #include "viewer_ui.h"


// //Create a plain container
// lv_obj_t *mk_plain(lv_obj_t *par,
//                     int32_t x, int32_t y,
//                     int32_t w, int32_t h,
//                     uint32_t col)
// {
//     // Create a base LVGL object 
//     lv_obj_t *o = lv_obj_create(par);

//     // Set position (x, y) relative to parent
//     lv_obj_set_pos(o, x, y);

//     lv_obj_set_size(o, w, h);    // Set width and height
//     // Set background color
//     lv_obj_set_style_bg_color(o, H(col), 0);
//     lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
//     lv_obj_set_style_border_width(o, 0, 0);
//     lv_obj_set_style_radius(o, 0, 0);
//     lv_obj_set_style_pad_all(o, 0, 0);
//     lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
//     return o;
// }

// //Create a label (text element)
// lv_obj_t *mk_lbl(lv_obj_t *par, const char *txt,
//                   uint32_t col, const lv_font_t *fnt,
//                   int32_t x, int32_t y)
// {
//     lv_obj_t *lb = lv_label_create(par);  // Create label inside parent
//     lv_label_set_text(lb, txt);
//     lv_obj_set_style_text_color(lb, H(col), 0);
//     lv_obj_set_style_text_font(lb, fnt, 0);
//     lv_obj_set_pos(lb, x, y);
//     lv_obj_set_style_bg_opa(lb, LV_OPA_TRANSP, 0);
//     return lb;
// }

// //  Create a button with a label and click event
// lv_obj_t *mk_btn(lv_obj_t *par, const char *label,
//                   lv_event_cb_t cb, int32_t x, int32_t w)
// {
//     lv_obj_t *btn = lv_obj_create(par);
//     lv_obj_set_pos(btn, x, 2);
//     lv_obj_set_size(btn, w, TOOLBAR_H - 4);
//     lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
//     lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
//     lv_obj_set_style_border_width(btn, 1, 0);
//     lv_obj_set_style_radius(btn, 4, 0);
//     lv_obj_set_style_pad_all(btn, 0, 0);
//     lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
//     lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

//     lv_obj_t *lb = lv_label_create(btn);
//     lv_label_set_text(lb, label);
//     lv_obj_set_style_text_color(lb, H(C_WHITE), 0);
//     lv_obj_set_style_text_font(lb, FONT_SM, 0);
//     lv_obj_center(lb);
//     return btn;
// }


/*==============================================================
 * src/ui/widgets.c
 * Low-level LVGL widget factory helpers.
 *==============================================================*/

#include "viewer.h"
#include "viewer_ui.h"

/* ──────────────────────────────────────────────────────────
 * mk_plain — bare rectangle container
 * ────────────────────────────────────────────────────────── */
lv_obj_t *mk_plain(lv_obj_t *par,
                    int32_t x, int32_t y,
                    int32_t w, int32_t h,
                    uint32_t col)
{
    lv_obj_t *o = lv_obj_create(par);
    lv_obj_set_pos(o, x, y);
    lv_obj_set_size(o, w, h);
    lv_obj_set_style_bg_color(o, H(col), 0);
    lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(o, 0, 0);
    lv_obj_set_style_radius(o, 0, 0);
    lv_obj_set_style_pad_all(o, 0, 0);
    lv_obj_set_scrollbar_mode(o, LV_SCROLLBAR_MODE_OFF);
    return o;
}

/* ──────────────────────────────────────────────────────────
 * mk_lbl — text label
 * ────────────────────────────────────────────────────────── */
lv_obj_t *mk_lbl(lv_obj_t *par, const char *txt,
                  uint32_t col, const lv_font_t *fnt,
                  int32_t x, int32_t y)
{
    lv_obj_t *lb = lv_label_create(par);
    lv_label_set_text(lb, txt);
    lv_obj_set_style_text_color(lb, H(col), 0);
    lv_obj_set_style_text_font(lb, fnt, 0);
    lv_obj_set_pos(lb, x, y);
    lv_obj_set_style_bg_opa(lb, LV_OPA_TRANSP, 0);
    return lb;
}

/* ──────────────────────────────────────────────────────────
 * mk_btn — compact top-toolbar button
 *   label: can include LV_SYMBOL_xxx + "\n" + text
 * ────────────────────────────────────────────────────────── */
lv_obj_t *mk_btn(lv_obj_t *par, const char *label,
                  lv_event_cb_t cb, int32_t x, int32_t w)
{
    lv_obj_t *btn = lv_obj_create(par);
    lv_obj_set_pos(btn, x, 2);
    lv_obj_set_size(btn, w, TOOLBAR_H - 4);
    lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
    lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_radius(btn, 4, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lb = lv_label_create(btn);
    lv_label_set_text(lb, label);
    lv_obj_set_style_text_color(lb, H(C_WHITE), 0);
    lv_obj_set_style_text_font(lb, FONT_SM, 0);
    lv_obj_set_style_text_align(lb, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lb);
    return btn;
}

/* ──────────────────────────────────────────────────────────
 * mk_tool_btn — large bottom-toolbar button
 *   Shows an icon symbol on top and a text label below,
 *   like the image: [▶] [STRAIGHT LINE]
 * ────────────────────────────────────────────────────────── */
lv_obj_t *mk_tool_btn(lv_obj_t *par, const char *symbol,
                       const char *label, lv_event_cb_t cb,
                       int32_t x, int32_t w)
{
    int32_t bh = BTOOLBAR_H - 6;

    lv_obj_t *btn = lv_obj_create(par);
    lv_obj_set_pos(btn, x, 3);
    lv_obj_set_size(btn, w, bh);
    lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
    lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_radius(btn, 5, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    /* Symbol icon on top half */
    lv_obj_t *icon = lv_label_create(btn);
    lv_label_set_text(icon, symbol);
    lv_obj_set_style_text_color(icon, H(C_CYAN), 0);
    lv_obj_set_style_text_font(icon, FONT_MD, 0);
    lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(icon, w);
    lv_obj_set_pos(icon, 0, 3);

    /* Text label on bottom half */
    lv_obj_t *txt = lv_label_create(btn);
    lv_label_set_text(txt, label);
    lv_obj_set_style_text_color(txt, H(C_WHITE), 0);
    lv_obj_set_style_text_font(txt, FONT_SM, 0);
    lv_obj_set_style_text_align(txt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(txt, w);
    lv_obj_set_pos(txt, 0, bh / 2 + 2);

    return btn;
}