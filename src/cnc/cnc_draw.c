/*==============================================================
 * src/cnc/cnc_draw.c
 * CNC Drawing Tool Implementation
 *
 * Features:
 *  • Multiple drawing tools (line, circle, rectangle, dimension)
 *  • Precision measurements in mm
 *  • Zoom and pan functionality
 *  • Grid background with major/minor ticks
 *  • Job management and status display
 *==============================================================*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "lvgl/lvgl.h"
#include "cnc_draw.h"

/* Global CNC state */
CNCState g_cnc = {0};

/* =========================================================
 * INITIALIZATION
 * ======================================================== */

void cnc_init(void)
{
    memset(&g_cnc, 0, sizeof(CNCState));
    
    g_cnc.current_tool = CNC_TOOL_SELECT;
    g_cnc.unit = CNC_UNIT_MM;
    g_cnc.is_drawing = false;
    
    g_cnc.line_width = 2;
    g_cnc.draw_color = lv_color_hex(0x00FF00);  // Green
    
    g_cnc.zoom = 1.0f;
    g_cnc.offset_x = 0;
    g_cnc.offset_y = 0;
    
    g_cnc.line_count = 0;
    g_cnc.circle_count = 0;
    g_cnc.rect_count = 0;
    g_cnc.dim_count = 0;
    
    /* Initialize job */
    strcpy(g_cnc.current_job.name, "New Job");
    g_cnc.current_job.width_mm = 100.0f;
    g_cnc.current_job.height_mm = 100.0f;
    g_cnc.current_job.rotation = 0.0f;
}

/* =========================================================
 * TOOL MANAGEMENT
 * ======================================================== */

void cnc_set_tool(CNCTool tool)
{
    g_cnc.current_tool = tool;
    g_cnc.is_drawing = false;
    
    char buf[64];
    const char *tool_name = "Unknown";
    
    switch(tool) {
        case CNC_TOOL_SELECT:     tool_name = "Select"; break;
        case CNC_TOOL_LINE:       tool_name = "Line"; break;
        case CNC_TOOL_CIRCLE:     tool_name = "Circle"; break;
        case CNC_TOOL_RECTANGLE:  tool_name = "Rectangle"; break;
        case CNC_TOOL_DIMENSION:  tool_name = "Dimension"; break;
        case CNC_TOOL_PAN:        tool_name = "Pan"; break;
        case CNC_TOOL_ZOOM:       tool_name = "Zoom"; break;
    }
    
    if (g_cnc.lbl_tool) {
        lv_snprintf(buf, sizeof(buf), "Tool: %s", tool_name);
        lv_label_set_text(g_cnc.lbl_tool, buf);
    }
}

/* =========================================================
 * DRAWING PRIMITIVES
 * ======================================================== */

void cnc_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (g_cnc.line_count >= 256) return;
    
    CNCLine *line = &g_cnc.lines[g_cnc.line_count];
    line->x1 = x1;
    line->y1 = y1;
    line->x2 = x2;
    line->y2 = y2;
    line->color = g_cnc.draw_color;
    line->width = g_cnc.line_width;
    line->timestamp = lv_tick_get();
    
    g_cnc.line_count++;
    cnc_update_status();
}

void cnc_draw_circle(int32_t cx, int32_t cy, int32_t radius)
{
    if (g_cnc.circle_count >= 64) return;
    
    CNCCircle *circle = &g_cnc.circles[g_cnc.circle_count];
    circle->cx = cx;
    circle->cy = cy;
    circle->radius = radius;
    circle->color = g_cnc.draw_color;
    circle->width = g_cnc.line_width;
    circle->timestamp = lv_tick_get();
    
    g_cnc.circle_count++;
    cnc_update_status();
}

void cnc_draw_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    if (g_cnc.rect_count >= 64) return;
    
    CNCRect *rect = &g_cnc.rects[g_cnc.rect_count];
    rect->x1 = x1;
    rect->y1 = y1;
    rect->x2 = x2;
    rect->y2 = y2;
    rect->color = g_cnc.draw_color;
    rect->width = g_cnc.line_width;
    rect->timestamp = lv_tick_get();
    
    g_cnc.rect_count++;
    cnc_update_status();
}

