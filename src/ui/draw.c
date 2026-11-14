/*==============================================================
 * src/ui/draw.c
 * Custom LVGL drawing callbacks for canvas, rulers, magnifier.
 *
 * FIXED: 
 * • Eliminated infinite recursion crash in ruler callbacks.
 * • Implemented adaptive zoom steps for a glitch-free professional CAD scale.
 *==============================================================*/

#include "lvgl.h"
#include "viewer.h"
#include "viewer_math.h"
#include "viewer_draw.h"
#include <math.h>
#include <limits.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ── Forward declarations ─────────────────────────────── */
static void draw_grid(lv_layer_t *layer, lv_area_t *coords,
                      int32_t ox, int32_t oy, int32_t CW, int32_t CH);
static void draw_committed_objects(lv_layer_t *layer,
                                   int32_t ox, int32_t oy);
static void draw_ghost_preview(lv_layer_t *layer,
                                int32_t ox, int32_t oy);

static float calculate_ruler_step(float ppmm);

static void draw_zigzag_box_fill(lv_layer_t *layer,          /* NEW */
                                 int32_t x1, int32_t y1,
                                 int32_t x2, int32_t y2,
                                 lv_color_t col, int32_t box_size);

static void draw_line_seg(lv_layer_t *layer,
                          int32_t x1, int32_t y1,
                          int32_t x2, int32_t y2,
                          lv_color_t col, int32_t width);
                          
static void draw_polygon_pattern_lines_clipped(lv_layer_t *layer,
                                                const float *vx, const float *vy,
                                                int n, lv_color_t col, int32_t row_step);
static void draw_polygon_pattern_zigzag_clipped(lv_layer_t *layer,
                                                 const float *vx, const float *vy,
                                                 int n, lv_color_t col, int32_t row_step);


                          
// static void draw_hatch_fill(lv_layer_t *layer,               /* NEW */
//                             int32_t x1, int32_t y1,
//                             int32_t x2, int32_t y2,
//                             lv_color_t col, int32_t spacing);


/* Zig-zag hatch confined to an arbitrary (non-rectangular) polygon —
 * used for the Forged profile's lower/mirrored half, since unlike
 * Bar/Pipe the cross-section isn't a simple rectangle. */
static void draw_polygon_zigzag_clipped(lv_layer_t *layer,
                                         const float *vx, const float *vy,
                                         int n, lv_color_t col)
{
    if (n < 3) return;

    int32_t miny = INT_MAX, maxy = INT_MIN;
    for (int i = 0; i < n; i++) {
        int32_t yi = (int32_t)floorf(vy[i]);
        if (yi < miny) miny = yi;
        if (yi > maxy) maxy = yi;
    }

    const int32_t row_step     = 5;
    const int32_t tick_spacing = 9;
    bool flip = false;

    for (int32_t y = miny; y <= maxy; y += row_step) {
        float xs[64];
        int xc = 0;
        for (int i = 0; i < n && xc < 64; i++) {
            int j = (i + 1) % n;
            float y1 = vy[i], y2 = vy[j];
            if ((y1 <= (float)y && y2 > (float)y) ||
                (y2 <= (float)y && y1 > (float)y)) {
                float t = ((float)y - y1) / (y2 - y1);
                xs[xc++] = vx[i] + t * (vx[j] - vx[i]);
            }
        }
        if (xc < 2) { flip = !flip; continue; }

        for (int a = 0; a + 1 < xc; a++)
            for (int b = a + 1; b < xc; b++)
                if (xs[b] < xs[a]) { float t = xs[a]; xs[a] = xs[b]; xs[b] = t; }

        for (int k = 0; k + 1 < xc; k += 2) {
            int32_t x1 = (int32_t)ceilf(xs[k]);
            int32_t x2 = (int32_t)floorf(xs[k+1]);
            if (x2 <= x1) continue;
            int32_t start_off = flip ? (tick_spacing / 2) : 0;
            for (int32_t tx = x1 + start_off; tx + 6 <= x2; tx += tick_spacing) {
                draw_line_seg(layer, tx, y, tx + 6, y + row_step, col, 1);
            }
        }
        flip = !flip;
    }
}


/* Returns a closed, image-space polygon usable for solid fill, hatch,
 * and Pattern Fill for a Forged profile — even if the user never
 * tap-closed it. Two ways a profile counts as "closed":
 *   1. Self-closed: last vertex snaps back onto the first (tap-to-close).
 *   2. Centerline-closed: the profile isn't self-closed, but the part
 *      has a centerline placed. The region is then the area between
 *      the profile (top boundary) and the centerline (bottom boundary,
 *      the lathe rotation axis), closed off by vertical drops at the
 *      profile's first and last X position. This mirrors standard
 *      lathe-profile convention and needs no extra drawing from the
 *      user. Only the fill/pattern region is synthesized this way —
 *      the visible outline still only ever shows segments the user
 *      actually drew.
 * Returns false (out_n = 0) if neither condition holds. */
bool forged_closed_polygon_image(const DrawObject *o, float *out_x, float *out_y, int *out_n)
{
    *out_n = 0;
    if (o->type != OBJ_FORGED || o->vert_count < 2) return false;

    int n = o->vert_count;
    if (n > MAX_POLY_VERTS) n = MAX_POLY_VERTS;

    /* 1. Self-closed check (zoom-independent viewport distance) — unchanged. */
    float dxr = o->verts_x[n - 1] - o->verts_x[0];
    float dyr = o->verts_y[n - 1] - o->verts_y[0];
    float dist_vp = sqrtf(dxr * dxr + dyr * dyr) * zoom_scale();

    if (dist_vp < 3.0f) {
        for (int i = 0; i < n; i++) {
            out_x[i] = o->verts_x[i];
            out_y[i] = o->verts_y[i];
        }
        *out_n = n;
        return true;
    }

    /* 2. Explicit centerline — kept for backward compatibility / for
     * anyone who deliberately wants an asymmetric/offset axis. */
    if (o->has_centerline) {
        for (int i = 0; i < n; i++) {
            out_x[i] = o->verts_x[i];
            out_y[i] = o->verts_y[i];
        }

        float cx1 = o->cl_x1, cy1 = o->cl_y1, cx2 = o->cl_x2, cy2 = o->cl_y2;
        float cdx = cx2 - cx1;

        float last_x = o->verts_x[n - 1];
        float first_x = o->verts_x[0];
        float cy_last, cy_first;

        if (fabsf(cdx) < 0.0001f) {
            cy_last  = cy1;
            cy_first = cy1;
        } else {
            float t_last  = (last_x  - cx1) / cdx;
            float t_first = (first_x - cx1) / cdx;
            cy_last  = cy1 + t_last  * (cy2 - cy1);
            cy_first = cy1 + t_first * (cy2 - cy1);
        }

        out_x[n]     = last_x;   out_y[n]     = cy_last;
        out_x[n + 1] = first_x;  out_y[n + 1] = cy_first;

        *out_n = n + 2;
        return true;
    }

    /* 3. NEW default — auto-mirror about the radial origin (X = 0).
     * Every point, line-segment vertex, and arc sample the user drew
     * gets a matching reflected twin at the same Z, negative Y, so the
     * shape is closed (and therefore fillable/patternable) the instant
     * a profile has 2+ points — no manual centerline step needed.
     * Buffer-safe: caller arrays are sized MAX_POLY_VERTS + 2, so cap
     * n such that 2n never exceeds that. */
    int max_half = (MAX_POLY_VERTS + 2) / 2;
    if (n > max_half) n = max_half;

    for (int i = 0; i < n; i++) {
        out_x[i] = o->verts_x[i];
        out_y[i] = o->verts_y[i];
    }
    for (int i = 0; i < n; i++) {
        out_x[n + i] =  o->verts_x[n - 1 - i];
        out_y[n + i] = -o->verts_y[n - 1 - i];   /* mirrored twin */
    }
    *out_n = n * 2;
    return true;
}


/* Parallel straight-line machining pattern, clipped to an arbitrary
 * (possibly concave) closed polygon. Independent horizontal passes —
 * safest possible clipping since each stroke is a sub-segment of a
 * single scanline interval, so it can never cross outside the
 * contour. */
static void draw_polygon_pattern_lines_clipped(lv_layer_t *layer,
                                                const float *vx, const float *vy,
                                                int n, lv_color_t col,
                                                int32_t row_step)
{
    if (n < 3) return;
    if (row_step < 2) row_step = 10;

    int32_t miny = INT_MAX, maxy = INT_MIN;
    for (int i = 0; i < n; i++) {
        int32_t yi = (int32_t)floorf(vy[i]);
        if (yi < miny) miny = yi;
        if (yi > maxy) maxy = yi;
    }

    for (int32_t y = miny; y <= maxy; y += row_step) {
        float xs[64];
        int xc = 0;
        for (int i = 0; i < n && xc < 64; i++) {
            int j = (i + 1) % n;
            float y1 = vy[i], y2 = vy[j];
            if ((y1 <= (float)y && y2 > (float)y) ||
                (y2 <= (float)y && y1 > (float)y)) {
                float t = ((float)y - y1) / (y2 - y1);
                xs[xc++] = vx[i] + t * (vx[j] - vx[i]);
            }
        }
        if (xc < 2) continue;

        for (int a = 0; a + 1 < xc; a++)
            for (int b = a + 1; b < xc; b++)
                if (xs[b] < xs[a]) { float t = xs[a]; xs[a] = xs[b]; xs[b] = t; }

        for (int k = 0; k + 1 < xc; k += 2) {
            int32_t x1 = (int32_t)ceilf(xs[k]);
            int32_t x2 = (int32_t)floorf(xs[k+1]);
            if (x2 > x1) draw_line_seg(layer, x1, y, x2, y, col, 1);
        }
    }
}

