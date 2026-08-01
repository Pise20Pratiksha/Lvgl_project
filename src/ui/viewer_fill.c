/*==============================================================
 * src/ui/viewer_fill.c  — v11.0 (production-ready)
 *
 * FEATURES:
 *  • Scanline polygon fill for closed line/arc boundaries
 *  • Box grid fill for axis-aligned rectangles
 *  • Full gap detection with original-endpoint tracking
 *  • No recursion — iterative, embedded-friendly
 *  • Snapped coordinates for fill, ORIGINAL coordinates for display
 *==============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "lvgl/lvgl.h"
#include "../include/viewer.h"
#include "../include/viewer_fill.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static FillState g_fill = {0};

#define SNAP_TOL      50.0f
#define MAX_SCAN_ROWS 512

/* =========================================================
 * FLAT SEGMENT LIST
 * ======================================================== */
typedef struct {
    float x1, y1, x2, y2;
    int   obj_idx;      /* source object in g.draw_objects */
    int   end_pt;       /* 0=segment starts at obj endpoint, 1=ends at obj endpoint */
} FSeg;

#define MAX_FSEGS  (MAX_DRAW_OBJECTS * 40)
static FSeg fs[MAX_FSEGS];
static int  fs_cnt = 0;

static void fs_push(float x1, float y1, float x2, float y2, int oi, int ep)
{
    if (fs_cnt >= MAX_FSEGS) return;
    if (hypotf(x2-x1, y2-y1) < 0.5f) return;
    fs[fs_cnt++] = (FSeg){x1, y1, x2, y2, oi, ep};
}

/* =========================================================
 * SNAP ENDPOINTS (in-place)
 * ======================================================== */
static void snap_endpoints(float tol)
{
    int n = fs_cnt * 2;
    static float ex[MAX_FSEGS * 2];
    static float ey[MAX_FSEGS * 2];

    for (int i = 0; i < fs_cnt; i++) {
        ex[i*2  ] = fs[i].x1; ey[i*2  ] = fs[i].y1;
        ex[i*2+1] = fs[i].x2; ey[i*2+1] = fs[i].y2;
    }

    float tol2 = tol * tol;
    for (int i = 0; i < n; i++) {
        for (int j = i+1; j < n; j++) {
            float dx = ex[i]-ex[j], dy = ey[i]-ey[j];
            if (dx*dx + dy*dy <= tol2) {
                float mx = (ex[i]+ex[j])*0.5f;
                float my = (ey[i]+ey[j])*0.5f;
                ex[i]=ex[j]=mx;
                ey[i]=ey[j]=my;
            }
        }
    }
    for (int i = 0; i < fs_cnt; i++) {
        fs[i].x1=ex[i*2  ]; fs[i].y1=ey[i*2  ];
        fs[i].x2=ex[i*2+1]; fs[i].y2=ey[i*2+1];
    }
}

/* =========================================================
 * COLLECT SEGMENTS FROM DRAW OBJECTS
 * ======================================================== */
static void collect_segments(void)
{
    fs_cnt = 0;
    const int ARC_DIV = 32;

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type == OBJ_LINE) {
            /* end_pt: 0=start (x1,y1), 1=end (x2,y2) */
            fs_push(o->x1, o->y1, o->x2, o->y2, i, 0);
        } else if (o->type == OBJ_ARC) {
            float px = o->x1, py = o->y1;
            for (int s = 1; s <= ARC_DIV; s++) {
                float t=(float)s/ARC_DIV, it=1.0f-t;
                float qx = it*it*o->x1 + 2*it*t*o->cx + t*t*o->x2;
                float qy = it*it*o->y1 + 2*it*t*o->cy + t*t*o->y2;
                /* First segment starts at obj endpoint, last ends at obj endpoint */
                int ep = (s == ARC_DIV) ? 1 : -1;
                fs_push(px, py, qx, qy, i, ep);
                px=qx; py=qy;
            }
        }
    }
    if (fs_cnt > 0) snap_endpoints(SNAP_TOL);
}

/* =========================================================
 * RAY CAST
 * ======================================================== */
static int ray_cast_count(float x, float y)
{
    int cnt = 0;
    for (int i = 0; i < fs_cnt; i++) {
        float ax=fs[i].x1, ay=fs[i].y1;
        float bx=fs[i].x2, by=fs[i].y2;
        if ((ay <= y && by > y) || (by <= y && ay > y)) {
            float t  = (y - ay) / (by - ay);
            float ix = ax + t * (bx - ax);
            if (x < ix) cnt++;
        }
    }
    return cnt;
}