void cnc_add_dimension(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const char *label)
{
    if (g_cnc.dim_count >= 128) return;
    
    CNCDimension *dim = &g_cnc.dims[g_cnc.dim_count];
    dim->x1 = x1;
    dim->y1 = y1;
    dim->x2 = x2;
    dim->y2 = y2;
    
    /* Calculate length in mm (assuming 1px = 0.1mm as default) */
    float dx = (float)(x2 - x1) * 0.1f;
    float dy = (float)(y2 - y1) * 0.1f;
    dim->length_mm = sqrtf(dx*dx + dy*dy);
    
    if (label) {
        strncpy(dim->label, label, sizeof(dim->label) - 1);
    } else {
        lv_snprintf(dim->label, sizeof(dim->label), "%.2f mm", dim->length_mm);
    }
    
    dim->timestamp = lv_tick_get();
    g_cnc.dim_count++;
    cnc_update_status();
}

/* =========================================================
 * CLEAR & UNDO
 * ======================================================== */

void cnc_clear_all(void)
{
    g_cnc.line_count = 0;
    g_cnc.circle_count = 0;
    g_cnc.rect_count = 0;
    g_cnc.dim_count = 0;
    cnc_update_status();
}

void cnc_undo(void)
{
    /* Simple undo: remove last element from most recent type */
    if (g_cnc.dim_count > 0) {
        g_cnc.dim_count--;
    } else if (g_cnc.rect_count > 0) {
        g_cnc.rect_count--;
    } else if (g_cnc.circle_count > 0) {
        g_cnc.circle_count--;
    } else if (g_cnc.line_count > 0) {
        g_cnc.line_count--;
    }
    cnc_update_status();
}

/* =========================================================
 * ZOOM & PAN
 * ======================================================== */

void cnc_zoom_in(void)
{
    g_cnc.zoom *= 1.2f;
    if (g_cnc.zoom > 10.0f) g_cnc.zoom = 10.0f;
}

void cnc_zoom_out(void)
{
    g_cnc.zoom /= 1.2f;
    if (g_cnc.zoom < 0.1f) g_cnc.zoom = 0.1f;
}

void cnc_fit_to_view(void)
{
    /* Calculate zoom to fit job in canvas */
    int32_t canvas_w = lv_obj_get_width(g_cnc.canvas_obj);
    int32_t canvas_h = lv_obj_get_height(g_cnc.canvas_obj);
    
    float job_w = g_cnc.current_job.width_mm * 10.0f;   /* pixels (assuming 10px per mm) */
    float job_h = g_cnc.current_job.height_mm * 10.0f;
    
    float zoom_x = (float)canvas_w / job_w;
    float zoom_y = (float)canvas_h / job_h;
    
    g_cnc.zoom = (zoom_x < zoom_y) ? zoom_x : zoom_y;
    g_cnc.offset_x = 0;
    g_cnc.offset_y = 0;
}

/* =========================================================
 * STATUS & INFO UPDATES
 * ======================================================== */

void cnc_update_status(void)
{
    char buf[96];
    
    if (g_cnc.lbl_dimensions) {
        lv_snprintf(buf, sizeof(buf), "Lines: %d | Circles: %d | Rects: %d | Dims: %d",
                   g_cnc.line_count, g_cnc.circle_count, g_cnc.rect_count, g_cnc.dim_count);
        lv_label_set_text(g_cnc.lbl_dimensions, buf);
    }
    
    if (g_cnc.lbl_zoom) {
        lv_snprintf(buf, sizeof(buf), "Zoom: %.1f%%", g_cnc.zoom * 100.0f);
        lv_label_set_text(g_cnc.lbl_zoom, buf);
    }
}

void cnc_update_job_info(void)
{
    char buf[128];
    
    if (g_cnc.lbl_job_name) {
        lv_label_set_text(g_cnc.lbl_job_name, g_cnc.current_job.name);
    }
    
    if (g_cnc.lbl_job_size) {
        lv_snprintf(buf, sizeof(buf), "%.1f x %.1f mm",
                   g_cnc.current_job.width_mm, g_cnc.current_job.height_mm);
        lv_label_set_text(g_cnc.lbl_job_size, buf);
    }
    
    if (g_cnc.lbl_job_elements) {
        lv_snprintf(buf, sizeof(buf), "%d elements",
                   g_cnc.line_count + g_cnc.circle_count + g_cnc.rect_count);
        lv_label_set_text(g_cnc.lbl_job_elements, buf);
    }
}

