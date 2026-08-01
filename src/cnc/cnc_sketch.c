/*==============================================================
 * src/cnc/cnc_sketch.c
 * CNC Sketch Management Implementation
 *
 * Provides functions to:
 *  • Save sketches in JSON format
 *  • Load sketches from JSON
 *  • Generate G-code for CNC machines
 *  • Manage sketch history
 *==============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>
#include "cnc_sketch.h"
#include "cnc_draw.h"
#include "cnc_dxf.h"
#include "../include/viewer.h"
#include "../include/viewer_math.h"

/* =========================================================
 * HELPER FUNCTIONS
 * ======================================================== */

void cnc_get_sketch_path(const char *filename, char *fullpath, int bufsize)
{
    snprintf(fullpath, bufsize, "%s%s", SKETCH_DIR, filename);
}

void cnc_get_gcode_path(const char *filename, char *fullpath, int bufsize)
{
    snprintf(fullpath, bufsize, "%s%s", GCODE_DIR, filename);
}

static void ensure_directories(void)
{
    mkdir(SKETCH_DIR, 0755);
    mkdir(GCODE_DIR, 0755);
}

/* =========================================================
 * SKETCH SAVE/LOAD - JSON FORMAT
 * ======================================================== */

// bool cnc_sketch_save(const char *filename, const char *description)
// {
//     ensure_directories();

//     /* Sync viewer draw objects into CNC state so the saved sketch
//      * contains what the user actually drew on the canvas. */
//     {
//         /* Clear current CNC storage */
//         // cnc_clear_all();

//         for (int i = 0; i < g.draw_count; i++) {
//             DrawObject *o = &g.draw_objects[i];
//             if (o->type == OBJ_LINE) {
//                 if (g_cnc.line_count < 256) {
//                     CNCLine *ln = &g_cnc.lines[g_cnc.line_count++];
//                     ln->x1 = (int32_t)roundf(o->x1);
//                     ln->y1 = (int32_t)roundf(o->y1);
//                     ln->x2 = (int32_t)roundf(o->x2);
//                     ln->y2 = (int32_t)roundf(o->y2);
//                     ln->width = (o->width>0)?o->width:g_cnc.line_width;
//                     ln->color = lv_color_hex(o->color);
//                     ln->timestamp = (uint32_t)lv_tick_get();
//                 }
//             } else if (o->type == OBJ_BOX) {
//                 if (g_cnc.rect_count < 64) {
//                     CNCRect *r = &g_cnc.rects[g_cnc.rect_count++];
//                     r->x1 = (int32_t)roundf(fminf(o->x1, o->x2));
//                     r->y1 = (int32_t)roundf(fminf(o->y1, o->y2));
//                     r->x2 = (int32_t)roundf(fmaxf(o->x1, o->x2));
//                     r->y2 = (int32_t)roundf(fmaxf(o->y1, o->y2));
//                     r->width = (o->width>0)?o->width:g_cnc.line_width;
//                     r->color = lv_color_hex(o->color);
//                     r->timestamp = (uint32_t)lv_tick_get();
//                 }
//             } else if (o->type == OBJ_ARC) {
//                 if (g_cnc.circle_count < 64) {
//                     CNCCircle *c = &g_cnc.circles[g_cnc.circle_count++];
//                     c->cx = (int32_t)roundf(o->cx);
//                     c->cy = (int32_t)roundf(o->cy);
//                     /* approximate radius from center to start point */
//                     float dx = o->x1 - o->cx;
//                     float dy = o->y1 - o->cy;
//                     c->radius = (int32_t)roundf(sqrtf(dx*dx + dy*dy));
//                     c->width = (o->width>0)?o->width:g_cnc.line_width;
//                     c->color = lv_color_hex(o->color);
//                     c->timestamp = (uint32_t)lv_tick_get();
//                 }
//             } else if (o->type == OBJ_POLY) {
//                 /* Simple heuristics: if 4 verts and axis-aligned, import as rect */
//                 if (o->vert_count == 4 && g_cnc.rect_count < 64) {
//                     float mnx = o->verts_x[0], mxx = o->verts_x[0];
//                     float mny = o->verts_y[0], mxy = o->verts_y[0];
//                     for (int k = 1; k < 4; k++) {
//                         if (o->verts_x[k] < mnx) mnx = o->verts_x[k];
//                         if (o->verts_x[k] > mxx) mxx = o->verts_x[k];
//                         if (o->verts_y[k] < mny) mny = o->verts_y[k];
//                         if (o->verts_y[k] > mxy) mxy = o->verts_y[k];
//                     }
//                     CNCRect *r = &g_cnc.rects[g_cnc.rect_count++];
//                     r->x1 = (int32_t)roundf(mnx);
//                     r->y1 = (int32_t)roundf(mny);
//                     r->x2 = (int32_t)roundf(mxx);
//                     r->y2 = (int32_t)roundf(mxy);
//                     r->width = (o->width>0)?o->width:g_cnc.line_width;
//                     r->color = lv_color_hex(o->color);
//                     r->timestamp = (uint32_t)lv_tick_get();
//                 }
//             }
//         }