/* =========================================================
 * SCANLINE X-CROSSINGS
 * ======================================================== */
static int scan_crossings(float y, float xs[], int max_xs)
{
    int cnt = 0;
    for (int i = 0; i < fs_cnt && cnt < max_xs; i++) {
        float ay=fs[i].y1, by=fs[i].y2;
        float ax=fs[i].x1, bx=fs[i].x2;
        if ((ay <= y && by > y) || (by <= y && ay > y)) {
            float t  = (y - ay) / (by - ay);
            float ix = ax + t * (bx - ax);
            int k = cnt;
            while (k > 0 && xs[k-1] > ix) { xs[k]=xs[k-1]; k--; }
            xs[k] = ix;
            cnt++;
        }
    }
    return cnt;
}

/* =========================================================
 * BASIC GEOMETRY
 * ======================================================== */
static bool point_in_box(const DrawObject *o, float x, float y)
{
    float mnx=(o->x1<o->x2)?o->x1:o->x2;
    float mxx=(o->x1>o->x2)?o->x1:o->x2;
    float mny=(o->y1<o->y2)?o->y1:o->y2;
    float mxy=(o->y1>o->y2)?o->y1:o->y2;
    return x>=mnx && x<=mxx && y>=mny && y<=mxy;
}

static bool point_in_polygon(float x, float y, const float *px,
                               const float *py, int n)
{
    bool in = false;
    for (int i = 0, j = n-1; i < n; j = i++) {
        if (((py[i]>y) != (py[j]>y)) &&
            (x < (px[j]-px[i])*(y-py[i])/(py[j]-py[i]) + px[i]))
            in = !in;
    }
    return in;
}

/* =========================================================
 * PHASE A — BOX GRID FILL
 * ======================================================== */
#define MAX_COORDS 256

static int float_cmp(const void *a, const void *b)
{
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}

static bool phase_a_box_grid_fill(float cx, float cy, uint32_t color)
{
    float xs[MAX_COORDS], ys[MAX_COORDS];
    int xc = 0, yc = 0;

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type != OBJ_BOX) continue;

        float mnx = (o->x1 < o->x2) ? o->x1 : o->x2;
        float mxx = (o->x1 > o->x2) ? o->x1 : o->x2;
        float mny = (o->y1 < o->y2) ? o->y1 : o->y2;
        float mxy = (o->y1 > o->y2) ? o->y1 : o->y2;

        bool hmnx = false, hmxx = false;
        bool hmny = false, hmxy = false;

        for (int k = 0; k < xc; k++) {
            if (fabsf(xs[k]-mnx) < 0.5f) hmnx = true;
            if (fabsf(xs[k]-mxx) < 0.5f) hmxx = true;
        }
        for (int k = 0; k < yc; k++) {
            if (fabsf(ys[k]-mny) < 0.5f) hmny = true;
            if (fabsf(ys[k]-mxy) < 0.5f) hmxy = true;
        }

        if (!hmnx && xc < MAX_COORDS) xs[xc++] = mnx;
        if (!hmxx && xc < MAX_COORDS) xs[xc++] = mxx;
        if (!hmny && yc < MAX_COORDS) ys[yc++] = mny;
        if (!hmxy && yc < MAX_COORDS) ys[yc++] = mxy;
    }

    if (xc < 2 || yc < 2) return false;

    qsort(xs, xc, sizeof(float), float_cmp);
    qsort(ys, yc, sizeof(float), float_cmp);

    int col = -1, row = -1;
    for (int k = 0; k < xc-1; k++) {
        if (cx >= xs[k] && cx < xs[k+1]) { col = k; break; }
    }
    for (int k = 0; k < yc-1; k++) {
        if (cy >= ys[k] && cy < ys[k+1]) { row = k; break; }
    }
    if (col < 0 || row < 0) return false;

    float cx1 = xs[col],   cy1 = ys[row];
    float cx2 = xs[col+1], cy2 = ys[row+1];
    float ccx = (cx1+cx2)*0.5f, ccy = (cy1+cy2)*0.5f;

    bool inside = false;
    for (int i = 0; i < g.draw_count && !inside; i++) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type == OBJ_BOX && point_in_box(o, ccx, ccy))
            inside = true;
    }
    if (!inside || g.draw_count >= MAX_DRAW_OBJECTS) return false;

    DrawObject *fill = &g.draw_objects[g.draw_count++];
    fill->type       = OBJ_POLY;
    fill->color      = color;
    fill->width      = -1;
    fill->vert_count = 4;
    fill->verts_x[0] = cx1; fill->verts_y[0] = cy1;
    fill->verts_x[1] = cx2; fill->verts_y[1] = cy1;
    fill->verts_x[2] = cx2; fill->verts_y[2] = cy2;
    fill->verts_x[3] = cx1; fill->verts_y[3] = cy2;
    fill->selected   = false;
    g.undo_top       = 0;
    return true;
}

