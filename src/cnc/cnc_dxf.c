/*==============================================================
 * src/cnc/cnc_dxf.c
 * DXF R12 save/load for the canvas drawing (g.draw_objects[]).
 *
 * Additive only — does not touch cnc_sketch.c / events.c / draw.c.
 * Reuses cnc_get_sketch_path()/SKETCH_DIR from cnc_sketch.h so files
 * land in the same folder as your JSON sketches, just with a .dxf name.
 *
 * Round-trip strategy:
 *   - Real DXF entities (LINE / POLYLINE) carry the visible geometry.
 *   - A "CNCVIEWER" XDATA block (group 1001 appid + 1000 "KEY=VALUE"
 *     strings) rides on every entity, carrying the exact original
 *     DrawObject fields (pipe_diameter, forged segment chain, etc.)
 *     so cnc_sketch_load_dxf() rebuilds objects exactly as drawn.
 *==============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "cnc_dxf.h"
#include "cnc_sketch.h"

#include "../include/viewer.h"
#include "../include/viewer_ui.h"   /* refresh_all(), update_panel()   */

#define DXF_APPID "CNCVIEWER"

/* =========================================================
 * WRITER — small helpers
 * ========================================================= */
static void dxf_write_prelude(FILE *f)
{
    fprintf(f,
        "0\nSECTION\n2\nHEADER\n"
        "9\n$ACADVER\n1\nAC1009\n"
        "0\nENDSEC\n"
        "0\nSECTION\n2\nTABLES\n"
        "0\nTABLE\n2\nAPPID\n70\n1\n"
        "0\nAPPID\n2\n" DXF_APPID "\n70\n0\n"
        "0\nENDTAB\n"
        "0\nENDSEC\n");
}

static void dxf_write_line(FILE *f, const char *layer,
                            float x1, float y1, float x2, float y2)
{
    fprintf(f,
        "0\nLINE\n8\n%s\n"
        "10\n%.6f\n20\n%.6f\n30\n0.0\n"
        "11\n%.6f\n21\n%.6f\n31\n0.0\n",
        layer, x1, y1, x2, y2);
}

static void dxf_write_polyline(FILE *f, const char *layer,
                                const float *xs, const float *ys,
                                int n, bool closed)
{
    fprintf(f, "0\nPOLYLINE\n8\n%s\n66\n1\n70\n%d\n",
            layer, closed ? 1 : 0);
    /* XDATA is written by the caller right after this header, before
     * the loop below — see write sites in cnc_sketch_save_dxf(). */
    (void)xs; (void)ys; (void)n;
}

static void dxf_write_polyline_verts(FILE *f, const char *layer,
                                      const float *xs, const float *ys, int n)
{
    for (int i = 0; i < n; i++) {
        fprintf(f, "0\nVERTEX\n8\n%s\n10\n%.6f\n20\n%.6f\n30\n0.0\n",
                layer, xs[i], ys[i]);
    }
    fprintf(f, "0\nSEQEND\n8\n%s\n", layer);
}

static void dxf_xdata_begin(FILE *f)  { fprintf(f, "1001\n%s\n", DXF_APPID); }
static void dxf_xdata_kv_str(FILE *f, const char *k, const char *v)
{ fprintf(f, "1000\n%s=%s\n", k, v); }
static void dxf_xdata_kv_int(FILE *f, const char *k, int v)
{ fprintf(f, "1000\n%s=%d\n", k, v); }
static void dxf_xdata_kv_float(FILE *f, const char *k, float v)
{ fprintf(f, "1000\n%s=%.6f\n", k, (double)v); }

/* =========================================================
 * WRITER — main entry
 * ========================================================= */
