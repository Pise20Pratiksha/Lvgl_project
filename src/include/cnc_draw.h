#ifndef CNC_DRAW_H
#define CNC_DRAW_H

#include "lvgl/lvgl.h"
#include <stdbool.h>
#include <stdint.h>

/* =========================================================
 * CNC DRAWING TOOL DEFINITIONS
 * ======================================================== */

typedef enum {
    CNC_TOOL_SELECT = 0,
    CNC_TOOL_LINE = 1,
    CNC_TOOL_CIRCLE = 2,
    CNC_TOOL_RECTANGLE = 3,
    CNC_TOOL_DIMENSION = 4,
    CNC_TOOL_PAN = 5,
    CNC_TOOL_ZOOM = 6
} CNCTool;

typedef enum {
    CNC_UNIT_MM = 0,
    CNC_UNIT_INCH = 1
} CNCUnit;

typedef struct {
    int32_t x1, y1;        // Start point
    int32_t x2, y2;        // End point
    lv_color_t color;
    int32_t width;
    uint32_t timestamp;
} CNCLine;

typedef struct {
    int32_t cx, cy;        // Center
    int32_t radius;
    lv_color_t color;
    int32_t width;
    uint32_t timestamp;
} CNCCircle;

typedef struct {
    int32_t x1, y1;        // Top-left
    int32_t x2, y2;        // Bottom-right
    lv_color_t color;
    int32_t width;
    uint32_t timestamp;
} CNCRect;

typedef struct {
    int32_t x1, y1;
    int32_t x2, y2;
    float length_mm;
    char label[32];
    uint32_t timestamp;
} CNCDimension;

typedef struct {
    float x, y;            // Position in mm
    float width_mm;
    float height_mm;
    float rotation;        // In degrees
    char name[64];
    uint32_t timestamp;
} CNCJob;

typedef struct {
    // Current drawing tool
    CNCTool current_tool;
    CNCUnit unit;
    bool is_drawing;
    
    // Tool parameters
    int32_t line_width;
    lv_color_t draw_color;
    
    // Drawing state
    int32_t start_x, start_y;
    
    // Zoom and pan
    float zoom;
    int32_t offset_x, offset_y;
    
    // Preview state for touch drag
    bool preview_active;
    int32_t preview_x;
    int32_t preview_y;
    
    // Arrays for drawn objects (simplified)
    CNCLine lines[256];
    int32_t line_count;
    
    CNCCircle circles[64];
    int32_t circle_count;
    
    CNCRect rects[64];
    int32_t rect_count;
    
    CNCDimension dims[128];
    int32_t dim_count;
    
    // Current job
    CNCJob current_job;
    
    // UI Elements
    lv_obj_t *canvas_obj;
    lv_obj_t *toolbar_obj;
    lv_obj_t *right_panel_obj;
    lv_obj_t *status_bar_obj;
    lv_obj_t *left_ruler_obj;
    
    // Status labels
    lv_obj_t *lbl_tool;
    lv_obj_t *lbl_coords;
    lv_obj_t *lbl_dimensions;
    lv_obj_t *lbl_zoom;
    lv_obj_t *lbl_grid;
    
    // Job info display
    lv_obj_t *lbl_job_name;
    lv_obj_t *lbl_job_size;
    lv_obj_t *lbl_job_elements;
    
} CNCState;

extern CNCState g_cnc;

/* =========================================================
 * CNC CANVAS COORDINATE TRANSFORMS
 * ======================================================== */
static inline float cnc_canvas_to_world_x(float cx) { return (cx - g_cnc.offset_x) / g_cnc.zoom; }
static inline float cnc_canvas_to_world_y(float cy) { return (cy - g_cnc.offset_y) / g_cnc.zoom; }
static inline float cnc_world_to_canvas_x(float wx) { return wx * g_cnc.zoom + g_cnc.offset_x; }
static inline float cnc_world_to_canvas_y(float wy) { return wy * g_cnc.zoom + g_cnc.offset_y; }

/* =========================================================
 * CNC DRAWING FUNCTIONS
 * ======================================================== */

void cnc_init(void);
void cnc_set_tool(CNCTool tool);
void cnc_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void cnc_draw_circle(int32_t cx, int32_t cy, int32_t radius);
void cnc_draw_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void cnc_add_dimension(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const char *label);
void cnc_clear_all(void);
void cnc_undo(void);
void cnc_zoom_in(void);
void cnc_zoom_out(void);
void cnc_fit_to_view(void);
void cnc_export_job(const char *filename);
void cnc_update_status(void);
void cnc_update_job_info(void);

/* =========================================================
 * CANVAS DRAWING (Low-level)
 * ======================================================== */

void cnc_canvas_draw_grid(lv_obj_t *canvas, lv_layer_t *layer);
void cnc_canvas_draw_all(lv_obj_t *canvas, lv_layer_t *layer);
void cnc_render_lines(lv_obj_t *canvas, lv_layer_t *layer);
void cnc_render_circles(lv_obj_t *canvas, lv_layer_t *layer);
void cnc_render_rects(lv_obj_t *canvas, lv_layer_t *layer);
void cnc_render_dimensions(lv_obj_t *canvas, lv_layer_t *layer);

#endif /* CNC_DRAW_H */