/* =========================================================
 * PHASE B — SCANLINE POLYGON FILL
 * ======================================================== */
static bool phase_b_scanline_fill(float cx, float cy, uint32_t color)
{
    if (fs_cnt < 2) return false;

    /* Confirm click is inside via ray cast */
    int crossings = ray_cast_count(cx, cy);
    printf("[phase_b] crossings=%d\n", crossings);
    if ((crossings % 2) == 0) return false;

    /* Find Y extent */
    float y_min = FLT_MAX, y_max = -FLT_MAX;
    for (int i = 0; i < fs_cnt; i++) {
        float ylo = (fs[i].y1 < fs[i].y2) ? fs[i].y1 : fs[i].y2;
        float yhi = (fs[i].y1 > fs[i].y2) ? fs[i].y1 : fs[i].y2;
        if (ylo < y_min) y_min = ylo;
        if (yhi > y_max) y_max = yhi;
    }
    if (y_max <= y_min) return false;

    float height = y_max - y_min;
    int   n_rows = MAX_SCAN_ROWS;
    float step   = height / (float)n_rows;
    if (step < 1.0f) {
        step   = 1.0f;
        n_rows = (int)(height + 1.0f);
        if (n_rows > MAX_SCAN_ROWS) n_rows = MAX_SCAN_ROWS;
    }

    static float left_x [MAX_SCAN_ROWS];
    static float right_x[MAX_SCAN_ROWS];
    static float row_y  [MAX_SCAN_ROWS];
    int valid_rows = 0;

    static float scan_xs[MAX_DRAW_OBJECTS * 32];

    for (int r = 0; r < n_rows; r++) {
        float y  = y_min + (r + 0.5f) * step;
        int   nc = scan_crossings(y, scan_xs, MAX_DRAW_OBJECTS * 32);
        if (nc < 2) continue;

        float lx = FLT_MAX, rx = -FLT_MAX;
        bool  found = false;

        /* Find the crossing pair that straddles cx */
        for (int k = 0; k+1 < nc && !found; k++) {
            if (scan_xs[k] <= cx && cx <= scan_xs[k+1]) {
                lx = scan_xs[k];
                rx = scan_xs[k+1];
                found = true;
            }
        }
        if (!found) continue;

        left_x [valid_rows] = lx;
        right_x[valid_rows] = rx;
        row_y  [valid_rows] = y;
        valid_rows++;
    }

    printf("[phase_b] valid_rows=%d\n", valid_rows);
    if (valid_rows < 2) return false;

    /* Build polygon: left edge top→bottom, right edge bottom→top */
    int stride = 1;
    while (((valid_rows / stride) * 2) > MAX_POLY_VERTS - 4) stride++;

    static float pvx[MAX_POLY_VERTS];
    static float pvy[MAX_POLY_VERTS];
    int pvc = 0;

    /* Left edge: top → bottom */
    for (int r = 0; r < valid_rows && pvc < MAX_POLY_VERTS/2; r += stride) {
        pvx[pvc] = left_x[r];
        pvy[pvc] = row_y [r];
        pvc++;
    }
    /* Right edge: bottom → top */
    for (int r = valid_rows-1; r >= 0 && pvc < MAX_POLY_VERTS-1; r -= stride) {
        pvx[pvc] = right_x[r];
        pvy[pvc] = row_y  [r];
        pvc++;
    }

    if (pvc < 3 || g.draw_count >= MAX_DRAW_OBJECTS) return false;

    DrawObject *fill = &g.draw_objects[g.draw_count++];
    fill->type       = OBJ_POLY;
    fill->color      = color;
    fill->width      = -1;
    fill->vert_count = pvc;
    for (int k = 0; k < pvc; k++) {
        fill->verts_x[k] = pvx[k];
        fill->verts_y[k] = pvy[k];
    }
    fill->selected = false;
    g.undo_top     = 0;

    printf("[phase_b] Scanline fill OK: %d verts\n", pvc);
    return true;
}