bool cnc_sketch_save_dxf(const char *filename)
{
    mkdir(SKETCH_DIR, 0755);

    char fullpath[256];
    cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));

    FILE *f = fopen(fullpath, "w");
    if (!f) {
        printf("[cnc_dxf] Error: Cannot open %s for writing\n", fullpath);
        return false;
    }

    dxf_write_prelude(f);
    fprintf(f, "0\nSECTION\n2\nENTITIES\n");

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];

        switch (o->type) {

        case OBJ_LINE:
            dxf_write_line(f, "LINE", o->x1, o->y1, o->x2, o->y2);
            dxf_xdata_begin(f);
            dxf_xdata_kv_str(f, "OBJTYPE", "LINE");
            dxf_xdata_kv_int(f, "WIDTH", o->width);
            dxf_xdata_kv_int(f, "COLOR", (int)o->color);
            break;

        case OBJ_ARC: {
            /* Flatten the quadratic bezier for visual geometry; exact
             * control point + endpoints are preserved in XDATA below. */
            float xs[33], ys[33];
            for (int s = 0; s <= 32; s++) {
                float t = (float)s / 32.0f, mt = 1.0f - t;
                xs[s] = mt*mt*o->x1 + 2.0f*mt*t*o->cx + t*t*o->x2;
                ys[s] = mt*mt*o->y1 + 2.0f*mt*t*o->cy + t*t*o->y2;
            }
            dxf_write_polyline(f, "ARC", xs, ys, 33, false);
            dxf_xdata_begin(f);
            dxf_xdata_kv_str(f,   "OBJTYPE", "ARC");
            dxf_xdata_kv_float(f, "X1", o->x1);
            dxf_xdata_kv_float(f, "Y1", o->y1);
            dxf_xdata_kv_float(f, "X2", o->x2);
            dxf_xdata_kv_float(f, "Y2", o->y2);
            dxf_xdata_kv_float(f, "CX", o->cx);
            dxf_xdata_kv_float(f, "CY", o->cy);
            dxf_xdata_kv_int(f,   "WIDTH", o->width);
            dxf_xdata_kv_int(f,   "COLOR", (int)o->color);
            dxf_write_polyline_verts(f, "ARC", xs, ys, 33);
            break;
        }

        case OBJ_BOX: {
            float xs[5] = { o->x1, o->x2, o->x2, o->x1, o->x1 };
            float ys[5] = { o->y1, o->y1, o->y2, o->y2, o->y1 };
            dxf_write_polyline(f, "BOX", xs, ys, 5, true);
            dxf_xdata_begin(f);
            dxf_xdata_kv_str(f, "OBJTYPE", "BOX");
            dxf_xdata_kv_int(f, "WIDTH", o->width);
            dxf_xdata_kv_int(f, "COLOR", (int)o->color);
            dxf_write_polyline_verts(f, "BOX", xs, ys, 5);
            break;
        }

        case OBJ_POLY:
            if (o->vert_count >= 3) {
                dxf_write_polyline(f, "POLY", o->verts_x, o->verts_y, o->vert_count, true);
                dxf_xdata_begin(f);
                dxf_xdata_kv_str(f, "OBJTYPE", "POLY");
                dxf_xdata_kv_int(f, "WIDTH", o->width);
                dxf_xdata_kv_int(f, "COLOR", (int)o->color);
                dxf_write_polyline_verts(f, "POLY", o->verts_x, o->verts_y, o->vert_count);
            }
            break;

        case OBJ_BAR: {
            float xs[5] = { o->x1, o->x2, o->x2, o->x1, o->x1 };
            float ys[5] = { o->y1, o->y1, o->y2, o->y2, o->y1 };
            dxf_write_polyline(f, "BAR", xs, ys, 5, true);
            dxf_xdata_begin(f);
            dxf_xdata_kv_str(f, "OBJTYPE", "BAR");
            dxf_xdata_kv_int(f, "WIDTH", o->width);
            dxf_write_polyline_verts(f, "BAR", xs, ys, 5);
            break;
        }

        case OBJ_PIPE: {
            float xs[5] = { o->x1, o->x2, o->x2, o->x1, o->x1 };
            float ys[5] = { o->y1, o->y1, o->y2, o->y2, o->y1 };
            dxf_write_polyline(f, "PIPE", xs, ys, 5, true);
            dxf_xdata_begin(f);
            dxf_xdata_kv_str(f,   "OBJTYPE", "PIPE");
            dxf_xdata_kv_int(f,   "WIDTH", o->width);
            dxf_xdata_kv_float(f, "PIPE_DIAM", o->pipe_diameter);
            dxf_write_polyline_verts(f, "PIPE", xs, ys, 5);
            break;
        }

        case OBJ_FORGED:
            /* Each segment becomes its own LINE entity (straight chord —
             * fine for a visual reference; the true arc/line geometry is
             * restored from XDATA on load). All segments belonging to
             * this object share FORGED_ID=i so they regroup correctly. */
            for (int k = 0; k < o->seg_count; k++) {
                ForgedSegment *sg = &o->seg_data[k];
                dxf_write_line(f, "FORGED", sg->x1, sg->y1, sg->x2, sg->y2);
                dxf_xdata_begin(f);
                dxf_xdata_kv_str(f,   "OBJTYPE",   "FORGED_SEG");
                dxf_xdata_kv_int(f,   "FORGED_ID", i);
                dxf_xdata_kv_int(f,   "SEG_IDX",   k);
                dxf_xdata_kv_str(f,   "SEG_TYPE",  sg->type == FSEG_ARC ? "ARC" : "LINE");
                dxf_xdata_kv_float(f, "X1", sg->x1);
                dxf_xdata_kv_float(f, "Y1", sg->y1);
                dxf_xdata_kv_float(f, "X2", sg->x2);
                dxf_xdata_kv_float(f, "Y2", sg->y2);
                dxf_xdata_kv_float(f, "ANGLE_DEG", sg->angle_deg);
                dxf_xdata_kv_int(f,   "DIRECTION",  sg->direction);
                dxf_xdata_kv_float(f, "CX", sg->cx);
                dxf_xdata_kv_float(f, "CY", sg->cy);
                dxf_xdata_kv_float(f, "RADIUS", sg->radius);
            }
            /* NEW: centerline, carried on its own LINE + XDATA entity so
             * it round-trips exactly like segments do. */
            if (o->has_centerline) {
                dxf_write_line(f, "FORGED", o->cl_x1, o->cl_y1, o->cl_x2, o->cl_y2);
                dxf_xdata_begin(f);
                dxf_xdata_kv_str(f,   "OBJTYPE",   "FORGED_CENTERLINE");
                dxf_xdata_kv_int(f,   "FORGED_ID", i);
                dxf_xdata_kv_float(f, "CL_X1", o->cl_x1);
                dxf_xdata_kv_float(f, "CL_Y1", o->cl_y1);
                dxf_xdata_kv_float(f, "CL_X2", o->cl_x2);
                dxf_xdata_kv_float(f, "CL_Y2", o->cl_y2);
            }

            /* NEW: pattern-fill metadata. There's no real DXF geometry
             * for this, so it rides on a degenerate zero-length LINE
             * purely as an XDATA carrier — same pattern as everything
             * else in this file. */
            {
                float mx = (o->seg_count > 0) ? o->seg_data[0].x1 : o->x1;
                float my = (o->seg_count > 0) ? o->seg_data[0].y1 : o->y1;
                dxf_write_line(f, "FORGED", mx, my, mx, my);
                dxf_xdata_begin(f);
                dxf_xdata_kv_str(f, "OBJTYPE",         "FORGED_META");
                dxf_xdata_kv_int(f, "FORGED_ID",       i);
                dxf_xdata_kv_int(f, "PATTERN_FILL",    (int)o->pattern_fill);
                dxf_xdata_kv_int(f, "PATTERN_SPACING", o->pattern_spacing);
                dxf_xdata_kv_int(f, "FILL_VISIBLE",    o->fill_visible ? 1 : 0);
            }
            break;
        }
    }

    fprintf(f, "0\nENDSEC\n0\nEOF\n");
    fclose(f);

    printf("[cnc_dxf] DXF saved: %s (%d draw objects)\n", filename, g.draw_count);
    return true;
}