/* Connected serpentine (zig-zag) machining toolpath, clipped to the
 * polygon. Consecutive rows are joined with a diagonal traverse only
 * when each row has a single scan interval — the common case for a
 * simple Forged profile. If a row splits into multiple disjoint spans
 * (a concave notch or a second lobe), bridging them would draw
 * outside the contour, so that row's spans are drawn independently
 * instead and the serpentine restarts on the next simple row. This
 * guarantees every stroke stays strictly inside the boundary. */
static void draw_polygon_pattern_zigzag_clipped(lv_layer_t *layer,
                                                 const float *vx, const float *vy,
                                                 int n, lv_color_t col,
                                                 int32_t row_step)
{
    if (n < 3) return;
    if (row_step < 2) row_step = 10;

    int32_t miny = INT_MAX, maxy = INT_MIN;
    for (int i = 0; i < n; i++) {
        int32_t yi = (int32_t)floorf(vy[i]);
        if (yi < miny) miny = yi;
        if (yi > maxy) maxy = yi;
    }

    bool  have_prev     = false;
    float prev_x = 0.0f, prev_y = 0.0f;
    bool  left_to_right = true;

    for (int32_t y = miny; y <= maxy; y += row_step) {
        float xs[64];
        int xc = 0;
        for (int i = 0; i < n && xc < 64; i++) {
            int j = (i + 1) % n;
            float y1 = vy[i], y2 = vy[j];
            if ((y1 <= (float)y && y2 > (float)y) ||
                (y2 <= (float)y && y1 > (float)y)) {
                float t = ((float)y - y1) / (y2 - y1);
                xs[xc++] = vx[i] + t * (vx[j] - vx[i]);
            }
        }
        if (xc < 2) { have_prev = false; continue; }

        for (int a = 0; a + 1 < xc; a++)
            for (int b = a + 1; b < xc; b++)
                if (xs[b] < xs[a]) { float t = xs[a]; xs[a] = xs[b]; xs[b] = t; }

        if (xc == 2) {
            /* Single interval — safe to connect to the previous row. */
            float row_a = left_to_right ? xs[0] : xs[1];
            float row_b = left_to_right ? xs[1] : xs[0];

            if (have_prev) {
                draw_line_seg(layer, (int32_t)prev_x, (int32_t)prev_y,
                                      (int32_t)row_a, y, col, 1);
            }
            draw_line_seg(layer, (int32_t)row_a, y, (int32_t)row_b, y, col, 1);

            prev_x = row_b; prev_y = (float)y;
            have_prev = true;
            left_to_right = !left_to_right;
        } else {
            /* Disjoint spans this row — draw each independently. */
            for (int k = 0; k + 1 < xc; k += 2) {
                int32_t sx1 = (int32_t)ceilf(xs[k]);
                int32_t sx2 = (int32_t)floorf(xs[k+1]);
                if (sx2 > sx1) draw_line_seg(layer, sx1, y, sx2, y, col, 1);
            }
            have_prev = false;
        }
    }
}


static void draw_outlined_hline(lv_layer_t *layer,
                                int32_t x1, int32_t y1,
                                int32_t x2, int32_t y2,
                                lv_color_t col, int32_t lw);

static void draw_forged_segment(lv_layer_t *layer, int32_t ox, int32_t oy,
                                 const ForgedSegment *seg, lv_color_t col,
                                 int32_t lw, bool show_labels);


extern bool g_show_stock_fill;

/* ── Helpers ──────────────────────────────────────────── */
static void draw_quick_rect(lv_layer_t *layer, lv_color_t color,
                              lv_opa_t opa, lv_area_t *area)
{
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa   = opa;
    dsc.border_width = 0;
    lv_draw_rect(layer, &dsc, area);
}

static void draw_line_seg(lv_layer_t *layer,
                           int32_t x1, int32_t y1,
                           int32_t x2, int32_t y2,
                           lv_color_t col, int32_t width)
{
    lv_draw_line_dsc_t d;
    lv_draw_line_dsc_init(&d);
    d.color = col;
    d.width = width;
    d.opa   = LV_OPA_COVER;
    d.p1.x = x1; d.p1.y = y1;
    d.p2.x = x2; d.p2.y = y2;
    lv_draw_line(layer, &d);
}

static void draw_bar_profile(lv_layer_t *layer,
                             float vx1, float vy1,
                             float vx2, float vy2,
                             lv_color_t col,
                             lv_color_t fill_col,
                             lv_color_t hatch_col,
                             int32_t lw)
{
    float x_left   = LV_MIN(vx1, vx2);
    float x_right  = LV_MAX(vx1, vx2);
    float y_top    = LV_MIN(vy1, vy2);
    float y_bottom = LV_MAX(vy1, vy2);
    float y_center = (y_top + y_bottom) * 0.5f;

    if (g_show_stock_fill) {
        /* Upper half: fill + its boundary line — shown only when SHOW */
        lv_area_t upper = {(int32_t)x_left, (int32_t)y_top,
                           (int32_t)x_right, (int32_t)y_center};
        draw_quick_rect(layer, fill_col, LV_OPA_COVER, &upper);
        draw_outlined_hline(layer, (int32_t)vx1, (int32_t)y_top, (int32_t)vx2, (int32_t)y_top, col, lw);
    }

    /* Lower half: hatch + its boundary lines — always shown */
    draw_zigzag_box_fill(layer,
                         (int32_t)x_left, (int32_t)y_center,
                         (int32_t)x_right, (int32_t)y_bottom,
                         hatch_col, 8);

    draw_outlined_hline(layer, (int32_t)vx1, (int32_t)y_center, (int32_t)vx2, (int32_t)y_center, col, lw);
    draw_outlined_hline(layer, (int32_t)vx1, (int32_t)y_bottom, (int32_t)vx2, (int32_t)y_bottom, col, lw);
}

static void draw_zigzag_box_fill(lv_layer_t *layer,
                                  int32_t x1, int32_t y1,
                                  int32_t x2, int32_t y2,
                                  lv_color_t col, int32_t spacing)
{
    if (spacing < 3) spacing = 8;
    int32_t w = x2 - x1;
    int32_t h = y2 - y1;
    if (w <= 0 || h <= 0) return;

    /* Diagonal lines going ↘ (top-left to bottom-right) */
    for (int32_t offset = -h; offset < w; offset += spacing) {
        int32_t lx1 = x1 + offset;
        int32_t ly1 = y1;
        int32_t lx2 = x1 + offset + h;
        int32_t ly2 = y2;

        if (lx1 < x1) { ly1 = y1 + (x1 - lx1); lx1 = x1; }
        if (lx2 > x2) { ly2 = y2 - (lx2 - x2); lx2 = x2; }
        if (lx1 < lx2 && ly1 < ly2) {
            draw_line_seg(layer, lx1, ly1, lx2, ly2, col, 1);
        }
    }

    /* Diagonal lines going ↙ (top-right to bottom-left) — crosses the first set */
    for (int32_t offset = -h; offset < w; offset += spacing) {
        int32_t lx1 = x2 - offset;
        int32_t ly1 = y1;
        int32_t lx2 = x2 - offset - h;
        int32_t ly2 = y2;

        if (lx1 > x2) { ly1 = y1 + (lx1 - x2); lx1 = x2; }
        if (lx2 < x1) { ly2 = y2 - (x1 - lx2); lx2 = x1; }
        if (lx2 < lx1 && ly1 < ly2) {
            draw_line_seg(layer, lx2, ly2, lx1, ly1, col, 1);
        }
    }
}


/* Draws a line with a dark outline behind it, so it stays visible
 * whether it's sitting over a solid fill or a zig-zag pattern. */
static void draw_outlined_hline(lv_layer_t *layer,
                                int32_t x1, int32_t y1,
                                int32_t x2, int32_t y2,
                                lv_color_t col, int32_t lw)
{
    draw_line_seg(layer, x1, y1, x2, y2, lv_color_hex(0x000000), lw + 2);  /* halo */
    draw_line_seg(layer, x1, y1, x2, y2, col, lw);                        /* actual line */
}

/* Diagonal zig-zag hatch fill inside a rect (engineering section-view style) */
// static void draw_hatch_fill(lv_layer_t *layer,
//                             int32_t x1, int32_t y1,
//                             int32_t x2, int32_t y2,
//                             lv_color_t col, int32_t spacing)
// {
//     if (spacing < 2) spacing = 4;
//     int32_t w = x2 - x1;
//     int32_t h = y2 - y1;
//     if (w <= 0 || h <= 0) return;

