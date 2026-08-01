/*==============================================================
 * src/ui/panel.c
 * Runtime panel updates: update_panel(), refresh_all(), set_mode(),
 * do_undo(), do_redo()
 *==============================================================*/

#include "viewer.h"
#include "viewer_math.h"
#include "viewer_ui.h"

/* =========================================================
 * refresh_all  —  invalidate all custom-drawn widgets
 * ========================================================= */
void refresh_all(void)
{
    lv_obj_invalidate(g.canvas_obj);
    lv_obj_invalidate(g.h_ruler_obj);
    lv_obj_invalidate(g.v_ruler_obj);
    if (g.mag_obj) lv_obj_invalidate(g.mag_obj);
}

/* =========================================================
 * update_panel  —  push live values to right-panel labels
 * ========================================================= */
void update_panel(void)
{
    char buf[96];
    float sc      = zoom_scale();
    float ppm_x   = px_per_mm_x();
    // float ppm_y   = px_per_mm_y();

    /* Zoom level  "1.0x (Max 10x)" */
    float z_abs = sc / ZOOM_DEFAULT;   /* relative to default */
    float z_max = ZOOM_MAX / ZOOM_DEFAULT;
    lv_snprintf(buf, sizeof(buf), "%.2fx (Max %.0fx)", z_abs, z_max);
    lv_label_set_text(g.lbl_zoom_pct, buf);

    /* Array Index — cursor position in image-space */
    float img_x = vp_to_img_x(g.cursor_vx);
    float img_y = vp_to_img_y(g.cursor_vy);
    int ix = (int)img_x, iy = (int)img_y;
    if (ix < 0) ix = 0;
    if (iy < 0) iy = 0;
    if (ix > VIRTUAL_W) ix = VIRTUAL_W;
    if (iy > VIRTUAL_H) iy = VIRTUAL_H;
    lv_snprintf(buf, sizeof(buf), "(0,0) (%d, %d)", ix, iy);
    lv_label_set_text(g.lbl_array_idx, buf);

    /* Array Size */
    lv_snprintf(buf, sizeof(buf), "%d X %d", VIRTUAL_W, VIRTUAL_H);
    lv_label_set_text(g.lbl_array_sz, buf);

    /* Resolution */
    lv_snprintf(buf, sizeof(buf), "%dx%d", IMG_W_PX, IMG_H_PX);
    lv_label_set_text(g.lbl_resolution, buf);

    /* Coordinate Data */
    lv_snprintf(buf, sizeof(buf), "X: %.3fmm | Z: %.3fmm",
                vp_to_mm_y(g.cursor_vy), vp_to_mm_x(g.cursor_vx));
    lv_label_set_text(g.lbl_coord, buf);

    /* Tool Precision (fixed physical) */
    lv_snprintf(buf, sizeof(buf), "X-Axis: %.3fmm | Z-Axis: %.3fmm",
                MM_PER_PX_Y, MM_PER_PX_X);
    lv_label_set_text(g.lbl_precision, buf);

    /* Scale bar: 10 mm in screen pixels */
    int32_t bar_px = (int32_t)(10.0f * ppm_x + 0.5f);
    if (bar_px < 1) bar_px = 1;
    int32_t bar_w  = bar_px > (RPANEL_W - 16) ? (RPANEL_W - 16) : bar_px;
    lv_obj_set_width(g.scale_bar_fill, bar_w);
    lv_snprintf(buf, sizeof(buf), "10mm = %d px", (int)bar_px);
    lv_label_set_text(g.lbl_scale_val, buf);

    /* Selection */
    if (g.has_crop) {
        float x1 = LV_MIN(g.crop_x1, g.crop_x2);
        float y1 = LV_MIN(g.crop_y1, g.crop_y2);
        float x2 = LV_MAX(g.crop_x1, g.crop_x2);
        float y2 = LV_MAX(g.crop_y1, g.crop_y2);
        float sw  = x2 - x1, sh = y2 - y1;
        lv_snprintf(buf, sizeof(buf), "%.0fx%.0fpx\n%.3fx%.3fmm",
                    sw, sh, sw * MM_PER_PX_X, sh * MM_PER_PX_Y);
    } else {
        lv_snprintf(buf, sizeof(buf), "No selection");
    }
    lv_label_set_text(g.lbl_sel, buf);

    /* Draw object counts */
    int n_line = 0, n_arc = 0, n_box = 0, n_poly = 0;
     int n_bar = 0, n_pipe = 0, n_forged = 0;
    for (int i = 0; i < g.draw_count; i++) {
        switch (g.draw_objects[i].type) {
            case OBJ_LINE: n_line++; break;
            case OBJ_ARC:  n_arc++;  break;
            case OBJ_BOX:  n_box++;  break;
            case OBJ_POLY: n_poly++; break;
            case OBJ_BAR:    n_bar++;    break;
            case OBJ_PIPE:   n_pipe++;   break;
            case OBJ_FORGED: n_forged++; break;
        }
    }
    lv_snprintf(buf, sizeof(buf), "Lines:%d  Arcs:%d  Boxes:%d  Fills:%d",
                n_line, n_arc, n_box, n_poly, n_bar, n_pipe, n_forged);
    lv_label_set_text(g.lbl_draw_count, buf);

    /* Memory estimate (virtual canvas RGB565) */
    int mem = VIRTUAL_W * VIRTUAL_H * 2;
    if (mem >= 1024 * 1024)
        lv_snprintf(buf, sizeof(buf), "%.0f MB", mem / (1024.0f * 1024.0f));
    else
        lv_snprintf(buf, sizeof(buf), "%d KB", mem / 1024);
    lv_label_set_text(g.lbl_mem, buf);

    /* Undo/redo button dim */
    if (g.btn_undo) {
        lv_obj_set_style_bg_color(g.btn_undo,
            g.undo_top > 0 ? H(C_BTN_NORM) : H(C_BORDER), 0);
    }
}