/* =========================================================
 * EXPORT & FILE OPERATIONS
 * ======================================================== */

void cnc_export_job(const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    fprintf(f, "CNC Job File\n");
    fprintf(f, "Job: %s\n", g_cnc.current_job.name);
    fprintf(f, "Size: %.1f x %.1f mm\n", g_cnc.current_job.width_mm, g_cnc.current_job.height_mm);
    fprintf(f, "\nLines: %d\n", g_cnc.line_count);
    
    for (int i = 0; i < g_cnc.line_count; i++) {
        CNCLine *line = &g_cnc.lines[i];
        fprintf(f, "  Line: (%d,%d) -> (%d,%d) width=%d\n",
               line->x1, line->y1, line->x2, line->y2, line->width);
    }
    
    fprintf(f, "\nCircles: %d\n", g_cnc.circle_count);
    for (int i = 0; i < g_cnc.circle_count; i++) {
        CNCCircle *circle = &g_cnc.circles[i];
        fprintf(f, "  Circle: center=(%d,%d) radius=%d\n",
               circle->cx, circle->cy, circle->radius);
    }
    
    fprintf(f, "\nRectangles: %d\n", g_cnc.rect_count);
    for (int i = 0; i < g_cnc.rect_count; i++) {
        CNCRect *rect = &g_cnc.rects[i];
        fprintf(f, "  Rect: (%d,%d) -> (%d,%d)\n",
               rect->x1, rect->y1, rect->x2, rect->y2);
    }
    
    fprintf(f, "\nDimensions: %d\n", g_cnc.dim_count);
    for (int i = 0; i < g_cnc.dim_count; i++) {
        CNCDimension *dim = &g_cnc.dims[i];
        fprintf(f, "  Dim: %.2f mm [%s]\n", dim->length_mm, dim->label);
    }
    
    fclose(f);
}

/* =========================================================
 * CANVAS RENDERING
 * ======================================================== */

void cnc_canvas_draw_grid(lv_obj_t *canvas, lv_layer_t *layer)
{
    int32_t w = lv_obj_get_width(canvas);
    int32_t h = lv_obj_get_height(canvas);

    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = lv_color_hex(0x1A1A2E);
    dsc.border_width = 0;

    lv_area_t area = {0, 0, w, h};
    lv_draw_rect(layer, &dsc, &area);

    /* Draw grid lines aligned to world origin and zoom */
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.width = 1;

    int32_t grid_major = 50;  /* World-space major grid every 50 units */
    int32_t grid_minor = 10;  /* World-space minor grid every 10 units */

    /* Draw a pixel-aligned grid that starts at the canvas origin (0,0). */
    for (int32_t x = 0; x < w; x += grid_minor) {
        bool is_major = ((x % grid_major) == 0);
        line_dsc.color = is_major ? lv_color_hex(0x3A4A5E) : lv_color_hex(0x1E2A3A);
        line_dsc.p1.x = x; line_dsc.p1.y = 0;
        line_dsc.p2.x = x; line_dsc.p2.y = h;
        lv_draw_line(layer, &line_dsc);
    }

    for (int32_t y = 0; y < h; y += grid_minor) {
        bool is_major = ((y % grid_major) == 0);
        line_dsc.color = is_major ? lv_color_hex(0x3A4A5E) : lv_color_hex(0x1E2A3A);
        line_dsc.p1.x = 0; line_dsc.p1.y = y;
        line_dsc.p2.x = w; line_dsc.p2.y = y;
        lv_draw_line(layer, &line_dsc);
    }

    /* Origin axes at top-left (0,0) for CNC-style canvas alignment. */
    lv_draw_line_dsc_t origin_dsc;
    lv_draw_line_dsc_init(&origin_dsc);
    origin_dsc.color = lv_color_hex(0x38BDF8);
    origin_dsc.width = 2;
    origin_dsc.opa = LV_OPA_90;

    origin_dsc.p1.x = 0;
    origin_dsc.p1.y = 0;
    origin_dsc.p2.x = LV_MIN(w, 16);
    origin_dsc.p2.y = 0;
    lv_draw_line(layer, &origin_dsc);

    origin_dsc.p1.x = 0;
    origin_dsc.p1.y = 0;
    origin_dsc.p2.x = 0;
    origin_dsc.p2.y = LV_MIN(h, 16);
    lv_draw_line(layer, &origin_dsc);
}