//     for (int32_t offset = -h; offset < w; offset += spacing) {
//         int32_t lx1 = x1 + offset;
//         int32_t ly1 = y1;
//         int32_t lx2 = x1 + offset + h;
//         int32_t ly2 = y2;

//         /* Clip against left edge */
//         if (lx1 < x1) {
//             ly1 = y1 + (x1 - lx1);
//             lx1 = x1;
//         }
//         /* Clip against right edge */
//         if (lx2 > x2) {
//             ly2 = y2 - (lx2 - x2);
//             lx2 = x2;
//         }
//         if (lx1 < lx2 && ly1 < ly2) {
//             draw_line_seg(layer, lx1, ly1, lx2, ly2, col, 1);
//         }
//     }
// }

/*static void draw_pipe_profile(lv_layer_t *layer,
                              float vx1, float vy1,
                              float vx2, float vy2,
                              float id_px,
                              lv_color_t col,
                              int32_t lw)
{
    float x_left  = LV_MIN(vx1, vx2);
    float x_right = LV_MAX(vx1, vx2);
    float y_top   = LV_MIN(vy1, vy2);
    float y_bot   = LV_MAX(vy1, vy2);
    float outer_h = y_bot - y_top;
    float inner_h = LV_MIN(id_px, outer_h);
    float y_center = y_top + outer_h * 0.5f;
    float y_inner_top = y_top + (outer_h - inner_h) * 0.5f;
    float y_inner_bot = y_inner_top + inner_h;

    draw_line_seg(layer, (int32_t)x_left,  (int32_t)y_top,       (int32_t)x_right, (int32_t)y_top,       col, lw);
    draw_line_seg(layer, (int32_t)x_left,  (int32_t)y_inner_top, (int32_t)x_right, (int32_t)y_inner_top, col, lw);
    draw_line_seg(layer, (int32_t)x_left,  (int32_t)y_center,    (int32_t)x_right, (int32_t)y_center,    col, 1);
    draw_line_seg(layer, (int32_t)x_left,  (int32_t)y_inner_bot, (int32_t)x_right, (int32_t)y_inner_bot, col, lw);
    draw_line_seg(layer, (int32_t)x_left,  (int32_t)y_bot,       (int32_t)x_right, (int32_t)y_bot,       col, lw);
}*/

// static void draw_pipe_profile(lv_layer_t *layer,
//                               float vx1, float vy1,
//                               float vx2, float vy2,
//                               float id_px,
//                               lv_color_t outer_col,
//                               lv_color_t inner_col,
//                               lv_color_t center_col,
//                               lv_color_t fill_col,
//                               lv_color_t hatch_col,
//                               int32_t lw)
// {
//     (void)inner_col;

//     float x_left  = LV_MIN(vx1, vx2);
//     float x_right = LV_MAX(vx1, vx2);
//     float y_top   = LV_MIN(vy1, vy2);
//     float y_bot   = LV_MAX(vy1, vy2);
//     float outer_h = y_bot - y_top;
//     float inner_h = LV_MIN(id_px, outer_h);
//     float y_center = y_top + outer_h * 0.5f;
//     float y_inner_top = y_top + (outer_h - inner_h) * 0.5f;
//     float y_inner_bot = y_inner_top + inner_h;

//     lv_color_t id_line_col = lv_color_hex(0x000000);

//     if (g_show_stock_fill) {
//         /* Upper half: fill + lines 1 & 2 — shown only when SHOW */
//         lv_area_t upper = {(int32_t)x_left, (int32_t)y_top,
//                            (int32_t)x_right, (int32_t)y_center};
//         draw_quick_rect(layer, fill_col, LV_OPA_COVER, &upper);

//         draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_top,       (int32_t)x_right, (int32_t)y_top,       outer_col,   lw); /* line 1 */
//         draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_inner_top, (int32_t)x_right, (int32_t)y_inner_top, id_line_col, lw); /* line 2 */
//     }

//     /* Lower half: hatch + lines 3, 4, 5 — always shown */
//     draw_zigzag_box_fill(layer,
//                          (int32_t)x_left, (int32_t)y_center,
//                          (int32_t)x_right, (int32_t)y_bot,
//                          hatch_col, 8);

//     draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_center,    (int32_t)x_right, (int32_t)y_center,    center_col,  1);  /* line 3 */
//     draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_inner_bot, (int32_t)x_right, (int32_t)y_inner_bot, id_line_col, lw); /* line 4 */
//     draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_bot,       (int32_t)x_right, (int32_t)y_bot,       outer_col,   lw); /* line 5 */
// }

static void draw_pipe_profile(lv_layer_t *layer,
                              float vx1, float vy1,
                              float vx2, float vy2,
                              float id_px,
                              lv_color_t outer_col,
                              lv_color_t inner_col,
                              lv_color_t center_col,
                              lv_color_t fill_col,
                              lv_color_t hatch_col,
                              int32_t lw)
{
    (void)inner_col;

    float x_left  = LV_MIN(vx1, vx2);
    float x_right = LV_MAX(vx1, vx2);
    float y_top   = LV_MIN(vy1, vy2);
    float y_bot   = LV_MAX(vy1, vy2);
    float outer_h = y_bot - y_top;
    float inner_h = LV_MIN(id_px, outer_h);
    float y_center = y_top + outer_h * 0.5f;
    float y_inner_top = y_top + (outer_h - inner_h) * 0.5f;
    float y_inner_bot = y_inner_top + inner_h;

    lv_color_t id_line_col = lv_color_hex(0x000000);

    if (g_show_stock_fill) {
        /* Upper WALL only: OD top -> ID top. Never paint past y_inner_top —
         * that's the hollow bore and must stay canvas-colored. */
        lv_area_t upper = {(int32_t)x_left, (int32_t)y_top,
                           (int32_t)x_right, (int32_t)y_inner_top};
        draw_quick_rect(layer, fill_col, LV_OPA_COVER, &upper);

        draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_top,       (int32_t)x_right, (int32_t)y_top,       outer_col,   lw); /* line 1 */
        draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_inner_top, (int32_t)x_right, (int32_t)y_inner_top, id_line_col, lw); /* line 2 */
    }

    /* Lower WALL only: ID bottom -> OD bottom. The bore
     * [y_inner_top, y_inner_bot] gets no fill/hatch at all — hollow. */
    draw_zigzag_box_fill(layer,
                         (int32_t)x_left, (int32_t)y_inner_bot,
                         (int32_t)x_right, (int32_t)y_bot,
                         hatch_col, 8);

    draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_center,    (int32_t)x_right, (int32_t)y_center,    center_col,  1);  /* line 3 */
    draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_inner_bot, (int32_t)x_right, (int32_t)y_inner_bot, id_line_col, lw); /* line 4 */
    draw_outlined_hline(layer, (int32_t)x_left, (int32_t)y_bot,       (int32_t)x_right, (int32_t)y_bot,       outer_col,   lw); /* line 5 */
}

/* Draw a small filled circle (node handle) */
static void draw_dot(lv_layer_t *layer, int32_t cx, int32_t cy,
                     int32_t r, lv_color_t col)
{
    lv_area_t a = {cx - r, cy - r, cx + r, cy + r};
    lv_draw_rect_dsc_t d;
    lv_draw_rect_dsc_init(&d);
    d.bg_color  = col;
    d.bg_opa    = LV_OPA_COVER;
    d.radius    = r;
    d.border_width = 0;
    lv_draw_rect(layer, &d, &a);
}

/* Approximate quadratic Bezier with adaptive line segments */
static void fill_polygon_layer(lv_layer_t *layer,
                               const float *vx,
                               const float *vy,
                               int count,
                               lv_color_t color)
{
    if (count < 3) return;

    int miny = INT_MAX;
    int maxy = INT_MIN;
    for (int i = 0; i < count; i++) {
        int yi = (int)floorf(vy[i]);
        if (yi < miny) miny = yi;
        if (yi > maxy) maxy = yi;
    }

    float xs[MAX_POLY_VERTS * 2];

    for (int y = miny; y <= maxy; y++) {
        int ix_count = 0;

        for (int i = 0; i < count; i++) {
            int j = (i + 1) % count;
            float y1 = vy[i];
            float y2 = vy[j];

            if ((y1 <= (float)y && y2 > (float)y) ||
                (y2 <= (int)y && y1 > (float)y)) {
                float t = (y - y1) / (y2 - y1);
                float x = vx[i] + t * (vx[j] - vx[i]);
                xs[ix_count++] = x;
            }
        }

        if (ix_count < 2) continue;

        for (int i = 0; i + 1 < ix_count; i++) {
            int best = i;
            for (int j = i + 1; j < ix_count; j++) {
                if (xs[j] < xs[best]) best = j;
            }
            if (best != i) {
                float tmp = xs[i];
                xs[i] = xs[best];
                xs[best] = tmp;
            }
        }

        for (int i = 0; i + 1 < ix_count; i += 2) {
            int x1 = (int)ceilf(xs[i]);
            int x2 = (int)floorf(xs[i + 1]);
            if (x2 < x1) continue;
            lv_area_t span = {x1, y, x2, y};
            draw_quick_rect(layer, color, LV_OPA_COVER, &span);
        }
    }
}

