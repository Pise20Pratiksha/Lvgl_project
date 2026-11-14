// // /*==============================================================
// //  * src/input/events.c
// //  * Touch/gesture/scroll event handlers + toolbar button callbacks.
// //  *
// //  * FIXES vs broken version:
// //  *  [1] Include order: viewer.h → viewer_math.h → viewer_ui.h → viewer_input.h
// //  *      viewer_math.h brings in math.h (for fabsf used below).
// //  *  [2] Removed bare <math.h> include — covered by viewer_math.h.
// //  *==============================================================*/

// // #include "viewer.h"
// // #include "viewer_math.h"   /* FIX [1]: also provides math.h → fabsf */
// // #include "viewer_ui.h"
// // #include "viewer_input.h"

// // /* =========================================================
// //  * CANVAS TOUCH / GESTURE
// //  * ========================================================= */
// // void canvas_event_cb(lv_event_t *e)
// // {
// //     lv_event_code_t code  = lv_event_get_code(e);
// //     lv_indev_t     *indev = lv_event_get_indev(e);
// //     if(!indev) return;

// //     lv_point_t pt;
// //     lv_indev_get_point(indev, &pt);

// //     lv_area_t coords;
// //     lv_obj_get_coords(g.canvas_obj, &coords);

// //     float vx = (float)(pt.x - coords.x1);
// //     float vy = (float)(pt.y - coords.y1);
// //     if(vx < 0)     vx = 0;
// //     if(vx >= VP_W) vx = VP_W - 1;
// //     if(vy < 0)     vy = 0;
// //     if(vy >= VP_H) vy = VP_H - 1;

// //     g.cursor_vx = vx;
// //     g.cursor_vy = vy;

// //     /* ── PRESSED ── */
// //     if(code == LV_EVENT_PRESSED) {
// //         if(g.mode == MODE_PAN) {
// //             g.dragging = true;
// //             g.drag_sx  = vx; g.drag_sy = vy;
// //             g.drag_ox  = g.offset_x; g.drag_oy = g.offset_y;
// //         } else {
// //             g.cropping = true;
// //             g.has_crop = false;
// //             g.crop_x1  = vp_to_img_x(vx);
// //             g.crop_y1  = vp_to_img_y(vy);
// //             g.crop_x2  = g.crop_x1;
// //             g.crop_y2  = g.crop_y1;
// //         }
// //     }

// //     /* ── PRESSING ── */
// //     else if(code == LV_EVENT_PRESSING) {
// //         lv_point_t vect;
// //         lv_indev_get_vect(indev, &vect);
// //         lv_dir_t gesture_dir = lv_indev_get_gesture_dir(indev);

// //         bool is_pinch = (gesture_dir == LV_DIR_TOP  ||
// //                          gesture_dir == LV_DIR_BOTTOM) ||
// //                         (LV_ABS(vect.y) > 6 &&
// //                          LV_ABS(vect.x) < LV_ABS(vect.y) * 1.5f);

// //         if(is_pinch && g.mode == MODE_PAN) {
// //             int32_t step = (vect.y > 0) ? (LV_ABS(vect.y) * 3)
// //                                         : -(LV_ABS(vect.y) * 3);
// //             zoom_at(g.zoom + step, vx, vy);
// //             refresh_all();
// //             update_panel();
// //             return;
// //         }

// //         if(g.mode == MODE_PAN && g.dragging) {
// //             g.offset_x = g.drag_ox + (vx - g.drag_sx);
// //             g.offset_y = g.drag_oy + (vy - g.drag_sy);
// //             refresh_all();
// //             update_panel();
// //         } else if(g.mode == MODE_CROP && g.cropping) {
// //             g.crop_x2  = vp_to_img_x(vx);
// //             g.crop_y2  = vp_to_img_y(vy);
// //             g.has_crop = true;
// //             refresh_all();
// //             update_panel();
// //         }
// //     }

// //     /* ── RELEASED ── */
// //     else if(code == LV_EVENT_RELEASED) {
// //         if(g.mode == MODE_CROP && g.cropping) {
// //             g.cropping = false;
// //             float w = fabsf(g.crop_x2 - g.crop_x1);
// //             float h = fabsf(g.crop_y2 - g.crop_y1);
// //             g.has_crop = (w * h > 25.f);
// //         }
// //         g.dragging = false;
// //         g.cropping = false;
// //         refresh_all();
// //         update_panel();
// //     }

// //     /* ── GESTURE (swipe backup) ── */
// //     else if(code == LV_EVENT_GESTURE) {
// //         lv_dir_t dir = lv_indev_get_gesture_dir(indev);
// //         float cx = (float)VP_W * 0.5f;
// //         float cy = (float)VP_H * 0.5f;

// //         if(dir == LV_DIR_TOP)    zoom_at(g.zoom + ZOOM_STEP_GEST, cx, cy);
// //         if(dir == LV_DIR_BOTTOM) zoom_at(g.zoom - ZOOM_STEP_GEST, cx, cy);
// //         if(dir == LV_DIR_LEFT  && g.mode == MODE_PAN) g.offset_x -= 60.f;
// //         if(dir == LV_DIR_RIGHT && g.mode == MODE_PAN) g.offset_x += 60.f;

// //         refresh_all();
// //         update_panel();
// //     }
// // }

// // /* =========================================================
// //  * SCROLL-WHEEL ZOOM
// //  * ========================================================= */
// // void scroll_zoom_cb(lv_event_t *e)
// // {
// //     lv_event_code_t code  = lv_event_get_code(e);
// //     lv_indev_t     *indev = lv_event_get_indev(e);
// //     if(!indev || code != LV_EVENT_SCROLL) return;

// //     lv_point_t vect;
// //     lv_indev_get_vect(indev, &vect);
// //     int32_t delta = (vect.y < 0) ? ZOOM_STEP_BTN : -ZOOM_STEP_BTN;

// //     lv_point_t pt;
// //     lv_indev_get_point(indev, &pt);
// //     lv_area_t coords;
// //     lv_obj_get_coords(g.canvas_obj, &coords);

// //     zoom_at(g.zoom + delta,
// //             (float)(pt.x - coords.x1),
// //             (float)(pt.y - coords.y1));
// //     refresh_all();
// //     update_panel();
// // }

// // /* =========================================================
// //  * TOOLBAR CALLBACKS
// //  * ========================================================= */
// // void cb_pan (lv_event_t *e) { (void)e; set_mode(MODE_PAN);  }
// // void cb_crop(lv_event_t *e) { (void)e; set_mode(MODE_CROP); }

// // void cb_zin(lv_event_t *e)
// // {
// //     (void)e;
// //     zoom_at(g.zoom + ZOOM_STEP_BTN, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
// //     refresh_all(); update_panel();
// // }

// // void cb_zout(lv_event_t *e)
// // {
// //     (void)e;
// //     zoom_at(g.zoom - ZOOM_STEP_BTN, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
// //     refresh_all(); update_panel();
// // }

// // void cb_reset(lv_event_t *e)
// // {
// //     (void)e;
// //     reset_view();
// //     g.has_crop = false; g.cropping = false;
// //     refresh_all(); update_panel();
// // }

// // void cb_fit(lv_event_t *e)
// // {
// //     (void)e;
// //     fit_image();
// //     refresh_all(); update_panel();
// // }

// // void cb_clear(lv_event_t *e)
// // {
// //     (void)e;
// //     g.has_crop = false; g.cropping = false;
// //     refresh_all(); update_panel();
// // }

// // void cb_zoom10x(lv_event_t *e)
// // {
// //     (void)e;
// //     int32_t z10 = 10 * ZOOM_100;
// //     if(z10 > ZOOM_MAX) z10 = ZOOM_MAX;

// //     if(g.has_crop) {
// //         g.zoom = z10;
// //         float sc      = zoom_scale();
// //         float crop_cx = (g.crop_x1 + g.crop_x2) * 0.5f;
// //         float crop_cy = (g.crop_y1 + g.crop_y2) * 0.5f;
// //         g.offset_x = (float)VP_W * 0.5f - crop_cx * sc;
// //         g.offset_y = (float)VP_H * 0.5f - crop_cy * sc;
// //     } else {
// //         zoom_at(z10, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
// //     }
// //     refresh_all(); update_panel();
// // }


// /*==============================================================
//  * src/input/events.c
//  * Touch/gesture/scroll + toolbar callbacks for large virtual canvas
//  *==============================================================*/

// #include "viewer.h"
// #include "viewer_math.h"
// #include "viewer_ui.h"
// // #include "viewer_input.h"   // uncomment if you have this file

// /* Zoom step constants (now using float) */
// #define ZOOM_STEP_BTN   0.05f     // 5% per button click
// #define ZOOM_STEP_GEST  0.08f     // 8% per gesture

// /* =========================================================
//  * CANVAS TOUCH / GESTURE / DRAG
//  * ========================================================= */
// void canvas_event_cb(lv_event_t *e)
// {
//     lv_event_code_t code  = lv_event_get_code(e);
//     lv_indev_t     *indev = lv_event_get_indev(e);
//     if (!indev) return;

//     lv_point_t pt;
//     lv_indev_get_point(indev, &pt);

//     lv_area_t coords;
//     lv_obj_get_coords(g.canvas_obj, &coords);

//     float vx = (float)(pt.x - coords.x1);
//     float vy = (float)(pt.y - coords.y1);

//     /* Clamp to viewport */
//     if (vx < 0) vx = 0;
//     if (vx >= VP_W) vx = VP_W - 1;
//     if (vy < 0) vy = 0;
//     if (vy >= VP_H) vy = VP_H - 1;

//     g.cursor_vx = vx;
//     g.cursor_vy = vy;

//     /* PRESSED */
//     if (code == LV_EVENT_PRESSED) {
//         if (g.mode == MODE_PAN) {
//             g.dragging = true;
//             g.drag_sx  = vx;
//             g.drag_sy  = vy;
//             g.drag_ox  = g.offset_x;
//             g.drag_oy  = g.offset_y;
//         } else if (g.mode == MODE_CROP) {
//             g.cropping = true;
//             g.has_crop = false;
//             g.crop_x1  = vp_to_img_x(vx);
//             g.crop_y1  = vp_to_img_y(vy);
//             g.crop_x2  = g.crop_x1;
//             g.crop_y2  = g.crop_y1;
//         }
//     }

//     /* PRESSING (drag or crop) */
//     else if (code == LV_EVENT_PRESSING) {
//         if (g.mode == MODE_PAN && g.dragging) {
//             g.offset_x = g.drag_ox + (vx - g.drag_sx);
//             g.offset_y = g.drag_oy + (vy - g.drag_sy);
//             refresh_all();
//             update_panel();
//         }
//         else if (g.mode == MODE_CROP && g.cropping) {
//             g.crop_x2 = vp_to_img_x(vx);
//             g.crop_y2 = vp_to_img_y(vy);
//             g.has_crop = true;
//             refresh_all();
//             update_panel();
//         }
//     }

//     /* RELEASED */
//     else if (code == LV_EVENT_RELEASED) {
//         if (g.mode == MODE_CROP && g.cropping) {
//             g.cropping = false;
//             float w = fabsf(g.crop_x2 - g.crop_x1);
//             float h = fabsf(g.crop_y2 - g.crop_y1);
//             g.has_crop = (w * h > 25.0f);
//         }
//         g.dragging = false;
//         g.cropping = false;
//         refresh_all();
//         update_panel();
//     }

//     /* GESTURE (pinch zoom) */
//     else if (code == LV_EVENT_GESTURE) {
//         lv_dir_t dir = lv_indev_get_gesture_dir(indev);
//         float cx = (float)VP_W * 0.5f;
//         float cy = (float)VP_H * 0.5f;

//         if (dir == LV_DIR_TOP) {
//             zoom_at(g.zoom + ZOOM_STEP_GEST, cx, cy);
//         } else if (dir == LV_DIR_BOTTOM) {
//             zoom_at(g.zoom - ZOOM_STEP_GEST, cx, cy);
//         } else if (dir == LV_DIR_LEFT && g.mode == MODE_PAN) {
//             g.offset_x -= 60.0f;
//         } else if (dir == LV_DIR_RIGHT && g.mode == MODE_PAN) {
//             g.offset_x += 60.0f;
//         }

//         refresh_all();
//         update_panel();
//     }
// }

// /* =========================================================
//  * SCROLL WHEEL ZOOM
//  * ========================================================= */
// void scroll_zoom_cb(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code != LV_EVENT_SCROLL) return;

//     lv_indev_t *indev = lv_event_get_indev(e);
//     if (!indev) return;

//     lv_point_t vect;
//     lv_indev_get_vect(indev, &vect);

//     float delta = (vect.y < 0) ? ZOOM_STEP_BTN : -ZOOM_STEP_BTN;

//     lv_point_t pt;
//     lv_indev_get_point(indev, &pt);

//     lv_area_t coords;
//     lv_obj_get_coords(g.canvas_obj, &coords);

//     float vx = (float)(pt.x - coords.x1);
//     float vy = (float)(pt.y - coords.y1);

//     zoom_at(g.zoom + delta, vx, vy);
//     refresh_all();
//     update_panel();
// }

// /* =========================================================
//  * TOOLBAR BUTTON CALLBACKS
//  * ========================================================= */
// void cb_pan (lv_event_t *e) { (void)e; set_mode(MODE_PAN);  }
// void cb_crop(lv_event_t *e) { (void)e; set_mode(MODE_CROP); }

// void cb_zin(lv_event_t *e)
// {
//     (void)e;
//     zoom_at(g.zoom + ZOOM_STEP_BTN, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
//     refresh_all();
//     update_panel();
// }

// void cb_zout(lv_event_t *e)
// {
//     (void)e;
//     zoom_at(g.zoom - ZOOM_STEP_BTN, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
//     refresh_all();
//     update_panel();
// }

// void cb_reset(lv_event_t *e)
// {
//     (void)e;
//     reset_view();
//     g.has_crop = false;
//     g.cropping = false;
//     refresh_all();
//     update_panel();
// }

// void cb_fit(lv_event_t *e)
// {
//     (void)e;
//     fit_image();
//     refresh_all();
//     update_panel();
// }

// void cb_clear(lv_event_t *e)
// {
//     (void)e;
//     g.has_crop = false;
//     g.cropping = false;
//     refresh_all();
//     update_panel();
// }

// void cb_zoom10x(lv_event_t *e)
// {
//     (void)e;
//     float target_zoom = 10.0f;   // 10x zoom

//     if (g.has_crop) {
//         /* Zoom into crop center */
//         float crop_cx = (g.crop_x1 + g.crop_x2) * 0.5f;
//         float crop_cy = (g.crop_y1 + g.crop_y2) * 0.5f;
//         g.zoom = target_zoom;
//         float sc = zoom_scale();
//         g.offset_x = (float)VP_W * 0.5f - crop_cx * sc;
//         g.offset_y = (float)VP_H * 0.5f - crop_cy * sc;
//     } else {
//         zoom_at(target_zoom, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
//     }

//     refresh_all();
//     update_panel();
// }


/*==============================================================
 * src/input/events.c
 * Touch/gesture/scroll + ALL toolbar callbacks.
 *
 * NEW vs original:
 *  • Drawing modes: MODE_LINE, MODE_ARC, MODE_BOX, MODE_EDIT
 *  • canvas_event_cb routes PRESSED/PRESSING/RELEASED to the
 *    correct handler based on g.mode.
 *  • Bottom toolbar: cb_tool_line, cb_tool_arc, cb_tool_box,
 *    cb_tool_edit, cb_tool_undo, cb_tool_redo.
 *  • Arc draw is 3-tap: start → control point → end.
 *==============================================================*/
/*==============================================================
 * src/input/events.c
 * Touch/gesture/scroll + ALL toolbar callbacks.
 *
 * NEW vs original:
 *  • Drawing modes: MODE_LINE, MODE_ARC, MODE_BOX, MODE_EDIT
 *  • canvas_event_cb routes PRESSED/PRESSING/RELEASED to the
 *    correct handler based on g.mode.
 *  • Bottom toolbar: cb_tool_line, cb_tool_arc, cb_tool_box,
 *    cb_tool_edit, cb_tool_undo, cb_tool_redo.
 *  • Arc draw is 3-tap: start → control point → end.
 *==============================================================*/
/*==============================================================
 * src/input/events.c
 * Touch/gesture/scroll + ALL toolbar callbacks.
 *
 * NEW vs original:
 *  • Drawing modes: MODE_LINE, MODE_ARC, MODE_BOX, MODE_EDIT
 *  • canvas_event_cb routes PRESSED/PRESSING/RELEASED to the
 *    correct handler based on g.mode.
 *  • Bottom toolbar: cb_tool_line, cb_tool_arc, cb_tool_box,
 *    cb_tool_edit, cb_tool_undo, cb_tool_redo.
 *  • Arc draw is 3-tap: start → control point → end.
 *==============================================================*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> 
#include <string.h>

#include "viewer.h"
#include "viewer_math.h"
#include "viewer_ui.h"
#include "viewer_input.h"
#include "viewer_fill.h"
#include "color_picker.h"
/* Gesture API (pinch scale) */
#include "../../lvgl/src/indev/lv_indev_gesture.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Zoom step constants */
#define ZOOM_STEP_BTN   0.05f
#define ZOOM_STEP_GEST  0.08f