/* =========================================================
 * READER
 * ========================================================= */
static void dxf_trim(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n-1]=='\n' || s[n-1]=='\r' || s[n-1]==' ' || s[n-1]=='\t')) s[--n] = '\0';
    char *p = s;
    while (*p==' ' || *p=='\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
}

static bool dxf_read_pair(FILE *f, int *code, char *val, size_t valsz)
{
    char line1[64];
    if (!fgets(line1, sizeof(line1), f)) return false;
    dxf_trim(line1);
    if (!fgets(val, (int)valsz, f)) return false;
    dxf_trim(val);
    *code = atoi(line1);
    return true;
}

/* ── tiny KEY=VALUE store for the current entity's XDATA ── */
typedef struct { char key[24]; char val[64]; } XKV;
#define MAX_XKV 24
static XKV s_xkv[MAX_XKV];
static int s_xkv_count = 0;

static void xkv_reset(void) { s_xkv_count = 0; }

static void xkv_add_raw(const char *raw)   /* raw = "KEY=VALUE" */
{
    if (s_xkv_count >= MAX_XKV) return;
    const char *eq = strchr(raw, '=');
    if (!eq) return;
    size_t klen = (size_t)(eq - raw);
    if (klen >= sizeof(s_xkv[0].key)) klen = sizeof(s_xkv[0].key) - 1;
    memcpy(s_xkv[s_xkv_count].key, raw, klen);
    s_xkv[s_xkv_count].key[klen] = '\0';
    strncpy(s_xkv[s_xkv_count].val, eq + 1, sizeof(s_xkv[0].val) - 1);
    s_xkv[s_xkv_count].val[sizeof(s_xkv[0].val) - 1] = '\0';
    s_xkv_count++;
}