static void draw_bezier(lv_layer_t *layer,
                         float px1, float py1,
                         float pcx, float pcy,
                         float px2, float py2,
                         lv_color_t col, int32_t w)
{
    float dx = px2 - px1;
    float dy = py2 - py1;
    float len = sqrtf(dx * dx + dy * dy);

    int steps = (int)(len * 0.25f) + 32; 
    if (steps < 32) steps = 32;
    if (steps > 512) steps = 512;

    float prev_x = px1, prev_y = py1;
    int32_t prev_ix = (int32_t)(prev_x + 0.5f);
    int32_t prev_iy = (int32_t)(prev_y + 0.5f);

    for (int i = 1; i <= steps; i++) {
        float t  = (float)i / (float)steps;
        float mt = 1.0f - t;
        float nx = mt * mt * px1 + 2.0f * mt * t * pcx + t * t * px2;
        float ny = mt * mt * py1 + 2.0f * mt * t * pcy + t * t * py2;
        int32_t next_ix = (int32_t)(nx + 0.5f);
        int32_t next_iy = (int32_t)(ny + 0.5f);
        draw_line_seg(layer, prev_ix, prev_iy, next_ix, next_iy, col, w);
        prev_x = nx; prev_y = ny;
        prev_ix = next_ix; prev_iy = next_iy;
    }
}

/* Dynamic calculation helper to find readable interval increments in mm */
static float calculate_ruler_step(float ppmm) {
    float min_pixel_gap = 50.0f; // Threshold to prevent label text overlaps
    
    float standard_intervals[] = {1.0f, 2.0f, 5.0f, 10.0f, 20.0f, 25.0f, 50.0f, 100.0f, 200.0f, 500.0f};
    int num_intervals = sizeof(standard_intervals) / sizeof(standard_intervals[0]);
    
    for (int i = 0; i < num_intervals; i++) {
        if (standard_intervals[i] * ppmm >= min_pixel_gap) {
            return standard_intervals[i];
        }
    }
    return 100.0f;
}

/* =========================================================
 * PROFESSIONAL HORIZONTAL RULER (Top)
 * ========================================================= */
void h_ruler_draw_cb(lv_event_t *e)
{
    lv_obj_t   *obj   = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t   coords;
    lv_obj_get_coords(obj, &coords);

    int32_t ox = coords.x1;
    int32_t oy = coords.y1;
    int32_t w  = lv_obj_get_width(obj);
    int32_t h  = lv_obj_get_height(obj);

    // Render dark theme background to match your canvas layout
    draw_quick_rect(layer, lv_color_hex(0x2C2C2C), LV_OPA_COVER, &coords);

    float ppmm = px_per_mm_x();

    float step_mm = calculate_ruler_step(ppmm);
    float sub_step_mm = step_mm / 10.0f; 

    float start_mm = floorf((-g.offset_x) / ppmm / sub_step_mm) * sub_step_mm;
    // start_mm = fmaxf(start_mm, 0.0f);
    float end_mm = start_mm + (w / ppmm) + step_mm;

    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = lv_color_hex(0xE0E0E0); // Light color for dark theme visibility
    line.width = 1;

    lv_draw_label_dsc_t label;
    lv_draw_label_dsc_init(&label);
    label.font  = FONT_SM;
    label.color = lv_color_hex(0xE0E0E0); // Light text for readability
    label.align = LV_TEXT_ALIGN_CENTER;

    char buf[16];

    for (float mm = start_mm; mm <= end_mm; mm += sub_step_mm) {
        int32_t px = (int32_t)(ox + g.offset_x + (mm * ppmm) + 0.5f);
        if (px < ox || px > ox + w) continue;

        bool is_major = (fmodf(fabsf(mm) + 0.0001f, step_mm) < sub_step_mm * 0.5f);
        bool is_half  = (fmodf(fabsf(mm) + 0.0001f, step_mm * 0.5f) < sub_step_mm * 0.5f);

        int32_t tick_len = is_major ? 14 : (is_half ? 9 : 5);

        line.p1.x = px; line.p1.y = oy + h - 1;
        line.p2.x = px; line.p2.y = oy + h - tick_len;
        lv_draw_line(layer, &line);

        if (is_major) {
            lv_snprintf(buf, sizeof(buf), "%d", (int)roundf(mm));
            lv_area_t ta = {px - 25, oy + 2, px + 25, oy + 16};
            label.text = buf;
            lv_draw_label(layer, &label, &ta);
        }
    }

    // Bottom boundary edge
    line.color = lv_color_hex(0x555555);
    line.p1.x = ox; line.p1.y = oy + h - 1;
    line.p2.x = ox + w; line.p2.y = oy + h - 1;
    lv_draw_line(layer, &line);
}

/* =========================================================
 * PROFESSIONAL VERTICAL RULER (Left)
 * ========================================================= */
void v_ruler_draw_cb(lv_event_t *e)
{
    lv_obj_t   *obj   = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t   coords;
    lv_obj_get_coords(obj, &coords);

    int32_t ox = coords.x1;
    int32_t oy = coords.y1;
    int32_t w  = lv_obj_get_width(obj);
    int32_t h  = lv_obj_get_height(obj);

    // Render dark theme background to match your canvas layout
    draw_quick_rect(layer, lv_color_hex(0x2C2C2C), LV_OPA_COVER, &coords);

    float ppmm = px_per_mm_y();

    float step_mm = calculate_ruler_step(ppmm);
    float sub_step_mm = step_mm / 10.0f;

    float start_mm = floorf((-g.offset_y) / ppmm / sub_step_mm) * sub_step_mm;
    // start_mm = fmaxf(start_mm, 0.0f);
    float end_mm = start_mm + (h / ppmm) + step_mm;

    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = lv_color_hex(0xE0E0E0); // Light ticks
    line.width = 1;

    lv_draw_label_dsc_t label;
    lv_draw_label_dsc_init(&label);
    label.font  = FONT_SM;
    label.color = lv_color_hex(0xE0E0E0); // Light text

    char buf[16];

    for (float mm = start_mm; mm <= end_mm; mm += sub_step_mm) {
        int32_t py = (int32_t)(oy + g.offset_y + (mm * ppmm) + 0.5f);
        if (py < oy || py > oy + h) continue;

        bool is_major = (fmodf(fabsf(mm) + 0.0001f, step_mm) < sub_step_mm * 0.5f);
        bool is_half  = (fmodf(fabsf(mm) + 0.0001f, step_mm * 0.5f) < sub_step_mm * 0.5f);

        int32_t tick_len = is_major ? 14 : (is_half ? 9 : 5);

        line.p1.x = ox + w - 1;        line.p1.y = py;
        line.p2.x = ox + w - tick_len; line.p2.y = py;
        lv_draw_line(layer, &line);

        if (is_major) {
            lv_snprintf(buf, sizeof(buf), "%d", (int)roundf(mm));
            lv_area_t ta = {ox + 2, py - 7, ox + w - 16, py + 7};
            label.text = buf;
            label.align = LV_TEXT_ALIGN_RIGHT;
            lv_draw_label(layer, &label, &ta);
        }
    }

    // Right boundary edge
    line.color = lv_color_hex(0x555555);
    line.p1.x = ox + w - 1; line.p1.y = oy;
    line.p2.x = ox + w - 1; line.p2.y = oy + h;
    lv_draw_line(layer, &line);
}

/* =========================================================
 * GRID (major + minor lines)
 * ========================================================= */