//         /* Update status labels */
//         cnc_update_status();
//     }
    
//     char fullpath[256];
//     cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));
    
//     FILE *f = fopen(fullpath, "w");
//     if (!f) {
//         printf("[cnc_sketch] Error: Cannot open %s for writing\n", fullpath);
//         return false;
//     }
    
//     time_t now = time(NULL);
    
//     // Write JSON header
//     fprintf(f, "{\n");
//     fprintf(f, "  \"version\": \"1.0\",\n");
//     fprintf(f, "  \"timestamp\": %ld,\n", now);
//     fprintf(f, "  \"description\": \"%s\",\n", description ? description : "");
//     fprintf(f, "  \"job\": {\n");
//     fprintf(f, "    \"name\": \"%s\",\n", g_cnc.current_job.name);
//     fprintf(f, "    \"width_mm\": %.2f,\n", g_cnc.current_job.width_mm);
//     fprintf(f, "    \"height_mm\": %.2f,\n", g_cnc.current_job.height_mm);
//     fprintf(f, "    \"rotation\": %.2f\n", g_cnc.current_job.rotation);
//     fprintf(f, "  },\n");
    
//     // Write lines
//     fprintf(f, "  \"lines\": [\n");
//     for (int i = 0; i < g_cnc.line_count; i++) {
//         CNCLine *line = &g_cnc.lines[i];
//         if (i > 0) fprintf(f, ",\n");
//         fprintf(f, "    {\n");
//         fprintf(f, "      \"x1\": %d,\n", line->x1);
//         fprintf(f, "      \"y1\": %d,\n", line->y1);
//         fprintf(f, "      \"x2\": %d,\n", line->x2);
//         fprintf(f, "      \"y2\": %d,\n", line->y2);
//         fprintf(f, "      \"width\": %d,\n", line->width);
//         fprintf(f, "      \"color\": \"#%02X%02X%02X\"\n", line->color.red, line->color.green, line->color.blue);
//         fprintf(f, "    }");
//     }
//     fprintf(f, "\n  ],\n");
    
//     // Write circles
//     fprintf(f, "  \"circles\": [\n");
//     for (int i = 0; i < g_cnc.circle_count; i++) {
//         CNCCircle *circle = &g_cnc.circles[i];
//         if (i > 0) fprintf(f, ",\n");
//         fprintf(f, "    {\n");
//         fprintf(f, "      \"cx\": %d,\n", circle->cx);
//         fprintf(f, "      \"cy\": %d,\n", circle->cy);
//         fprintf(f, "      \"radius\": %d,\n", circle->radius);
//         fprintf(f, "      \"width\": %d,\n", circle->width);
//         fprintf(f, "      \"color\": \"#%02X%02X%02X\"\n", circle->color.red, circle->color.green, circle->color.blue);
//         fprintf(f, "    }");
//     }
//     fprintf(f, "\n  ],\n");
    
//     // Write rectangles
//     fprintf(f, "  \"rectangles\": [\n");
//     for (int i = 0; i < g_cnc.rect_count; i++) {
//         CNCRect *rect = &g_cnc.rects[i];
//         if (i > 0) fprintf(f, ",\n");
//         fprintf(f, "    {\n");
//         fprintf(f, "      \"x1\": %d,\n", rect->x1);
//         fprintf(f, "      \"y1\": %d,\n", rect->y1);
//         fprintf(f, "      \"x2\": %d,\n", rect->x2);
//         fprintf(f, "      \"y2\": %d,\n", rect->y2);
//         fprintf(f, "      \"width\": %d,\n", rect->width);
//         fprintf(f, "      \"color\": \"#%02X%02X%02X\"\n", rect->color.red, rect->color.green, rect->color.blue);
//         fprintf(f, "    }");
//     }
//     fprintf(f, "\n  ],\n");
    
//     // Write dimensions
//     fprintf(f, "  \"dimensions\": [\n");
//     for (int i = 0; i < g_cnc.dim_count; i++) {
//         CNCDimension *dim = &g_cnc.dims[i];
//         if (i > 0) fprintf(f, ",\n");
//         fprintf(f, "    {\n");
//         fprintf(f, "      \"x1\": %d,\n", dim->x1);
//         fprintf(f, "      \"y1\": %d,\n", dim->y1);
//         fprintf(f, "      \"x2\": %d,\n", dim->x2);
//         fprintf(f, "      \"y2\": %d,\n", dim->y2);
//         fprintf(f, "      \"length_mm\": %.2f,\n", dim->length_mm);
//         fprintf(f, "      \"label\": \"%s\"\n", dim->label);
//         fprintf(f, "    }");
//     }
//     fprintf(f, "\n  ]\n");
//     fprintf(f, "}\n");
    
//     fclose(f);
//     printf("[cnc_sketch] Sketch saved: %s (%d lines, %d circles, %d rects, %d dims)\n",
//            filename, g_cnc.line_count, g_cnc.circle_count, g_cnc.rect_count, g_cnc.dim_count);
    
//     return true;
// }

/* =========================================================
 * SYNC viewer → g_cnc (call before every save/export)
 * ======================================================== */