static const char *xkv_get(const char *k)
{
    for (int i = 0; i < s_xkv_count; i++)
        if (strcmp(s_xkv[i].key, k) == 0) return s_xkv[i].val;
    return NULL;
}
static float xkv_getf(const char *k, float def)
{ const char *v = xkv_get(k); return v ? strtof(v, NULL) : def; }
static int xkv_geti(const char *k, int def)
{ const char *v = xkv_get(k); return v ? atoi(v) : def; }

/* ── pending forged segments (grouped by FORGED_ID after parse) ── */
#define MAX_PENDING_FORGED 2048
typedef struct { int forged_id; int seg_idx; ForgedSegment seg; } PendingForged;
static PendingForged s_pending_forged[MAX_PENDING_FORGED];
static int s_pending_forged_count = 0;

/* ── pending centerlines / pattern-fill metadata (grouped by FORGED_ID) ── */
#define MAX_PENDING_META 512

typedef struct { int forged_id; float x1, y1, x2, y2; } PendingCenterline;
static PendingCenterline s_pending_centerline[MAX_PENDING_META];
static int s_pending_centerline_count = 0;

typedef struct { int forged_id; int pattern_fill; int pattern_spacing; int fill_visible; } PendingMeta;
static PendingMeta s_pending_meta[MAX_PENDING_META];
static int s_pending_meta_count = 0;


static void pending_forged_add(int fid, int sidx, const ForgedSegment *sg)
{
    if (fid < 0 || s_pending_forged_count >= MAX_PENDING_FORGED) return;
    s_pending_forged[s_pending_forged_count].forged_id = fid;
    s_pending_forged[s_pending_forged_count].seg_idx   = sidx;
    s_pending_forged[s_pending_forged_count].seg       = *sg;
    s_pending_forged_count++;
}