static void draw_grid(lv_layer_t *layer, lv_area_t *coords,
                      int32_t ox, int32_t oy, int32_t CW, int32_t CH)
{
    draw_quick_rect(layer, H(C_BG), LV_OPA_COVER, coords);

    float ppx = px_per_mm_x();
    float ppy = px_per_mm_y();

    float major_mm = nice_interval(ppx, 10.0f, MIN_GAP_PX_MAJOR);
    float minor_mm = nice_interval(ppx, 1.0f,  MIN_GAP_PX_MINOR);

    /* Vertical grid lines */
    float mm_start_x = floorf((-g.offset_x) / (minor_mm * ppx)) * minor_mm;
    // mm_start_x = fmaxf(mm_start_x, 0.0f);
    float mm_end_x   = mm_start_x + (CW / ppx) + minor_mm;
    for (float mm = mm_start_x; mm <= mm_end_x; mm += minor_mm) {
        float sx = g.offset_x + mm * ppx;
        if (sx < 0 || sx > CW) continue;
        int32_t px = (int32_t)(ox + sx);
        bool is_major = is_on_interval(mm, major_mm);
        uint32_t col = is_major ? C_GRID_MAJOR : C_GRID_MINOR;
        lv_area_t ta = {px, oy, px, oy + CH - 1};
        draw_quick_rect(layer, H(col), LV_OPA_COVER, &ta);
    }

    /* Horizontal grid lines */
    float major_mm_y = nice_interval(ppy, 10.0f, MIN_GAP_PX_MAJOR);
    float minor_mm_y = nice_interval(ppy, 1.0f,  MIN_GAP_PX_MINOR);
    float mm_start_y = floorf((-g.offset_y) / (minor_mm_y * ppy)) * minor_mm_y;
    // mm_start_y = fmaxf(mm_start_y, 0.0f);
    float mm_end_y   = mm_start_y + (CH / ppy) + minor_mm_y;
    for (float mm = mm_start_y; mm <= mm_end_y; mm += minor_mm_y) {
        float sy = g.offset_y + mm * ppy;
        if (sy < 0 || sy > CH) continue;
        int32_t py = (int32_t)(oy + sy);
        bool is_major = is_on_interval(mm, major_mm_y);
        uint32_t col = is_major ? C_GRID_MAJOR : C_GRID_MINOR;
        lv_area_t ta = {ox, py, ox + CW - 1, py};
        draw_quick_rect(layer, H(col), LV_OPA_COVER, &ta);
    }
}

/* =========================================================
 * DRAW COMMITTED OBJECTS
 * ========================================================= */
static void draw_committed_objects(lv_layer_t *layer, int32_t ox, int32_t oy)
{
    /* ═══════════════════════════════════════════════════════════════
     * RENDER ORDER (critical for correct overlap handling):
     * 1. All filled rects (OBJ_BOX/OBJ_BAR with width < 0)
     * 2. All filled POLYs (OBJ_POLY) — on top so they render clearly
     * 3. All strokes (outlines) — rendered last so borders are visible
     * ═══════════════════════════════════════════════════════════════ */

    /* PHASE 1: Draw all filled boxes and filled bars underneath */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *obj = &g.draw_objects[i];

        if (obj->type == OBJ_BOX  && obj->width < 0) {
            float vx1 = img_to_vp_x(obj->x1) + ox;
            float vy1 = img_to_vp_y(obj->y1) + oy;
            float vx2 = img_to_vp_x(obj->x2) + ox;
            float vy2 = img_to_vp_y(obj->y2) + oy;
            int32_t lx = (int32_t)LV_MIN(vx1, vx2);
            int32_t rx = (int32_t)LV_MAX(vx1, vx2);
            int32_t ty = (int32_t)LV_MIN(vy1, vy2);
            int32_t by = (int32_t)LV_MAX(vy1, vy2);
            lv_area_t a = {lx, ty, rx, by};
            draw_quick_rect(layer, lv_color_hex(obj->color), LV_OPA_COVER, &a);
        }
    }

    /* PHASE 2: Draw all filled polygons ON TOP so they render within boundaries */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *obj = &g.draw_objects[i];

        if (obj->type == OBJ_POLY && obj->vert_count >= 3) {
            float vx[MAX_POLY_VERTS];
            float vy[MAX_POLY_VERTS];
            for (int j = 0; j < obj->vert_count; j++) {
                vx[j] = img_to_vp_x(obj->verts_x[j]) + ox;
                vy[j] = img_to_vp_y(obj->verts_y[j]) + oy;
            }
            fill_polygon_layer(layer, vx, vy, obj->vert_count, lv_color_hex(obj->color));
        }
    }

    /* PHASE 3: Draw all strokes ON TOP (lines, arcs, box outlines) */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *obj = &g.draw_objects[i];

        float vx1 = img_to_vp_x(obj->x1) + ox;
        float vy1 = img_to_vp_y(obj->y1) + oy;
        float vx2 = img_to_vp_x(obj->x2) + ox;
        float vy2 = img_to_vp_y(obj->y2) + oy;
        float vcx = img_to_vp_x(obj->cx)  + ox;
        float vcy = img_to_vp_y(obj->cy)  + oy;

        lv_color_t col;
        switch (obj->type) {
            case OBJ_LINE: col = H(C_LINE_DRAW); break;
            case OBJ_ARC:  col = H(C_ARC_DRAW);  break;
            case OBJ_BOX:  col = H(C_BOX_DRAW);  break;
            case OBJ_BAR:    col = lv_color_hex(0xC97A2E); break;  /* bronze - bar stock */
            case OBJ_PIPE:   col = lv_color_hex(0x9B59B6); break;  /* purple - pipe      */
            case OBJ_FORGED: col = lv_color_hex(0xE74C3C); break;  /* red    - forged    */
            default:       col = H(C_WHITE);      break;
        }
        int32_t lw = obj->selected ? 3 : (obj->width > 0 ? obj->width : 2);

        switch (obj->type) {
            case OBJ_LINE:
                draw_line_seg(layer, (int32_t)vx1, (int32_t)vy1, (int32_t)vx2, (int32_t)vy2, col, lw);
                break;
            case OBJ_ARC:
                draw_bezier(layer, vx1, vy1, vcx, vcy, vx2, vy2, col, lw);
                break;
            case OBJ_BOX:
                if (obj->width >= 0) {
                    draw_line_seg(layer, (int32_t)vx1, (int32_t)vy1, (int32_t)vx2, (int32_t)vy1, col, lw);
                    draw_line_seg(layer, (int32_t)vx2, (int32_t)vy1, (int32_t)vx2, (int32_t)vy2, col, lw);
                    draw_line_seg(layer, (int32_t)vx2, (int32_t)vy2, (int32_t)vx1, (int32_t)vy2, col, lw);
                    draw_line_seg(layer, (int32_t)vx1, (int32_t)vy2, (int32_t)vx1, (int32_t)vy1, col, lw);
                }
                break;

            case OBJ_BAR: {
                lv_color_t fill_col  = lv_color_hex(0xE0A868);   /* NEW: warm tan, upper solid fill */
                lv_color_t hatch_col = lv_color_hex(0xE0E0E0);   /* NEW: light gray zig-zag, lower half */
                draw_bar_profile(layer, vx1, vy1, vx2, vy2, col, fill_col, hatch_col, lw);
                break;
            }

            case OBJ_PIPE: {
                float id_px_vp = obj->pipe_diameter * zoom_scale();  /* diameter into viewport space */
                lv_color_t outer_col  = col;                         /* existing purple 0x9B59B6, lines 1 & 5 */
                lv_color_t inner_col  = lv_color_hex(0x5DADE2);       /* light blue, lines 2 & 4 */
                lv_color_t center_col = lv_color_hex(0xF1C40F);       /* yellow, centerline */
                lv_color_t hatch_col  = lv_color_hex(0xE0E0E0);       /* NEW: light gray zig-zag, upper wall */
                lv_color_t fill_col   = lv_color_hex(0xE0A868);       /* NEW: muted purple solid, lower wall */

                draw_pipe_profile(layer, vx1, vy1, vx2, vy2, id_px_vp,
                                outer_col, inner_col, center_col,
                                fill_col, hatch_col,  lw);
                break;
            }

            case OBJ_FORGED: {
                if (obj->vert_count >= 2) {
                    int vn = obj->vert_count;
                    if (vn > MAX_POLY_VERTS) vn = MAX_POLY_VERTS;

                    float vx[MAX_POLY_VERTS];
                    float vy[MAX_POLY_VERTS];
                    for (int k = 0; k < vn; k++) {
                        vx[k] = img_to_vp_x(obj->verts_x[k]) + ox;
                        vy[k] = img_to_vp_y(obj->verts_y[k]) + oy;
                    }

                    lv_color_t solid_col = lv_color_hex(0xC97A5A);
                    lv_color_t hatch_col = lv_color_hex(0xE0E0E0);
                    lv_color_t line_col  = col;
                    int32_t    line_w    = lw;

                    float region_ix[MAX_POLY_VERTS + 2];
                    float region_iy[MAX_POLY_VERTS + 2];
                    int   region_n = 0;
                    bool  closed = forged_closed_polygon_image(obj, region_ix, region_iy, &region_n);

                    float fvx[MAX_POLY_VERTS + 2];
                    float fvy[MAX_POLY_VERTS + 2];
                    if (closed) {
                        for (int k = 0; k < region_n; k++) {
                            fvx[k] = img_to_vp_x(region_ix[k]) + ox;
                            fvy[k] = img_to_vp_y(region_iy[k]) + oy;
                        }
                    }

                    bool has_pattern = (obj->pattern_fill != PATTERN_FILL_NONE);

                    if (g_show_stock_fill && closed && !has_pattern && obj->fill_visible) {
                        fill_polygon_layer(layer, fvx, fvy, region_n, solid_col);
                    }
                    if (closed && !has_pattern && obj->fill_visible) {
                        draw_polygon_zigzag_clipped(layer, fvx, fvy, region_n, hatch_col);
                    }

                    /* Pattern Fill takes over from the default cut-away hatch
                    * once the user has explicitly applied one. */
                    if (closed && has_pattern) {
                        lv_color_t pattern_col = lv_color_hex(0x00D4FF);
                        int32_t spacing = (obj->pattern_spacing > 0) ? obj->pattern_spacing : 10;
                        if (obj->pattern_fill == PATTERN_FILL_ZIGZAG) {
                            draw_polygon_pattern_zigzag_clipped(layer, fvx, fvy, region_n, pattern_col, spacing);
                        } else {
                            draw_polygon_pattern_lines_clipped(layer, fvx, fvy, region_n, pattern_col, spacing);
                        }
                    } else if (closed) {
                        draw_polygon_zigzag_clipped(layer, fvx, fvy, region_n, hatch_col);
                    }

                    /* The profile itself — exactly as drawn, open or closed. */
                    for (int k = 0; k + 1 < vn; k++) {
                        draw_line_seg(layer, (int32_t)vx[k], (int32_t)vy[k],
                                    (int32_t)vx[k+1], (int32_t)vy[k+1], line_col, line_w);
                    }

                    /* Only draw the closing stroke when the user actually
                    * tap-closed the shape themselves — never for a
                    * centerline-only closure. */
                    float sdx = obj->verts_x[vn - 1] - obj->verts_x[0];
                    float sdy = obj->verts_y[vn - 1] - obj->verts_y[0];
                    bool self_closed = (sqrtf(sdx * sdx + sdy * sdy) * zoom_scale()) < 3.0f;
                    if (self_closed) {
                        draw_line_seg(layer, (int32_t)vx[vn-1], (int32_t)vy[vn-1],
                                    (int32_t)vx[0], (int32_t)vy[0], line_col, line_w);
                    }

                    /* NEW: default symmetric mode — mirror the drawn
                     * half about X=0 so the full, balanced profile is
                     * shown on screen, point-for-point / arc-for-arc
                     * matching what forged_closed_polygon_image() now
                     * fills by default. */
                    if (!obj->has_centerline && !self_closed) {
                        for (int k = 0; k + 1 < vn; k++) {
                            float mx1 = img_to_vp_x(obj->verts_x[k])     + ox;
                            float my1 = img_to_vp_y(-obj->verts_y[k])    + oy;
                            float mx2 = img_to_vp_x(obj->verts_x[k+1])   + ox;
                            float my2 = img_to_vp_y(-obj->verts_y[k+1])  + oy;
                            draw_line_seg(layer, (int32_t)mx1, (int32_t)my1,
                                        (int32_t)mx2, (int32_t)my2, line_col, line_w);
                        }
                    }

                    /* Center line — drawn unconditionally here, after the fill/
                    * pattern/hatch and after the profile strokes, so it always
                    * sits on top and is never hidden by any fill mode (solid,
                    * Pattern Fill, or default hatch). This was missing entirely
                    * in the previous pass, not merely obscured. */
                    if (obj->has_centerline) {
                        float cvx1 = img_to_vp_x(obj->cl_x1) + ox;
                        float cvy1 = img_to_vp_y(obj->cl_y1) + oy;
                        float cvx2 = img_to_vp_x(obj->cl_x2) + ox;
                        float cvy2 = img_to_vp_y(obj->cl_y2) + oy;

                        lv_draw_line_dsc_t cl;
                        lv_draw_line_dsc_init(&cl);
                        cl.color      = lv_color_hex(0xF1C40F);
                        cl.width      = 1;
                        cl.opa        = LV_OPA_COVER;
                        cl.dash_width = 10;
                        cl.dash_gap   = 5;
                        cl.p1.x = (int32_t)cvx1; cl.p1.y = (int32_t)cvy1;
                        cl.p2.x = (int32_t)cvx2; cl.p2.y = (int32_t)cvy2;
                        lv_draw_line(layer, &cl);
                    }

                    if (obj->seg_count > 0) {
                        for (int k = 0; k < obj->seg_count; k++) {
                            draw_forged_segment(layer, ox, oy, &obj->seg_data[k], col, lw, true);
                        }
                    }
                }
                break;
            }

            case OBJ_POLY:
                /* Polygon outline will be rendered as line objects already present. */
                break;
        }

        if (obj->selected || g.mode == MODE_EDIT) {
            draw_dot(layer, (int32_t)vx1, (int32_t)vy1, 4, H(C_NODE_DOT));
            draw_dot(layer, (int32_t)vx2, (int32_t)vy2, 4, H(C_NODE_DOT));
            if (obj->type == OBJ_ARC)
                draw_dot(layer, (int32_t)vcx, (int32_t)vcy, 4, H(C_YELLOW));
        }
    }
}