static void sync_viewer_to_cnc(void)
{
    g_cnc.line_count   = 0;
    g_cnc.circle_count = 0;
    g_cnc.rect_count   = 0;
    g_cnc.dim_count    = 0;

    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];

        /* Convert from image/world coords → canvas viewport pixels */
        float vx1 = img_to_vp_x(o->x1);
        float vy1 = img_to_vp_y(o->y1);
        float vx2 = img_to_vp_x(o->x2);
        float vy2 = img_to_vp_y(o->y2);
        float vcx = img_to_vp_x(o->cx);
        float vcy = img_to_vp_y(o->cy);

        printf("[sync] obj[%d] type=%d vp=(%.1f,%.1f)->(%.1f,%.1f)\n",
               i, o->type, vx1, vy1, vx2, vy2);

        if (o->type == OBJ_LINE && g_cnc.line_count < 256) {
            CNCLine *ln = &g_cnc.lines[g_cnc.line_count++];
            ln->x1    = (int32_t)roundf(vx1);
            ln->y1    = (int32_t)roundf(vy1);
            ln->x2    = (int32_t)roundf(vx2);
            ln->y2    = (int32_t)roundf(vy2);
            ln->width = (o->width > 0) ? o->width : g_cnc.line_width;
            ln->color = lv_color_hex(o->color);
            ln->timestamp = (uint32_t)lv_tick_get();
            printf("[sync] LINE %d: (%d,%d)->(%d,%d)\n",
                   g_cnc.line_count, ln->x1, ln->y1, ln->x2, ln->y2);
        }
        else if (o->type == OBJ_BOX && g_cnc.rect_count < 64) {
            CNCRect *r = &g_cnc.rects[g_cnc.rect_count++];
            r->x1    = (int32_t)roundf(fminf(vx1, vx2));
            r->y1    = (int32_t)roundf(fminf(vy1, vy2));
            r->x2    = (int32_t)roundf(fmaxf(vx1, vx2));
            r->y2    = (int32_t)roundf(fmaxf(vy1, vy2));
            r->width = (o->width > 0) ? o->width : g_cnc.line_width;
            r->color = lv_color_hex(o->color);
            r->timestamp = (uint32_t)lv_tick_get();
            printf("[sync] RECT %d: (%d,%d)->(%d,%d)\n",
                   g_cnc.rect_count, r->x1, r->y1, r->x2, r->y2);
        }
        else if (o->type == OBJ_ARC && g_cnc.circle_count < 64) {
            CNCCircle *c  = &g_cnc.circles[g_cnc.circle_count++];
            c->cx         = (int32_t)roundf(vcx);
            c->cy         = (int32_t)roundf(vcy);
            float dx      = vx1 - vcx;
            float dy      = vy1 - vcy;
            c->radius     = (int32_t)roundf(sqrtf(dx*dx + dy*dy));
            c->width      = (o->width > 0) ? o->width : g_cnc.line_width;
            c->color      = lv_color_hex(o->color);
            c->timestamp  = (uint32_t)lv_tick_get();
            printf("[sync] CIRCLE %d: center(%d,%d) r=%d\n",
                   g_cnc.circle_count, c->cx, c->cy, c->radius);
        }
    }

    printf("[sync] Total: %d lines, %d circles, %d rects from %d draw objects\n",
           g_cnc.line_count, g_cnc.circle_count, g_cnc.rect_count, g.draw_count);
}