/* ── append helpers into g.draw_objects[] ── */
static void dxf_append_simple(DrawObjType type,
                               float x1, float y1, float x2, float y2,
                               float cx, float cy, int width, uint32_t color)
{
    if (g.draw_count >= MAX_DRAW_OBJECTS) return;
    DrawObject *o = &g.draw_objects[g.draw_count];
    memset(o, 0, sizeof(*o));
    o->type = type;
    o->x1 = x1; o->y1 = y1; o->x2 = x2; o->y2 = y2;
    o->cx = cx; o->cy = cy;
    o->width = width;
    o->color = color;
    g.draw_count++;
}

static void dxf_append_poly(const float *xs, const float *ys, int n, int width, uint32_t color)
{
    if (g.draw_count >= MAX_DRAW_OBJECTS || n < 3) return;
    int vc = (n > MAX_POLY_VERTS) ? MAX_POLY_VERTS : n;
    /* drop the closing duplicate vertex if the writer repeated vertex[0] */
    if (vc > 1 && fabsf(xs[vc-1]-xs[0]) < 0.0005f && fabsf(ys[vc-1]-ys[0]) < 0.0005f) vc--;

    DrawObject *o = &g.draw_objects[g.draw_count];
    memset(o, 0, sizeof(*o));
    o->type = OBJ_POLY;
    for (int i = 0; i < vc; i++) { o->verts_x[i] = xs[i]; o->verts_y[i] = ys[i]; }
    o->vert_count = vc;
    o->x1 = xs[0]; o->y1 = ys[0];
    o->x2 = xs[vc-1]; o->y2 = ys[vc-1];
    o->width = width;
    o->color = color;
    g.draw_count++;
}

static void dxf_append_pipe(float x1, float y1, float x2, float y2, float diameter, int width)
{
    if (g.draw_count >= MAX_DRAW_OBJECTS) return;
    DrawObject *o = &g.draw_objects[g.draw_count];
    memset(o, 0, sizeof(*o));
    o->type = OBJ_PIPE;
    o->x1 = x1; o->y1 = y1; o->x2 = x2; o->y2 = y2;
    o->pipe_diameter = diameter;
    o->width = width;
    g.draw_count++;
}

static void bbox_of(const float *xs, const float *ys, int n,
                     float *ox1, float *oy1, float *ox2, float *oy2)
{
    float x1 = xs[0], y1 = ys[0], x2 = xs[0], y2 = ys[0];
    for (int i = 1; i < n; i++) {
        if (xs[i] < x1) x1 = xs[i];
        if (xs[i] > x2) x2 = xs[i];
        if (ys[i] < y1) y1 = ys[i];
        if (ys[i] > y2) y2 = ys[i];
    }
    *ox1 = x1; *oy1 = y1; *ox2 = x2; *oy2 = y2;
}

/* ── finalize a completed simple LINE entity ── */
static void finalize_line(float x1, float y1, float x2, float y2)
{
    const char *objtype = xkv_get("OBJTYPE");
    if (!objtype) return;

    if (strcmp(objtype, "LINE") == 0) {
        dxf_append_simple(OBJ_LINE, x1, y1, x2, y2, 0, 0,
                           xkv_geti("WIDTH", 2), (uint32_t)xkv_geti("COLOR", 0xFFFFFF));
    }
    else if (strcmp(objtype, "FORGED_SEG") == 0) {
        ForgedSegment sg;
        memset(&sg, 0, sizeof(sg));
        const char *st = xkv_get("SEG_TYPE");
        sg.type      = (st && strcmp(st, "ARC") == 0) ? FSEG_ARC : FSEG_LINE;
        sg.x1        = xkv_getf("X1", x1);
        sg.y1        = xkv_getf("Y1", y1);
        sg.x2        = xkv_getf("X2", x2);
        sg.y2        = xkv_getf("Y2", y2);
        sg.angle_deg = xkv_getf("ANGLE_DEG", 0.0f);
        sg.direction = xkv_geti("DIRECTION", 1);
        sg.cx        = xkv_getf("CX", 0.0f);
        sg.cy        = xkv_getf("CY", 0.0f);
        sg.radius    = xkv_getf("RADIUS", 0.0f);
        pending_forged_add(xkv_geti("FORGED_ID", -1), xkv_geti("SEG_IDX", 0), &sg);
    }
    else if (strcmp(objtype, "FORGED_CENTERLINE") == 0) {           /* NEW */
        if (s_pending_centerline_count < MAX_PENDING_META) {
            PendingCenterline *pc = &s_pending_centerline[s_pending_centerline_count++];
            pc->forged_id = xkv_geti("FORGED_ID", -1);
            pc->x1 = xkv_getf("CL_X1", x1);
            pc->y1 = xkv_getf("CL_Y1", y1);
            pc->x2 = xkv_getf("CL_X2", x2);
            pc->y2 = xkv_getf("CL_Y2", y2);
        }
    }
    else if (strcmp(objtype, "FORGED_META") == 0) {                 /* NEW */
        if (s_pending_meta_count < MAX_PENDING_META) {
            PendingMeta *pm = &s_pending_meta[s_pending_meta_count++];
            pm->forged_id       = xkv_geti("FORGED_ID", -1);
            pm->pattern_fill    = xkv_geti("PATTERN_FILL", 0);
            pm->pattern_spacing = xkv_geti("PATTERN_SPACING", 0);
            pm->fill_visible    = xkv_geti("FILL_VISIBLE", 0);
        }
    }
}