/* =========================================================
 * DRAW GHOST (in-progress) PREVIEW
 * ========================================================= */
static void draw_ghost_preview(lv_layer_t *layer, int32_t ox, int32_t oy)
{
    if (!g.drawing) return;

    float svx1 = img_to_vp_x(g.draw_x1) + ox;
    float svy1 = img_to_vp_y(g.draw_y1) + oy;
    float cvx  = g.cursor_vx + ox;
    float cvy  = g.cursor_vy + oy;

    lv_draw_line_dsc_t ghost;
    lv_draw_line_dsc_init(&ghost);
    ghost.width    = 1;
    ghost.opa      = LV_OPA_60;
    ghost.dash_width = 4;
    ghost.dash_gap   = 4;

    switch (g.mode) {
        case MODE_LINE:
            ghost.color = H(C_LINE_DRAW);
            ghost.p1.x = (int32_t)svx1; ghost.p1.y = (int32_t)svy1;
            ghost.p2.x = (int32_t)cvx;  ghost.p2.y = (int32_t)cvy;
            lv_draw_line(layer, &ghost);
            draw_dot(layer, (int32_t)svx1, (int32_t)svy1, 4, H(C_NODE_DOT));
            break;

        case MODE_ARC:
            ghost.color = H(C_ARC_DRAW);
            if (g.drawing && g.draw_phase == 2) {
                float svx2 = img_to_vp_x(g.draw_x2) + ox;
                float svy2 = img_to_vp_y(g.draw_y2) + oy;
                float svcx = img_to_vp_x(g.draw_cx) + ox;
                float svcy = img_to_vp_y(g.draw_cy) + oy;
                draw_bezier(layer, svx1, svy1, svcx, svcy, svx2, svy2, H(C_ARC_DRAW), 1);
                draw_dot(layer, (int32_t)svcx, (int32_t)svcy, 5, H(C_YELLOW));
                draw_dot(layer, (int32_t)svx2, (int32_t)svy2, 4, H(C_LINE_DRAW));
            } else if (g.drawing && g.draw_phase == 1) {
                ghost.p1.x = (int32_t)svx1; ghost.p1.y = (int32_t)svy1;
                ghost.p2.x = (int32_t)cvx;  ghost.p2.y = (int32_t)cvy;
                lv_draw_line(layer, &ghost);
            }
            draw_dot(layer, (int32_t)svx1, (int32_t)svy1, 5, H(C_NODE_DOT));
            break;

        case MODE_BOX: {
            float x1 = svx1, y1 = svy1, x2 = cvx, y2 = cvy;
            ghost.color = H(C_BOX_DRAW);
            ghost.p1.x = (int32_t)x1; ghost.p1.y = (int32_t)y1;
            ghost.p2.x = (int32_t)x2; ghost.p2.y = (int32_t)y1;
            lv_draw_line(layer, &ghost);
            ghost.p1.x = (int32_t)x2; ghost.p1.y = (int32_t)y1;
            ghost.p2.x = (int32_t)x2; ghost.p2.y = (int32_t)y2;
            lv_draw_line(layer, &ghost);
            ghost.p1.x = (int32_t)x2; ghost.p1.y = (int32_t)y2;
            ghost.p2.x = (int32_t)x1; ghost.p2.y = (int32_t)y2;
            lv_draw_line(layer, &ghost);
            ghost.p1.x = (int32_t)x1; ghost.p1.y = (int32_t)y2;
            ghost.p2.x = (int32_t)x1; ghost.p2.y = (int32_t)y1;
            lv_draw_line(layer, &ghost);
            break;
        }
        case MODE_BAR: {
            float x1 = svx1;
            float x2 = cvx;
            float y1 = LV_MIN(svy1, cvy);
            float y2 = LV_MAX(svy1, cvy);
            ghost.color = lv_color_hex(0xC97A2E);
            lv_color_t ghost_fill  = lv_color_hex(0x6B4A2E);   /* NEW: muted preview fill */
            lv_color_t ghost_hatch = lv_color_hex(0x808080);   /* NEW: muted preview hatch */
            draw_bar_profile(layer, x1, y1, x2, y2, ghost.color, ghost_fill, ghost_hatch, 1);
            break;
        }

        case MODE_PIPE:
            ghost.color = lv_color_hex(0x9B59B6);
            if (g.drawing && g.draw_phase == 1) {
                float svx2 = img_to_vp_x(g.draw_x2) + ox;
                float svy2 = img_to_vp_y(g.draw_y2) + oy;
                ghost.p1.x = (int32_t)svx1; ghost.p1.y = (int32_t)svy1;
                ghost.p2.x = (int32_t)svx2; ghost.p2.y = (int32_t)svy2;
                lv_draw_line(layer, &ghost);
            }
            break;
        default: break;
    }
}