bool cnc_sketch_save(const char *filename, const char *description)
{
    ensure_directories();

    // REMOVED: the cnc_clear_all() + sync block that was wiping state
    // g_cnc.lines/circles/rects are already populated by cnc_draw_line(),
    // cnc_draw_circle(), cnc_draw_rect() at draw time — no re-sync needed.

    /* Sync viewer → g_cnc so saved file contains what user drew */
    sync_viewer_to_cnc();


    char fullpath[256];
    cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));

    FILE *f = fopen(fullpath, "w");
    if (!f) {
        printf("[cnc_sketch] Error: Cannot open %s for writing\n", fullpath);
        return false;
    }

    time_t now = time(NULL);

    fprintf(f, "{\n");
    fprintf(f, "  \"version\": \"1.0\",\n");
    fprintf(f, "  \"timestamp\": %ld,\n", now);
    fprintf(f, "  \"description\": \"%s\",\n", description ? description : "");
    fprintf(f, "  \"job\": {\n");
    fprintf(f, "    \"name\": \"%s\",\n", g_cnc.current_job.name);
    fprintf(f, "    \"width_mm\": %.2f,\n", g_cnc.current_job.width_mm);
    fprintf(f, "    \"height_mm\": %.2f,\n", g_cnc.current_job.height_mm);
    fprintf(f, "    \"rotation\": %.2f\n", g_cnc.current_job.rotation);
    fprintf(f, "  },\n");

    // ... rest of JSON writing unchanged ...
    //     // Write lines
    fprintf(f, "  \"lines\": [\n");
    for (int i = 0; i < g_cnc.line_count; i++) {
        CNCLine *line = &g_cnc.lines[i];
        if (i > 0) fprintf(f, ",\n");
        fprintf(f, "    {\n");
        fprintf(f, "      \"x1\": %d,\n", line->x1);
        fprintf(f, "      \"y1\": %d,\n", line->y1);
        fprintf(f, "      \"x2\": %d,\n", line->x2);
        fprintf(f, "      \"y2\": %d,\n", line->y2);
        fprintf(f, "      \"width\": %d,\n", line->width);
        fprintf(f, "      \"color\": \"#%02X%02X%02X\"\n", line->color.red, line->color.green, line->color.blue);
        fprintf(f, "    }");
    }
    fprintf(f, "\n  ],\n");
    
    // Write circles
    fprintf(f, "  \"circles\": [\n");
    for (int i = 0; i < g_cnc.circle_count; i++) {
        CNCCircle *circle = &g_cnc.circles[i];
        if (i > 0) fprintf(f, ",\n");
        fprintf(f, "    {\n");
        fprintf(f, "      \"cx\": %d,\n", circle->cx);
        fprintf(f, "      \"cy\": %d,\n", circle->cy);
        fprintf(f, "      \"radius\": %d,\n", circle->radius);
        fprintf(f, "      \"width\": %d,\n", circle->width);
        fprintf(f, "      \"color\": \"#%02X%02X%02X\"\n", circle->color.red, circle->color.green, circle->color.blue);
        fprintf(f, "    }");
    }
    fprintf(f, "\n  ],\n");
    
    // Write rectangles
    fprintf(f, "  \"rectangles\": [\n");
    for (int i = 0; i < g_cnc.rect_count; i++) {
        CNCRect *rect = &g_cnc.rects[i];
        if (i > 0) fprintf(f, ",\n");
        fprintf(f, "    {\n");
        fprintf(f, "      \"x1\": %d,\n", rect->x1);
        fprintf(f, "      \"y1\": %d,\n", rect->y1);
        fprintf(f, "      \"x2\": %d,\n", rect->x2);
        fprintf(f, "      \"y2\": %d,\n", rect->y2);
        fprintf(f, "      \"width\": %d,\n", rect->width);
        fprintf(f, "      \"color\": \"#%02X%02X%02X\"\n", rect->color.red, rect->color.green, rect->color.blue);
        fprintf(f, "    }");
    }
    fprintf(f, "\n  ],\n");
    
    // Write dimensions
    fprintf(f, "  \"dimensions\": [\n");
    for (int i = 0; i < g_cnc.dim_count; i++) {
        CNCDimension *dim = &g_cnc.dims[i];
        if (i > 0) fprintf(f, ",\n");
        fprintf(f, "    {\n");
        fprintf(f, "      \"x1\": %d,\n", dim->x1);
        fprintf(f, "      \"y1\": %d,\n", dim->y1);
        fprintf(f, "      \"x2\": %d,\n", dim->x2);
        fprintf(f, "      \"y2\": %d,\n", dim->y2);
        fprintf(f, "      \"length_mm\": %.2f,\n", dim->length_mm);
        fprintf(f, "      \"label\": \"%s\"\n", dim->label);
        fprintf(f, "    }");
    }
    fprintf(f, "\n  ]\n");
    fprintf(f, "}\n");

    fclose(f);

    // Guard: only call cnc_update_status() if canvas_obj is still valid
    if (g_cnc.canvas_obj) {
        cnc_update_status();
    }

    printf("[cnc_sketch] Sketch saved: %s (%d lines, %d circles, %d rects, %d dims)\n",
           filename, g_cnc.line_count, g_cnc.circle_count,
           g_cnc.rect_count, g_cnc.dim_count);

    return true;
}

// bool cnc_sketch_load(const char *filename)
// {
//     char fullpath[256];
//     cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));
    
//     FILE *f = fopen(fullpath, "r");
//     if (!f) {
//         printf("[cnc_sketch] Error: Cannot open %s for reading\n", fullpath);
//         return false;
//     }
    
//     // Simple JSON parser (basic implementation)
//     // For production, use a proper JSON library like cJSON
    
//     char buffer[8192];
//     size_t read_size = fread(buffer, 1, sizeof(buffer) - 1, f);
//     fclose(f);
//     buffer[read_size] = '\0';
    
//     // Clear current drawing
//     cnc_clear_all();
    
//     // Extract job info from JSON
//     char job_name[64] = "Loaded Job";
//     sscanf(buffer, "{ \"version\": \"1.0\", \"timestamp\": %*d, \"description\": \"%*[^\"]\" , \"job\": { \"name\": \"%63[^\"]\"", job_name);
//     strncpy(g_cnc.current_job.name, job_name, sizeof(g_cnc.current_job.name) - 1);
    
//     // Note: Full JSON parsing would require a JSON library
//     // This is a simplified parser that captures basic structure
//     // For production, integrate cJSON or jsmn library
    
//     printf("[cnc_sketch] Sketch loaded: %s\n", filename);
//     cnc_update_status();
    
//     return true;
// }