/* ── finalize a completed POLYLINE entity ── */
static void finalize_polyline(const float *xs, const float *ys, int n)
{
    const char *objtype = xkv_get("OBJTYPE");
    if (!objtype || n < 1) return;

    if (strcmp(objtype, "ARC") == 0) {
        float x1 = xkv_getf("X1", xs[0]),   y1 = xkv_getf("Y1", ys[0]);
        float x2 = xkv_getf("X2", xs[n-1]), y2 = xkv_getf("Y2", ys[n-1]);
        float cx = xkv_getf("CX", (x1+x2)*0.5f), cy = xkv_getf("CY", (y1+y2)*0.5f);
        dxf_append_simple(OBJ_ARC, x1, y1, x2, y2, cx, cy,
                           xkv_geti("WIDTH", 2), (uint32_t)xkv_geti("COLOR", 0xFFFFFF));
    }
    else if (strcmp(objtype, "BOX") == 0) {
        float x1, y1, x2, y2;
        bbox_of(xs, ys, n, &x1, &y1, &x2, &y2);
        dxf_append_simple(OBJ_BOX, x1, y1, x2, y2, 0, 0,
                           xkv_geti("WIDTH", 2), (uint32_t)xkv_geti("COLOR", 0xFFFFFF));
    }
    else if (strcmp(objtype, "POLY") == 0) {
        dxf_append_poly(xs, ys, n, xkv_geti("WIDTH", 2), (uint32_t)xkv_geti("COLOR", 0xFFFFFF));
    }
    else if (strcmp(objtype, "BAR") == 0) {
        float x1, y1, x2, y2;
        bbox_of(xs, ys, n, &x1, &y1, &x2, &y2);
        dxf_append_simple(OBJ_BAR, x1, y1, x2, y2, 0, 0, xkv_geti("WIDTH", -1), 0);
    }
    else if (strcmp(objtype, "PIPE") == 0) {
        float x1, y1, x2, y2;
        bbox_of(xs, ys, n, &x1, &y1, &x2, &y2);
        dxf_append_pipe(x1, y1, x2, y2, xkv_getf("PIPE_DIAM", 0.0f), xkv_geti("WIDTH", -1));
    }
}