static void draw_gap_markers(lv_layer_t *layer, int32_t ox, int32_t oy)
{
    if (g.gap_pts_count <= 0) return;
 
    /* ── Shared drawing descriptors ── */
 
    /* Outer pulsing ring */
    lv_draw_arc_dsc_t ring_dsc;
    lv_draw_arc_dsc_init(&ring_dsc);
    ring_dsc.color       = lv_color_hex(0xFF3B3B);
    ring_dsc.width       = 3;
    ring_dsc.opa         = LV_OPA_80;
    ring_dsc.start_angle = 0;
    ring_dsc.end_angle   = 360;
 
    /* Centre filled dot */
    lv_draw_rect_dsc_t dot_dsc;
    lv_draw_rect_dsc_init(&dot_dsc);
    dot_dsc.bg_color     = lv_color_hex(0xFF3B3B);
    dot_dsc.bg_opa       = LV_OPA_COVER;
    dot_dsc.border_width = 0;
    dot_dsc.radius       = LV_RADIUS_CIRCLE;
 
    /* Cross-hair arms */
    lv_draw_line_dsc_t cross_dsc;
    lv_draw_line_dsc_init(&cross_dsc);
    cross_dsc.color      = lv_color_hex(0xFF3B3B);
    cross_dsc.width      = 2;
    cross_dsc.opa        = LV_OPA_COVER;
 
    /* Dark pill background for "OPEN" label */
    lv_draw_rect_dsc_t pill_dsc;
    lv_draw_rect_dsc_init(&pill_dsc);
    pill_dsc.bg_color     = lv_color_hex(0x1A0000);
    pill_dsc.bg_opa       = LV_OPA_80;
    pill_dsc.border_width = 1;
    pill_dsc.border_color = lv_color_hex(0xFF3B3B);
    pill_dsc.border_opa   = LV_OPA_COVER;
    pill_dsc.radius       = 3;
 
    /* "OPEN" text label */
    lv_draw_label_dsc_t lbl_dsc;
    lv_draw_label_dsc_init(&lbl_dsc);
    lbl_dsc.font          = FONT_SM;
    lbl_dsc.color         = lv_color_hex(0xFF6B6B);
    lbl_dsc.align         = LV_TEXT_ALIGN_LEFT;
 
    /* ── Per-marker rendering ── */
    for (int i = 0; i < g.gap_pts_count; i++) {
 
        /* Image-space → viewport pixel */
        int32_t vx = (int32_t)(img_to_vp_x(g.gap_pts_x[i]) + (float)ox);
        int32_t vy = (int32_t)(img_to_vp_y(g.gap_pts_y[i]) + (float)oy);
 
        /* Cull markers that are completely off-canvas */
        if (vx < ox - 24 || vx > ox + VP_W + 24) continue;
        if (vy < oy - 24 || vy > oy + VP_H + 24) continue;
 
        const int32_t RING_R  = 12;   /* outer ring radius, px   */
        const int32_t DOT_R   =  4;   /* centre dot radius, px   */
        const int32_t ARM_LEN = 20;   /* cross arm half-length   */
 
        /* 1. Outer ring */
        ring_dsc.center.x = vx;
        ring_dsc.center.y = vy;
        ring_dsc.radius   = RING_R;
        lv_draw_arc(layer, &ring_dsc);
 
        /* 2. Second, slightly larger faint ring for "glow" effect */
        ring_dsc.radius = RING_R + 5;
        ring_dsc.opa    = LV_OPA_30;
        lv_draw_arc(layer, &ring_dsc);
        ring_dsc.opa    = LV_OPA_80;   /* restore */
        ring_dsc.radius = RING_R;      /* restore */
 
        /* 3. Cross — horizontal */
        cross_dsc.p1.x = vx - ARM_LEN; cross_dsc.p1.y = vy;
        cross_dsc.p2.x = vx + ARM_LEN; cross_dsc.p2.y = vy;
        lv_draw_line(layer, &cross_dsc);
 
        /* 4. Cross — vertical */
        cross_dsc.p1.x = vx; cross_dsc.p1.y = vy - ARM_LEN;
        cross_dsc.p2.x = vx; cross_dsc.p2.y = vy + ARM_LEN;
        lv_draw_line(layer, &cross_dsc);
 
        /* 5. Centre dot (drawn last so it sits on top) */
        lv_area_t dot_a = {vx - DOT_R, vy - DOT_R,
                           vx + DOT_R, vy + DOT_R};
        lv_draw_rect(layer, &dot_dsc, &dot_a);
 
        /* 6. "OPEN" label pill — top-right of the marker */
        lv_area_t pill_a = {vx + RING_R + 3,  vy - 10,
                            vx + RING_R + 46,  vy +  2};
        lv_draw_rect(layer, &pill_dsc, &pill_a);
 
        lbl_dsc.text = "OPEN";
        lv_area_t lbl_a = {pill_a.x1 + 4, pill_a.y1 + 1,
                           pill_a.x2,      pill_a.y2};
        lv_draw_label(layer, &lbl_dsc, &lbl_a);
    }
 
    /* ── Global warning banner at canvas top-left ── */
    {
        char warn_buf[72];
        lv_snprintf(warn_buf, sizeof(warn_buf),
                    LV_SYMBOL_WARNING
                    "  Shape not closed — %d open endpoint(s) found",
                    g.gap_pts_count);
 
        lv_area_t banner = {ox + 6, oy + 6,
                            ox + 370, oy + 24};
 
        /* Banner background */
        lv_draw_rect_dsc_t bg;
        lv_draw_rect_dsc_init(&bg);
        bg.bg_color     = lv_color_hex(0x1A0000);
        bg.bg_opa       = LV_OPA_90;
        bg.border_color = lv_color_hex(0xFF3B3B);
        bg.border_width = 1;
        bg.border_opa   = LV_OPA_COVER;
        bg.radius       = 4;
        lv_draw_rect(layer, &bg, &banner);
 
        /* Banner text */
        lv_draw_label_dsc_t warn;
        lv_draw_label_dsc_init(&warn);
        warn.font  = FONT_SM;
        warn.color = lv_color_hex(0xFF6B6B);
        warn.text  = warn_buf;
        lv_area_t wl = {banner.x1 + 6, banner.y1 + 3,
                        banner.x2,     banner.y2};
        lv_draw_label(layer, &warn, &wl);
    }
}

/* Formats a value the way the user typed it — no forced trailing
 * zeros. Internally rounds to 3 decimals (matches storage precision)
 * then strips trailing zeros and a trailing decimal point, so 1.000
 * displays as "1" and 1.250 displays as "1.25". Display-only: does
 * not change any stored coordinate or drawing math. */
static void format_mm_trimmed(char *buf, size_t bufsz, float val)
{
    lv_snprintf(buf, bufsz, "%.3f", (double)val);
    char *p = buf + strlen(buf) - 1;
    while (p > buf && *p == '0') { *p = '\0'; p--; }
    if (p > buf && *p == '.') { *p = '\0'; }
}

/* X in the Forged dialog is a total diameter; each point is stored
 * at radius (X/2) internally. This converts back to the diameter mm
 * value for display — mirrors the copy in events.c (kept as a
 * separate static inline since each .c file needs its own). */
static inline float forged_display_x_mm(float y_px)
{
    return img_to_mm_y(y_px) * 2.0f;
}

/* Renders one Forged segment (line or arc) with CNC-style coordinate
 * labels and an angle glyph for arcs. Shared by the in-progress
 * preview and the committed OBJ_FORGED render path. */