bool cnc_sketch_load(const char *filename)
{
    char fullpath[256];
    cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));

    FILE *f = fopen(fullpath, "r");
    if (!f) {
        printf("[cnc_sketch] Error: Cannot open %s for reading\n", fullpath);
        return false;
    }

    char buffer[16384];
    size_t read_size = fread(buffer, 1, sizeof(buffer) - 1, f);
    fclose(f);
    buffer[read_size] = '\0';

    cnc_clear_all();  // Safe here — we're about to repopulate from file

    // --- Parse lines ---
    char *p = buffer;
    while ((p = strstr(p, "\"x1\":")) != NULL) {
        // Peek ahead: if "radius" follows before next "x1", it's a circle — skip here
        char *next_x1 = strstr(p + 5, "\"x1\":");
        char *radius_check = strstr(p, "\"radius\":");
        if (radius_check && (!next_x1 || radius_check < next_x1)) {
            p += 5; continue; // circle block, handled below
        }

        int x1, y1, x2, y2, w;
        if (sscanf(p,
            "\"x1\": %d, \"y1\": %d, \"x2\": %d, \"y2\": %d, \"width\": %d",
            &x1, &y1, &x2, &y2, &w) == 5)
        {
            // Distinguish line vs rect by checking what section we're in
            // Simple heuristic: check if "rectangles" appears before this position
            char *rect_sec = strstr(buffer, "\"rectangles\"");
            char *line_sec = strstr(buffer, "\"lines\"");
            if (rect_sec && p > rect_sec && g_cnc.rect_count < 64) {
                CNCRect *r = &g_cnc.rects[g_cnc.rect_count++];
                r->x1 = x1; r->y1 = y1; r->x2 = x2; r->y2 = y2;
                r->width = w;
                r->color = lv_color_hex(0x38BDF8);
            } else if (line_sec && p > line_sec && g_cnc.line_count < 256) {
                CNCLine *ln = &g_cnc.lines[g_cnc.line_count++];
                ln->x1 = x1; ln->y1 = y1; ln->x2 = x2; ln->y2 = y2;
                ln->width = w;
                ln->color = lv_color_hex(0x38BDF8);
            }
        }
        p += 5;
    }

    // --- Parse circles ---
    p = buffer;
    char *circ_sec = strstr(buffer, "\"circles\"");
    if (circ_sec) {
        p = circ_sec;
        while ((p = strstr(p, "\"cx\":")) != NULL) {
            int cx, cy, radius, w;
            if (sscanf(p,
                "\"cx\": %d, \"cy\": %d, \"radius\": %d, \"width\": %d",
                &cx, &cy, &radius, &w) == 4
                && g_cnc.circle_count < 64)
            {
                CNCCircle *c = &g_cnc.circles[g_cnc.circle_count++];
                c->cx = cx; c->cy = cy; c->radius = radius; c->width = w;
                c->color = lv_color_hex(0x38BDF8);
            }
            p += 5;
            // Stop if we've passed the circles section
            char *next_sec = strstr(p, "\"rectangles\"");
            if (next_sec && p > next_sec) break;
        }
    }

    if (g_cnc.canvas_obj) {
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
    cnc_update_status();

    printf("[cnc_sketch] Loaded: %s (%d lines, %d circles, %d rects)\n",
           filename, g_cnc.line_count, g_cnc.circle_count, g_cnc.rect_count);
    return true;
}

bool cnc_sketch_delete(const char *filename)
{
    char fullpath[256];
    cnc_get_sketch_path(filename, fullpath, sizeof(fullpath));
    
    if (remove(fullpath) == 0) {
        printf("[cnc_sketch] Sketch deleted: %s\n", filename);
        return true;
    }
    
    printf("[cnc_sketch] Error: Cannot delete %s\n", filename);
    return false;
}

/* =========================================================
 * IMAGE EXPORT FUNCTION
 * ======================================================== */