void cnc_render_lines(lv_obj_t *canvas, lv_layer_t *layer)
{
    (void)canvas;
    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    
    for (int i = 0; i < g_cnc.line_count; i++) {
        CNCLine *line = &g_cnc.lines[i];
        dsc.color = line->color;
        dsc.width = line->width;
        dsc.opa = LV_OPA_COVER;
        dsc.p1.x = line->x1;
        dsc.p1.y = line->y1;
        dsc.p2.x = line->x2;
        dsc.p2.y = line->y2;
        lv_draw_line(layer, &dsc);
    }
}

void cnc_render_circles(lv_obj_t *canvas, lv_layer_t *layer)
{
    (void)canvas;
    lv_draw_arc_dsc_t dsc;
    lv_draw_arc_dsc_init(&dsc);
    
    for (int i = 0; i < g_cnc.circle_count; i++) {
        CNCCircle *circle = &g_cnc.circles[i];
        dsc.color = circle->color;
        dsc.width = circle->width;
        dsc.opa = LV_OPA_COVER;
        dsc.center.x = circle->cx;
        dsc.center.y = circle->cy;
        dsc.radius = circle->radius;
        dsc.start_angle = 0;
        dsc.end_angle = 360;
        lv_draw_arc(layer, &dsc);
    }
}

void cnc_render_rects(lv_obj_t *canvas, lv_layer_t *layer)
{
    (void)canvas;
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_opa = LV_OPA_TRANSP;
    
    for (int i = 0; i < g_cnc.rect_count; i++) {
        CNCRect *rect = &g_cnc.rects[i];
        dsc.border_color = rect->color;
        dsc.border_width = rect->width;
        dsc.border_opa = LV_OPA_COVER;
        
        lv_area_t area;
        area.x1 = (rect->x1 < rect->x2) ? rect->x1 : rect->x2;
        area.y1 = (rect->y1 < rect->y2) ? rect->y1 : rect->y2;
        area.x2 = (rect->x1 < rect->x2) ? rect->x2 : rect->x1;
        area.y2 = (rect->y1 < rect->y2) ? rect->y2 : rect->y1;
        
        lv_draw_rect(layer, &dsc, &area);
    }
}

void cnc_render_dimensions(lv_obj_t *canvas, lv_layer_t *layer)
{
    (void)canvas;
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_hex(0x38BDF8);
    line_dsc.width = 1;
    line_dsc.opa = LV_OPA_COVER;
    
    for (int i = 0; i < g_cnc.dim_count; i++) {
        CNCDimension *dim = &g_cnc.dims[i];
        
        /* Draw dimension line */
        line_dsc.p1.x = dim->x1;
        line_dsc.p1.y = dim->y1;
        line_dsc.p2.x = dim->x2;
        line_dsc.p2.y = dim->y2;
        lv_draw_line(layer, &line_dsc);
        
        /* Draw endpoints */
        lv_draw_rect_dsc_t rect_dsc;
        lv_draw_rect_dsc_init(&rect_dsc);
        rect_dsc.bg_color = lv_color_hex(0x38BDF8);
        lv_area_t pt;
        pt.x1 = dim->x1 - 2; pt.y1 = dim->y1 - 2;
        pt.x2 = dim->x1 + 2; pt.y2 = dim->y1 + 2;
        lv_draw_rect(layer, &rect_dsc, &pt);
        
        pt.x1 = dim->x2 - 2; pt.y1 = dim->y2 - 2;
        pt.x2 = dim->x2 + 2; pt.y2 = dim->y2 + 2;
        lv_draw_rect(layer, &rect_dsc, &pt);
    }
}

void cnc_canvas_draw_all(lv_obj_t *canvas, lv_layer_t *layer)
{
    if (!canvas || !layer) return;
    
    cnc_canvas_draw_grid(canvas, layer);
    cnc_render_lines(canvas, layer);
    cnc_render_circles(canvas, layer);
    cnc_render_rects(canvas, layer);
    cnc_render_dimensions(canvas, layer);
}