/* ── after parsing, regroup FORGED_SEG entries into OBJ_FORGED objects ── */
static void build_forged_objects_from_pending(void)
{
    if (s_pending_forged_count == 0) return;

    int seen_ids[MAX_DRAW_OBJECTS];
    int seen_count = 0;
    for (int i = 0; i < s_pending_forged_count; i++) {
        int fid = s_pending_forged[i].forged_id;
        bool found = false;
        for (int j = 0; j < seen_count; j++) if (seen_ids[j] == fid) { found = true; break; }
        if (!found && seen_count < MAX_DRAW_OBJECTS) seen_ids[seen_count++] = fid;
    }

    for (int gidx = 0; gidx < seen_count; gidx++) {
        if (g.draw_count >= MAX_DRAW_OBJECTS) break;
        int fid = seen_ids[gidx];

        ForgedSegment tmp[MAX_FORGED_SEGS];
        int tidx[MAX_FORGED_SEGS];
        int tcount = 0;
        for (int i = 0; i < s_pending_forged_count && tcount < MAX_FORGED_SEGS; i++) {
            if (s_pending_forged[i].forged_id == fid) {
                tmp[tcount]  = s_pending_forged[i].seg;
                tidx[tcount] = s_pending_forged[i].seg_idx;
                tcount++;
            }
        }
        if (tcount == 0) continue;

        /* insertion sort by seg_idx — counts are small (<= MAX_FORGED_SEGS) */
        for (int a = 0; a < tcount - 1; a++)
            for (int b = a + 1; b < tcount; b++)
                if (tidx[b] < tidx[a]) {
                    int ti = tidx[a]; tidx[a] = tidx[b]; tidx[b] = ti;
                    ForgedSegment ts = tmp[a]; tmp[a] = tmp[b]; tmp[b] = ts;
                }

        DrawObject *o = &g.draw_objects[g.draw_count];
        memset(o, 0, sizeof(*o));
        o->type = OBJ_FORGED;
        o->seg_count = tcount;
        for (int k = 0; k < tcount; k++) o->seg_data[k] = tmp[k];

        o->vert_count = 0;
        o->verts_x[o->vert_count] = tmp[0].x1;
        o->verts_y[o->vert_count] = tmp[0].y1;
        o->vert_count++;
        for (int k = 0; k < tcount && o->vert_count < MAX_POLY_VERTS; k++) {
            o->verts_x[o->vert_count] = tmp[k].x2;
            o->verts_y[o->vert_count] = tmp[k].y2;
            o->vert_count++;
        }
        o->x1 = tmp[0].x1;        o->y1 = tmp[0].y1;
        o->x2 = tmp[tcount-1].x2; o->y2 = tmp[tcount-1].y2;
        o->width = 2;

        /* NEW: reattach centerline for this FORGED_ID, if one was saved */
        o->has_centerline = false;
        for (int m = 0; m < s_pending_centerline_count; m++) {
            if (s_pending_centerline[m].forged_id == fid) {
                o->has_centerline = true;
                o->cl_x1 = s_pending_centerline[m].x1;
                o->cl_y1 = s_pending_centerline[m].y1;
                o->cl_x2 = s_pending_centerline[m].x2;
                o->cl_y2 = s_pending_centerline[m].y2;
                break;
            }
        }

        /* NEW: reattach pattern-fill metadata for this FORGED_ID */
        o->pattern_fill    = PATTERN_FILL_NONE;
        o->pattern_spacing = 0;
        o->fill_visible    = false;
        for (int m = 0; m < s_pending_meta_count; m++) {
            if (s_pending_meta[m].forged_id == fid) {
                o->pattern_fill    = (PatternFillType)s_pending_meta[m].pattern_fill;
                o->pattern_spacing = s_pending_meta[m].pattern_spacing;
                o->fill_visible    = (s_pending_meta[m].fill_visible != 0);
                break;
            }
        }

        g.draw_count++;
    }
}

/* =========================================================
 * READER — main entry
 * ========================================================= */