/* Hit-test radius for node editing (screen pixels) */
#define EDIT_HIT_R   12.0f

/* =========================================================
 * INTERNAL: add a new draw object
 * ========================================================= */
static void add_draw_object(DrawObjType type,
                             float x1, float y1,
                             float x2, float y2,
                             float cx, float cy,
                             int width)
{
    if (g.draw_count >= MAX_DRAW_OBJECTS) return;
    DrawObject *o = &g.draw_objects[g.draw_count];
    o->type     = type;
    o->x1 = x1; o->y1 = y1;
    o->x2 = x2; o->y2 = y2;
    o->cx = cx; o->cy = cy;
    o->width = width;
    o->vert_count = 0;
    o->selected = false;
    g.draw_count++;
    g.undo_top = 0;   /* clear redo stack */
}

/* NEW: pipe variant — stores wall offset (diameter) alongside the line */
static void add_pipe_object(float x1, float y1, float x2, float y2, float diameter, int width)
{
    if (g.draw_count >= MAX_DRAW_OBJECTS) return;
    DrawObject *o = &g.draw_objects[g.draw_count];
    o->type          = OBJ_PIPE;
    o->x1 = x1; o->y1 = y1;
    o->x2 = x2; o->y2 = y2;
    o->cx = 0;  o->cy = 0;
    o->width         = width;
    o->pipe_diameter = diameter;
    o->vert_count    = 0;
    o->selected      = false;
    g.draw_count++;
    g.undo_top = 0;
}

/* forged variant — commits the accumulated tap-polygon */
static void add_forged_object(void)
{
    if (g.forged_vcount < 3 || g.draw_count >= MAX_DRAW_OBJECTS) return;
    DrawObject *o = &g.draw_objects[g.draw_count];
    o->type       = OBJ_FORGED;
    o->vert_count = g.forged_vcount;
    for (int i = 0; i < g.forged_vcount; i++) {
        o->verts_x[i] = g.forged_vx[i];
        o->verts_y[i] = g.forged_vy[i];
    }
    o->x1 = g.forged_vx[0]; o->y1 = g.forged_vy[0];
    o->x2 = g.forged_vx[g.forged_vcount - 1];
    o->y2 = g.forged_vy[g.forged_vcount - 1];
    o->width    = (int)g.brush_size;
    o->selected = false;
    o->fill_visible = false; 
    g.draw_count++;
    g.undo_top  = 0;
    g.forged_vcount = 0;
}

#define FORGED_ARC_SAMPLES 12

static int flatten_seg_chain_to_verts(const ForgedSegment *segs, int seg_count,
                                       float *out_x, float *out_y, int max_verts)
{
    int n = 0;
    if (seg_count <= 0 || max_verts <= 0) return 0;

    out_x[n] = segs[0].x1;
    out_y[n] = segs[0].y1;
    n++;

    for (int i = 0; i < seg_count; i++) {
        const ForgedSegment *seg = &segs[i];

        if (seg->type == FSEG_LINE || seg->radius <= 0.001f) {
            if (n < max_verts) {
                out_x[n] = seg->x2;
                out_y[n] = seg->y2;
                n++;
            }
            continue;
        }

        float ang1 = atan2f(seg->y1 - seg->cy, seg->x1 - seg->cx);
        float ang2 = atan2f(seg->y2 - seg->cy, seg->x2 - seg->cx);

        if (seg->direction >= 0) {
            if (ang2 < ang1) ang2 += 2.0f * (float)M_PI;
        } else {
            if (ang2 > ang1) ang2 -= 2.0f * (float)M_PI;
        }

        for (int s = 1; s <= FORGED_ARC_SAMPLES && n < max_verts; s++) {
            float t = (float)s / (float)FORGED_ARC_SAMPLES;
            float a = ang1 + (ang2 - ang1) * t;
            out_x[n] = seg->cx + seg->radius * cosf(a);
            out_y[n] = seg->cy + seg->radius * sinf(a);
            n++;
        }
    }
    return n;
}

/* Re-derives a committed OBJ_FORGED's fill/hatch/bounds after any of
 * its seg_data[] entries change (used by Edit-mode node dragging). */
void forged_resync_object(DrawObject *o)
{
    if (o->seg_count <= 0) return;
    o->vert_count = flatten_seg_chain_to_verts(o->seg_data, o->seg_count,
                                                o->verts_x, o->verts_y, MAX_POLY_VERTS);
    o->x1 = o->seg_data[0].x1;
    o->y1 = o->seg_data[0].y1;
    o->x2 = o->seg_data[o->seg_count - 1].x2;
    o->y2 = o->seg_data[o->seg_count - 1].y2;
}



static inline float mid_coord(float a, float b)
{
    return 0.5f * (a + b);
}

static inline bool point_near(float x1, float y1,
                              float x2, float y2,
                              float radius)
{
    float dx = x1 - x2;
    float dy = y1 - y2;
    return (dx * dx + dy * dy) <= radius * radius;
}

static inline bool approx_equal(float a, float b)
{
    return fabsf(a - b) <= 3.0f;
}

static inline bool range_covers(float a1, float a2, float b1, float b2)
{
    if (a1 > a2) { float t = a1; a1 = a2; a2 = t; }
    if (b1 > b2) { float t = b1; b1 = b2; b2 = t; }
    return b1 <= a2 + 3.0f && b2 >= a1 - 3.0f;
}

static inline float orient2d(float px, float py,
                              float qx, float qy,
                              float rx, float ry)
{
    return (qx - px) * (ry - py) - (qy - py) * (rx - px);
}

static inline bool on_segment(float px, float py,
                               float qx, float qy,
                               float rx, float ry)
{
    return (qx <= fmaxf(px, rx) + 0.01f) && (qx >= fminf(px, rx) - 0.01f) &&
           (qy <= fmaxf(py, ry) + 0.01f) && (qy >= fminf(py, ry) - 0.01f);
}

/* Standard ray-casting point-in-polygon test, image-space coords. */
static bool point_in_polygon(float px, float py, const float *vx, const float *vy, int n)
{
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((vy[i] > py) != (vy[j] > py)) &&
            (px < (vx[j] - vx[i]) * (py - vy[i]) / (vy[j] - vy[i]) + vx[i])) {
            inside = !inside;
        }
    }
    return inside;
}

// /* Mirrors the exact "closed" test draw.c uses when rendering
//  * OBJ_FORGED, so a shape that visually looks closed is exactly the
//  * shape that's clickable for Pattern Fill — and vice versa. */
// static bool forged_object_is_closed(const DrawObject *o)
// {
//     if (o->type != OBJ_FORGED || o->vert_count < 2) return false;
//     float dx = o->verts_x[o->vert_count - 1] - o->verts_x[0];
//     float dy = o->verts_y[o->vert_count - 1] - o->verts_y[0];
//     float dist_vp = sqrtf(dx * dx + dy * dy) * zoom_scale();
//     return dist_vp < 3.0f;
// }

static bool segments_intersect(float ax1, float ay1, float ax2, float ay2,
                                float bx1, float by1, float bx2, float by2)
{
    float o1 = orient2d(ax1, ay1, ax2, ay2, bx1, by1);
    float o2 = orient2d(ax1, ay1, ax2, ay2, bx2, by2);
    float o3 = orient2d(bx1, by1, bx2, by2, ax1, ay1);
    float o4 = orient2d(bx1, by1, bx2, by2, ax2, ay2);

    if (((o1 > 0) != (o2 > 0)) && ((o3 > 0) != (o4 > 0)) &&
        o1 != 0.0f && o2 != 0.0f && o3 != 0.0f && o4 != 0.0f) {
        return true;
    }
    if (fabsf(o1) < 1e-3f && on_segment(ax1, ay1, bx1, by1, ax2, ay2)) return true;
    if (fabsf(o2) < 1e-3f && on_segment(ax1, ay1, bx2, by2, ax2, ay2)) return true;
    if (fabsf(o3) < 1e-3f && on_segment(bx1, by1, ax1, ay1, bx2, by2)) return true;
    if (fabsf(o4) < 1e-3f && on_segment(bx1, by1, ax2, ay2, bx2, by2)) return true;
    return false;
}

#define VALIDATE_ARC_SAMPLES 32
#define VALIDATE_MAX_PTS     (VALIDATE_ARC_SAMPLES + 1)

static int flatten_object(const DrawObject *o, float *out_x, float *out_y)
{
    if (o->type == OBJ_LINE) {
        out_x[0] = o->x1; out_y[0] = o->y1;
        out_x[1] = o->x2; out_y[1] = o->y2;
        return 2;
    }
    if (o->type == OBJ_ARC) {
        for (int s = 0; s <= VALIDATE_ARC_SAMPLES; s++) {
            float t = (float)s / (float)VALIDATE_ARC_SAMPLES, mt = 1.0f - t;
            out_x[s] = mt * mt * o->x1 + 2.0f * mt * t * o->cx + t * t * o->x2;
            out_y[s] = mt * mt * o->y1 + 2.0f * mt * t * o->cy + t * t * o->y2;
        }
        return VALIDATE_ARC_SAMPLES + 1;
    }
    if (o->type == OBJ_BOX) {
        out_x[0]=o->x1; out_y[0]=o->y1; out_x[1]=o->x2; out_y[1]=o->y1;
        out_x[2]=o->x2; out_y[2]=o->y2; out_x[3]=o->x1; out_y[3]=o->y2;
        out_x[4]=o->x1; out_y[4]=o->y1;
        return 5;
    }
    return 0;
}

static bool objects_intersect(const DrawObject *a, const DrawObject *b)
{
    float ax[VALIDATE_MAX_PTS], ay[VALIDATE_MAX_PTS];
    float bx[VALIDATE_MAX_PTS], by[VALIDATE_MAX_PTS];
    int na = flatten_object(a, ax, ay);
    int nb = flatten_object(b, bx, by);
    if (na < 2 || nb < 2) return false;

    for (int i = 0; i < na - 1; i++)
        for (int j = 0; j < nb - 1; j++)
            if (segments_intersect(ax[i], ay[i], ax[i+1], ay[i+1],
                                    bx[j], by[j], bx[j+1], by[j+1]))
                return true;
    return false;
}

static bool endpoint_connected_to_any(float ex, float ey, int skip_idx, float tol)
{
    float tol2 = tol * tol;
    for (int j = 0; j < g.draw_count; j++) {
        if (j == skip_idx) continue;
        DrawObject *o = &g.draw_objects[j];
        if (o->type != OBJ_LINE && o->type != OBJ_ARC) continue;
        float d1x = ex - o->x1, d1y = ey - o->y1;
        float d2x = ex - o->x2, d2y = ey - o->y2;
        if (d1x*d1x + d1y*d1y <= tol2) return true;
        if (d2x*d2x + d2y*d2y <= tol2) return true;
    }
    return false;
}

__attribute__((unused))
static int find_box_at(float ix, float iy)
{
    for (int i = g.draw_count - 1; i >= 0; i--) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type != OBJ_BOX) continue;
        float x1 = LV_MIN(o->x1, o->x2);
        float y1 = LV_MIN(o->y1, o->y2);
        float x2 = LV_MAX(o->x1, o->x2);
        float y2 = LV_MAX(o->y1, o->y2);
        if (ix >= x1 && ix <= x2 && iy >= y1 && iy <= y2) {
            return i;
        }
    }
    return -1;
}

/* Unused: These functions were replaced by flood fill implementation
   Kept for reference but marked as unused to suppress warnings */