static void draw_forged_segment(lv_layer_t *layer, int32_t ox, int32_t oy,
                                 const ForgedSegment *seg, lv_color_t col,
                                 int32_t lw, bool show_labels)
{
    float vx1 = img_to_vp_x(seg->x1) + ox;
    float vy1 = img_to_vp_y(seg->y1) + oy;
    float vx2 = img_to_vp_x(seg->x2) + ox;
    float vy2 = img_to_vp_y(seg->y2) + oy;

    if (seg->type == FSEG_LINE || seg->radius <= 0.001f) {
        draw_line_seg(layer, (int32_t)vx1, (int32_t)vy1, (int32_t)vx2, (int32_t)vy2, col, lw);
    } else {
        float vcx = img_to_vp_x(seg->cx) + ox;
        float vcy = img_to_vp_y(seg->cy) + oy;
        float vr  = seg->radius * zoom_scale();

        float ang1 = atan2f(vy1 - vcy, vx1 - vcx) * 180.0f / (float)M_PI;
        float ang2 = atan2f(vy2 - vcy, vx2 - vcx) * 180.0f / (float)M_PI;
        if (ang1 < 0) ang1 += 360.0f;
        if (ang2 < 0) ang2 += 360.0f;

        lv_draw_arc_dsc_t adsc;
        lv_draw_arc_dsc_init(&adsc);
        adsc.color    = col;
        adsc.width    = lw;
        adsc.opa      = LV_OPA_COVER;
        adsc.center.x = (int32_t)vcx;
        adsc.center.y = (int32_t)vcy;
        adsc.radius   = (int32_t)vr;

        /* LVGL sweeps clockwise start_angle -> end_angle. For CCW-tagged
         * arcs the pair is swapped so the visual sweep still matches the
         * requested direction. If arcs bulge the wrong way on-screen,
         * flip this if/else. */
        if (seg->direction >= 0) {
            adsc.start_angle = (int32_t)ang1;
            adsc.end_angle   = (int32_t)ang2;
        } else {
            adsc.start_angle = (int32_t)ang2;
            adsc.end_angle   = (int32_t)ang1;
        }
        lv_draw_arc(layer, &adsc);

        if (show_labels) {
            float mid_ang = (ang1 + ang2) * 0.5f;
            float lx = vcx + (vr + 14) * cosf(mid_ang * (float)M_PI / 180.0f);
            float ly = vcy + (vr + 14) * sinf(mid_ang * (float)M_PI / 180.0f);

            char abuf[32];
            lv_snprintf(abuf, sizeof(abuf), LV_SYMBOL_LOOP " %.1f" "\xC2\xB0",
                        (double)seg->angle_deg);

            lv_draw_label_dsc_t adl;
            lv_draw_label_dsc_init(&adl);
            adl.font  = FONT_SM;
            adl.color = lv_color_hex(0xF0B429);
            adl.text  = abuf;
            lv_area_t aa = {(int32_t)lx - 30, (int32_t)ly - 8, (int32_t)lx + 30, (int32_t)ly + 8};
            lv_draw_label(layer, &adl, &aa);
        }
    }

    if (show_labels) {
        char x1s[16], z1s[16], x2s[16], z2s[16];
        format_mm_trimmed(x1s, sizeof(x1s), forged_display_x_mm(seg->y1));
        format_mm_trimmed(z1s, sizeof(z1s), img_to_mm_x(seg->x1));
        format_mm_trimmed(x2s, sizeof(x2s), forged_display_x_mm(seg->y2));
        format_mm_trimmed(z2s, sizeof(z2s), img_to_mm_x(seg->x2));

        char buf1[40], buf2[40];
        lv_snprintf(buf1, sizeof(buf1), "X%s Z%s", x1s, z1s);
        lv_snprintf(buf2, sizeof(buf2), "X%s Z%s", x2s, z2s);

        
        lv_draw_label_dsc_t ld;
        lv_draw_label_dsc_init(&ld);
        ld.font  = FONT_SM;
        ld.color = H(C_WHITE);

        ld.text = buf1;
        lv_area_t a1 = {(int32_t)vx1 - 40, (int32_t)vy1 - 20, (int32_t)vx1 + 40, (int32_t)vy1 - 6};
        lv_draw_label(layer, &ld, &a1);

        ld.text = buf2;
        lv_area_t a2 = {(int32_t)vx2 - 40, (int32_t)vy2 - 20, (int32_t)vx2 + 40, (int32_t)vy2 - 6};
        lv_draw_label(layer, &ld, &a2);
    }

    

    draw_dot(layer, (int32_t)vx1, (int32_t)vy1, 3, H(C_NODE_DOT));
    draw_dot(layer, (int32_t)vx2, (int32_t)vy2, 3, H(C_NODE_DOT));
}

static void draw_forged_progress(lv_layer_t *layer, int32_t ox, int32_t oy)
{
    if (g.mode != MODE_FORGED || g.forged_seg_count == 0) return;

    lv_color_t col = lv_color_hex(0xE74C3C);
    for (int i = 0; i < g.forged_seg_count; i++) {
        draw_forged_segment(layer, ox, oy, &g.forged_segs[i], col, 2, true);
    }
}


/* =========================================================
 * CANVAS DRAW CB — main entry point
 * ========================================================= */
void canvas_draw_cb(lv_event_t *e)
{
    lv_obj_t   *obj   = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t   coords;
    lv_obj_get_coords(obj, &coords);

    int32_t ox = coords.x1, oy = coords.y1;
    int32_t CW = lv_obj_get_width(obj), CH = lv_obj_get_height(obj);

    /* ── Grid background ── */
    draw_grid(layer, &coords, ox, oy, CW, CH);

    /* ── Crop selection overlay ── */
    if (g.has_crop || g.cropping) {
        float cx1 = img_to_vp_x(LV_MIN(g.crop_x1, g.crop_x2)) + ox;
        float cy1 = img_to_vp_y(LV_MIN(g.crop_y1, g.crop_y2)) + oy;
        float cx2 = img_to_vp_x(LV_MAX(g.crop_x1, g.crop_x2)) + ox;
        float cy2 = img_to_vp_y(LV_MAX(g.crop_y1, g.crop_y2)) + oy;

        lv_area_t ca = {(int32_t)cx1, (int32_t)cy1, (int32_t)cx2, (int32_t)cy2};
        lv_draw_rect_dsc_t sel;
        lv_draw_rect_dsc_init(&sel);
        sel.bg_color     = H(C_CROP_SEL);
        sel.bg_opa       = LV_OPA_20;
        sel.border_color = H(C_CROP_SEL);
        sel.border_width = 1;
        sel.border_opa   = LV_OPA_COVER;
        lv_draw_rect(layer, &sel, &ca);
    }

    /* ── Committed draw objects ── */
    draw_committed_objects(layer, ox, oy);

    /* ── Ghost preview ── */
    draw_ghost_preview(layer, ox, oy);

    draw_forged_progress(layer, ox, oy);

    /* ── Crosshair ── */
    lv_draw_line_dsc_t ch_dsc;
    lv_draw_line_dsc_init(&ch_dsc);
    ch_dsc.color     = H(C_XH_LINE);
    ch_dsc.width     = 1;
    ch_dsc.dash_width = 4;
    ch_dsc.dash_gap   = 4;
    ch_dsc.opa        = LV_OPA_40;

    int32_t cvx = (int32_t)g.cursor_vx;
    int32_t cvy = (int32_t)g.cursor_vy;

    ch_dsc.p1.x = ox;      ch_dsc.p1.y = oy + cvy;
    ch_dsc.p2.x = ox + CW; ch_dsc.p2.y = oy + cvy;
    lv_draw_line(layer, &ch_dsc);

    ch_dsc.p1.x = ox + cvx; ch_dsc.p1.y = oy;
    ch_dsc.p2.x = ox + cvx; ch_dsc.p2.y = oy + CH;
    lv_draw_line(layer, &ch_dsc);

    /* ── Coordinate tooltip ── */
    char cbuf[48];
    lv_snprintf(cbuf, sizeof(cbuf), "X: %.4f mm  Z: %.4f mm",
                vp_to_mm_y(g.cursor_vy), vp_to_mm_x(g.cursor_vx));

    lv_draw_rect_dsc_t tbg;
    lv_draw_rect_dsc_init(&tbg);
    tbg.bg_color = lv_color_hex(0x000000);
    tbg.bg_opa   = LV_OPA_80;
    tbg.radius   = 3;

    lv_draw_label_dsc_t cld;
    lv_draw_label_dsc_init(&cld);
    cld.font  = FONT_SM;
    cld.color = H(C_CYAN);
    cld.text  = cbuf;

    lv_area_t tbga = {ox + cvx + 10, oy + cvy - 24, ox + cvx + 190, oy + cvy - 5};
    lv_draw_rect(layer, &tbg, &tbga);
    lv_area_t tla  = {tbga.x1 + 5, tbga.y1 + 2, tbga.x2, tbga.y2};
    lv_draw_label(layer, &cld, &tla);

    draw_gap_markers(layer, ox, oy);

    /* ── Canvas border ── */
    lv_draw_rect_dsc_t ob;
    lv_draw_rect_dsc_init(&ob);
    ob.bg_opa      = LV_OPA_TRANSP;
    ob.border_color = H(C_BORDER);
    ob.border_width = 1;
    lv_draw_rect(layer, &ob, &coords);
}

/* =========================================================
 * MAGNIFIER (crop preview placeholder)
 * ========================================================= */
void mag_draw_cb(lv_event_t *e)
{
    lv_obj_t   *obj   = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t   coords;
    lv_obj_get_coords(obj, &coords);

    draw_quick_rect(layer, H(C_MAG_BG), LV_OPA_COVER, &coords);

    if (!g.has_crop) {
        lv_draw_label_dsc_t ld;
        lv_draw_label_dsc_init(&ld);
        ld.text  = "Crop to view";
        ld.color = H(C_DIM);
        ld.font  = FONT_SM;
        ld.align = LV_TEXT_ALIGN_CENTER;
        lv_draw_label(layer, &ld, &coords);
    }

    lv_draw_rect_dsc_t mb;
    lv_draw_rect_dsc_init(&mb);
    mb.bg_opa      = LV_OPA_TRANSP;
    mb.border_color = H(C_CYAN);
    mb.border_width = 1;
    lv_draw_rect(layer, &mb, &coords);
}