bool cnc_sketch_load_dxf(const char *filename)
{
    char fullpath[256];
    cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));

    FILE *f = fopen(fullpath, "r");
    if (!f) {
        printf("[cnc_dxf] Error: Cannot open %s for reading\n", fullpath);
        return false;
    }

    /* Reset the canvas the same way cb_clear() does — this only touches
     * g.draw_objects[]/g.draw_count, not your JSON g_cnc state. */
    g.draw_count    = 0;
    g.selected_obj  = -1;
    g.undo_top      = 0;
    g.has_gaps      = false;
    g.gap_count     = 0;
    g.gap_pts_count = 0;

    s_pending_forged_count = 0;

    s_pending_centerline_count  = 0;   
    s_pending_meta_count        = 0;

    xkv_reset();

    char cur_entity[16] = "";
    bool in_polyline = false;
    bool pending_vertex = false;

    float line_x1 = 0, line_y1 = 0, line_x2 = 0, line_y2 = 0;

    static float poly_xs[MAX_POLY_VERTS + 8];
    static float poly_ys[MAX_POLY_VERTS + 8];
    int poly_n = 0;
    float pend_vx = 0, pend_vy = 0;

    int code;
    char val[256];

    while (dxf_read_pair(f, &code, val, sizeof(val))) {

        if (code == 0) {
            if (in_polyline) {
                pending_vertex = false;
                if (strcmp(val, "VERTEX") == 0) {
                    pending_vertex = true;
                } else if (strcmp(val, "SEQEND") == 0) {
                    finalize_polyline(poly_xs, poly_ys, poly_n);
                    in_polyline = false;
                    xkv_reset();
                    cur_entity[0] = '\0';
                } else {
                    /* unexpected — close out the polyline defensively */
                    finalize_polyline(poly_xs, poly_ys, poly_n);
                    in_polyline = false;
                    xkv_reset();
                    cur_entity[0] = '\0';
                }
                continue;
            }

            /* finalize whatever simple entity was pending */
            if (strcmp(cur_entity, "LINE") == 0) {
                finalize_line(line_x1, line_y1, line_x2, line_y2);
            }
            xkv_reset();

            if (strcmp(val, "LINE") == 0) {
                strcpy(cur_entity, "LINE");
                line_x1 = line_y1 = line_x2 = line_y2 = 0;
            } else if (strcmp(val, "POLYLINE") == 0) {
                strcpy(cur_entity, "POLYLINE");
                in_polyline = true;
                poly_n = 0;
                pending_vertex = false;
            } else {
                cur_entity[0] = '\0';   /* SECTION/ENDSEC/TABLE/EOF/etc — ignore */
            }
            continue;
        }

        /* non-zero group codes */
        if (in_polyline) {
            if (pending_vertex) {
                if (code == 10) pend_vx = (float)atof(val);
                else if (code == 20) {
                    pend_vy = (float)atof(val);
                    if (poly_n < (int)(sizeof(poly_xs)/sizeof(poly_xs[0]))) {
                        poly_xs[poly_n] = pend_vx;
                        poly_ys[poly_n] = pend_vy;
                        poly_n++;
                    }
                }
            } else {
                if (code == 1000) xkv_add_raw(val);
                /* code 70 (closed flag) and 1001 (appid marker) not needed on read */
            }
        } else if (strcmp(cur_entity, "LINE") == 0) {
            if      (code == 10) line_x1 = (float)atof(val);
            else if (code == 20) line_y1 = (float)atof(val);
            else if (code == 11) line_x2 = (float)atof(val);
            else if (code == 21) line_y2 = (float)atof(val);
            else if (code == 1000) xkv_add_raw(val);
        }
    }

    /* finalize any entity still pending at EOF */
    if (in_polyline) {
        finalize_polyline(poly_xs, poly_ys, poly_n);
    } else if (strcmp(cur_entity, "LINE") == 0) {
        finalize_line(line_x1, line_y1, line_x2, line_y2);
    }

    fclose(f);

    build_forged_objects_from_pending();

    /* same fix you already needed elsewhere: invalidate the actual
     * canvas object so freshly loaded geometry repaints. */
    if (g.canvas_obj) lv_obj_invalidate(g.canvas_obj);
    refresh_all();
    update_panel();

    printf("[cnc_dxf] DXF loaded: %s (%d draw objects)\n", filename, g.draw_count);
    return true;
}