bool cnc_sketch_export_image(const char *filename, int width, int height)
{
    ensure_directories();
    mkdir(SKETCH_IMG_DIR, 0755);

    if (width <= 0 || height <= 0) return false;

    uint8_t *img = (uint8_t *)malloc(width * height * 3);
    if (!img) return false;

    /* Dark background #0D0D1A */
    for (int i = 0; i < width * height * 3; i += 3) {
        img[i] = 0x0D; img[i+1] = 0x0D; img[i+2] = 0x1A;
    }

    #define PLOT(px, py, R, G, B) do { \
        int _x=(px), _y=(py); \
        if (_x>=0 && _x<width && _y>=0 && _y<height) { \
            int _i=(_y*width+_x)*3; \
            img[_i]=(R); img[_i+1]=(G); img[_i+2]=(B); \
        } } while(0)

    #define BLINE(x0,y0,x1,y1,R,G,B) do { \
        int _x0=(x0),_y0=(y0),_x1=(x1),_y1=(y1); \
        int _dx=abs(_x1-_x0), _sx=_x0<_x1?1:-1; \
        int _dy=abs(_y1-_y0), _sy=_y0<_y1?1:-1; \
        int _err=_dx-_dy; \
        while(1){ PLOT(_x0,_y0,R,G,B); \
            if(_x0==_x1&&_y0==_y1) break; \
            int _e2=2*_err; \
            if(_e2>-_dy){_err-=_dy;_x0+=_sx;} \
            if(_e2< _dx){_err+=_dx;_y0+=_sy;} \
        } } while(0)

    /* ── Theme stroke colors matching draw.c ── */
    /* C_LINE_DRAW, C_ARC_DRAW, C_BOX_DRAW from viewer.h */
    /* These must match the H() macro values in draw.c    */
    #define EXP_LINE_R 0x38
    #define EXP_LINE_G 0xBD
    #define EXP_LINE_B 0xF8   /* C_LINE_DRAW = 0x38BDF8 cyan */

    #define EXP_ARC_R  0xA7
    #define EXP_ARC_G  0xF3
    #define EXP_ARC_B  0xD0   /* C_ARC_DRAW  = 0xA7F3D0 mint */

    #define EXP_BOX_R  0xFF
    #define EXP_BOX_G  0x8C
    #define EXP_BOX_B  0x00   /* C_BOX_DRAW  = 0xFF8C00 orange */

    /* PHASE 1: filled BOX (width < 0) */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];

        float vx1 = img_to_vp_x(o->x1);
        float vy1 = img_to_vp_y(o->y1);
        float vx2 = img_to_vp_x(o->x2);
        float vy2 = img_to_vp_y(o->y2);

        uint8_t cr = (o->color >> 16) & 0xFF;
        uint8_t cg = (o->color >>  8) & 0xFF;
        uint8_t cb =  o->color        & 0xFF;

        if (o->type == OBJ_BOX && o->width < 0) {
            int lx = (int)fminf(vx1, vx2);
            int rx = (int)fmaxf(vx1, vx2);
            int ty = (int)fminf(vy1, vy2);
            int by = (int)fmaxf(vy1, vy2);
            for (int y = ty; y <= by; y++)
                for (int x = lx; x <= rx; x++)
                    PLOT(x, y, cr, cg, cb);
        }
    }

    /* PHASE 2: filled POLY — scanline fill */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];

        uint8_t cr = (o->color >> 16) & 0xFF;
        uint8_t cg = (o->color >>  8) & 0xFF;
        uint8_t cb =  o->color        & 0xFF;

        if (o->type == OBJ_POLY && o->vert_count >= 3) {
            float vx[MAX_POLY_VERTS], vy[MAX_POLY_VERTS];
            for (int j = 0; j < o->vert_count; j++) {
                vx[j] = img_to_vp_x(o->verts_x[j]);
                vy[j] = img_to_vp_y(o->verts_y[j]);
            }
            int miny = (int)vy[0], maxy = (int)vy[0];
            for (int j = 1; j < o->vert_count; j++) {
                if ((int)vy[j] < miny) miny = (int)vy[j];
                if ((int)vy[j] > maxy) maxy = (int)vy[j];
            }
            for (int y = miny; y <= maxy; y++) {
                float xs[MAX_POLY_VERTS * 2];
                int xc = 0;
                for (int j = 0; j < o->vert_count; j++) {
                    int k = (j+1) % o->vert_count;
                    float y1f = vy[j], y2f = vy[k];
                    if ((y1f <= y && y2f > y) || (y2f <= y && y1f > y)) {
                        xs[xc++] = vx[j] + (y - y1f) / (y2f - y1f) * (vx[k] - vx[j]);
                    }
                }
                for (int a = 0; a < xc-1; a++)
                    for (int b = a+1; b < xc; b++)
                        if (xs[b] < xs[a]) { float t=xs[a]; xs[a]=xs[b]; xs[b]=t; }
                for (int a = 0; a+1 < xc; a += 2) {
                    int x1i = (int)ceilf(xs[a]);
                    int x2i = (int)floorf(xs[a+1]);
                    for (int x = x1i; x <= x2i; x++)
                        PLOT(x, y, cr, cg, cb);
                }
            }
        }
    }

    /* PHASE 3: strokes — use SAME color logic as draw.c, not o->color */
    for (int i = 0; i < g.draw_count; i++) {
        DrawObject *o = &g.draw_objects[i];

        float vx1 = img_to_vp_x(o->x1);
        float vy1 = img_to_vp_y(o->y1);
        float vx2 = img_to_vp_x(o->x2);
        float vy2 = img_to_vp_y(o->y2);
        float vcx = img_to_vp_x(o->cx);
        float vcy = img_to_vp_y(o->cy);

        /* Match draw.c: stroke color comes from theme, NOT o->color */
        uint8_t sr, sg, sb;
        switch (o->type) {
            case OBJ_LINE:
                sr = EXP_LINE_R; sg = EXP_LINE_G; sb = EXP_LINE_B;
                break;
            case OBJ_ARC:
                sr = EXP_ARC_R;  sg = EXP_ARC_G;  sb = EXP_ARC_B;
                break;
            case OBJ_BOX:
                sr = EXP_BOX_R;  sg = EXP_BOX_G;  sb = EXP_BOX_B;
                break;
            default:
                sr = 0xFF; sg = 0xFF; sb = 0xFF;
                break;
        }

        switch (o->type) {
            case OBJ_LINE:
                BLINE((int)vx1,(int)vy1,(int)vx2,(int)vy2, sr,sg,sb);
                break;

            case OBJ_ARC: {
                float px1=vx1, py1=vy1, pcx=vcx, pcy=vcy, px2=vx2, py2=vy2;
                float dx=px2-px1, dy=py2-py1;
                float len=sqrtf(dx*dx+dy*dy);
                int steps=(int)(len*0.25f)+32;
                if(steps<32)  steps=32;
                if(steps>512) steps=512;
                float prev_x=px1, prev_y=py1;
                for(int s=1; s<=steps; s++){
                    float t=(float)s/steps, mt=1.0f-t;
                    float nx=mt*mt*px1+2*mt*t*pcx+t*t*px2;
                    float ny=mt*mt*py1+2*mt*t*pcy+t*t*py2;
                    BLINE((int)roundf(prev_x),(int)roundf(prev_y),
                          (int)roundf(nx),    (int)roundf(ny),
                          sr,sg,sb);
                    prev_x=nx; prev_y=ny;
                }
                break;
            }

            case OBJ_BOX:
                if (o->width >= 0) {
                    int x1i=(int)vx1, y1i=(int)vy1;
                    int x2i=(int)vx2, y2i=(int)vy2;
                    /* Normalize so x1<x2, y1<y2 */
                    if (x1i > x2i) { int t=x1i; x1i=x2i; x2i=t; }
                    if (y1i > y2i) { int t=y1i; y1i=y2i; y2i=t; }
                    BLINE(x1i,y1i, x2i,y1i, sr,sg,sb); /* top    */
                    BLINE(x2i,y1i, x2i,y2i, sr,sg,sb); /* right  */
                    BLINE(x2i,y2i, x1i,y2i, sr,sg,sb); /* bottom */
                    BLINE(x1i,y2i, x1i,y1i, sr,sg,sb); /* left   */
                }
                break;

            default: break;
        }
    }

    #undef PLOT
    #undef BLINE
    #undef EXP_LINE_R
    #undef EXP_LINE_G
    #undef EXP_LINE_B
    #undef EXP_ARC_R
    #undef EXP_ARC_G
    #undef EXP_ARC_B
    #undef EXP_BOX_R
    #undef EXP_BOX_G
    #undef EXP_BOX_B

    /* Write PPM */
    char ppm_path[256];
    snprintf(ppm_path, sizeof(ppm_path), "%s%s.ppm", SKETCH_IMG_DIR, filename);
    FILE *f = fopen(ppm_path, "wb");
    if (!f) { free(img); return false; }
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    fwrite(img, 1, width * height * 3, f);
    fclose(f);

    /* Convert to PNG */
    char png_path[256];
    snprintf(png_path, sizeof(png_path), "%s%s.png", SKETCH_IMG_DIR, filename);
    char cmd[600];
    snprintf(cmd, sizeof(cmd), "convert %s %s 2>/dev/null", ppm_path, png_path);
    system(cmd);

    free(img);
    printf("[cnc_sketch] Image exported: %s.ppm + .png\n", filename);
    return true;
}