/* =========================================================
 * set_mode  —  switch interaction mode & update button highlight
 * ========================================================= */
void set_mode(ViewMode m)
{

    /* Read-only DXF review: only Pan/Crop are allowed while active */
    if (g.dxf_review_mode && m != MODE_PAN && m != MODE_CROP) {
        return;
    }

    g.mode = m;

    /* Reset all bottom tool buttons to normal */
    /*lv_obj_t *draw_btns[] = {
        g.btn_line, g.btn_arc, g.btn_box, g.btn_edit, g.btn_fill, g.btn_check
    };
    for (int i = 0; i < 6; i++) {
        if (draw_btns[i])
            lv_obj_set_style_bg_color(draw_btns[i], H(C_BTN_NORM), 0);
    }*/

    lv_obj_t *draw_btns[] = {
    g.btn_draw_object, g.btn_edit, g.btn_fill, g.btn_check, g.btn_pattern
    };
    for (int i = 0; i < 5; i++) {
        if (draw_btns[i])
            lv_obj_set_style_bg_color(draw_btns[i], H(C_BTN_NORM), 0);
    }

    /* Reset top toolbar pan/crop */
    if (g.btn_pan)  lv_obj_set_style_bg_color(g.btn_pan,  H(C_BTN_NORM), 0);
    if (g.btn_crop) lv_obj_set_style_bg_color(g.btn_crop, H(C_BTN_NORM), 0);

    /* Clear gap markers whenever the user switches mode */
    if (m != MODE_FILL) {
        g.gap_pts_count = 0;
        /* keep g.has_gaps so panel still shows count if user switches back */
    }

    /* Highlight active button and update hint */
    switch (m) {
        case MODE_PAN:
            if (g.btn_pan)
                lv_obj_set_style_bg_color(g.btn_pan, H(C_BTN_PAN), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Pan  |  Scroll/Swipe = Zoom");
            break;

        case MODE_CROP:
            if (g.btn_crop)
                lv_obj_set_style_bg_color(g.btn_crop, H(C_BTN_CROP), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Crop  |  Drag to select region");
            break;

        /*case MODE_LINE:
            if (g.btn_line)
                lv_obj_set_style_bg_color(g.btn_line, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Straight Line  |  Tap start, tap end");
            g.drawing = false;
            g.draw_phase = 0;
            break;

        case MODE_ARC:
            if (g.btn_arc)
                lv_obj_set_style_bg_color(g.btn_arc, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Curve Line  |  Tap start, tap end, drag control point");
            g.drawing = false;
            g.draw_phase = 0;
            g.curve_dragging = false;
            break;

        case MODE_BOX:
            if (g.btn_box)
                lv_obj_set_style_bg_color(g.btn_box, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Box Creation  |  Drag to draw rectangle");
            g.drawing = false;
            g.draw_phase = 0;
            break;*/

        case MODE_LINE:
            if (g.btn_draw_object)
                lv_obj_set_style_bg_color(g.btn_draw_object, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Straight Line  |  Tap start, tap end");
            g.drawing = false;
            g.draw_phase = 0;
            break;

        case MODE_ARC:
            if (g.btn_draw_object)
                lv_obj_set_style_bg_color(g.btn_draw_object, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Curve Line  |  Tap start, tap end, drag control point");
            g.drawing = false;
            g.draw_phase = 0;
            g.curve_dragging = false;
            break;

        case MODE_BOX:
            if (g.btn_draw_object)
                lv_obj_set_style_bg_color(g.btn_draw_object, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Box Creation  |  Drag to draw rectangle");
            g.drawing = false;
            g.draw_phase = 0;
            break;

        /* ── NEW: Bar / Pipes / Forged ── */
        case MODE_BAR:
            if (g.btn_draw_object)
                lv_obj_set_style_bg_color(g.btn_draw_object, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Bar  |  Drag to draw bar stock");
            g.drawing = false;
            g.draw_phase = 0;
            break;

        case MODE_PIPE:
            if (g.btn_draw_object)
                lv_obj_set_style_bg_color(g.btn_draw_object, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Pipe  |  Drag centerline, sets pipe diameter");
            g.drawing = false;
            g.draw_phase = 0;
            break;

        case MODE_FORGED:
            if (g.btn_draw_object)
                lv_obj_set_style_bg_color(g.btn_draw_object, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Forged Part  |  Tap each vertex, tap first point to close");
            g.drawing = false;
            g.draw_phase = 0;
            g.forged_vcount = 0;
            break;

        case MODE_FORGED_CENTER:
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Center Line");
            break;

        case MODE_EDIT:
            if (g.btn_edit)
                lv_obj_set_style_bg_color(g.btn_edit, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Edit  |  Tap object to select / move nodes");
            break;

        case MODE_FILL:
            if (g.btn_fill)
                lv_obj_set_style_bg_color(g.btn_fill, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Fill Color  |  Tap to fill enclosed area");
            break;

        case MODE_PATTERN_FILL:
            if (g.btn_pattern)
                lv_obj_set_style_bg_color(g.btn_pattern, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl)
                lv_label_set_text(g.hint_lbl,
                    "Pattern Fill  |  Tap inside a closed Forged profile");
            break;

        case MODE_CHECK:
            if (g.btn_check)
                lv_obj_set_style_bg_color(g.btn_check, H(C_BTN_ACTIVE), 0);
            if (g.hint_lbl) {
                validate_drawing();
                lv_label_set_text(g.hint_lbl, g.validation_msg);
            }
            break;
    }
}

/* =========================================================
 * do_undo / do_redo - Enhanced with color history support
 * ========================================================= */
void do_undo(void)
{
    if (g.dxf_review_mode) return;

    extern bool viewer_color_undo(void);
    if (viewer_color_undo()) {
        refresh_all();
        update_panel();
        return;
    }

    if (g.draw_count <= 0) return;

    DrawObject *last = &g.draw_objects[g.draw_count - 1];

    if (last->type == OBJ_FORGED && last->seg_count > 1) {
        /* Pop one segment, keep the object */
        last->seg_count--;
        forged_resync_object(last);

        if (g.undo_top < MAX_DRAW_OBJECTS) {
            g.undo_stack[g.undo_top]  = g.draw_count - 1;
            g.undo_is_seg[g.undo_top] = true;
            g.undo_top++;
        }
    } else {
        /* Whole-object removal — last remaining Forged segment, or
         * any other object type. Existing behavior. */
        if (g.undo_top < MAX_DRAW_OBJECTS) {
            g.undo_stack[g.undo_top]  = g.draw_count - 1;
            g.undo_is_seg[g.undo_top] = false;
            g.undo_top++;
        }
        g.draw_count--;
    }

    refresh_all();
    update_panel();
}

void do_redo(void)
{
    if (g.dxf_review_mode) return;

    extern bool viewer_color_redo(void);
    if (viewer_color_redo()) {
        refresh_all();
        update_panel();
        return;
    }

    if (g.undo_top <= 0) return;

    g.undo_top--;
    int idx = g.undo_stack[g.undo_top];

    if (g.undo_is_seg[g.undo_top]) {
        if (idx >= 0 && idx < g.draw_count) {
            DrawObject *o = &g.draw_objects[idx];
            if (o->seg_count < MAX_FORGED_SEGS) {
                o->seg_count++;
                forged_resync_object(o);
            }
        }
    } else {
        g.draw_count++;
    }

    refresh_all();
    update_panel();
}