/* =========================================================
 * OPEN ENDPOINT DETECTION — ORIGINAL COORDINATES
 *
 * Detects gaps using snapped segments for connectivity,
 * but stores ORIGINAL object endpoints for display.
 * This prevents markers from appearing at wrong locations
 * on curved segments.
 * ======================================================== */
static void detect_and_highlight_open_endpoints(void)
{
    g.gap_pts_count = 0;
    g.has_gaps      = false;
    g.gap_count     = 0;

    if (fs_cnt == 0) collect_segments();
    if (fs_cnt == 0) return;

    float tol2 = SNAP_TOL * SNAP_TOL;

    /* 
     * For each original object, check if its endpoints are connected
     * to any other object's endpoint (using snapped coordinates).
     * Store the ORIGINAL coordinates for display markers.
     */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type != OBJ_LINE && o->type != OBJ_ARC) continue;

        /* Each object has 2 endpoints to check */
        struct {
            float orig_x, orig_y;   /* Original drawn coordinate */
            float snap_x, snap_y;   /* Coordinate after snapping */
        } endpoints[2];

        /* Endpoint 0: start point (x1,y1) */
        endpoints[0].orig_x = o->x1;
        endpoints[0].orig_y = o->y1;

        /* Endpoint 1: end point (x2,y2) */
        endpoints[1].orig_x = o->x2;
        endpoints[1].orig_y = o->y2;

        /* Find snapped positions for each original endpoint */
        for (int e = 0; e < 2; e++) {
            endpoints[e].snap_x = endpoints[e].orig_x;
            endpoints[e].snap_y = endpoints[e].orig_y;

            /* Search fs[] for a segment from this object that matches */
            for (int j = 0; j < fs_cnt; j++) {
                if (fs[j].obj_idx != i) continue;

                float d1 = hypotf(endpoints[e].orig_x - fs[j].x1,
                                  endpoints[e].orig_y - fs[j].y1);
                float d2 = hypotf(endpoints[e].orig_x - fs[j].x2,
                                  endpoints[e].orig_y - fs[j].y2);

                if (d1 < 2.0f) {
                    endpoints[e].snap_x = fs[j].x1;
                    endpoints[e].snap_y = fs[j].y1;
                    break;
                } else if (d2 < 2.0f) {
                    endpoints[e].snap_x = fs[j].x2;
                    endpoints[e].snap_y = fs[j].y2;
                    break;
                }
            }
        }

        /* Check connectivity for each endpoint */
        for (int e = 0; e < 2; e++) {
            float sx = endpoints[e].snap_x;
            float sy = endpoints[e].snap_y;

            /* An endpoint is connected if ANY segment from ANOTHER object
             * has an endpoint within SNAP_TOL of our snapped position */
            bool connected = false;
            for (int j = 0; j < fs_cnt && !connected; j++) {
                if (fs[j].obj_idx == i) continue; /* Same object */

                float d1x = sx - fs[j].x1, d1y = sy - fs[j].y1;
                float d2x = sx - fs[j].x2, d2y = sy - fs[j].y2;

                if (d1x*d1x + d1y*d1y <= tol2) connected = true;
                if (d2x*d2x + d2y*d2y <= tol2) connected = true;
            }

            if (!connected) {
                /* Store ORIGINAL coordinate for display */
                float ox = endpoints[e].orig_x;
                float oy = endpoints[e].orig_y;

                /* Prevent duplicate markers at same location */
                bool already = false;
                for (int k = 0; k < g.gap_pts_count; k++) {
                    float dx = g.gap_pts_x[k] - ox;
                    float dy = g.gap_pts_y[k] - oy;
                    if (dx*dx + dy*dy < 9.0f) { /* 3px tolerance */
                        already = true; break;
                    }
                }

                if (!already && g.gap_pts_count < MAX_GAP_MARKERS) {
                    g.gap_pts_x[g.gap_pts_count] = ox;
                    g.gap_pts_y[g.gap_pts_count] = oy;
                    g.gap_pts_count++;
                }
            }
        }
    }

    if (g.gap_pts_count > 0) {
        g.has_gaps  = true;
        g.gap_count = g.gap_pts_count;
    }
    printf("[gap] %d open endpoint(s) at original coords\n", g.gap_pts_count);
}

/* =========================================================
 * INTERNAL FLOOD FILL
 * ======================================================== */