__attribute__((unused))
static bool find_closed_rectangle_outline(float ix, float iy,
                                          float *out_x1, float *out_y1,
                                          float *out_x2, float *out_y2)
{
    float xs[MAX_DRAW_OBJECTS * 2];
    float ys[MAX_DRAW_OBJECTS * 2];
    int nx = 0, ny = 0;

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type != OBJ_LINE) continue;
        xs[nx++] = o->x1; xs[nx++] = o->x2;
        ys[ny++] = o->y1; ys[ny++] = o->y2;
    }

    if (nx < 8 || ny < 8) return false;

    /* Unique coordinates with tolerance */
    for (int i = 0; i < nx; i++) {
        for (int j = i + 1; j < nx; ) {
            if (approx_equal(xs[i], xs[j])) {
                xs[j] = xs[--nx];
            } else j++;
        }
    }
    for (int i = 0; i < ny; i++) {
        for (int j = i + 1; j < ny; ) {
            if (approx_equal(ys[i], ys[j])) {
                ys[j] = ys[--ny];
            } else j++;
        }
    }

    for (int xi = 0; xi < nx; xi++) {
        for (int xj = xi + 1; xj < nx; xj++) {
            float rx1 = LV_MIN(xs[xi], xs[xj]);
            float rx2 = LV_MAX(xs[xi], xs[xj]);
            if (!(ix >= rx1 && ix <= rx2)) continue;
            for (int yi = 0; yi < ny; yi++) {
                for (int yj = yi + 1; yj < ny; yj++) {
                    float ry1 = LV_MIN(ys[yi], ys[yj]);
                    float ry2 = LV_MAX(ys[yi], ys[yj]);
                    if (!(iy >= ry1 && iy <= ry2)) continue;

                    bool has_top = false;
                    bool has_bot = false;
                    bool has_left = false;
                    bool has_right = false;

                    for (int i = 0; i < g.draw_count; i++) {
                        DrawObject *o = &g.draw_objects[i];
                        if (o->type != OBJ_LINE) continue;

                        if (approx_equal(o->y1, o->y2)) {
                            /* horizontal */
                            if (approx_equal(o->y1, ry1) && range_covers(rx1, rx2, o->x1, o->x2)) {
                                has_top = true;
                            }
                            if (approx_equal(o->y1, ry2) && range_covers(rx1, rx2, o->x1, o->x2)) {
                                has_bot = true;
                            }
                        }
                        if (approx_equal(o->x1, o->x2)) {
                            /* vertical */
                            if (approx_equal(o->x1, rx1) && range_covers(ry1, ry2, o->y1, o->y2)) {
                                has_left = true;
                            }
                            if (approx_equal(o->x1, rx2) && range_covers(ry1, ry2, o->y1, o->y2)) {
                                has_right = true;
                            }
                        }
                    }

                    if (has_top && has_bot && has_left && has_right) {
                        *out_x1 = rx1;
                        *out_y1 = ry1;
                        *out_x2 = rx2;
                        *out_y2 = ry2;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

static void apply_crop_selection(void)
{
    if (!g.has_crop) return;

    float x1 = LV_MIN(g.crop_x1, g.crop_x2);
    float y1 = LV_MIN(g.crop_y1, g.crop_y2);
    float x2 = LV_MAX(g.crop_x1, g.crop_x2);
    float y2 = LV_MAX(g.crop_y1, g.crop_y2);
    float sw = x2 - x1;
    float sh = y2 - y1;
    if (sw < 4.0f || sh < 4.0f) return;

    float sx = (float)VP_W / sw;
    float sy = (float)VP_H / sh;
    float s  = LV_MIN(sx, sy);
    g.zoom = clamp_zoom(s);

    float sc = zoom_scale();
    float cx = x1 + sw * 0.5f;
    float cy = y1 + sh * 0.5f;
    g.offset_x = (float)VP_W * 0.5f - cx * sc;
    g.offset_y = (float)VP_H * 0.5f - cy * sc;

    g.has_crop  = false;
    g.cropping  = false;
    g.crop_x1   = 0; g.crop_y1 = 0;
    g.crop_x2   = 0; g.crop_y2 = 0;
}

/* =========================================================
 * INTERNAL: find nearest draw object (edit mode)
 * ========================================================= */
static int find_nearest_object(float vx, float vy)
{
    float best_d = EDIT_HIT_R * EDIT_HIT_R;
    int   best   = -1;

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];
        /* Check endpoints */
        float ex1 = img_to_vp_x(o->x1), ey1 = img_to_vp_y(o->y1);
        float ex2 = img_to_vp_x(o->x2), ey2 = img_to_vp_y(o->y2);
        float d1  = (vx - ex1) * (vx - ex1) + (vy - ey1) * (vy - ey1);
        float d2  = (vx - ex2) * (vx - ex2) + (vy - ey2) * (vy - ey2);
        float d   = LV_MIN(d1, d2);
        if (d < best_d) { best_d = d; best = i; }
    }
    return best;
}

static void compute_arc_center(ForgedSegment *seg);
static void forged_dialog_abort_event(lv_event_t *e);
static void update_forged_start_label(void);
bool forged_build_undo(void);

static PatternFillType g_pattern_fill_selected = PATTERN_FILL_LINE;
static lv_obj_t       *g_pattern_menu = NULL;

/* =========================================================
 * CANVAS TOUCH / GESTURE / DRAG
 * ========================================================= */
void canvas_event_cb(lv_event_t *e)
{
    lv_event_code_t code  = lv_event_get_code(e);
    lv_indev_t     *indev = lv_event_get_indev(e);
    if (!indev) return;

    lv_point_t pt;
    lv_indev_get_point(indev, &pt);

    lv_area_t coords;
    lv_obj_get_coords(g.canvas_obj, &coords);

    float vx = (float)(pt.x - coords.x1);
    float vy = (float)(pt.y - coords.y1);
    if (vx < 0) vx = 0;
    if (vx >= VP_W) vx = VP_W - 1;
    if (vy < 0) vy = 0;
    if (vy >= VP_H) vy = VP_H - 1;

    g.cursor_vx = vx;
    g.cursor_vy = vy;

    /* Image-space position under cursor */
    float ix = vp_to_img_x(vx);
    float iy = vp_to_img_y(vy);

    if (ix < 0.0f) ix = 0.0f;
    if (iy < 0.0f) iy = 0.0f;
    if (ix > (float)VIRTUAL_W) ix = (float)VIRTUAL_W;
    if (iy > (float)VIRTUAL_H) iy = (float)VIRTUAL_H;

    /* ──────────────────────────────────────
     * PRESSED
     * ────────────────────────────────────── */
    if (code == LV_EVENT_PRESSED) {

        switch (g.mode) {

            /* ── Pan ── */
            case MODE_PAN:
                g.dragging = true;
                g.drag_sx  = vx; g.drag_sy = vy;
                g.drag_ox  = g.offset_x; g.drag_oy = g.offset_y;
                break;

            /* ── Crop ── */
            case MODE_CROP:
                g.cropping = true;
                g.has_crop = false;
                g.crop_x1  = ix; g.crop_y1 = iy;
                g.crop_x2  = ix; g.crop_y2 = iy;
                break;

            /* ── Straight Line: tap 1 = start, tap 2 = end ── */
            case MODE_LINE:
                if (!g.drawing) {
                    g.drawing    = true;
                    g.draw_x1    = ix; g.draw_y1 = iy;
                    g.draw_x2    = ix; g.draw_y2 = iy;
                    g.draw_phase = 1;
                } else {
                    add_draw_object(OBJ_LINE,
                                    g.draw_x1, g.draw_y1,
                                    ix, iy, 0, 0,
                                    (int)g.brush_size);
                    g.drawing = false;
                    g.draw_phase = 0;
                    refresh_all();
                    update_panel();
                }
                break;

            /* ── Arc/Curve: press-drag-release to create a curved line
             *    or drag an existing curve handle to reshape it. ── */
            case MODE_ARC: {
                if (!g.drawing) {
                    int found = -1;
                    float best_d = EDIT_HIT_R * EDIT_HIT_R;
                    for (int i = 0; i < g.draw_count; i++) {
                        DrawObject *o = &g.draw_objects[i];
                        if (o->type != OBJ_ARC) continue;
                        float hx = img_to_vp_x(o->cx);
                        float hy = img_to_vp_y(o->cy);
                        float dx = vx - hx;
                        float dy = vy - hy;
                        float d2 = dx * dx + dy * dy;
                        if (d2 < best_d) {
                            best_d = d2;
                            found = i;
                        }
                    }
                    if (found >= 0) {
                        g.selected_obj = found;
                        g.curve_dragging = true;
                        break;
                    }

                    g.curve_dragging = false;
                    g.drawing    = true;
                    g.draw_x1    = ix; g.draw_y1 = iy;
                    g.draw_x2    = ix; g.draw_y2 = iy;
                    g.draw_cx    = ix; g.draw_cy = iy;
                    g.draw_phase = 1;
                }
                break;
            }

            /* ── Box: drag corner → corner ── */
            case MODE_BOX:
                g.drawing  = true;
                g.draw_x1  = ix; g.draw_y1 = iy;
                g.draw_phase = 1;
                g.crop_x1  = ix; g.crop_y1 = iy;
                g.crop_x2  = ix; g.crop_y2 = iy;
                break;

            /* ── Fill: paint bucket flood fill with gap detection ── */
            // case MODE_FILL: {
            //     /* Perform flood fill from clicked position */
            //     if (viewer_flood_fill((float)ix, (float)iy, g.fill_color, g.fill_tolerance)) {
            //         refresh_all();
            //         update_panel();
            //     } else {
            //         /* Flood fill did not complete - detect gaps in closed area */
            //         GapDetectionResult gaps = viewer_detect_gaps((float)ix - 50, (float)iy - 50,
            //                                                        (float)ix + 50, (float)iy + 50, (float)g.fill_tolerance);
            //         if (gaps.has_gaps) {
            //             g.has_gaps = true;
            //             g.gap_count = gaps.count;
            //             /* Highlight gaps to user */
            //             viewer_highlight_gaps(&gaps, 0xFF6B6B);
            //             printf("[FILL] Detected %d gaps - shape may not be closed!\n", gaps.count);
            //         }
            //         refresh_all();
            //         update_panel();
            //     }
            //     break;
            // }

            /* ── NEW: Bar — same drag-rectangle gesture as Box ── */
            case MODE_BAR:
                g.drawing    = true;
                g.draw_x1    = ix; g.draw_y1 = iy;
                g.draw_phase = 1;
                g.crop_x1    = ix; g.crop_y1 = iy;   /* reuse crop coords for live preview */
                g.crop_x2    = ix; g.crop_y2 = iy;
                break;

            /* ── NEW: Pipe — drag from centerline start to end; release sets diameter ── */
            case MODE_PIPE:
                g.drawing    = true;
                g.draw_x1    = ix; g.draw_y1 = iy;
                g.draw_x2    = ix; g.draw_y2 = iy;
                g.draw_phase = 1;
                break;

            /* ── NEW: Forged — tap to add vertex; tap near first vertex closes shape ── */
            case MODE_FORGED: {
                if (g.forged_vcount >= 3) {
                    float fx = g.forged_vx[0], fy = g.forged_vy[0];
                    float fvx = img_to_vp_x(fx), fvy = img_to_vp_y(fy);
                    if (point_near(vx, vy, fvx, fvy, EDIT_HIT_R)) {
                        add_forged_object();
                        refresh_all();
                        update_panel();
                        break;
                    }
                }
                if (g.forged_vcount < MAX_POLY_VERTS) {
                    g.forged_vx[g.forged_vcount] = ix;
                    g.forged_vy[g.forged_vcount] = iy;
                    g.forged_vcount++;
                }
                refresh_all();
                break;
            }

            case MODE_FORGED_CENTER: {
                
                break;
            }

            case MODE_FILL: {
                /*
                 * Clear stale gap markers before every fill attempt.
                 * viewer_flood_fill() will re-detect and store new ones
                 * if the fill fails due to an open shape.
                 */
                g.gap_pts_count = 0;
                g.has_gaps      = false;
                g.gap_count     = 0;
 
                /* Attempt fill — internally detects gaps on failure */
                viewer_flood_fill((float)ix, (float)iy,
                                  g.fill_color, g.fill_tolerance);
 
                /* Always refresh so draw.c can render/clear markers */
                refresh_all();
                update_panel();
                break;
            }

            /* ── Pattern Fill: tap inside a closed (or centerline-closed)
             * Forged profile ── */
            case MODE_PATTERN_FILL: {
                int found = -1;
                for (int i = g.draw_count - 1; i >= 0; i--) {
                    DrawObject *o = &g.draw_objects[i];
                    if (o->type != OBJ_FORGED) continue;

                    float region_x[MAX_POLY_VERTS + 2];
                    float region_y[MAX_POLY_VERTS + 2];
                    int region_n = 0;
                    if (!forged_closed_polygon_image(o, region_x, region_y, &region_n)) continue;

                    if (point_in_polygon(ix, iy, region_x, region_y, region_n)) {
                        found = i;
                        break;
                    }
                }
                if (found >= 0) {
                    g.draw_objects[found].pattern_fill = g_pattern_fill_selected;
                    g.draw_objects[found].fill_visible =
                                (g_pattern_fill_selected != PATTERN_FILL_NONE);
                    if (g.hint_lbl) {
                        lv_label_set_text(g.hint_lbl,
                            g_pattern_fill_selected == PATTERN_FILL_ZIGZAG ? "Pattern Fill: Zig-Zag applied" :
                            g_pattern_fill_selected == PATTERN_FILL_LINE   ? "Pattern Fill: Straight Line applied" :
                                                                              "Pattern Fill: cleared");
                    }
                } else if (g.hint_lbl) {
                    lv_label_set_text(g.hint_lbl,
                        "Pattern Fill  |  Tap inside a closed Forged profile");
                }
                refresh_all();
                update_panel();
                break;
            }

            
            /* ── Edit: select nearest object ── */
            case MODE_EDIT: {
                /* First pass: check every committed OBJ_FORGED's internal
                 * segment nodes — these aren't covered by
                 * find_nearest_object()'s whole-object endpoint check. */
                int forged_obj = -1, forged_seg = -1, forged_point = 0;
                {
                    float best_d = EDIT_HIT_R * EDIT_HIT_R;
                    for (int oi = 0; oi < g.draw_count; oi++) {
                        DrawObject *fo = &g.draw_objects[oi];
                        if (fo->type != OBJ_FORGED) continue;
                        for (int si = 0; si < fo->seg_count; si++) {
                            ForgedSegment *seg = &fo->seg_data[si];
                            float sx1 = img_to_vp_x(seg->x1), sy1 = img_to_vp_y(seg->y1);
                            float sx2 = img_to_vp_x(seg->x2), sy2 = img_to_vp_y(seg->y2);
                            float d1 = (vx - sx1) * (vx - sx1) + (vy - sy1) * (vy - sy1);
                            float d2 = (vx - sx2) * (vx - sx2) + (vy - sy2) * (vy - sy2);
                            if (d1 < best_d) { best_d = d1; forged_obj = oi; forged_seg = si; forged_point = 1; }
                            if (d2 < best_d) { best_d = d2; forged_obj = oi; forged_seg = si; forged_point = 3; }
                        }
                    }
                }

                if (g.selected_obj >= 0 && g.selected_obj < g.draw_count)
                    g.draw_objects[g.selected_obj].selected = false;

                g.edit_dragging   = false;
                g.edit_drag_point = 0;
                g.edit_forged_seg = -1;

                if (forged_obj >= 0) {
                    g.selected_obj = forged_obj;
                    g.draw_objects[forged_obj].selected = true;
                    g.edit_dragging   = true;
                    g.edit_drag_point = forged_point;
                    g.edit_forged_seg = forged_seg;
                    refresh_all();
                    update_panel();
                    break;
                }

                int found = find_nearest_object(vx, vy);
                g.selected_obj = found;
                if (found >= 0) {
                    DrawObject *o = &g.draw_objects[found];
                    o->selected = true;
                    if (o->type == OBJ_ARC) {
                        float px1 = img_to_vp_x(o->x1);
                        float py1 = img_to_vp_y(o->y1);
                        float pcx = img_to_vp_x(o->cx);
                        float pcy = img_to_vp_y(o->cy);
                        float px2 = img_to_vp_x(o->x2);
                        float py2 = img_to_vp_y(o->y2);
                        if (point_near(vx, vy, pcx, pcy, EDIT_HIT_R)) {
                            g.edit_dragging = true;
                            g.edit_drag_point = 2;
                        } else if (point_near(vx, vy, px1, py1, EDIT_HIT_R)) {
                            g.edit_dragging = true;
                            g.edit_drag_point = 1;
                        } else if (point_near(vx, vy, px2, py2, EDIT_HIT_R)) {
                            g.edit_dragging = true;
                            g.edit_drag_point = 3;
                        }
                    } else {
                        float px1 = img_to_vp_x(o->x1);
                        float py1 = img_to_vp_y(o->y1);
                        float px2 = img_to_vp_x(o->x2);
                        float py2 = img_to_vp_y(o->y2);
                        if (point_near(vx, vy, px1, py1, EDIT_HIT_R)) {
                            g.edit_dragging = true;
                            g.edit_drag_point = 1;
                        } else if (point_near(vx, vy, px2, py2, EDIT_HIT_R)) {
                            g.edit_dragging = true;
                            g.edit_drag_point = 3;
                        }
                    }
                }
                refresh_all();
                update_panel();
                break;
            }

            default: break;
        }
    }

    /* ──────────────────────────────────────
     * PRESSING  (drag update)
     * ────────────────────────────────────── */
    else if (code == LV_EVENT_PRESSING) {

        switch (g.mode) {

            case MODE_PAN:
                if (g.dragging) {
                    g.offset_x = g.drag_ox + (vx - g.drag_sx);
                    g.offset_y = g.drag_oy + (vy - g.drag_sy);
                    refresh_all();
                    update_panel();
                }
                break;

            case MODE_CROP:
                if (g.cropping) {
                    g.crop_x2 = ix; g.crop_y2 = iy;
                    g.has_crop = true;
                    refresh_all();
                    update_panel();
                }
                break;

            case MODE_BOX:
                if (g.drawing) {
                    /* Reuse crop coords for live box preview */
                    g.crop_x2 = ix; g.crop_y2 = iy;
                    refresh_all();
                }
                break;

            /* ── NEW: Bar live preview (reuse crop coords like Box does) ── */
            case MODE_BAR:
                if (g.drawing) {
                    g.crop_x2 = ix; g.crop_y2 = iy;
                    refresh_all();
                }
                break;

            /* ── NEW: Pipe live preview — second point sets length/angle ── */
            case MODE_PIPE:
                if (g.drawing && g.draw_phase == 1) {
                    g.draw_x2 = ix;
                    g.draw_y2 = iy;
                    refresh_all();
                }
                break;

            case MODE_ARC:
                if (g.curve_dragging && g.selected_obj >= 0 && g.selected_obj < g.draw_count) {
                    DrawObject *o = &g.draw_objects[g.selected_obj];
                    o->cx = ix;
                    o->cy = iy;
                    refresh_all();
                    update_panel();
                } else if (g.drawing && g.draw_phase == 1) {
                    g.draw_x2 = ix;
                    g.draw_y2 = iy;
                    refresh_all();
                    update_panel();
                }
                break;

            case MODE_EDIT:
                if (g.edit_dragging && g.selected_obj >= 0 && g.selected_obj < g.draw_count) {
                    DrawObject *o = &g.draw_objects[g.selected_obj];

                    if (o->type == OBJ_FORGED && g.edit_forged_seg >= 0 &&
                        g.edit_forged_seg < o->seg_count) {
                        ForgedSegment *seg = &o->seg_data[g.edit_forged_seg];

                        if (g.edit_drag_point == 1) {
                            seg->x1 = ix; seg->y1 = iy;
                            if (g.edit_forged_seg > 0) {
                                ForgedSegment *prev = &o->seg_data[g.edit_forged_seg - 1];
                                prev->x2 = ix; prev->y2 = iy;
                                if (prev->type == FSEG_ARC) compute_arc_center(prev);
                            }
                        } else if (g.edit_drag_point == 3) {
                            seg->x2 = ix; seg->y2 = iy;
                            if (g.edit_forged_seg < o->seg_count - 1) {
                                ForgedSegment *next = &o->seg_data[g.edit_forged_seg + 1];
                                next->x1 = ix; next->y1 = iy;
                                if (next->type == FSEG_ARC) compute_arc_center(next);
                            }
                        }
                        if (seg->type == FSEG_ARC) compute_arc_center(seg);
                        forged_resync_object(o);
                        refresh_all();
                        update_panel();
                        break;
                    }

                    if (g.edit_drag_point == 1) {
                        o->x1 = ix;
                        o->y1 = iy;
                    } else if (g.edit_drag_point == 2 && o->type == OBJ_ARC) {
                        o->cx = ix;
                        o->cy = iy;
                    } else if (g.edit_drag_point == 3) {
                        o->x2 = ix;
                        o->y2 = iy;
                    }
                    refresh_all();
                    update_panel();
                }
                break;

            /* For LINE just refresh to show ghost */
            case MODE_LINE:
                if (g.drawing && g.draw_phase == 1) {
                    g.draw_x2 = ix;
                    g.draw_y2 = iy;
                }
                refresh_all();
                break;

            default: break;
        }
    }

    /* ──────────────────────────────────────
     * RELEASED
     * ────────────────────────────────────── */
    else if (code == LV_EVENT_RELEASED) {

        switch (g.mode) {

            case MODE_CROP:
                if (g.cropping) {
                    g.cropping = false;
                    float w = fabsf(g.crop_x2 - g.crop_x1);
                    float h = fabsf(g.crop_y2 - g.crop_y1);
                    g.has_crop = (w * h > 25.0f);
                }
                break;

            case MODE_ARC:
                if (g.curve_dragging) {
                    g.curve_dragging = false;
                    g.selected_obj = -1;
                } else if (g.drawing && g.draw_phase == 1) {
                    float dx = fabsf(ix - g.draw_x1);
                    float dy = fabsf(iy - g.draw_y1);
                    if (dx > 3.0f || dy > 3.0f) {
                        g.draw_x2 = ix;
                        g.draw_y2 = iy;
                        g.draw_cx = 0.5f * (g.draw_x1 + g.draw_x2);
                        g.draw_cy = 0.5f * (g.draw_y1 + g.draw_y2);
                        add_draw_object(OBJ_ARC,
                                        g.draw_x1, g.draw_y1,
                                        g.draw_x2, g.draw_y2,
                                        g.draw_cx, g.draw_cy,
                                        (int)g.brush_size);
                    }
                    g.drawing = false;
                    g.draw_phase = 0;
                    refresh_all();
                    update_panel();
                }
                break;

            case MODE_LINE:
                if (g.drawing && g.draw_phase == 1) {
                    float dx = fabsf(ix - g.draw_x1);
                    float dy = fabsf(iy - g.draw_y1);
                    if (dx > 3.0f || dy > 3.0f) {
                        add_draw_object(OBJ_LINE,
                                        g.draw_x1, g.draw_y1,
                                        ix, iy, 0, 0,
                                        (int)g.brush_size);
                        g.drawing = false;
                        g.draw_phase = 0;
                        refresh_all();
                        update_panel();
                    }
                }
                break;

            case MODE_BOX:
                if (g.drawing) {
                    g.drawing = false;
                    g.draw_phase = 0;
                    float w = fabsf(ix - g.draw_x1);
                    float h = fabsf(iy - g.draw_y1);
                    if (w * h > 4.0f) {
                        add_draw_object(OBJ_BOX,
                                        g.draw_x1, g.draw_y1,
                                        ix, iy, 0, 0,
                                        (int)g.brush_size);
                    }
                    /* Clear crop ghost */
                    g.crop_x1 = g.crop_y1 = g.crop_x2 = g.crop_y2 = 0;
                    g.has_crop = false;
                    refresh_all();
                    update_panel();
                }
                break;

            /* ── NEW: Bar — commit on release, same threshold as Box ── */
            case MODE_BAR:
                if (g.drawing) {
                    g.drawing = false;
                    g.draw_phase = 0;
                    float len = fabsf(ix - g.draw_x1);
                    float thickness = fabsf(iy - g.draw_y1);
                    if (len > 3.0f && thickness > 1.0f) {
                        float x1 = LV_MIN(g.draw_x1, ix);
                        float x2 = LV_MAX(g.draw_x1, ix);
                        float y1 = LV_MIN(g.draw_y1, iy);
                        float y2 = LV_MAX(g.draw_y1, iy);
                        add_draw_object(OBJ_BAR,
                                        x1, y1,
                                        x2, y2, 0, 0,
                                        -1);
                    }
                    g.crop_x1 = g.crop_y1 = g.crop_x2 = g.crop_y2 = 0;
                    g.has_crop = false;
                    refresh_all();
                    update_panel();
                }
                break;

            /* ── NEW: Pipe — commit on release; diameter = perpendicular drag distance
            *        approximated here as a fixed fraction of length, refine later ── */
            case MODE_PIPE:
                if (g.drawing && g.draw_phase == 1) {
                    float dx = fabsf(ix - g.draw_x1);
                    float dy = fabsf(iy - g.draw_y1);
                    if (dx > 3.0f || dy > 3.0f) {
                        float len = sqrtf(dx * dx + dy * dy);
                        float diameter = (len * 0.15f) + 6.0f;  /* simple default wall offset */
                        add_pipe_object(g.draw_x1, g.draw_y1, ix, iy, diameter,
                                        (int)g.brush_size);
                    }
                    g.drawing = false;
                    g.draw_phase = 0;
                    refresh_all();
                    update_panel();
                }
                break;

            default:
                g.dragging = false;
                g.edit_dragging = false;
                g.edit_drag_point = 0;
                g.edit_dragging = false;
                g.edit_drag_point = 0;
                break;

        
        }

        refresh_all();
        update_panel();
    }

    /* ──────────────────────────────────────
     * GESTURE (pinch / swipe)
     * ────────────────────────────────────── */
    else if (code == LV_EVENT_GESTURE) {
        float cx = (float)VP_W * 0.5f;
        float cy = (float)VP_H * 0.5f;

#if defined(LV_USE_GESTURE_RECOGNITION) && (LV_USE_GESTURE_RECOGNITION)
        /* Prefer pinch scale when available for smooth two-finger zoom */
        float pinch = lv_event_get_pinch_scale(e);
        if (pinch > 0.0f && pinch != 1.0f) {
            /* lv_event_get_pinch_scale() returns a multiplicative scale.
             * Apply it to current zoom so pinch feels natural. */
            float new_zoom = g.zoom * pinch;
            zoom_at(new_zoom, cx, cy);
        } else {
            /* Fallback to directional gesture (swipe) behaviour */
            lv_dir_t dir = lv_indev_get_gesture_dir(indev);
            if (dir == LV_DIR_TOP)    zoom_at(g.zoom + ZOOM_STEP_GEST, cx, cy);
            if (dir == LV_DIR_BOTTOM) zoom_at(g.zoom - ZOOM_STEP_GEST, cx, cy);
            if (dir == LV_DIR_LEFT  && g.mode == MODE_PAN) g.offset_x -= 60.0f;
            if (dir == LV_DIR_RIGHT && g.mode == MODE_PAN) g.offset_x += 60.0f;
        }
#else
        /* Gesture recognition disabled: use directional gesture behaviour */
        lv_dir_t dir = lv_indev_get_gesture_dir(indev);
        if (dir == LV_DIR_TOP)    zoom_at(g.zoom + ZOOM_STEP_GEST, cx, cy);
        if (dir == LV_DIR_BOTTOM) zoom_at(g.zoom - ZOOM_STEP_GEST, cx, cy);
        if (dir == LV_DIR_LEFT  && g.mode == MODE_PAN) g.offset_x -= 60.0f;
        if (dir == LV_DIR_RIGHT && g.mode == MODE_PAN) g.offset_x += 60.0f;
#endif

        refresh_all();
        update_panel();
    }
}

/* =========================================================
 * SCROLL WHEEL ZOOM
 * ========================================================= */
void scroll_zoom_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCROLL) return;
    lv_indev_t *indev = lv_event_get_indev(e);
    if (!indev) return;

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);
    float delta = (vect.y < 0) ? ZOOM_STEP_BTN : -ZOOM_STEP_BTN;

    lv_point_t pt;
    lv_indev_get_point(indev, &pt);
    lv_area_t coords;
    lv_obj_get_coords(g.canvas_obj, &coords);

    zoom_at(g.zoom + delta,
            (float)(pt.x - coords.x1),
            (float)(pt.y - coords.y1));
    refresh_all();
    update_panel();
}