int cnc_sketch_list(SketchInfo *sketches, int max_count)
{
    ensure_directories();
    
    // Suppress unused parameter warnings
    (void)sketches;
    (void)max_count;
    
    // This would require directory reading (platform-specific)
    // For now, return 0 (simplified implementation)
    // In production, use opendir/readdir on Linux
    
    return 0;
}

/* =========================================================
 * G-CODE GENERATION
 * ======================================================== */

static float pixels_to_mm(int32_t pixels)
{
    // Conversion factor: 0.1 mm per pixel (adjust based on your calibration)
    return pixels * 0.1f;
}

/* Future use: Convert mm to G-code integer units (typically 1/1000 mm) */
/* static int mm_to_gcode_units(float mm)
{
    return (int)(mm * 1000.0f);
} */

bool cnc_gcode_generate(const char *output_file, 
                        float tool_diameter_mm, 
                        float feed_rate, 
                        uint32_t spindle_speed)
{
    ensure_directories();

    sync_viewer_to_cnc();
    
    FILE *f = fopen(output_file, "w");
    if (!f) {
        printf("[cnc_sketch] Error: Cannot open %s for writing G-code\n", output_file);
        return false;
    }
    
    // Write G-code header
    fprintf(f, "(Generated by CNC Sketch Manager)\n");
    fprintf(f, "(Job: %s)\n", g_cnc.current_job.name);
    fprintf(f, "(Date: %s)\n", __DATE__);
    fprintf(f, "(Tool Diameter: %.2f mm, Feed Rate: %.2f, Spindle: %d RPM)\n\n",
           tool_diameter_mm, feed_rate, spindle_speed);
    
    // Initialization
    fprintf(f, "G21          (Metric units)\n");
    fprintf(f, "G90          (Absolute positioning)\n");
    fprintf(f, "G17          (XY plane)\n");
    fprintf(f, "M3 S%d       (Start spindle at %d RPM)\n", spindle_speed, spindle_speed);
    fprintf(f, "G4 P1        (Wait 1 second for spindle to spin up)\n\n");
    
    // Move to safe height
    float safe_z = 5.0f;
    fprintf(f, "G0 Z%.2f      (Move to safe height)\n\n", safe_z);
    
    // Generate code for lines (cuts)
    if (g_cnc.line_count > 0) {
        fprintf(f, "(========== LINES ==========)\n");
        for (int i = 0; i < g_cnc.line_count; i++) {
            CNCLine *line = &g_cnc.lines[i];
            float x1_mm = pixels_to_mm(line->x1);
            float y1_mm = pixels_to_mm(line->y1);
            float x2_mm = pixels_to_mm(line->x2);
            float y2_mm = pixels_to_mm(line->y2);
            
            fprintf(f, "(Line %d)\n", i + 1);
            fprintf(f, "G0 X%.2f Y%.2f                (Rapid move to start)\n", x1_mm, y1_mm);
            fprintf(f, "G1 Z-1.0 F%.2f            (Move to cutting depth)\n", feed_rate);
            fprintf(f, "G1 X%.2f Y%.2f F%.2f      (Cut line)\n", x2_mm, y2_mm, feed_rate);
            fprintf(f, "G0 Z%.2f                  (Retract)\n\n", safe_z);
        }
    }
    
    // Generate code for circles (outline or pocket)
    if (g_cnc.circle_count > 0) {
        fprintf(f, "(========== CIRCLES ==========)\n");
        for (int i = 0; i < g_cnc.circle_count; i++) {
            CNCCircle *circle = &g_cnc.circles[i];
            float cx_mm = pixels_to_mm(circle->cx);
            float cy_mm = pixels_to_mm(circle->cy);
            float r_mm = pixels_to_mm(circle->radius);
            
            fprintf(f, "(Circle %d - Center: (%.2f, %.2f), Radius: %.2f mm)\n", 
                   i + 1, cx_mm, cy_mm, r_mm);
            fprintf(f, "G0 X%.2f Y%.2f           (Move to circle center)\n", cx_mm, cy_mm);
            fprintf(f, "G1 Z-1.0 F%.2f        (Move to cutting depth)\n", feed_rate);
            
            // Generate circular pocket with multiple passes
            fprintf(f, "G2 X%.2f Y%.2f I%.2f J0 F%.2f  (Circular cut)\n",
                   cx_mm + r_mm * 0.9f, cy_mm, r_mm * 0.9f, feed_rate);
            fprintf(f, "G0 Z%.2f                (Retract)\n\n", safe_z);
        }
    }
    
    // Generate code for rectangles (pocket or outline)
    if (g_cnc.rect_count > 0) {
        fprintf(f, "(========== RECTANGLES ==========)\n");
        for (int i = 0; i < g_cnc.rect_count; i++) {
            CNCRect *rect = &g_cnc.rects[i];
            float x1_mm = pixels_to_mm(rect->x1);
            float y1_mm = pixels_to_mm(rect->y1);
            float x2_mm = pixels_to_mm(rect->x2);
            float y2_mm = pixels_to_mm(rect->y2);
            
            fprintf(f, "(Rectangle %d)\n", i + 1);
            fprintf(f, "G0 X%.2f Y%.2f           (Move to corner 1)\n", x1_mm, y1_mm);
            fprintf(f, "G1 Z-1.0 F%.2f        (Move to cutting depth)\n", feed_rate);
            fprintf(f, "G1 X%.2f Y%.2f F%.2f      (Cut to corner 2)\n", x2_mm, y1_mm, feed_rate);
            fprintf(f, "G1 X%.2f Y%.2f F%.2f      (Cut to corner 3)\n", x2_mm, y2_mm, feed_rate);
            fprintf(f, "G1 X%.2f Y%.2f F%.2f      (Cut to corner 4)\n", x1_mm, y2_mm, feed_rate);
            fprintf(f, "G1 X%.2f Y%.2f F%.2f      (Cut back to corner 1)\n", x1_mm, y1_mm, feed_rate);
            fprintf(f, "G0 Z%.2f                (Retract)\n\n", safe_z);
        }
    }
    
    // Shutdown
    fprintf(f, "(========== END OF PROGRAM ==========)\n");
    fprintf(f, "G0 Z%.2f           (Move to safe height)\n", safe_z);
    fprintf(f, "G0 X0 Y0          (Move to origin)\n");
    fprintf(f, "M5                (Stop spindle)\n");
    fprintf(f, "M30               (End program)\n");
    
    fclose(f);
    
    printf("[cnc_sketch] G-code generated: %s\n", output_file);
    printf("  - Lines: %d, Circles: %d, Rectangles: %d\n",
           g_cnc.line_count, g_cnc.circle_count, g_cnc.rect_count);
    
    return true;
}

bool cnc_gcode_from_file(const char *sketch_file, 
                         const char *output_file,
                         float tool_diameter_mm, 
                         float feed_rate, 
                         uint32_t spindle_speed)
{
    // Load sketch, then generate G-code
    if (!cnc_sketch_load(sketch_file)) {
        return false;
    }
    
    return cnc_gcode_generate(output_file, tool_diameter_mm, feed_rate, spindle_speed);
}