static bool viewer_flood_fill_internal(float x, float y, uint32_t new_color,
                                        int tolerance, bool record_history)
{
    (void)tolerance;

    if (x < -5000.0f || y < -5000.0f ||
        x > (float)(VIRTUAL_W+5000) || y > (float)(VIRTUAL_H+5000))
        return false;

    bool filled = false;

    /* Phase A: box */
    for (int i = 0; i < g.draw_count && !filled; i++) {
        if (g.draw_objects[i].type == OBJ_BOX &&
            point_in_box(&g.draw_objects[i], x, y)) {
            filled = phase_a_box_grid_fill(x, y, new_color);
            printf("[fill] Phase A: %s\n", filled ? "OK" : "fail");
        }
    }

    /* Phase B: scanline */
    if (!filled) {
        filled = phase_b_scanline_fill(x, y, new_color);
        printf("[fill] Phase B: %s\n", filled ? "OK" : "fail");
    }

    /* Phase C: recolor existing primitive under click */
    if (!filled) {
        for (int i = g.draw_count-1; i >= 0; i--) {
            DrawObject *obj = &g.draw_objects[i];
            if (obj->type == OBJ_BOX && point_in_box(obj, x, y)) {
                uint32_t old = obj->color;
                if (old == new_color && obj->width < 0) return true;
                if (obj->width >= 0)
                    obj->width = -(obj->width > 0 ? obj->width : 2);
                obj->color = new_color;
                if (record_history)
                    viewer_color_history_add(new_color, old, x, y);
                return true;
            }
            if (obj->type == OBJ_POLY && obj->vert_count > 2 &&
                point_in_polygon(x, y, obj->verts_x, obj->verts_y,
                                 obj->vert_count)) {
                uint32_t old = obj->color;
                if (old == new_color) return true;
                obj->color = new_color;
                if (obj->width >= 0) obj->width = -1;
                if (record_history)
                    viewer_color_history_add(new_color, old, x, y);
                return true;
            }
        }
    }

    if (filled && record_history)
        viewer_color_history_add(new_color, 0x000000, x, y);
    return filled;
}

/* =========================================================
 * PUBLIC: viewer_flood_fill
 * ======================================================== */
bool viewer_flood_fill(float x, float y, uint32_t new_color, int tolerance)
{
    /* Clear any previous gap markers */
    g.gap_pts_count = 0;
    g.has_gaps      = false;
    g.gap_count     = 0;

    /* Always rebuild segment list fresh */
    fs_cnt = 0;
    collect_segments();

    bool filled = viewer_flood_fill_internal(x, y, new_color, tolerance, true);

    if (filled) {
        /* Successful fill — ensure no gap markers remain */
        g.gap_pts_count = 0;
        g.has_gaps      = false;
        g.gap_count     = 0;
        if (g.hint_lbl)
            lv_label_set_text(g.hint_lbl,
                "Fill Color  |  Tap to fill enclosed area");
    } else {
        /* Fill failed — detect and highlight ALL open endpoints */
        detect_and_highlight_open_endpoints();

        if (g.hint_lbl) {
            if (g.gap_pts_count > 0) {
                char buf[96];
                lv_snprintf(buf, sizeof(buf),
                    LV_SYMBOL_WARNING
                    "  Shape open — %d gap(s) found, color would leak",
                    g.gap_pts_count);
                lv_label_set_text(g.hint_lbl, buf);
            } else {
                lv_label_set_text(g.hint_lbl,
                    "Fill  |  Tap inside a closed shape");
            }
        }
    }

    return filled;
}

/* =========================================================
 * PUBLIC GAP DETECTION (CHECK mode)
 * ======================================================== */
GapDetectionResult viewer_detect_gaps(float x1, float y1, float x2, float y2,
                                       float min_gap_px)
{
    GapDetectionResult result = {0};
    result.min_gap_size_px = min_gap_px;
    int gap_idx = 0;

    for (int i = 0; i < g.draw_count && gap_idx < MAX_DETECTED_GAPS; i++) {
        DrawObject *obj = &g.draw_objects[i];
        if (obj->type != OBJ_LINE && obj->type != OBJ_ARC) continue;

        float ep[2][2] = {{obj->x1,obj->y1},{obj->x2,obj->y2}};
        for (int ei = 0; ei < 2 && gap_idx < MAX_DETECTED_GAPS; ei++) {
            float ex = ep[ei][0], ey = ep[ei][1];
            if (ex < x1 || ex > x2 || ey < y1 || ey > y2) continue;

            bool connected = false;
            for (int j = 0; j < g.draw_count && !connected; j++) {
                DrawObject *other = &g.draw_objects[j];
                if (other == obj) continue;
                if (other->type != OBJ_LINE && other->type != OBJ_ARC) continue;
                if (hypotf(ex-other->x1,ey-other->y1) < min_gap_px) connected=true;
                if (hypotf(ex-other->x2,ey-other->y2) < min_gap_px) connected=true;
            }
            if (!connected) {
                result.gaps[gap_idx].x    = ex;
                result.gaps[gap_idx].y    = ey;
                result.gaps[gap_idx].size = (int)min_gap_px;
                gap_idx++;
                result.has_gaps = true;
            }
        }
    }
    result.count = gap_idx;
    return result;
}