/* =========================================================
 * TOP TOOLBAR CALLBACKS
 * ========================================================= */
void cb_pan (lv_event_t *e)
{
    (void)e;
    g.cropping = false;
    g.has_crop = false;
    g.crop_x1 = g.crop_y1 = g.crop_x2 = g.crop_y2 = 0;
    g.draw_phase = 0;
    set_mode(MODE_PAN);
}

void cb_crop(lv_event_t *e)
{
    (void)e;

    if (g.mode == MODE_CROP && g.has_crop) {
        apply_crop_selection();
        if (g.hint_lbl)
            lv_label_set_text(g.hint_lbl,
                "Crop applied. Drag to select a new region.");
        refresh_all();
        update_panel();
        return;
    }

    set_mode(MODE_CROP);
    if (!g.has_crop) {
        g.cropping  = false;
        g.crop_x1   = 0; g.crop_y1 = 0;
        g.crop_x2   = 0; g.crop_y2 = 0;
    }
}

void cb_zin(lv_event_t *e)
{
    (void)e;
    zoom_at(g.zoom + ZOOM_STEP_BTN, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
    refresh_all(); update_panel();
}

void cb_zout(lv_event_t *e)
{
    (void)e;
    zoom_at(g.zoom - ZOOM_STEP_BTN, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
    refresh_all(); update_panel();
}

void cb_reset(lv_event_t *e)
{
    (void)e;
    reset_view();
    g.has_crop = false; g.cropping = false;
    g.drawing  = false; g.draw_phase = 0;
    refresh_all(); update_panel();
}

void cb_fit(lv_event_t *e)
{
    (void)e;
    fit_image();
    refresh_all(); update_panel();
}

void cb_clear(lv_event_t *e)
{
    (void)e;
    if (g.dxf_review_mode) return;   /* don't let review clear the backup */
    /* Reset crop state AND coordinates so overlay stops rendering */
    g.has_crop  = false;
    g.cropping  = false;
    g.crop_x1   = 0; g.crop_y1 = 0;
    g.crop_x2   = 0; g.crop_y2 = 0;
    /* Cancel any in-progress drawing stroke */
    g.drawing    = false;
    g.draw_phase = 0;
    g.draw_x1 = g.draw_y1 = g.draw_x2 = g.draw_y2 = g.draw_cx = g.draw_cy = 0.0f;
    g.curve_dragging = false;
    g.edit_dragging  = false;
    g.edit_drag_point = 0;
    g.dragging   = false;
    /* Clear all drawn objects and undo history */
    g.draw_count = 0;
    g.gap_pts_count = 0;
    g.has_gaps      = false;
    g.gap_count     = 0;
    g.undo_top  = 0;
    /* Reset object selection state */
    for (int i = 0; i < MAX_DRAW_OBJECTS; i++) {
        g.draw_objects[i].selected = false;
    }
    g.selected_obj = -1;
    refresh_all();
    update_panel();
}

void cb_zoom10x(lv_event_t *e)
{
    (void)e;
    float target_zoom = 10.0f;
    if (g.has_crop) {
        float crop_cx = (g.crop_x1 + g.crop_x2) * 0.5f;
        float crop_cy = (g.crop_y1 + g.crop_y2) * 0.5f;
        g.zoom = target_zoom;
        float sc = zoom_scale();
        g.offset_x = (float)VP_W * 0.5f - crop_cx * sc;
        g.offset_y = (float)VP_H * 0.5f - crop_cy * sc;
    } else {
        zoom_at(target_zoom, (float)VP_W * 0.5f, (float)VP_H * 0.5f);
    }
    refresh_all(); update_panel();
}

void cb_settings(lv_event_t *e)
{
    (void)e;
    /* Open the Settings modal (Save / Export) */
    show_settings_modal();
}

/* =========================================================
 * BOTTOM TOOLBAR DRAWING TOOL CALLBACKS
 * ========================================================= */

void cb_tool_line(lv_event_t *e)
{
    (void)e;
    if (g.mode == MODE_LINE) {
        set_mode(MODE_PAN);
    } else {
        set_mode(MODE_LINE);
    }
}

void cb_tool_arc(lv_event_t *e)
{
    (void)e;
    if (g.mode == MODE_ARC) {
        set_mode(MODE_PAN);
    } else {
        set_mode(MODE_ARC);
    }
}

void cb_tool_box(lv_event_t *e)
{
    (void)e;
    if (g.mode == MODE_BOX) {
        set_mode(MODE_PAN);
    } else {
        set_mode(MODE_BOX);
    }
}

void cb_tool_edit(lv_event_t *e)
{
    (void)e;
    if (g.mode == MODE_EDIT) {
        set_mode(MODE_PAN);
    } else {
        set_mode(MODE_EDIT);
    }
}

void cb_tool_fill(lv_event_t *e)
{
    (void)e;
    /* Toggle between fill mode and pan mode */
    if (g.mode == MODE_FILL) {
        set_mode(MODE_PAN);
        return;
    }

    /* Show professional color picker dialog */
    show_professional_color_picker();
}

void cb_tool_check(lv_event_t *e)
{
    (void)e;
    if (g.mode == MODE_CHECK) {
        set_mode(MODE_PAN);
    } else {
        set_mode(MODE_CHECK);
    }
}

typedef enum {
    STOCK_MODE_BAR,
    STOCK_MODE_PIPE,
} StockMode;

static StockMode g_stock_mode = STOCK_MODE_BAR;

void cb_tool_bar(lv_event_t *e)
{
    (void)e;
    g_stock_mode = STOCK_MODE_BAR;
    show_bar_stock_dialog();
}

void cb_tool_pipe(lv_event_t *e)
{
    (void)e;
    g_stock_mode = STOCK_MODE_PIPE;
    show_bar_stock_dialog();
}

static void forged_dialog_cancel_event(lv_event_t *e);

void show_center_line_dialog(void); /* dialog for centerline placement */
static void format_mm_trimmed(char *buf, size_t bufsz, float val);

static ForgedSegType g_forged_type = FSEG_LINE;
static int           g_forged_dir  = 1;   /* +1 CW, -1 CCW */

void cb_tool_forged(lv_event_t *e)
{
    if (g.mode == MODE_FORGED) {
        forged_dialog_abort_event(e);   /* toolbar toggle-off = full abort */
        return;
    }
    g.forged_seg_count = 0;
    g.forged_undo_top  = 0;             /* NEW: clear build-time redo stack */
    g.forged_cur_x = 0.0f;
    g.forged_cur_y = 0.0f;
    g.forged_active = true;
    g_forged_type = FSEG_LINE;
    g_forged_dir  = 1;
    set_mode(MODE_FORGED);
    g.last_draw_submode = MODE_FORGED;
    show_forged_segment_dialog();
}

void cb_tool_forged_center(lv_event_t *e)
{
    (void)e;
    // if (g.mode == MODE_FORGED_CENTER) {
    //     set_mode(MODE_PAN);
    //     return;
    // }
    /* Target the most recently committed OBJ_FORGED, or the currently
     * selected one if editing. User can reposition it however many
     * times they like — nothing here locks it once placed. */
    int target = -1;
    if (g.selected_obj >= 0 && g.selected_obj < g.draw_count &&
        g.draw_objects[g.selected_obj].type == OBJ_FORGED) {
        target = g.selected_obj;
    } else {
        for (int i = g.draw_count - 1; i >= 0; i--) {
            if (g.draw_objects[i].type == OBJ_FORGED) { target = i; break; }
        }
    }

    if (target < 0) {
        if (g.hint_lbl)
            lv_label_set_text(g.hint_lbl,
                "Center Line ");
        return;
    }
    // if (target < 0) return;   /* nothing to attach a centerline to yet */

    g.forged_centerline_target = target;
    set_mode(MODE_FORGED_CENTER);

    show_center_line_dialog();
}


typedef struct {
    const char     *label;
    PatternFillType type;
} PatternMenuItem;

static const PatternMenuItem g_pattern_menu_items[] = {
    { "Straight Line", PATTERN_FILL_LINE   },
    { "Zig-Zag",       PATTERN_FILL_ZIGZAG },
    { "Clear Pattern", PATTERN_FILL_NONE   },
};
#define PATTERN_MENU_ITEM_COUNT (int)(sizeof(g_pattern_menu_items)/sizeof(g_pattern_menu_items[0]))

static void pattern_menu_item_event(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    intptr_t idx = (intptr_t)lv_obj_get_user_data(btn);

    if (g_pattern_menu) {
        lv_obj_del(g_pattern_menu);
        g_pattern_menu = NULL;
    }
    if (idx >= 0 && idx < PATTERN_MENU_ITEM_COUNT) {
        g_pattern_fill_selected = g_pattern_menu_items[idx].type;
    }
    set_mode(MODE_PATTERN_FILL);
}

void show_pattern_fill_menu(void)
{
    if (g_pattern_menu) {
        lv_obj_del(g_pattern_menu);
        g_pattern_menu = NULL;
        return;
    }

    int n = PATTERN_MENU_ITEM_COUNT;

    lv_obj_t *modal = lv_obj_create(lv_scr_act());
    g_pattern_menu = modal;
    lv_obj_set_size(modal, 180, n * 38 + 16);
    lv_obj_align(modal, LV_ALIGN_BOTTOM_LEFT, 8, -BTOOLBAR_H - 8);
    lv_obj_set_style_radius(modal, 6, 0);
    lv_obj_set_style_bg_color(modal, H(C_PANEL), 0);
    lv_obj_set_style_border_color(modal, H(C_BORDER), 0);
    lv_obj_set_style_border_width(modal, 1, 0);
    lv_obj_set_style_pad_all(modal, 8, 0);
    lv_obj_set_flex_flow(modal, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(modal, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(modal, 6, 0);

    for (int i = 0; i < n; i++) {
        lv_obj_t *btn = lv_button_create(modal);
        lv_obj_set_size(btn, 160, 32);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);
        lv_obj_add_event_cb(btn, pattern_menu_item_event, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
        lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 4, 0);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, g_pattern_menu_items[i].label);
        lv_obj_set_style_text_color(lbl, H(C_WHITE), 0);
        lv_obj_set_style_text_font(lbl, FONT_SM, 0);
        lv_obj_center(lbl);
    }
}

void cb_tool_pattern_fill(lv_event_t *e)
{
    (void)e;
    if (g.mode == MODE_PATTERN_FILL) {
        set_mode(MODE_PAN);
        return;
    }
    show_pattern_fill_menu();
}


/* =========================================================
 * Legacy color palette callbacks (kept for compatibility)
 * ========================================================= */
void color_palette_select_cb(lv_event_t *ev)
{
    (void)ev;
    /* Now handled by show_professional_color_picker() */
}

void color_palette_cancel_cb(lv_event_t *ev)
{
    (void)ev;
    /* Now handled by show_professional_color_picker() */
}



bool g_show_stock_fill = true;

void cb_tool_hide(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    if (!btn) return;

    lv_obj_t *icon = lv_obj_get_child(btn, 0);
    lv_obj_t *txt  = lv_obj_get_child(btn, 1);
    if (!icon || !txt) return;

    g_show_stock_fill = !g_show_stock_fill;
    lv_label_set_text(txt, g_show_stock_fill ? "HIDE" : "SHOW");
    lv_label_set_text(icon, g_show_stock_fill ? LV_SYMBOL_MINUS : LV_SYMBOL_PLUS);

    refresh_all();
}

/* =========================================================
 * VALIDATION & GAP DETECTION
 * ========================================================= */
void validate_drawing(void)
{
    g.is_valid = true;
    g.gap_count = 0;
    g.has_gaps = false;

    if (g.draw_count == 0) {
        lv_snprintf(g.validation_msg, sizeof(g.validation_msg),
                    "Drawing is empty");
        g.is_valid = false;
        return;
    }

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

    const float ENDPOINT_TOL = 6.0f;
    int gap_count = 0;

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *a = &g.draw_objects[i];
        if (a->type != OBJ_LINE && a->type != OBJ_ARC) continue;

        bool start_ok = endpoint_connected_to_any(a->x1, a->y1, i, ENDPOINT_TOL);
        bool end_ok   = endpoint_connected_to_any(a->x2, a->y2, i, ENDPOINT_TOL);

        if (!start_ok || !end_ok) {
            for (int j = 0; j < g.draw_count; j++) {
                if (j == i) continue;
                if (objects_intersect(a, &g.draw_objects[j])) {
                    start_ok = true;
                    end_ok   = true;
                    break;
                }
            }
        }

        if (!start_ok) gap_count++;
        if (!end_ok)   gap_count++;
    }

    g.has_gaps  = (gap_count > 0);

    g.gap_count = gap_count;

    if (g.has_gaps) {
        g.is_valid = false;
        lv_snprintf(g.validation_msg, sizeof(g.validation_msg),
                    "WARNING: %d gap(s) detected. Drawing may be incomplete.",
                    gap_count);
    } else {
        g.is_valid = true;
        lv_snprintf(g.validation_msg, sizeof(g.validation_msg),
                    "✓ Drawing valid. Lines:%d Arcs:%d Boxes:%d",
                    n_line, n_arc, n_box);
    }
}

/* =========================================================
 * BAR / PIPE STOCK DIALOG  (Length, OD, ID → generated geometry)
 * ========================================================= */
static lv_obj_t *g_bar_dialog   = NULL;
static lv_obj_t *g_bar_card     = NULL;
static lv_obj_t *g_bar_ta_len   = NULL;
static lv_obj_t *g_bar_ta_od    = NULL;
static lv_obj_t *g_bar_ta_id    = NULL;
static lv_obj_t *g_bar_err_lbl  = NULL;
static lv_obj_t *g_bar_kb       = NULL;

static bool bar_dialog_is_child_of(lv_obj_t *obj, lv_obj_t *ancestor)
{
    while (obj) {
        if (obj == ancestor) return true;
        obj = lv_obj_get_parent(obj);
    }
    return false;
}

static void close_bar_stock_dialog(void)
{
    if (g_bar_kb) {
        lv_obj_del(g_bar_kb);
        g_bar_kb = NULL;
    }
    if (g_bar_dialog) {
        lv_obj_del(g_bar_dialog);
        g_bar_dialog = NULL;
    }
    g_bar_card = NULL;
    g_bar_ta_len = g_bar_ta_od = g_bar_ta_id = g_bar_err_lbl = NULL;
}


/* =========================================================
 * FORGED PROFILE DIALOG (segment-by-segment: Line / Arc)
 * ========================================================= */
static lv_obj_t *g_forged_dialog   = NULL;
static lv_obj_t *g_forged_card     = NULL;
static lv_obj_t *g_forged_start_lbl= NULL;
static lv_obj_t *g_forged_ta_x1    = NULL;   
static lv_obj_t *g_forged_ta_z1    = NULL; 
static lv_obj_t *g_forged_ta_x2    = NULL;
static lv_obj_t *g_forged_ta_z2    = NULL;
static lv_obj_t *g_forged_ta_angle = NULL;
static lv_obj_t *g_forged_angle_row= NULL;
static lv_obj_t *g_forged_dir_row  = NULL;
static lv_obj_t *g_forged_btn_line = NULL;
static lv_obj_t *g_forged_btn_arc  = NULL;
static lv_obj_t *g_forged_btn_cw   = NULL;
static lv_obj_t *g_forged_btn_ccw  = NULL;
static lv_obj_t *g_forged_err_lbl  = NULL;
static lv_obj_t *g_forged_kb       = NULL;

/* Solve for arc center/radius given chord endpoints, sweep angle
 * (deg) and direction. Two circles pass through any chord+angle
 * pair, mirrored across the chord — `direction` selects which one. */
static void compute_arc_center(ForgedSegment *seg)
{
    float dx = seg->x2 - seg->x1;
    float dy = seg->y2 - seg->y1;
    float chord = sqrtf(dx * dx + dy * dy);
    float theta = seg->angle_deg * ((float)M_PI / 180.0f);

    if (chord < 0.5f || theta < 0.01f) {
        seg->cx = seg->x1; seg->cy = seg->y1;
        seg->radius = 0.0f;   /* degenerate -> rendered as a straight line */
        return;
    }

    float r = chord / (2.0f * sinf(theta * 0.5f));
    float h = r * cosf(theta * 0.5f);

    float ux = dx / chord, uy = dy / chord;
    float px = -uy, py = ux;                 /* perpendicular to chord */

    float sign = (seg->direction >= 0) ? 1.0f : -1.0f;
    float mx = (seg->x1 + seg->x2) * 0.5f;
    float my = (seg->y1 + seg->y2) * 0.5f;

    seg->cx = mx + sign * h * px;
    seg->cy = my + sign * h * py;
    seg->radius = fabsf(r);
}

static bool forged_dialog_is_child_of(lv_obj_t *obj, lv_obj_t *ancestor)
{
    while (obj) {
        if (obj == ancestor) return true;
        obj = lv_obj_get_parent(obj);
    }
    return false;
}

static void close_forged_dialog(void)
{
    if (g_forged_kb)     { lv_obj_del(g_forged_kb);     g_forged_kb = NULL; }
    if (g_forged_dialog) { lv_obj_del(g_forged_dialog); g_forged_dialog = NULL; }
    g_forged_card = g_forged_start_lbl = NULL;
    g_forged_ta_x1 = g_forged_ta_z1 = NULL; 
    g_forged_ta_x2 = g_forged_ta_z2 = g_forged_ta_angle = NULL;
    g_forged_angle_row = g_forged_dir_row = NULL;
    g_forged_btn_line = g_forged_btn_arc = g_forged_btn_cw = g_forged_btn_ccw = NULL;
    g_forged_err_lbl = NULL;
}


/* =========================================================
 * CENTER LINE DIALOG — X1/Z1, X2/Z2 numeric placement for the
 * Forged part's reference centerline. Writes straight into the
 * target object's cl_x1/cl_y1/cl_x2/cl_y2. After Set, MODE_FORGED_CENTER
 * stays active so the user can still drag the endpoint dots on
 * canvas to fine-tune, exactly like editing any other node.
 * ========================================================= */
static lv_obj_t *g_center_dialog  = NULL;
static lv_obj_t *g_center_card    = NULL;
static lv_obj_t *g_center_ta_x1   = NULL;
static lv_obj_t *g_center_ta_z1   = NULL;
static lv_obj_t *g_center_ta_x2   = NULL;
static lv_obj_t *g_center_ta_z2   = NULL;
static lv_obj_t *g_center_err_lbl = NULL;
static lv_obj_t *g_center_kb      = NULL;

static bool center_dialog_is_child_of(lv_obj_t *obj, lv_obj_t *ancestor)
{
    while (obj) {
        if (obj == ancestor) return true;
        obj = lv_obj_get_parent(obj);
    }
    return false;
}

static void close_center_line_dialog(void)
{
    if (g_center_kb)     { lv_obj_del(g_center_kb);     g_center_kb = NULL; }
    if (g_center_dialog) { lv_obj_del(g_center_dialog); g_center_dialog = NULL; }
    g_center_card = NULL;
    g_center_ta_x1 = g_center_ta_z1 = g_center_ta_x2 = g_center_ta_z2 = NULL;
    g_center_err_lbl = NULL;
}

static void center_dialog_kb_event(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    if (g_center_kb) {
        lv_keyboard_set_textarea(g_center_kb, ta);
        lv_obj_clear_flag(g_center_kb, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_scroll_to_view_recursive(ta, LV_ANIM_ON);
}

static void center_dialog_cancel_event(lv_event_t *e)
{
    (void)e;
    close_center_line_dialog();
    /* Leave MODE_FORGED_CENTER active — canceling the dialog shouldn't
     * cancel the whole tool, only the numeric-entry step. If the target
     * already has a centerline the user can still drag it; if not, they
     * can reopen the dialog from the toolbar. */
}

static void center_dialog_set_event(lv_event_t *e)
{
    (void)e;
    int t = g.forged_centerline_target;
    if (t < 0 || t >= g.draw_count) { close_center_line_dialog(); return; }
    if (!g_center_ta_x1 || !g_center_ta_z1 || !g_center_ta_x2 || !g_center_ta_z2) return;

    const char *s_x1 = lv_textarea_get_text(g_center_ta_x1);
    const char *s_z1 = lv_textarea_get_text(g_center_ta_z1);
    const char *s_x2 = lv_textarea_get_text(g_center_ta_x2);
    const char *s_z2 = lv_textarea_get_text(g_center_ta_z2);

    char *endp;
    float x1_mm = strtof(s_x1, &endp);  bool x1_ok = (endp != s_x1);
    float z1_mm = strtof(s_z1, &endp);  bool z1_ok = (endp != s_z1);
    float x2_mm = strtof(s_x2, &endp);  bool x2_ok = (endp != s_x2);
    float z2_mm = strtof(s_z2, &endp);  bool z2_ok = (endp != s_z2);

    const char *err = NULL;
    if (!x1_ok)      err = "X1 must be a valid number";
    else if (!z1_ok) err = "Z1 must be a valid number";
    else if (!x2_ok) err = "X2 must be a valid number";
    else if (!z2_ok) err = "Z2 must be a valid number";
    else if (fabsf(x2_mm - x1_mm) < 0.0005f && fabsf(z2_mm - z1_mm) < 0.0005f)
        err = "Start and End points must differ";

    if (err) {
        lv_label_set_text(g_center_err_lbl, err);
        lv_obj_clear_flag(g_center_err_lbl, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    /* Same mm -> image px convention as forged segments: X is the
     * radial axis (image-y), Z is the length axis (image-x). */
    DrawObject *o = &g.draw_objects[t];
    o->has_centerline = true;
    o->cl_x1 = z1_mm / MM_PER_PX_X;
    o->cl_y1 = x1_mm / MM_PER_PX_Y;
    o->cl_x2 = z2_mm / MM_PER_PX_X;
    o->cl_y2 = x2_mm / MM_PER_PX_Y;

    close_center_line_dialog();

    set_mode(MODE_PAN);

    refresh_all();
    update_panel();
    /* Stay in MODE_FORGED_CENTER — the endpoint dots are now live and
     * draggable for fine adjustment, matching the tap-to-drag behavior
     * already wired into canvas_event_cb(). */
}

static lv_obj_t *center_dialog_field(lv_obj_t *parent, const char *label_txt, const char *placeholder)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, 280, 36);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label_txt);
    lv_obj_set_style_text_color(lbl, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(lbl, FONT_SM, 0);
    lv_obj_set_width(lbl, 90);

    lv_obj_t *ta = lv_textarea_create(row);
    lv_obj_set_size(ta, 170, 36);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_accepted_chars(ta, "0123456789.");
    lv_obj_set_style_bg_color(ta, H(C_BG), 0);
    lv_obj_set_style_text_color(ta, H(C_WHITE), 0);
    lv_obj_set_style_border_color(ta, H(C_BORDER), 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_radius(ta, 4, 0);
    lv_obj_set_style_border_side(ta, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR);
    lv_obj_set_style_border_width(ta, 2, LV_PART_CURSOR);
    lv_obj_set_style_border_color(ta, H(C_CYAN), LV_PART_CURSOR);
    lv_obj_set_style_anim_time(ta, 500, LV_PART_CURSOR);
    lv_obj_set_style_border_opa(ta, LV_OPA_TRANSP, LV_PART_CURSOR);
    lv_obj_set_style_border_opa(ta, LV_OPA_COVER, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(ta, center_dialog_kb_event, LV_EVENT_FOCUSED, NULL);

    lv_obj_t *unit = lv_label_create(row);
    lv_label_set_text(unit, "mm");
    lv_obj_set_style_text_color(unit, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(unit, FONT_SM, 0);
    lv_obj_set_width(unit, 28);

    return ta;
}

static void center_dialog_overlay_event(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    if (center_dialog_is_child_of(target, g_center_card) ||
        center_dialog_is_child_of(target, g_center_kb)) {
        return;
    }
    if (g_center_kb) {
        lv_keyboard_set_textarea(g_center_kb, NULL);
        lv_obj_add_flag(g_center_kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (lv_indev_get_act()) lv_indev_reset(lv_indev_get_act(), NULL);
}

void show_center_line_dialog(void)
{
    const int32_t DIALOG_W = 320;
    const int32_t avail_h  = SCR_H - BTOOLBAR_H;
    int32_t kb_h   = LV_MIN(180, avail_h / 3);
    int32_t card_h = LV_MIN(260, avail_h - kb_h - 40);
    
    if (g_center_dialog) close_center_line_dialog();

    int t = g.forged_centerline_target;
    if (t < 0 || t >= g.draw_count) return;
    DrawObject *o = &g.draw_objects[t];

    lv_obj_t *overlay = lv_obj_create(lv_scr_act());
    g_center_dialog = overlay;
    lv_obj_set_size(overlay, SCR_W,  avail_h);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(overlay, center_dialog_overlay_event, LV_EVENT_CLICKED, NULL);


    lv_obj_t *card = lv_obj_create(overlay);
    g_center_card = card;
    lv_obj_set_size(card, DIALOG_W, card_h);
    lv_obj_align(card, LV_ALIGN_TOP_RIGHT, -8, 20);
    lv_obj_set_style_bg_color(card, H(C_PANEL), 0);
    lv_obj_set_style_border_color(card, H(C_BORDER), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 14, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(card, 8, 0);
    lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(card, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Center Line");
    lv_obj_set_style_text_color(title, H(C_PNL_TITLE), 0);
    lv_obj_set_style_text_font(title, FONT_MD, 0);

    lv_obj_t *sub = lv_label_create(card);
    lv_label_set_text(sub, "Enter start (X1,Z1) and end (X2,Z2) in mm.\nDrag the yellow dots afterward to fine-tune.");
    lv_obj_set_width(sub, 290);
    lv_obj_set_style_text_color(sub, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(sub, FONT_SM, 0);
    lv_obj_set_style_text_align(sub, LV_TEXT_ALIGN_CENTER, 0);

    g_center_ta_x1 = center_dialog_field(card, "X1:", "0");
    g_center_ta_z1 = center_dialog_field(card, "Z1:", "0");
    g_center_ta_x2 = center_dialog_field(card, "X2:", "0");
    g_center_ta_z2 = center_dialog_field(card, "Z2:", "10");

    /* Pre-fill with the existing centerline if this object already has
     * one, so reopening the dialog to adjust it starts from real values
     * instead of blanks. */
    if (o->has_centerline) {
        char buf[32];
        format_mm_trimmed(buf, sizeof(buf), img_to_mm_y(o->cl_y1));
        lv_textarea_set_text(g_center_ta_x1, buf);
        format_mm_trimmed(buf, sizeof(buf), img_to_mm_x(o->cl_x1));
        lv_textarea_set_text(g_center_ta_z1, buf);
        format_mm_trimmed(buf, sizeof(buf), img_to_mm_y(o->cl_y2));
        lv_textarea_set_text(g_center_ta_x2, buf);
        format_mm_trimmed(buf, sizeof(buf), img_to_mm_x(o->cl_x2));
        lv_textarea_set_text(g_center_ta_z2, buf);
    }

    g_center_err_lbl = lv_label_create(card);
    lv_label_set_text(g_center_err_lbl, "");
    lv_obj_set_width(g_center_err_lbl, 290);
    lv_obj_set_style_text_color(g_center_err_lbl, H(C_RED), 0);
    lv_obj_set_style_text_font(g_center_err_lbl, FONT_SM, 0);
    lv_obj_set_style_text_align(g_center_err_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(g_center_err_lbl, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *btn_row = lv_obj_create(card);
    lv_obj_set_size(btn_row, 290, 40);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(btn_row, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btn_cancel = lv_button_create(btn_row);
    lv_obj_set_size(btn_cancel, 130, 36);
    lv_obj_add_event_cb(btn_cancel, center_dialog_cancel_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_cancel, H(C_BTN_NORM), 0);
    lv_obj_set_style_border_color(btn_cancel, H(C_BTN_BRD), 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_radius(btn_cancel, 4, 0);
    lv_obj_t *cancel_lbl = lv_label_create(btn_cancel);
    lv_label_set_text(cancel_lbl, "Cancel");
    lv_obj_set_style_text_color(cancel_lbl, H(C_WHITE), 0);
    lv_obj_center(cancel_lbl);

    lv_obj_t *btn_set = lv_button_create(btn_row);
    lv_obj_set_size(btn_set, 130, 36);
    lv_obj_add_event_cb(btn_set, center_dialog_set_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_set, H(C_BTN_ACTIVE), 0);
    lv_obj_set_style_border_width(btn_set, 0, 0);
    lv_obj_set_style_radius(btn_set, 4, 0);
    lv_obj_t *set_lbl = lv_label_create(btn_set);
    lv_label_set_text(set_lbl, "Set");
    lv_obj_set_style_text_color(set_lbl, H(C_WHITE), 0);
    lv_obj_center(set_lbl);

    g_center_kb = lv_keyboard_create(overlay);
    lv_keyboard_set_mode(g_center_kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_size(g_center_kb, DIALOG_W, kb_h);
    lv_obj_align_to(g_center_kb, card, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 6);
    lv_obj_add_flag(g_center_kb, LV_OBJ_FLAG_HIDDEN);
}

static inline bool forged_is_first_segment(void)
{
    return (g.forged_seg_count == 0);
}

static void forged_dialog_abort_event(lv_event_t *e)
{
    (void)e;
    close_forged_dialog();
    g.forged_seg_count = 0;
    g.forged_undo_top  = 0;
    g.forged_active    = false;
    set_mode(MODE_PAN);
    refresh_all();
}

static void update_forged_start_label(void)
{
    if (!g_forged_start_lbl) return;
    /* Intentionally blank — user requested no text shown below
     * the "Forged" title. Function kept as a no-op stub so all
     * existing call sites (dialog open, after each Add) still work
     * without needing further changes elsewhere. */
    lv_label_set_text(g_forged_start_lbl, "");
}

static void forged_type_restyle(void)
{
    bool is_arc = (g_forged_type == FSEG_ARC);
    if (g_forged_btn_line)
        lv_obj_set_style_bg_color(g_forged_btn_line, is_arc ? H(C_BTN_NORM) : H(C_BTN_ACTIVE), 0);
    if (g_forged_btn_arc)
        lv_obj_set_style_bg_color(g_forged_btn_arc, is_arc ? H(C_BTN_ACTIVE) : H(C_BTN_NORM), 0);
    if (g_forged_angle_row) {
        if (is_arc) lv_obj_clear_flag(g_forged_angle_row, LV_OBJ_FLAG_HIDDEN);
        else        lv_obj_add_flag(g_forged_angle_row, LV_OBJ_FLAG_HIDDEN);
    }
    if (g_forged_dir_row) {
        if (is_arc) lv_obj_clear_flag(g_forged_dir_row, LV_OBJ_FLAG_HIDDEN);
        else        lv_obj_add_flag(g_forged_dir_row, LV_OBJ_FLAG_HIDDEN);
    }
}

static void forged_dir_restyle(void)
{
    bool cw = (g_forged_dir >= 0);
    if (g_forged_btn_cw)
        lv_obj_set_style_bg_color(g_forged_btn_cw, cw ? H(C_BTN_ACTIVE) : H(C_BTN_NORM), 0);
    if (g_forged_btn_ccw)
        lv_obj_set_style_bg_color(g_forged_btn_ccw, cw ? H(C_BTN_NORM) : H(C_BTN_ACTIVE), 0);
}

static void forged_type_line_event(lv_event_t *e) { (void)e; g_forged_type = FSEG_LINE; forged_type_restyle(); }
static void forged_type_arc_event (lv_event_t *e) { (void)e; g_forged_type = FSEG_ARC;  forged_type_restyle(); }
static void forged_dir_cw_event   (lv_event_t *e) { (void)e; g_forged_dir  = 1;         forged_dir_restyle();  }
static void forged_dir_ccw_event  (lv_event_t *e) { (void)e; g_forged_dir  = -1;        forged_dir_restyle();  }

static void format_mm_trimmed(char *buf, size_t bufsz, float val)
{
    lv_snprintf(buf, bufsz, "%.3f", (double)val);
    char *p = buf + strlen(buf) - 1;
    while (p > buf && *p == '0') { *p = '\0'; p--; }
    if (p > buf && *p == '.') { *p = '\0'; }
}


/* X in the Forged dialog is a total diameter, standard CNC-lathe
 * convention. Internally each point is stored at radius (X/2) so it
 * mirrors cleanly about the centerline. This converts a stored
 * radius (image px) back to the diameter mm value the user typed,
 * for every place that redisplays it in a field or label. */
static inline float forged_display_x_mm(float y_px)
{
    return img_to_mm_y(y_px) * 2.0f;
}
/* Z in the Forged dialog is the length axis (image-x). Unlike X, it's
 * not halved/mirrored — it maps straight through img_to_mm_x(). */
static inline float forged_display_z_mm(float x_px)
{
    return img_to_mm_x(x_px);
}

static void forged_dialog_kb_event(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    if (g_forged_kb) {
        lv_keyboard_set_textarea(g_forged_kb, ta);
        lv_obj_clear_flag(g_forged_kb, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_scroll_to_view_recursive(ta, LV_ANIM_ON);
}

// static void forged_dialog_cancel_event(lv_event_t *e)
// {
//     (void)e;
//     forged_dialog_abort_event(e);   /* always close + discard, regardless of seg_count */
// }

// static void forged_dialog_cancel_event(lv_event_t *e)
// {
//     (void)e;

//     // if (g.forged_seg_count <= 0) {
//     //     forged_dialog_abort_event(e);
//     //     return;
//     // }

//     char buf[32];
//     format_mm_trimmed(buf, sizeof(buf), img_to_mm_y(g.forged_cur_y));
//     lv_textarea_set_text(g_forged_ta_x1, buf);
//     format_mm_trimmed(buf, sizeof(buf), img_to_mm_x(g.forged_cur_x));
//     lv_textarea_set_text(g_forged_ta_z1, buf);
//     lv_textarea_set_text(g_forged_ta_x2, "");
//     lv_textarea_set_text(g_forged_ta_z2, "");
//     if (g_forged_ta_angle) lv_textarea_set_text(g_forged_ta_angle, "");
//     if (g_forged_err_lbl)  lv_obj_add_flag(g_forged_err_lbl, LV_OBJ_FLAG_HIDDEN);
// }

static void forged_dialog_cancel_event(lv_event_t *e)
{
    (void)e;

    /* Cancel = step-by-step Undo, scoped to this Forged part only.
     * Each press removes just the most recently added segment (line
     * or arc), leaving every earlier segment fully intact and still
     * part of the editable in-progress chain. This never touches
     * g.draw_objects[] (already-committed shapes already on the
     * canvas) or any other tool/mode's state — forged_build_undo()
     * itself refuses to run unless g.mode == MODE_FORGED. */
    if (forged_build_undo()) {
        /* Clear the End-point/Angle fields so the dialog doesn't show
         * stale values for a segment that no longer exists. Start
         * (X1/Z1) is already re-synced to the new last point inside
         * forged_build_undo(). */
        if (g_forged_ta_x2)    lv_textarea_set_text(g_forged_ta_x2, "");
        if (g_forged_ta_z2)    lv_textarea_set_text(g_forged_ta_z2, "");
        if (g_forged_ta_angle) lv_textarea_set_text(g_forged_ta_angle, "");
        if (g_forged_err_lbl)  lv_obj_add_flag(g_forged_err_lbl, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    /* Nothing left to undo — no segments exist yet in this part, so
     * there's nothing to preserve. Equivalent to aborting a part that
     * was never started (closes the dialog, returns to Pan). This is
     * the ONLY case where Cancel closes anything. */
    forged_dialog_abort_event(e);
}

static void forged_dialog_add_event(lv_event_t *e)
{
    (void)e;
    if (!g_forged_ta_x1 || !g_forged_ta_z1 || !g_forged_ta_x2 || !g_forged_ta_z2) return;

    const char *s_x1 = lv_textarea_get_text(g_forged_ta_x1);
    const char *s_z1 = lv_textarea_get_text(g_forged_ta_z1);
    const char *s_x2 = lv_textarea_get_text(g_forged_ta_x2);
    const char *s_z2 = lv_textarea_get_text(g_forged_ta_z2);

    char *endp;
    float x1_mm = strtof(s_x1, &endp);  bool x1_ok = (endp != s_x1);
    float z1_mm = strtof(s_z1, &endp);  bool z1_ok = (endp != s_z1);
    float x2_mm = strtof(s_x2, &endp);  bool x2_ok = (endp != s_x2);
    float z2_mm = strtof(s_z2, &endp);  bool z2_ok = (endp != s_z2);

    const char *err = NULL;
    if (!x1_ok)      err = "X1 must be a valid number";
    else if (!z1_ok) err = "Z1 must be a valid number";
    else if (!x2_ok) err = "X2 must be a valid number";
    else if (!z2_ok) err = "Z2 must be a valid number";
    else if (fabsf(x2_mm - x1_mm) < 0.0005f && fabsf(z2_mm - z1_mm) < 0.0005f)
        err = "Start and End points must differ";

    float angle_deg = 0.0f;
    if (!err && g_forged_type == FSEG_ARC) {
        const char *s_ang = lv_textarea_get_text(g_forged_ta_angle);
        angle_deg = strtof(s_ang, &endp);
        bool ang_ok = (endp != s_ang) && angle_deg > 0.0f && angle_deg < 360.0f;
        if (!ang_ok) err = "Angle must be between 0 and 360 deg";
    }
    if (!err && g.forged_seg_count >= MAX_FORGED_SEGS) err = "Maximum segments reached";

    if (err) {
        lv_label_set_text(g_forged_err_lbl, err);
        lv_obj_clear_flag(g_forged_err_lbl, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    /* mm -> image px  (X maps to image-y / radial axis, Z maps to image-x / length axis) */
    ForgedSegment *seg = &g.forged_segs[g.forged_seg_count];
    seg->type = g_forged_type;
    seg->x1 = z1_mm / MM_PER_PX_X;
    seg->y1 = (x1_mm * 0.5f) / MM_PER_PX_Y;
    seg->x2 = z2_mm / MM_PER_PX_X;
    seg->y2 = (x2_mm * 0.5f) / MM_PER_PX_Y;

    if (g_forged_type == FSEG_ARC) {
        seg->angle_deg = angle_deg;
        seg->direction = g_forged_dir;
        compute_arc_center(seg);
    } else {
        seg->angle_deg = 0.0f;
        seg->direction = 0;
        seg->cx = seg->cy = seg->radius = 0.0f;
    }

    g.forged_seg_count++;
    g.forged_undo_top = 0;
    g.forged_cur_x = seg->x2;
    g.forged_cur_y = seg->y2;

    /* Chain: pre-fill next Start with this segment's End, but leave it
     * editable — the user can override it to start a disjoint segment. */
    char buf[32];
    format_mm_trimmed(buf, sizeof(buf), forged_display_x_mm(g.forged_cur_y));
    lv_textarea_set_text(g_forged_ta_x1, buf);
    format_mm_trimmed(buf, sizeof(buf), forged_display_z_mm(g.forged_cur_x));
    lv_textarea_set_text(g_forged_ta_z1, buf);

    lv_textarea_set_text(g_forged_ta_x2, "");
    lv_textarea_set_text(g_forged_ta_z2, "");
    if (g_forged_ta_angle) lv_textarea_set_text(g_forged_ta_angle, "");
    lv_obj_add_flag(g_forged_err_lbl, LV_OBJ_FLAG_HIDDEN);
    update_forged_start_label();

    refresh_all();
    update_panel();
}

/* Flattens g.forged_segs[] into a dense polyline, sampling arcs so the
 * fill/hatch polygon follows the curve instead of cutting a straight
 * chord across it. Mirrors the render-time sweep direction used in
 * draw_forged_segment() so the flattened shape matches what's drawn. */
#define FORGED_ARC_SAMPLES 12

static int flatten_forged_segments_to_verts(float *out_x, float *out_y, int max_verts)
{
    return flatten_seg_chain_to_verts(g.forged_segs, g.forged_seg_count, out_x, out_y, max_verts);
}

// static int flatten_forged_segments_to_verts(float *out_x, float *out_y, int max_verts)
// {
//     int n = 0;
//     if (g.forged_seg_count <= 0 || max_verts <= 0) return 0;

//     out_x[n] = g.forged_segs[0].x1;
//     out_y[n] = g.forged_segs[0].y1;
//     n++;

//     for (int i = 0; i < g.forged_seg_count; i++) {
//         ForgedSegment *seg = &g.forged_segs[i];

//         if (seg->type == FSEG_LINE || seg->radius <= 0.001f) {
//             if (n < max_verts) {
//                 out_x[n] = seg->x2;
//                 out_y[n] = seg->y2;
//                 n++;
//             }
//             continue;
//         }

//         float ang1 = atan2f(seg->y1 - seg->cy, seg->x1 - seg->cx);
//         float ang2 = atan2f(seg->y2 - seg->cy, seg->x2 - seg->cx);

//         /* Match the sweep direction drawn by draw_forged_segment() */
//         if (seg->direction >= 0) {
//             if (ang2 < ang1) ang2 += 2.0f * (float)M_PI;
//         } else {
//             if (ang2 > ang1) ang2 -= 2.0f * (float)M_PI;
//         }

//         for (int s = 1; s <= FORGED_ARC_SAMPLES && n < max_verts; s++) {
//             float t = (float)s / (float)FORGED_ARC_SAMPLES;
//             float a = ang1 + (ang2 - ang1) * t;
//             out_x[n] = seg->cx + seg->radius * cosf(a);
//             out_y[n] = seg->cy + seg->radius * sinf(a);
//             n++;
//         }
//     }
//     return n;
// }


/* Undo/Redo while a Forged profile is still being built (dialog open,
 * not yet Finished): operates on the segment chain, not the committed
 * draw_objects list, so one bad segment can be pulled back without
 * losing the rest of the in-progress part. */
bool forged_build_undo(void)
{
    if (g.mode != MODE_FORGED || g.forged_seg_count <= 0) return false;

    g.forged_seg_count--;
    if (g.forged_undo_top < MAX_FORGED_SEGS)
        g.forged_undo_stack[g.forged_undo_top++] = g.forged_segs[g.forged_seg_count];

    if (g.forged_seg_count > 0) {
        ForgedSegment *last = &g.forged_segs[g.forged_seg_count - 1];
        g.forged_cur_x = last->x2;
        g.forged_cur_y = last->y2;
    } else {
        g.forged_cur_x = 0.0f;
        g.forged_cur_y = 0.0f;
    }

    if (g_forged_ta_x1 && g_forged_ta_z1) {
        char buf[32];
        format_mm_trimmed(buf, sizeof(buf), forged_display_x_mm(g.forged_cur_y));
        lv_textarea_set_text(g_forged_ta_x1, buf);
        format_mm_trimmed(buf, sizeof(buf), forged_display_z_mm(g.forged_cur_x));
        lv_textarea_set_text(g_forged_ta_z1, buf);
    }
    update_forged_start_label();
    refresh_all();
    update_panel();
    return true;
}

bool forged_build_redo(void)
{
    if (g.mode != MODE_FORGED || g.forged_undo_top <= 0) return false;
    if (g.forged_seg_count >= MAX_FORGED_SEGS) return false;

    g.forged_undo_top--;
    g.forged_segs[g.forged_seg_count] = g.forged_undo_stack[g.forged_undo_top];
    g.forged_seg_count++;

    ForgedSegment *last = &g.forged_segs[g.forged_seg_count - 1];
    g.forged_cur_x = last->x2;
    g.forged_cur_y = last->y2;

    if (g_forged_ta_x1 && g_forged_ta_z1) {
        char buf[32];
        format_mm_trimmed(buf, sizeof(buf), forged_display_x_mm(g.forged_cur_y));
        lv_textarea_set_text(g_forged_ta_x1, buf);
        format_mm_trimmed(buf, sizeof(buf), forged_display_z_mm(g.forged_cur_x));
        lv_textarea_set_text(g_forged_ta_z1, buf);
    }
    update_forged_start_label();
    refresh_all();
    update_panel();
    return true;
}

void cb_tool_undo(lv_event_t *e)
{
    (void)e;
    if (forged_build_undo()) return;
    do_undo();
}

void cb_tool_redo(lv_event_t *e)
{
    (void)e;
    if (forged_build_redo()) return;
    do_redo();
}




static void forged_dialog_finish_event(lv_event_t *e)
{
    (void)e;
    if (g.forged_seg_count > 0 && g.draw_count < MAX_DRAW_OBJECTS) {
        DrawObject *o = &g.draw_objects[g.draw_count];
        o->type = OBJ_FORGED;
        o->seg_count = g.forged_seg_count;
        for (int i = 0; i < g.forged_seg_count; i++) o->seg_data[i] = g.forged_segs[i];

        /* Dense vertex list (arc-aware) — used for fill/hatch/outline
         * so they follow the actual profile instead of straight chords. */
        o->vert_count = flatten_forged_segments_to_verts(o->verts_x, o->verts_y, MAX_POLY_VERTS);

        o->pattern_fill    = PATTERN_FILL_NONE;
        o->pattern_spacing = 0;
        o->fill_visible    = false;
        
        o->x1 = g.forged_segs[0].x1;
        o->y1 = g.forged_segs[0].y1;
        o->x2 = g.forged_segs[g.forged_seg_count - 1].x2;
        o->y2 = g.forged_segs[g.forged_seg_count - 1].y2;
        o->width    = (int)g.brush_size;
        o->selected = false;
        g.draw_count++;
        g.undo_top  = 0;
    }

    close_forged_dialog();
    g.forged_seg_count = 0;
    g.forged_active = false;
    set_mode(MODE_PAN);
    refresh_all();
    update_panel();
}

static lv_obj_t *forged_dialog_field(lv_obj_t *parent, const char *label_txt, const char *placeholder)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, 280, 36);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label_txt);
    lv_obj_set_style_text_color(lbl, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(lbl, FONT_SM, 0);
    lv_obj_set_width(lbl, 90);

    lv_obj_t *ta = lv_textarea_create(row);
    lv_obj_set_size(ta, 170, 36);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_accepted_chars(ta, "0123456789.");
    lv_obj_set_style_bg_color(ta, H(C_BG), 0);
    lv_obj_set_style_text_color(ta, H(C_WHITE), 0);
    lv_obj_set_style_border_color(ta, H(C_BORDER), 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_radius(ta, 4, 0);
    lv_obj_set_style_border_side(ta, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR);
    lv_obj_set_style_border_width(ta, 2, LV_PART_CURSOR);
    lv_obj_set_style_border_color(ta, H(C_CYAN), LV_PART_CURSOR);
    lv_obj_set_style_anim_time(ta, 500, LV_PART_CURSOR);
    lv_obj_set_style_border_opa(ta, LV_OPA_TRANSP, LV_PART_CURSOR);
    lv_obj_set_style_border_opa(ta, LV_OPA_COVER, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(ta, forged_dialog_kb_event, LV_EVENT_FOCUSED, NULL);

    lv_obj_t *unit = lv_label_create(row);
    lv_label_set_text(unit, "mm");
    lv_obj_set_style_text_color(unit, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(unit, FONT_SM, 0);
    lv_obj_set_width(unit, 28);

    return ta;
}

static void forged_dialog_overlay_event(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    if (forged_dialog_is_child_of(target, g_forged_card) ||
        forged_dialog_is_child_of(target, g_forged_kb)) {
        return;
    }
    if (g_forged_kb) {
        lv_keyboard_set_textarea(g_forged_kb, NULL);
        lv_obj_add_flag(g_forged_kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (lv_indev_get_act()) lv_indev_reset(lv_indev_get_act(), NULL);
}

void show_forged_segment_dialog(void)
{
    const int32_t DIALOG_W = 320;
    const int32_t avail_h  = SCR_H - BTOOLBAR_H;
    int32_t kb_h   = LV_MIN(180, avail_h / 3);
    int32_t card_h = LV_MIN(320, avail_h - kb_h - 40);   /* leaves room for kb + margins */


    if (g_forged_dialog) close_forged_dialog();

    lv_obj_t *overlay = lv_obj_create(lv_scr_act());
    g_forged_dialog = overlay;
    lv_obj_set_size(overlay, SCR_W, avail_h);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(overlay, forged_dialog_overlay_event, LV_EVENT_CLICKED, NULL);

    
    lv_obj_t *card = lv_obj_create(overlay);
    g_forged_card = card;
    lv_obj_set_size(card, DIALOG_W, card_h);
    lv_obj_align(card, LV_ALIGN_TOP_RIGHT, -8, 20);
    lv_obj_set_style_bg_color(card, H(C_PANEL), 0);
    lv_obj_set_style_border_color(card, H(C_BORDER), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 14, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(card, 4, 0);
    lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(card, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_AUTO);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Forged ");
    lv_obj_set_style_text_color(title, H(C_PNL_TITLE), 0);
    lv_obj_set_style_text_font(title, FONT_MD, 0);

    /* g_forged_start_lbl intentionally not created — no start-coordinate
     * text is shown below the title, and skipping the label removes the
     * extra vertical gap it used to reserve in the card's flex layout. */
    g_forged_start_lbl = NULL;

    /* Line/Arc type toggle */
    lv_obj_t *type_row = lv_obj_create(card);
    lv_obj_set_size(type_row, 280, 40);
    lv_obj_set_style_bg_opa(type_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(type_row, 0, 0);
    lv_obj_set_style_pad_all(type_row, 0, 0);
    lv_obj_set_flex_flow(type_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(type_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(type_row, LV_SCROLLBAR_MODE_OFF);

    g_forged_btn_line = lv_button_create(type_row);
    lv_obj_set_size(g_forged_btn_line, 130, 34);
    lv_obj_set_style_radius(g_forged_btn_line, 4, 0);
    lv_obj_add_event_cb(g_forged_btn_line, forged_type_line_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *line_lbl = lv_label_create(g_forged_btn_line);
    lv_label_set_text(line_lbl, "Line");
    lv_obj_set_style_text_color(line_lbl, H(C_WHITE), 0);
    lv_obj_center(line_lbl);

    g_forged_btn_arc = lv_button_create(type_row);
    lv_obj_set_size(g_forged_btn_arc, 130, 34);
    lv_obj_set_style_radius(g_forged_btn_arc, 4, 0);
    lv_obj_add_event_cb(g_forged_btn_arc, forged_type_arc_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *arc_lbl = lv_label_create(g_forged_btn_arc);
    lv_label_set_text(arc_lbl, "Arc");
    lv_obj_set_style_text_color(arc_lbl, H(C_WHITE), 0);
    lv_obj_center(arc_lbl);

    g_forged_ta_x1 = forged_dialog_field(card, "X1:", "0");
    g_forged_ta_z1 = forged_dialog_field(card, "Z1:", "0");
    g_forged_ta_x2 = forged_dialog_field(card, "X2:",   "5");
    g_forged_ta_z2 = forged_dialog_field(card, "Z2:",   "10");

    /* Pre-fill Start with current pen position — still editable */
    if (forged_is_first_segment()) {
    lv_textarea_set_text(g_forged_ta_x1, "");
    lv_textarea_set_text(g_forged_ta_z1, "");
    } else {
        char buf[32];
        // format_mm_trimmed(buf, sizeof(buf), img_to_mm_y(g.forged_cur_y));
        // lv_textarea_set_text(g_forged_ta_x1, buf);
        format_mm_trimmed(buf, sizeof(buf), forged_display_x_mm(g.forged_cur_y));
        lv_textarea_set_text(g_forged_ta_x1, buf);
        format_mm_trimmed(buf, sizeof(buf), forged_display_z_mm(g.forged_cur_x));
        lv_textarea_set_text(g_forged_ta_z1, buf);
    }

    g_forged_angle_row = lv_obj_create(card);
    lv_obj_set_size(g_forged_angle_row, 280, 36);
    lv_obj_set_style_bg_opa(g_forged_angle_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_forged_angle_row, 0, 0);
    lv_obj_set_style_pad_all(g_forged_angle_row, 0, 0);
    g_forged_ta_angle = forged_dialog_field(g_forged_angle_row, "Angle:", "90");
    lv_obj_set_pos(g_forged_ta_angle == NULL ? g_forged_angle_row : g_forged_angle_row, 0, 0);

    /* Direction toggle */
    g_forged_dir_row = lv_obj_create(card);
    lv_obj_set_size(g_forged_dir_row, 280, 40);
    lv_obj_set_style_bg_opa(g_forged_dir_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_forged_dir_row, 0, 0);
    lv_obj_set_style_pad_all(g_forged_dir_row, 0, 0);
    lv_obj_set_flex_flow(g_forged_dir_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(g_forged_dir_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(g_forged_dir_row, LV_SCROLLBAR_MODE_OFF);

    g_forged_btn_cw = lv_button_create(g_forged_dir_row);
    lv_obj_set_size(g_forged_btn_cw, 130, 34);
    lv_obj_set_style_radius(g_forged_btn_cw, 4, 0);
    lv_obj_add_event_cb(g_forged_btn_cw, forged_dir_cw_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *cw_lbl = lv_label_create(g_forged_btn_cw);
    lv_label_set_text(cw_lbl, "+  CW");
    lv_obj_set_style_text_color(cw_lbl, H(C_WHITE), 0);
    lv_obj_center(cw_lbl);

    g_forged_btn_ccw = lv_button_create(g_forged_dir_row);
    lv_obj_set_size(g_forged_btn_ccw, 130, 34);
    lv_obj_set_style_radius(g_forged_btn_ccw, 4, 0);
    lv_obj_add_event_cb(g_forged_btn_ccw, forged_dir_ccw_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *ccw_lbl = lv_label_create(g_forged_btn_ccw);
    lv_label_set_text(ccw_lbl, "-  CCW");
    lv_obj_set_style_text_color(ccw_lbl, H(C_WHITE), 0);
    lv_obj_center(ccw_lbl);

    g_forged_err_lbl = lv_label_create(card);
    lv_label_set_text(g_forged_err_lbl, "");
    lv_obj_set_width(g_forged_err_lbl, 290);
    lv_obj_set_style_text_color(g_forged_err_lbl, H(C_RED), 0);
    lv_obj_set_style_text_font(g_forged_err_lbl, FONT_SM, 0);
    lv_obj_set_style_text_align(g_forged_err_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(g_forged_err_lbl, LV_OBJ_FLAG_HIDDEN);

    /* Add / Finish / Cancel row */
    lv_obj_t *btn_row = lv_obj_create(card);
    lv_obj_set_size(btn_row, 290, 40);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(btn_row, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btn_cancel = lv_button_create(btn_row);
    lv_obj_set_size(btn_cancel, 88, 36);
    lv_obj_add_event_cb(btn_cancel, forged_dialog_cancel_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_cancel, H(C_BTN_NORM), 0);
    lv_obj_set_style_border_color(btn_cancel, H(C_BTN_BRD), 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_radius(btn_cancel, 4, 0);
    lv_obj_t *cancel_lbl = lv_label_create(btn_cancel);
    lv_label_set_text(cancel_lbl, "Cancel");
    lv_obj_set_style_text_color(cancel_lbl, H(C_WHITE), 0);
    lv_obj_center(cancel_lbl);

    lv_obj_t *btn_add = lv_button_create(btn_row);
    lv_obj_set_size(btn_add, 88, 36);
    lv_obj_add_event_cb(btn_add, forged_dialog_add_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_add, H(C_BTN_DRAW), 0);
    lv_obj_set_style_border_width(btn_add, 0, 0);
    lv_obj_set_style_radius(btn_add, 4, 0);
    lv_obj_t *add_lbl = lv_label_create(btn_add);
    lv_label_set_text(add_lbl, "+ Add");
    lv_obj_set_style_text_color(add_lbl, H(C_WHITE), 0);
    lv_obj_center(add_lbl);

    lv_obj_t *btn_finish = lv_button_create(btn_row);
    lv_obj_set_size(btn_finish, 88, 36);
    lv_obj_add_event_cb(btn_finish, forged_dialog_finish_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_finish, H(C_BTN_ACTIVE), 0);
    lv_obj_set_style_border_width(btn_finish, 0, 0);
    lv_obj_set_style_radius(btn_finish, 4, 0);
    lv_obj_t *finish_lbl = lv_label_create(btn_finish);
    lv_label_set_text(finish_lbl, "Finish");
    lv_obj_set_style_text_color(finish_lbl, H(C_WHITE), 0);
    lv_obj_center(finish_lbl);

    g_forged_kb = lv_keyboard_create(overlay);
    lv_keyboard_set_mode(g_forged_kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_size(g_forged_kb, DIALOG_W, kb_h);
    lv_obj_align_to(g_forged_kb, card, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 6);
    lv_obj_add_flag(g_forged_kb, LV_OBJ_FLAG_HIDDEN);

    // lv_obj_set_size(g_forged_kb, SCR_W, SCR_H / 2);
    // lv_obj_align(g_forged_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    // lv_obj_add_flag(g_forged_kb, LV_OBJ_FLAG_HIDDEN);

    forged_type_restyle();
    forged_dir_restyle();
}


/* Centers the viewport on a newly generated image-space rect and
 * zooms so it comfortably fills the canvas — same math style as
 * apply_crop_selection() but callable without g.has_crop. */
static void center_view_on_rect(float x1, float y1, float x2, float y2)
{
    float sw = fabsf(x2 - x1);
    float sh = fabsf(y2 - y1);
    if (sw < 1.0f || sh < 1.0f) return;

    /* Add ~25% margin so the stock isn't edge-to-edge */
    float sx = (float)VP_W  / (sw * 1.25f);
    float sy = (float)VP_H  / (sh * 1.25f);
    float s  = LV_MIN(sx, sy);
    g.zoom = clamp_zoom(s);

    float sc = zoom_scale();
    float cx = (x1 + x2) * 0.5f;
    float cy = (y1 + y2) * 0.5f;
    g.offset_x = (float)VP_W * 0.5f - cx * sc;
    g.offset_y = (float)VP_H * 0.5f - cy * sc;
}

static void bar_dialog_kb_event(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    if (g_bar_kb) {
        lv_keyboard_set_textarea(g_bar_kb, ta);
        lv_obj_clear_flag(g_bar_kb, LV_OBJ_FLAG_HIDDEN);
    }
    /*the focused field into view above the keyboard */
    lv_obj_scroll_to_view_recursive(ta, LV_ANIM_ON);
}

static void bar_dialog_cancel_event(lv_event_t *e)
{
    (void)e;
    close_bar_stock_dialog();
}

static void bar_dialog_create_event(lv_event_t *e)
{
    (void)e;
    if (!g_bar_ta_len || !g_bar_ta_od) return;
    if (g_stock_mode == STOCK_MODE_PIPE && !g_bar_ta_id) return;

    const char *s_len = lv_textarea_get_text(g_bar_ta_len);
    const char *s_od  = lv_textarea_get_text(g_bar_ta_od);

    char *endp;
    float len_mm = strtof(s_len, &endp);
    bool len_ok  = (endp != s_len) && (*endp == '\0' || *endp == ' ');
    float od_mm  = strtof(s_od, &endp);
    bool od_ok   = (endp != s_od) && (*endp == '\0' || *endp == ' ');

    /* ID only exists/applies for Pipe mode. Bar is always solid (id_mm = 0). */
    float id_mm = 0.0f;
    bool  id_ok = true;
    if (g_stock_mode == STOCK_MODE_PIPE) {
        const char *s_id = lv_textarea_get_text(g_bar_ta_id);
        if (s_id && s_id[0] != '\0') {
            id_mm = strtof(s_id, &endp);
            id_ok = (endp != s_id) && (*endp == '\0' || *endp == ' ');
        }
    }

    const char *err = NULL;
    if (!len_ok || len_mm <= 0.0f)      err = "Length must be a positive number";
    else if (!od_ok  || od_mm  <= 0.0f) err = "OD must be a positive number";
    else if (g_stock_mode == STOCK_MODE_PIPE && !id_ok)         err = "ID must be a valid number";
    else if (g_stock_mode == STOCK_MODE_PIPE && id_mm < 0.0f)   err = "ID cannot be negative";
    else if (g_stock_mode == STOCK_MODE_PIPE && id_mm >= od_mm) err = "ID must be smaller than OD";

    if (err) {
        if (g_bar_err_lbl) {
            lv_label_set_text(g_bar_err_lbl, err);
            lv_obj_clear_flag(g_bar_err_lbl, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    /* Convert mm → image-space px using the lathe axis convention.
     * Length runs along Z (horizontal image axis).
     * OD/ID run along the radial axis (vertical image axis). */
    // float len_px = len_mm / MM_PER_PX_X;
    // float od_px  = od_mm  / MM_PER_PX_Y;
    // float id_px  = id_mm  / MM_PER_PX_Y;

    // /* Fixed logical origin: top-left corner of the TOP line is always (0,0). */
    // float x1 = 0.0f;
    // float y1 = 0.0f;
    // float x2 = len_px;
    // float y2 = od_px;

    // if (g_stock_mode == STOCK_MODE_PIPE) {
    //     /* Pipe: rendered by draw_pipe_profile() as 5 horizontal lines —
    //      * OD top, ID top, centerline, ID bottom, OD bottom. */
    //     add_pipe_object(x1, y1, x2, y2, id_px, -1);
    // } else {
    //     /* Bar: always solid, ID not applicable. */
    //     add_draw_object(OBJ_BAR, x1, y1, x2, y2, 0, 0, -1);
    // }

    float len_px       = len_mm / MM_PER_PX_X;
    float od_radius_px = (od_mm * 0.5f) / MM_PER_PX_Y;
    float id_px        = id_mm  / MM_PER_PX_Y;   /* full ID — draw_pipe_profile already
                                                    centers this within outer_h itself */

    /* Centerline fixed at the radial origin (0): the shape spans
     * -OD/2 .. +OD/2 so it's symmetric about the (0,0) axis. */
    float x1 = 0.0f;
    float y1 = -od_radius_px;
    float x2 = len_px;
    float y2 =  od_radius_px;

    if (g_stock_mode == STOCK_MODE_PIPE) {
        add_pipe_object(x1, y1, x2, y2, id_px, -1);
    } else {
        add_draw_object(OBJ_BAR, x1, y1, x2, y2, 0, 0, -1);
    }

    close_bar_stock_dialog();
    set_mode(MODE_PAN);
    center_view_on_rect(x1, y1, x2, y2);
    refresh_all();
    update_panel();
}

static lv_obj_t *bar_dialog_field(lv_obj_t *parent, const char *label_txt,
                                   const char *placeholder)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_size(row, 280, 44);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *lbl = lv_label_create(row);
    lv_label_set_text(lbl, label_txt);
    lv_obj_set_style_text_color(lbl, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(lbl, FONT_SM, 0);
    lv_obj_set_width(lbl, 90);

    lv_obj_t *ta = lv_textarea_create(row);
    lv_obj_set_size(ta, 170, 36);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder);
    lv_textarea_set_accepted_chars(ta, "0123456789.");
    lv_obj_set_style_bg_color(ta, H(C_BG), 0);
    lv_obj_set_style_text_color(ta, H(C_WHITE), 0);
    lv_obj_set_style_border_color(ta, H(C_BORDER), 0);
    lv_obj_set_style_border_width(ta, 1, 0);
    lv_obj_set_style_radius(ta, 4, 0);


    /* NEW: cursor style — hidden by default, only shown while this field is focused */
    lv_obj_set_style_border_side(ta, LV_BORDER_SIDE_LEFT, LV_PART_CURSOR);
    lv_obj_set_style_border_width(ta, 2, LV_PART_CURSOR);
    lv_obj_set_style_border_color(ta, H(C_CYAN), LV_PART_CURSOR);
    lv_obj_set_style_anim_time(ta, 500, LV_PART_CURSOR);
    lv_obj_set_style_border_opa(ta, LV_OPA_TRANSP, LV_PART_CURSOR);                        /* default: invisible */
    lv_obj_set_style_border_opa(ta, LV_OPA_COVER, LV_PART_CURSOR | LV_STATE_FOCUSED);      /* visible only when focused */

    lv_obj_add_event_cb(ta, bar_dialog_kb_event, LV_EVENT_FOCUSED, NULL);

    // lv_obj_add_event_cb(ta, bar_dialog_kb_event, LV_EVENT_FOCUSED, NULL);

    lv_obj_t *unit = lv_label_create(row);
    lv_label_set_text(unit, "mm");
    lv_obj_set_style_text_color(unit, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(unit, FONT_SM, 0);
    lv_obj_set_width(unit, 28);

    return ta;
}

static void bar_dialog_overlay_event(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);

    /* Ignore clicks that land on any dialog content or the keyboard itself. */
    if (bar_dialog_is_child_of(target, g_bar_card) ||
        bar_dialog_is_child_of(target, g_bar_kb)) {
        return;
    }

    if (g_bar_kb) {
        lv_keyboard_set_textarea(g_bar_kb, NULL);
        lv_obj_add_flag(g_bar_kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (lv_indev_get_act())
        lv_indev_reset(lv_indev_get_act(), NULL);
}

void show_bar_stock_dialog(void)
{
    const int32_t avail_h = SCR_H - BTOOLBAR_H;

    if (g_bar_dialog) {
        close_bar_stock_dialog();
    }

    /* Dim background overlay */
    lv_obj_t *overlay = lv_obj_create(lv_scr_act());
    g_bar_dialog = overlay;
    lv_obj_set_size(overlay, SCR_W, avail_h);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_set_style_pad_all(overlay, 0, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(overlay, bar_dialog_overlay_event, LV_EVENT_CLICKED, NULL);

    bool is_pipe = (g_stock_mode == STOCK_MODE_PIPE);
    /* Card is shorter for Bar since it has one less field (no ID row). */
    int32_t card_h = is_pipe ? 280 : 236;

    /* NEW: cap the card so it always fits above the keyboard (which takes
    * the bottom half of the screen), and enable scrolling in case it
    * still doesn't fully fit on smaller displays. */
    int32_t max_card_h = (avail_h / 2) - 24;
    

    if (card_h > max_card_h) card_h = max_card_h;

    /* Card */
    lv_obj_t *card = lv_obj_create(overlay);
    g_bar_card = card;
    lv_obj_set_size(card, 320, card_h);
    // lv_obj_center(card);
    lv_obj_align(card, LV_ALIGN_TOP_RIGHT, -8, 20);
    lv_obj_set_style_bg_color(card, H(C_PANEL), 0);
    lv_obj_set_style_border_color(card, H(C_BORDER), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 14, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(card, 10, 0);
    lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(card, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_AUTO);

    // lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);

    const char *title_txt = is_pipe ? "Pipe " : "Bar ";
    const char *sub_txt = is_pipe
        ? "Enter Length, OD and ID in mm. "
        : "Enter Length and OD in mm. ";

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, title_txt);
    lv_obj_set_style_text_color(title, H(C_PNL_TITLE), 0);
    lv_obj_set_style_text_font(title, FONT_MD, 0);

    lv_obj_t *sub = lv_label_create(card);
    lv_label_set_text(sub, sub_txt);
    lv_obj_set_width(sub, 290);
    lv_obj_set_style_text_color(sub, H(C_PNL_KEY), 0);
    lv_obj_set_style_text_font(sub, FONT_SM, 0);
    lv_obj_set_style_text_align(sub, LV_TEXT_ALIGN_CENTER, 0);

    /* Order requested: Length, then OD, then ID (ID only for Pipe). */
    g_bar_ta_len = bar_dialog_field(card, "Length:", "10");
    g_bar_ta_od  = bar_dialog_field(card, "OD:",     "5");
    if (is_pipe) {
        g_bar_ta_id = bar_dialog_field(card, "ID:", "7");
    } else {
        g_bar_ta_id = NULL;   /* Bar has no ID field */
    }

    g_bar_err_lbl = lv_label_create(card);
    lv_label_set_text(g_bar_err_lbl, "");
    lv_obj_set_width(g_bar_err_lbl, 290);
    lv_obj_set_style_text_color(g_bar_err_lbl, H(C_RED), 0);
    lv_obj_set_style_text_font(g_bar_err_lbl, FONT_SM, 0);
    lv_obj_set_style_text_align(g_bar_err_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(g_bar_err_lbl, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *btn_row = lv_obj_create(card);
    lv_obj_set_size(btn_row, 290, 40);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(btn_row, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btn_cancel = lv_button_create(btn_row);
    lv_obj_set_size(btn_cancel, 130, 36);
    lv_obj_add_event_cb(btn_cancel, bar_dialog_cancel_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_cancel, H(C_BTN_NORM), 0);
    lv_obj_set_style_border_color(btn_cancel, H(C_BTN_BRD), 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_radius(btn_cancel, 4, 0);
    lv_obj_t *cancel_lbl = lv_label_create(btn_cancel);
    lv_label_set_text(cancel_lbl, "Cancel");
    lv_obj_set_style_text_color(cancel_lbl, H(C_WHITE), 0);
    lv_obj_center(cancel_lbl);

    lv_obj_t *btn_create = lv_button_create(btn_row);
    lv_obj_set_size(btn_create, 130, 36);
    lv_obj_add_event_cb(btn_create, bar_dialog_create_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_create, H(C_BTN_ACTIVE), 0);
    lv_obj_set_style_border_width(btn_create, 0, 0);
    lv_obj_set_style_radius(btn_create, 4, 0);
    lv_obj_t *create_lbl = lv_label_create(btn_create);
    lv_label_set_text(create_lbl, "Create");
    lv_obj_set_style_text_color(create_lbl, H(C_WHITE), 0);
    lv_obj_center(create_lbl);

    /* On-screen keyboard, numeric mode, hidden until a field is focused */
    g_bar_kb = lv_keyboard_create(overlay);
    lv_keyboard_set_mode(g_bar_kb, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_size(g_bar_kb, LV_MIN(220, SCR_W / 3), LV_MIN(210, avail_h - 20));
    lv_obj_align(g_bar_kb, LV_ALIGN_BOTTOM_RIGHT, -6, -6);
    lv_obj_add_flag(g_bar_kb, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_set_size(g_bar_kb, SCR_W, SCR_H / 2);
    // lv_obj_align(g_bar_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    // lv_obj_add_flag(g_bar_kb, LV_OBJ_FLAG_HIDDEN);

    /* NEW: auto-focus the first field so the cursor blinks immediately */
    if (g_bar_ta_len) {
        lv_obj_add_state(g_bar_ta_len, LV_STATE_FOCUSED);
        lv_obj_send_event(g_bar_ta_len, LV_EVENT_FOCUSED, NULL);
        // lv_event_send(g_bar_ta_len, LV_EVENT_FOCUSED, NULL);
    }
}

/* =========================================================
 * DRAW OBJECT — popup submenu
 * ========================================================= */
typedef struct {
    const char *label;
    lv_event_cb_t cb;
} DrawMenuItem;


static const DrawMenuItem g_draw_menu_items[] = {
    { "Straight Line", cb_tool_line   },
    { "Curve Line",    cb_tool_arc    },
    { "Box Creation",  cb_tool_box    },
    { "Bar (solid)",   cb_tool_bar    },
    { "Pipe (hollow)", cb_tool_pipe   },
    { "Forged",        cb_tool_forged },
    { "Center Line",   cb_tool_forged_center },
};
#define DRAW_MENU_ITEM_COUNT (int)(sizeof(g_draw_menu_items)/sizeof(g_draw_menu_items[0]))

static void draw_menu_item_event(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    intptr_t idx = (intptr_t)lv_obj_get_user_data(btn);

    if (g.draw_menu) {
        lv_obj_del(g.draw_menu);
        g.draw_menu = NULL;
    }
    if (idx >= 0 && idx < DRAW_MENU_ITEM_COUNT && g_draw_menu_items[idx].cb) {
        g_draw_menu_items[idx].cb(e);
    }
}

void cb_draw_object_menu(lv_event_t *e)
{
    (void)e;

    if (g.draw_menu) {
        lv_obj_del(g.draw_menu);
        g.draw_menu = NULL;
        return;
    }

    int n = DRAW_MENU_ITEM_COUNT;

    lv_obj_t *modal = lv_obj_create(lv_scr_act());
    g.draw_menu = modal;
    lv_obj_set_size(modal, 180, n * 38 + 16);
    lv_obj_align(modal, LV_ALIGN_BOTTOM_LEFT, 8, -BTOOLBAR_H - 8);
    lv_obj_set_style_radius(modal, 6, 0);
    lv_obj_set_style_bg_color(modal, H(C_PANEL), 0);
    lv_obj_set_style_border_color(modal, H(C_BORDER), 0);
    lv_obj_set_style_border_width(modal, 1, 0);
    lv_obj_set_style_pad_all(modal, 8, 0);
    lv_obj_set_flex_flow(modal, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(modal, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(modal, 6, 0);

    for (int i = 0; i < n; i++) {
        lv_obj_t *btn = lv_button_create(modal);
        lv_obj_set_size(btn, 160, 32);
        lv_obj_set_user_data(btn, (void *)(intptr_t)i);   /* store index, not a fn pointer */
        lv_obj_add_event_cb(btn, draw_menu_item_event, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
        lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 4, 0);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, g_draw_menu_items[i].label);
        lv_obj_set_style_text_color(lbl, H(C_WHITE), 0);
        lv_obj_set_style_text_font(lbl, FONT_SM, 0);
        lv_obj_center(lbl);
    }
}

// static void draw_menu_item_event(lv_event_t *e)
// {
//     lv_obj_t *btn = lv_event_get_target(e);
//     intptr_t idx = (intptr_t)lv_obj_get_user_data(btn);  /* integer round-trip, not pointer-cast */

//     extern const DrawMenuItem *g_draw_menu_items;  /* see below */
//     lv_event_cb_t target_cb = g_draw_menu_items[idx].cb;
//     // lv_event_cb_t target_cb = (lv_event_cb_t)lv_obj_get_user_data(btn);
//     if (g.draw_menu) {
//         lv_obj_del(g.draw_menu);
//         g.draw_menu = NULL;
//     }
//     if (target_cb) target_cb(e);
// }

// void cb_draw_object_menu(lv_event_t *e)
// {
//     (void)e;

//     if (g.draw_menu) {
//         lv_obj_del(g.draw_menu);
//         g.draw_menu = NULL;
//         return;
//     }

//     const DrawMenuItem items[] = {
//         { "Straight Line", cb_tool_line   },
//         { "Curve Line",    cb_tool_arc    },
//         { "Box Creation",  cb_tool_box    },
//         { "Bar (solid)",   cb_tool_bar    },
//         { "Pipe (hollow)", cb_tool_pipe   },
//         { "Forged",        cb_tool_forged },
//     };
//     int n = (int)(sizeof(items) / sizeof(items[0]));

//     lv_obj_t *modal = lv_obj_create(lv_scr_act());
//     g.draw_menu = modal;
//     lv_obj_set_size(modal, 180, n * 38 + 16);
//     lv_obj_align(modal, LV_ALIGN_BOTTOM_LEFT, 8, -BTOOLBAR_H - 8);
//     lv_obj_set_style_radius(modal, 6, 0);
//     lv_obj_set_style_bg_color(modal, H(C_PANEL), 0);
//     lv_obj_set_style_border_color(modal, H(C_BORDER), 0);
//     lv_obj_set_style_border_width(modal, 1, 0);
//     lv_obj_set_style_pad_all(modal, 8, 0);
//     lv_obj_set_flex_flow(modal, LV_FLEX_FLOW_COLUMN);
//     lv_obj_set_flex_align(modal, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
//     lv_obj_set_style_pad_gap(modal, 6, 0);

//     for (int i = 0; i < n; i++) {
//         lv_obj_t *btn = lv_button_create(modal);
//         lv_obj_set_size(btn, 160, 32);
//         lv_obj_set_user_data(btn, (void *)items[i].cb);
//         lv_obj_add_event_cb(btn, draw_menu_item_event, LV_EVENT_CLICKED, NULL);
//         lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
//         lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
//         lv_obj_set_style_border_width(btn, 1, 0);
//         lv_obj_set_style_radius(btn, 4, 0);

//         lv_obj_t *lbl = lv_label_create(btn);
//         lv_label_set_text(lbl, items[i].label);
//         lv_obj_set_style_text_color(lbl, H(C_WHITE), 0);
//         lv_obj_set_style_text_font(lbl, FONT_SM, 0);
//         lv_obj_center(lbl);
//     }
// }