void viewer_highlight_gaps(const GapDetectionResult *result, uint32_t color)
{
    (void)color;
    g.gap_pts_count = 0;
    if (!result || !result->has_gaps || result->count == 0) return;
    int store = (result->count < MAX_GAP_MARKERS)
                ? result->count : MAX_GAP_MARKERS;
    for (int i = 0; i < store; i++) {
        g.gap_pts_x[i] = result->gaps[i].x;
        g.gap_pts_y[i] = result->gaps[i].y;
    }
    g.gap_pts_count = store;
    g.has_gaps      = true;
    g.gap_count     = store;
}

bool viewer_point_in_closed_shape(float x, float y)
{
    int cr = 0;
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];
        if (o->type != OBJ_LINE) continue;
        float x1=o->x1, y1=o->y1, x2=o->x2, y2=o->y2;
        if ((y1<=y && y<y2) || (y2<=y && y<y1))
            if (x < x1 + (y-y1)*(x2-x1)/(y2-y1)) cr++;
    }
    return (cr % 2) == 1;
}

/* =========================================================
 * COLOR HISTORY
 * ======================================================== */
const uint32_t *viewer_get_palette(void) { return professional_palette; }

void viewer_color_history_add(uint32_t color, uint32_t target, float x, float y)
{
    if (g_fill.color_history.top >= MAX_COLOR_HISTORY) {
        memmove(&g_fill.color_history.actions[0],
                &g_fill.color_history.actions[1],
                sizeof(ColorFillAction) * (MAX_COLOR_HISTORY - 1));
        g_fill.color_history.top--;
    }
    ColorFillAction *a = &g_fill.color_history.actions[g_fill.color_history.top];
    a->color=color; a->target_color=target; a->fill_x=x; a->fill_y=y;
    g_fill.color_history.top++;
    g_fill.color_history.max = g_fill.color_history.top;
}

bool viewer_color_undo(void)
{
    if (g_fill.color_history.top <= 0) return false;
    g_fill.color_history.top--;
    ColorFillAction *a =
        &g_fill.color_history.actions[g_fill.color_history.top];
    viewer_flood_fill_internal(a->fill_x, a->fill_y, a->target_color, 5, false);
    return true;
}

bool viewer_color_redo(void)
{
    if (g_fill.color_history.top >= g_fill.color_history.max) return false;
    ColorFillAction *a =
        &g_fill.color_history.actions[g_fill.color_history.top];
    viewer_flood_fill_internal(a->fill_x, a->fill_y, a->color, 5, false);
    g_fill.color_history.top++;
    return true;
}

void viewer_remove_color(float x, float y, float s)
{
    (void)x; (void)y; (void)s;
}

/* =========================================================
 * FILL STATE
 * ======================================================== */
void viewer_fill_init(void)
{
    memset(&g_fill, 0, sizeof(FillState));
    g_fill.current_color   = 0x38BDF8;
    g_fill.highlight_color = 0xFF6B6B;
    g.fill_color           = g_fill.current_color;
}

void     viewer_set_fill_color(uint32_t c) { g_fill.current_color=c; g.fill_color=c; }
uint32_t viewer_get_fill_color(void)       { return g.fill_color; }
void     viewer_set_gap_display(bool e)    { g_fill.show_gaps = e; }

GapDetectionResult viewer_validate_closed_shape(float x1, float y1,
                                                 float x2, float y2)
{
    g_fill.last_gaps = viewer_detect_gaps(x1, y1, x2, y2, SNAP_TOL);
    if (g_fill.last_gaps.has_gaps && g_fill.show_gaps)
        viewer_highlight_gaps(&g_fill.last_gaps, g_fill.highlight_color);
    return g_fill.last_gaps;
}