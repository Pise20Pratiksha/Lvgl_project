/*==============================================================
 * src/ui/cnc_ui.c
 * CNC Drawing Interface UI Builder
 *
 * Creates a professional CAD-like interface with:
 *  • Toolbar with drawing tools
 *  • Canvas area for drawing
 *  • Right panel with job info and properties
 *  • Status bar with measurements
 *  • Ruler guides
 *==============================================================*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "lvgl/lvgl.h"
#include "cnc_draw.h"
#include "cnc_sketch.h"

#include "cnc_dxf.h"              /* NEW: declares cnc_sketch_save_dxf() */
#include "cnc_dwg.h"
#include "viewer_dxf_review.h"    /* NEW: declares show_dxf_import_dialog() */

/* =========================================================
 * COLOR SCHEME
 * ======================================================== */
#define C_BG        lv_color_hex(0x0D0D1A)
#define C_TOOLBAR   lv_color_hex(0x0F1729)
#define C_PANEL     lv_color_hex(0x0A0F1E)
#define C_BORDER    lv_color_hex(0x1E3A5F)
#define C_CYAN      lv_color_hex(0x38BDF8)
#define C_GREEN     lv_color_hex(0x10B981)
#define C_RED       lv_color_hex(0xEF4444)
#define C_WHITE     lv_color_hex(0xE2E8F0)
#define C_DIM       lv_color_hex(0x64748B)

/* =========================================================
 * EVENT HANDLERS
 * ======================================================== */

static void draw_touch_preview(lv_event_t *e)
{
    if (!g_cnc.preview_active) return;

    lv_layer_t *layer = lv_event_get_layer(e);
    if (!layer) return;

    lv_draw_line_dsc_t dsc;
    lv_draw_line_dsc_init(&dsc);
    dsc.color = C_CYAN;
    dsc.width = g_cnc.line_width;
    dsc.opa = LV_OPA_60;

    switch (g_cnc.current_tool) {
        case CNC_TOOL_LINE:
            dsc.p1.x = g_cnc.start_x;
            dsc.p1.y = g_cnc.start_y;
            dsc.p2.x = g_cnc.preview_x;
            dsc.p2.y = g_cnc.preview_y;
            lv_draw_line(layer, &dsc);
            break;
        case CNC_TOOL_RECTANGLE: {
            lv_area_t area;
            area.x1 = LV_MIN(g_cnc.start_x, g_cnc.preview_x);
            area.y1 = LV_MIN(g_cnc.start_y, g_cnc.preview_y);
            area.x2 = LV_MAX(g_cnc.start_x, g_cnc.preview_x);
            area.y2 = LV_MAX(g_cnc.start_y, g_cnc.preview_y);
            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.border_color = C_CYAN;
            rect_dsc.border_width = 1;
            rect_dsc.border_opa = LV_OPA_80;
            rect_dsc.bg_opa = LV_OPA_TRANSP;
            lv_draw_rect(layer, &rect_dsc, &area);
            break;
        }
        case CNC_TOOL_CIRCLE: {
            int32_t dx = g_cnc.preview_x - g_cnc.start_x;
            int32_t dy = g_cnc.preview_y - g_cnc.start_y;
            int32_t radius = (int32_t)sqrtf((float)dx * dx + (float)dy * dy);
            lv_draw_arc_dsc_t arc_dsc;
            lv_draw_arc_dsc_init(&arc_dsc);
            arc_dsc.color = C_CYAN;
            arc_dsc.width = 1;
            arc_dsc.opa = LV_OPA_80;
            arc_dsc.center.x = g_cnc.start_x;
            arc_dsc.center.y = g_cnc.start_y;
            arc_dsc.radius = radius;
            arc_dsc.start_angle = 0;
            arc_dsc.end_angle = 360;
            lv_draw_arc(layer, &arc_dsc);
            break;
        }
        case CNC_TOOL_DIMENSION:
            dsc.p1.x = g_cnc.start_x;
            dsc.p1.y = g_cnc.start_y;
            dsc.p2.x = g_cnc.preview_x;
            dsc.p2.y = g_cnc.preview_y;
            lv_draw_line(layer, &dsc);
            break;
        default:
            break;
    }
}

static void ruler_draw_event(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);

    lv_draw_rect_dsc_t bg_dsc;
    lv_draw_rect_dsc_init(&bg_dsc);
    bg_dsc.bg_color = C_PANEL;
    lv_draw_rect(layer, &bg_dsc, &coords);

    lv_draw_line_dsc_t tick_dsc;
    lv_draw_line_dsc_init(&tick_dsc);
    tick_dsc.width = 1;

    int32_t minor = 10;
    int32_t major = 50;

    for (int32_t y = coords.y1 + minor; y < coords.y2; y += minor) {
        bool is_major = ((y - coords.y1) % major) == 0;
        tick_dsc.color = is_major ? C_WHITE : C_DIM;
        tick_dsc.p1.x = coords.x2 - (is_major ? 10 : 6);
        tick_dsc.p1.y = y;
        tick_dsc.p2.x = coords.x2 - 1;
        tick_dsc.p2.y = y;
        lv_draw_line(layer, &tick_dsc);
    }

    tick_dsc.color = C_BORDER;
    tick_dsc.p1.x = coords.x2;
    tick_dsc.p1.y = coords.y1;
    tick_dsc.p2.x = coords.x2;
    tick_dsc.p2.y = coords.y2;
    lv_draw_line(layer, &tick_dsc);
}

static void canvas_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    lv_indev_t *indev = lv_indev_get_act();
    lv_point_t p = {0, 0};
    if (indev) lv_indev_get_point(indev, &p);
    lv_area_t obj_coords;
    lv_obj_get_coords(obj, &obj_coords);
    p.x -= obj_coords.x1;
    p.y -= obj_coords.y1;

    if (code == LV_EVENT_PRESSED) {
        g_cnc.start_x = p.x;
        g_cnc.start_y = p.y;
        g_cnc.preview_x = p.x;
        g_cnc.preview_y = p.y;
        g_cnc.preview_active = true;
        g_cnc.is_drawing = true;
        if (g_cnc.lbl_coords) {
            char buf[64];
            lv_snprintf(buf, sizeof(buf), "X: %d Y: %d", p.x, p.y);
            lv_label_set_text(g_cnc.lbl_coords, buf);
        }
    }
    else if (code == LV_EVENT_PRESSING) {
        if (!g_cnc.is_drawing) return;
        g_cnc.preview_x = p.x;
        g_cnc.preview_y = p.y;
        if (g_cnc.lbl_coords) {
            char buf[64];
            lv_snprintf(buf, sizeof(buf), "X: %d Y: %d", p.x, p.y);
            lv_label_set_text(g_cnc.lbl_coords, buf);
        }
        lv_obj_invalidate(obj);
    }
    else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        if (g_cnc.is_drawing) {
            switch (g_cnc.current_tool) {
                case CNC_TOOL_LINE:
                    cnc_draw_line(g_cnc.start_x, g_cnc.start_y, p.x, p.y);
                    break;
                case CNC_TOOL_RECTANGLE:
                    cnc_draw_rect(g_cnc.start_x, g_cnc.start_y, p.x, p.y);
                    break;
                case CNC_TOOL_CIRCLE: {
                    int32_t dx = p.x - g_cnc.start_x;
                    int32_t dy = p.y - g_cnc.start_y;
                    int32_t radius = (int32_t)sqrtf((float)dx * dx + (float)dy * dy);
                    cnc_draw_circle(g_cnc.start_x, g_cnc.start_y, radius);
                    break;
                }
                case CNC_TOOL_DIMENSION:
                    cnc_add_dimension(g_cnc.start_x, g_cnc.start_y, p.x, p.y, NULL);
                    break;
                default:
                    break;
            }
            g_cnc.is_drawing = false;
            g_cnc.preview_active = false;
            g_cnc.preview_x = p.x;
            g_cnc.preview_y = p.y;
            lv_obj_invalidate(obj);
        }
    }
    else if (code == LV_EVENT_DRAW_MAIN) {
        lv_layer_t *layer = lv_event_get_layer(e);
        cnc_canvas_draw_all(obj, layer);
        draw_touch_preview(e);
    }
}

static void btn_clear_event(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        cnc_clear_all();
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
}

static void btn_undo_event(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        cnc_undo();
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
}

static void btn_zoom_in_event(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        cnc_zoom_in();
        cnc_update_status();
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
}

static void btn_zoom_out_event(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        cnc_zoom_out();
        cnc_update_status();
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
}

static void btn_fit_event(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        cnc_fit_to_view();
        cnc_update_status();
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
}

/* =========================================================
 * TOOLBAR WITH DRAWING TOOLS
 * ======================================================== */

static lv_obj_t *g_current_tool_btn = NULL;  /* Track current selected tool button */

static void update_tool_btn_style(lv_obj_t *btn, bool selected)
{
    if (selected) {
        lv_obj_set_style_bg_color(btn, C_CYAN, 0);
        lv_obj_set_style_text_color(btn, C_BG, 0);
        lv_obj_set_style_border_color(btn, C_GREEN, 0);
        lv_obj_set_style_border_width(btn, 2, 0);
    } else {
        lv_obj_set_style_bg_color(btn, C_PANEL, 0);
        lv_obj_set_style_text_color(btn, C_CYAN, 0);
        lv_obj_set_style_border_color(btn, C_BORDER, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
    }
}

static void toolbar_btn_event_enhanced(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    intptr_t id = (intptr_t)lv_obj_get_user_data(btn);
    
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        /* Deselect previous tool */
        if (g_current_tool_btn && g_current_tool_btn != btn) {
            update_tool_btn_style(g_current_tool_btn, false);
        }
        
        /* Select new tool */
        g_current_tool_btn = btn;
        update_tool_btn_style(btn, true);
        
        cnc_set_tool((CNCTool)id);
        lv_obj_invalidate(g_cnc.canvas_obj);
    }
}

/* ===
 * SAVE & EXPORT EVENT HANDLERS
 * === */

// static void btn_save_sketch_event(lv_event_t *e)
// {
//     int code = lv_event_get_code(e);
//     printf("[btn_save_sketch_event] event=%d\n", code);
//     if (code == LV_EVENT_CLICKED || code == LV_EVENT_SHORT_CLICKED) {
//         char filename[64];
//         time_t now = time(NULL);
//         struct tm *tm_info = localtime(&now);
//         strftime(filename, sizeof(filename), "sketch_%Y%m%d_%H%M%S", tm_info);
        
//         /* Save JSON sketch */
//         if (cnc_sketch_save(filename, "User sketch from CNC UI")) {
//             printf("[btn_save_sketch_event] Saved sketch: %s\n", filename);
            
//             /* Also export as image for visual verification */
//             int canvas_w = lv_obj_get_width(g_cnc.canvas_obj);
//             int canvas_h = lv_obj_get_height(g_cnc.canvas_obj);
//             if (cnc_sketch_export_image(filename, canvas_w, canvas_h)) {
//                 printf("[btn_save_sketch_event] Image exported: %s.ppm\n", filename);
//             }
            
//             if (g_cnc.lbl_tool) {
//                 lv_label_set_text(g_cnc.lbl_tool, "✓ Sketch & Image Saved!");
//                 lv_obj_set_style_text_color(g_cnc.lbl_tool, C_GREEN, 0);
//             }
//         }
//     }
// }

static void btn_save_sketch_event(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    if (code != LV_EVENT_CLICKED) return;

    // Read canvas dimensions FIRST before any save operation touches state
    int canvas_w = (g_cnc.canvas_obj) ? lv_obj_get_width(g_cnc.canvas_obj)  : 600;
    int canvas_h = (g_cnc.canvas_obj) ? lv_obj_get_height(g_cnc.canvas_obj) : 375;

    char filename[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(filename, sizeof(filename), "sketch_%Y%m%d_%H%M%S", tm_info);

    if (cnc_sketch_save(filename, "User sketch from CNC UI")) {
        printf("[btn_save_sketch_event] Saved sketch: %s\n", filename);

        if (cnc_sketch_export_image(filename, canvas_w, canvas_h)) {
            printf("[btn_save_sketch_event] Image exported: %s.ppm\n", filename);
        }

        /* NEW: also write an equivalent DXF — this path captures
         * Bar/Pipe/Forged geometry exactly, unlike the JSON writer. */
        char dxf_name[80];
        snprintf(dxf_name, sizeof(dxf_name), "%s.dxf", filename);
        bool dxf_ok = cnc_sketch_save_dxf(dxf_name);
        if (dxf_ok) {
            printf("[btn_save_sketch_event] DXF exported: %s\n", dxf_name);
        }

        bool dwg_ok = cnc_sketch_save_dwg(filename);
        if (dwg_ok) {
            printf("[btn_save_sketch_event] DWG exported: %s.dwg\n", filename);
        }

        if (g_cnc.lbl_tool) {
            if (dxf_ok && dwg_ok) {
                lv_label_set_text(g_cnc.lbl_tool, "✓ Sketch, DXF & DWG Saved!");
            } else if (dxf_ok) {
                lv_label_set_text(g_cnc.lbl_tool, "✓ Sketch & DXF Saved (DWG failed — see log)");
            } else {
                lv_label_set_text(g_cnc.lbl_tool, "✓ Sketch Saved (DXF/DWG failed — see log)");
            }
            lv_obj_set_style_text_color(g_cnc.lbl_tool, C_GREEN, 0);
        }
    }
}

// static void btn_export_gcode_event(lv_event_t *e)
// {
//     int code = lv_event_get_code(e);
//     printf("[btn_export_gcode_event] event=%d\n", code);
//     if (code == LV_EVENT_CLICKED || code == LV_EVENT_SHORT_CLICKED) {
//         char gcode_file[128];
//         time_t now = time(NULL);
//         struct tm *tm_info = localtime(&now);
//         strftime(gcode_file, sizeof(gcode_file), "job_%Y%m%d_%H%M%S", tm_info);
        
//         char fullpath[256];
//         cnc_get_gcode_path(gcode_file, fullpath, sizeof(fullpath));
        
//         /* Generate G-code with default parameters */
//         /* Tool diameter: 1.5mm, Feed rate: 100 mm/min, Spindle: 3000 RPM */
//         char gcode_filename[sizeof(gcode_file) + 16];
//         snprintf(gcode_filename, sizeof(gcode_filename), "%s.gcode", gcode_file);
        
//         if (cnc_gcode_generate(fullpath, 1.5f, 100.0f, 3000)) {
//             printf("[btn_export_gcode_event] G-code exported: %s\n", gcode_filename);
            
//             /* Also export image with G-code for visual reference */
//             int canvas_w = lv_obj_get_width(g_cnc.canvas_obj);
//             int canvas_h = lv_obj_get_height(g_cnc.canvas_obj);
//             cnc_sketch_export_image(gcode_file, canvas_w, canvas_h);
            
//             if (g_cnc.lbl_tool) {
//                 lv_label_set_text(g_cnc.lbl_tool, "✓ G-Code & Image Ready!");
//                 lv_obj_set_style_text_color(g_cnc.lbl_tool, C_GREEN, 0);
//             }
//         }
//     }
// }

static void btn_export_gcode_event(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    printf("[btn_export_gcode_event] event=%d\n", code);

    if (code != LV_EVENT_CLICKED) return;

    /* Read canvas size FIRST before any operation that calls cnc_update_status() */
    int canvas_w = (g_cnc.canvas_obj) ? lv_obj_get_width(g_cnc.canvas_obj)  : 600;
    int canvas_h = (g_cnc.canvas_obj) ? lv_obj_get_height(g_cnc.canvas_obj) : 375;

    char gcode_file[128];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(gcode_file, sizeof(gcode_file), "job_%Y%m%d_%H%M%S", tm_info);

    char fullpath[256];
    cnc_get_gcode_path(gcode_file, fullpath, sizeof(fullpath));

    /* Append .gcode extension to the full path */
    char fullpath_ext[272];
    snprintf(fullpath_ext, sizeof(fullpath_ext), "%s.gcode", fullpath);

    if (cnc_gcode_generate(fullpath_ext, 1.5f, 100.0f, 3000)) {
        printf("[btn_export_gcode_event] G-code exported: %s.gcode\n", gcode_file);

        cnc_sketch_export_image(gcode_file, canvas_w, canvas_h);

        if (g_cnc.lbl_tool) {
            lv_label_set_text(g_cnc.lbl_tool, "✓ G-Code & Image Ready!");
            lv_obj_set_style_text_color(g_cnc.lbl_tool, C_GREEN, 0);
        }
    }
}

/* =========================================================
 * SETTINGS MODAL
 * Provides a Settings popup that exposes Save and Export
 * buttons so they remain visible on small displays.
 * ======================================================== */

static lv_obj_t *g_settings_modal = NULL;

static void close_settings_modal(void)
{
    if (!g_settings_modal) return;
    lv_obj_del(g_settings_modal);
    g_settings_modal = NULL;
}

static void btn_import_dxf_event(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    close_settings_modal();
    show_dxf_import_dialog();
}

static void btn_settings_close_event(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    printf("[btn_settings_close_event] event=%d\n", code);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_SHORT_CLICKED) {
        close_settings_modal();
    }
}

static void btn_settings_event(lv_event_t *e)
{
    int code = lv_event_get_code(e);
    printf("[btn_settings_event] event=%d, modal=%s\n", code, g_settings_modal ? "open" : "closed");
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_SHORT_CLICKED) return;

    /* Toggle modal */
    if (g_settings_modal) {
        close_settings_modal();
        return;
    }

    lv_obj_t *modal = lv_obj_create(lv_scr_act());
    g_settings_modal = modal;
    lv_obj_set_size(modal, 220, 190);
    lv_obj_center(modal);
    lv_obj_set_style_radius(modal, 8, 0);
    lv_obj_set_style_bg_color(modal, C_PANEL, 0);
    lv_obj_set_style_border_color(modal, C_BORDER, 0);
    lv_obj_set_style_border_width(modal, 1, 0);

    lv_obj_t *hdr = lv_label_create(modal);
    lv_label_set_text(hdr, "Settings");
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_color(hdr, C_CYAN, 0);
    lv_obj_set_style_text_font(hdr, &lv_font_montserrat_12, 0);

    // lv_obj_t *msg = lv_label_create(modal);
    // lv_label_set_text(msg, "Save or export your sketch from here.");
    // lv_obj_set_width(msg, 200);
    // lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    // lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *btn_row = lv_obj_create(modal);
    lv_obj_set_size(btn_row, 200, 44);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_align(btn_row, LV_ALIGN_CENTER, 0, 4);

    lv_obj_t *m_save = lv_button_create(btn_row);
    lv_obj_set_size(m_save, 90, 36);
    lv_obj_add_event_cb(m_save, btn_save_sketch_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *m_save_lbl = lv_label_create(m_save);
    lv_label_set_text(m_save_lbl, "Save");
    lv_obj_center(m_save_lbl);
    lv_obj_set_style_bg_color(m_save, C_PANEL, 0);
    lv_obj_set_style_text_color(m_save, C_CYAN, 0);
    lv_obj_set_style_border_color(m_save, C_BORDER, 0);
    lv_obj_set_style_border_width(m_save, 1, 0);
    lv_obj_set_style_radius(m_save, 4, 0);

    lv_obj_t *m_export = lv_button_create(btn_row);
    lv_obj_set_size(m_export, 90, 36);
    lv_obj_add_event_cb(m_export, btn_export_gcode_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *m_export_lbl = lv_label_create(m_export);
    lv_label_set_text(m_export_lbl, "Export");
    lv_obj_center(m_export_lbl);
    lv_obj_set_style_bg_color(m_export, C_PANEL, 0);
    lv_obj_set_style_text_color(m_export, C_CYAN, 0);
    lv_obj_set_style_border_color(m_export, C_BORDER, 0);
    lv_obj_set_style_border_width(m_export, 1, 0);
    lv_obj_set_style_radius(m_export, 4, 0);

    lv_obj_t *btn_row2 = lv_obj_create(modal);
    lv_obj_set_size(btn_row2, 200, 40);
    lv_obj_set_flex_flow(btn_row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_row2, LV_OPA_TRANSP, 0);
    lv_obj_align(btn_row2, LV_ALIGN_CENTER, 0, 52);

    lv_obj_t *m_import = lv_button_create(btn_row2);
    lv_obj_set_size(m_import, 140, 34);
    lv_obj_add_event_cb(m_import, btn_import_dxf_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *m_import_lbl = lv_label_create(m_import);
    lv_label_set_text(m_import_lbl, "Import DXF");
    lv_obj_center(m_import_lbl);
    lv_obj_set_style_bg_color(m_import, C_PANEL, 0);
    lv_obj_set_style_text_color(m_import, C_CYAN, 0);
    lv_obj_set_style_border_color(m_import, C_BORDER, 0);
    lv_obj_set_style_border_width(m_import, 1, 0);
    lv_obj_set_style_radius(m_import, 4, 0);

    /* Close button */
    lv_obj_t *m_close = lv_button_create(modal);
    lv_obj_set_size(m_close, 22, 22);
    lv_obj_align(m_close, LV_ALIGN_TOP_RIGHT, -6, 6);
    lv_obj_add_event_cb(m_close, btn_settings_close_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(m_close, btn_settings_close_event, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_t *m_close_lbl = lv_label_create(m_close);
    lv_label_set_text(m_close_lbl, "✕");
    lv_obj_center(m_close_lbl);
}

/* Public wrapper to allow other modules to open/close the settings modal */
void show_settings_modal(void)
{
    /* Toggle modal (reuse same behaviour as the button handler) */
    if (g_settings_modal) {
        close_settings_modal();
        return;
    }

    lv_obj_t *modal = lv_obj_create(lv_scr_act());
    g_settings_modal = modal;
    lv_obj_set_size(modal, 220, 190);
    lv_obj_center(modal);
    lv_obj_set_style_radius(modal, 8, 0);
    lv_obj_set_style_bg_color(modal, C_PANEL, 0);
    lv_obj_set_style_border_color(modal, C_BORDER, 0);
    lv_obj_set_style_border_width(modal, 1, 0);

    lv_obj_t *hdr = lv_label_create(modal);
    lv_label_set_text(hdr, "Settings");
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_color(hdr, C_CYAN, 0);
    lv_obj_set_style_text_font(hdr, &lv_font_montserrat_12, 0);

    // lv_obj_t *msg = lv_label_create(modal);
    // lv_label_set_text(msg, "Save or export your sketch from here.");
    // lv_obj_set_width(msg, 200);
    // lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    // lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *btn_row = lv_obj_create(modal);
    lv_obj_set_size(btn_row, 200, 44);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_align(btn_row, LV_ALIGN_CENTER, 0, 4);

    lv_obj_t *m_save = lv_button_create(btn_row);
    lv_obj_set_size(m_save, 90, 36);
    lv_obj_add_event_cb(m_save, btn_save_sketch_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(m_save, btn_save_sketch_event, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_t *m_save_lbl = lv_label_create(m_save);
    lv_label_set_text(m_save_lbl, "Save");
    lv_obj_center(m_save_lbl);
    lv_obj_set_style_bg_color(m_save, C_PANEL, 0);
    lv_obj_set_style_text_color(m_save, C_CYAN, 0);
    lv_obj_set_style_border_color(m_save, C_BORDER, 0);
    lv_obj_set_style_border_width(m_save, 1, 0);
    lv_obj_set_style_radius(m_save, 4, 0);

    lv_obj_t *m_export = lv_button_create(btn_row);
    lv_obj_set_size(m_export, 90, 36);
    lv_obj_add_event_cb(m_export, btn_export_gcode_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(m_export, btn_export_gcode_event, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_t *m_export_lbl = lv_label_create(m_export);
    lv_label_set_text(m_export_lbl, "Export");
    lv_obj_center(m_export_lbl);
    lv_obj_set_style_bg_color(m_export, C_PANEL, 0);
    lv_obj_set_style_text_color(m_export, C_CYAN, 0);
    lv_obj_set_style_border_color(m_export, C_BORDER, 0);
    lv_obj_set_style_border_width(m_export, 1, 0);
    lv_obj_set_style_radius(m_export, 4, 0);

    lv_obj_t *btn_row2 = lv_obj_create(modal);
    lv_obj_set_size(btn_row2, 200, 40);
    lv_obj_set_flex_flow(btn_row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_row2, LV_OPA_TRANSP, 0);
    lv_obj_align(btn_row2, LV_ALIGN_CENTER, 0, 52);

    lv_obj_t *m_import = lv_button_create(btn_row2);
    lv_obj_set_size(m_import, 140, 34);
    lv_obj_add_event_cb(m_import, btn_import_dxf_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *m_import_lbl = lv_label_create(m_import);
    lv_label_set_text(m_import_lbl, "Import DXF");
    lv_obj_center(m_import_lbl);
    lv_obj_set_style_bg_color(m_import, C_PANEL, 0);
    lv_obj_set_style_text_color(m_import, C_CYAN, 0);
    lv_obj_set_style_border_color(m_import, C_BORDER, 0);
    lv_obj_set_style_border_width(m_import, 1, 0);
    lv_obj_set_style_radius(m_import, 4, 0);

    /* Close button */
    lv_obj_t *m_close = lv_button_create(modal);
    lv_obj_set_size(m_close, 22, 22);
    lv_obj_align(m_close, LV_ALIGN_TOP_RIGHT, -6, 6);
    lv_obj_add_event_cb(m_close, btn_settings_close_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(m_close, btn_settings_close_event, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_t *m_close_lbl = lv_label_create(m_close);
    lv_label_set_text(m_close_lbl, "✕");
    lv_obj_center(m_close_lbl);
}

static void create_toolbar(lv_obj_t *parent)
{
    /* Main toolbar container */
    lv_obj_t *toolbar = lv_obj_create(parent);
    g_cnc.toolbar_obj = toolbar;
    
    lv_obj_set_size(toolbar, LV_PCT(100), 45);
    lv_obj_align(toolbar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toolbar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(toolbar, C_TOOLBAR, 0);
    lv_obj_set_style_pad_all(toolbar, 6, 0);
    lv_obj_set_style_pad_gap(toolbar, 4, 0);
    lv_obj_set_style_border_width(toolbar, 1, 0);
    lv_obj_set_style_border_color(toolbar, C_BORDER, 0);
    
    /* ────────────────────────────────────
     * DRAWING TOOLS SECTION
     * ──────────────────────────────────── */
    
    const char *tool_labels[] = {"✎", "◯", "▭", "⌫"};
    const CNCTool tool_ids[] = {CNC_TOOL_LINE, CNC_TOOL_CIRCLE, CNC_TOOL_RECTANGLE, CNC_TOOL_DIMENSION};
    
    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_button_create(toolbar);
        lv_obj_set_size(btn, 38, 33);
        lv_obj_set_user_data(btn, (void*)(intptr_t)tool_ids[i]);
        lv_obj_add_event_cb(btn, toolbar_btn_event_enhanced, LV_EVENT_CLICKED, NULL);
        
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, tool_labels[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
        lv_obj_center(label);
        
        lv_obj_set_style_bg_color(btn, C_PANEL, 0);
        lv_obj_set_style_text_color(btn, C_CYAN, 0);
        lv_obj_set_style_border_color(btn, C_BORDER, 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_set_style_radius(btn, 4, 0);
        
        if (i == 0) {
            g_current_tool_btn = btn;
            update_tool_btn_style(btn, true);
        }
    }
    
    /* Separator */
    lv_obj_t *sep1 = lv_obj_create(toolbar);
    lv_obj_set_size(sep1, 1, 30);
    lv_obj_set_style_bg_color(sep1, C_BORDER, 0);
    
    /* ────────────────────────────────────
     * EDIT SECTION
     * ──────────────────────────────────── */
    
    /* Undo button */
    lv_obj_t *btn_undo = lv_button_create(toolbar);
    lv_obj_set_size(btn_undo, 38, 33);
    lv_obj_add_event_cb(btn_undo, btn_undo_event, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl_undo = lv_label_create(btn_undo);
    lv_label_set_text(lbl_undo, "↶");
    lv_obj_set_style_text_font(lbl_undo, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_undo);
    lv_obj_set_style_bg_color(btn_undo, C_PANEL, 0);
    lv_obj_set_style_text_color(btn_undo, C_WHITE, 0);
    lv_obj_set_style_border_color(btn_undo, C_BORDER, 0);
    lv_obj_set_style_border_width(btn_undo, 1, 0);
    lv_obj_set_style_radius(btn_undo, 4, 0);
    
    /* Clear button */
    lv_obj_t *btn_clear = lv_button_create(toolbar);
    lv_obj_set_size(btn_clear, 38, 33);
    lv_obj_add_event_cb(btn_clear, btn_clear_event, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl_clear = lv_label_create(btn_clear);
    lv_label_set_text(lbl_clear, "✕");
    lv_obj_set_style_text_font(lbl_clear, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_clear);
    lv_obj_set_style_bg_color(btn_clear, C_RED, 0);
    lv_obj_set_style_text_color(btn_clear, C_WHITE, 0);
    lv_obj_set_style_border_width(btn_clear, 1, 0);
    lv_obj_set_style_radius(btn_clear, 4, 0);
    
    /* Separator */
    lv_obj_t *sep2 = lv_obj_create(toolbar);
    lv_obj_set_size(sep2, 1, 30);
    lv_obj_set_style_bg_color(sep2, C_BORDER, 0);
    
    /* ────────────────────────────────────
     * ZOOM SECTION
     * ──────────────────────────────────── */
    
    lv_obj_t *btn_zoom_in = lv_button_create(toolbar);
    lv_obj_set_size(btn_zoom_in, 38, 33);
    lv_obj_add_event_cb(btn_zoom_in, btn_zoom_in_event, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl_zin = lv_label_create(btn_zoom_in);
    lv_label_set_text(lbl_zin, "+");
    lv_obj_set_style_text_font(lbl_zin, &lv_font_montserrat_24, 0);
    lv_obj_center(lbl_zin);
    lv_obj_set_style_bg_color(btn_zoom_in, C_PANEL, 0);
    lv_obj_set_style_text_color(btn_zoom_in, C_GREEN, 0);
    lv_obj_set_style_border_color(btn_zoom_in, C_BORDER, 0);
    lv_obj_set_style_border_width(btn_zoom_in, 1, 0);
    lv_obj_set_style_radius(btn_zoom_in, 4, 0);
    
    lv_obj_t *btn_zoom_out = lv_button_create(toolbar);
    lv_obj_set_size(btn_zoom_out, 38, 33);
    lv_obj_add_event_cb(btn_zoom_out, btn_zoom_out_event, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl_zout = lv_label_create(btn_zoom_out);
    lv_label_set_text(lbl_zout, "−");
    lv_obj_set_style_text_font(lbl_zout, &lv_font_montserrat_24, 0);
    lv_obj_center(lbl_zout);
    lv_obj_set_style_bg_color(btn_zoom_out, C_PANEL, 0);
    lv_obj_set_style_text_color(btn_zoom_out, C_GREEN, 0);
    lv_obj_set_style_border_color(btn_zoom_out, C_BORDER, 0);
    lv_obj_set_style_border_width(btn_zoom_out, 1, 0);
    lv_obj_set_style_radius(btn_zoom_out, 4, 0);
    
    lv_obj_t *btn_fit = lv_button_create(toolbar);
    lv_obj_set_size(btn_fit, 38, 33);
    lv_obj_add_event_cb(btn_fit, btn_fit_event, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *lbl_fit = lv_label_create(btn_fit);
    lv_label_set_text(lbl_fit, "⬚");
    lv_obj_set_style_text_font(lbl_fit, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_fit);
    lv_obj_set_style_bg_color(btn_fit, C_PANEL, 0);
    lv_obj_set_style_text_color(btn_fit, C_GREEN, 0);
    lv_obj_set_style_border_color(btn_fit, C_BORDER, 0);
    lv_obj_set_style_border_width(btn_fit, 1, 0);
    lv_obj_set_style_radius(btn_fit, 4, 0);
    
    /* Separator */
    lv_obj_t *sep3 = lv_obj_create(toolbar);
    lv_obj_set_size(sep3, 1, 30);
    lv_obj_set_style_bg_color(sep3, C_BORDER, 0);

    /* Settings button (opens modal with a status message) */
    lv_obj_t *btn_settings = lv_button_create(toolbar);
    lv_obj_set_size(btn_settings, 38, 33);
    lv_obj_add_flag(btn_settings, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn_settings, btn_settings_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_settings, btn_settings_event, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_t *lbl_settings = lv_label_create(btn_settings);
    lv_label_set_text(lbl_settings, "SET");
    lv_obj_set_style_text_font(lbl_settings, &lv_font_montserrat_20, 0);
    lv_obj_center(lbl_settings);
    lv_obj_set_style_bg_color(btn_settings, C_PANEL, 0);
    lv_obj_set_style_text_color(btn_settings, C_WHITE, 0);
    lv_obj_set_style_border_color(btn_settings, C_BORDER, 0);
    lv_obj_set_style_border_width(btn_settings, 1, 0);
    lv_obj_set_style_radius(btn_settings, 4, 0);
}

/* =========================================================
 * PROPERTIES TOOLBAR (stroke width, color, grid)
 * ======================================================== */

static void stroke_width_event(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t width = (int32_t)lv_slider_get_value(slider);
    g_cnc.line_width = LV_MAX(1, LV_MIN(20, width));
}

static void color_btn_event(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        g_cnc.draw_color = lv_obj_get_style_bg_color(btn, 0);
        /* Update border to show selected */
        lv_obj_t *parent = lv_obj_get_parent(btn);
        for (uint32_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if (lv_obj_get_class(child) == &lv_button_class) {
                if (child == btn) {
                    lv_obj_set_style_border_width(child, 3, 0);
                    lv_obj_set_style_border_color(child, C_WHITE, 0);
                } else {
                    lv_obj_set_style_border_width(child, 2, 0);
                    lv_obj_set_style_border_color(child, C_BORDER, 0);
                }
            }
        }
    }
}

static void grid_toggle_event(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        bool is_pressed = lv_obj_has_state(btn, LV_STATE_CHECKED);
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text(label, is_pressed ? "Grid: ON" : "Grid: OFF");
        lv_obj_set_style_bg_color(btn, is_pressed ? C_GREEN : C_PANEL, 0);
    }
}

static void create_properties_bar(lv_obj_t *parent)
{
    lv_obj_t *prop_bar = lv_obj_create(parent);
    
    lv_obj_set_size(prop_bar, LV_PCT(100), 35);
    lv_obj_align(prop_bar, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_flex_flow(prop_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(prop_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(prop_bar, C_PANEL, 0);
    lv_obj_set_style_pad_all(prop_bar, 5, 0);
    lv_obj_set_style_pad_gap(prop_bar, 8, 0);
    lv_obj_set_style_border_width(prop_bar, 1, 0);
    lv_obj_set_style_border_color(prop_bar, C_BORDER, 0);
    
    /* ────────────────────────────────────
     * STROKE WIDTH SECTION
     * ──────────────────────────────────── */
    
    lv_obj_t *lbl_width = lv_label_create(prop_bar);
    lv_label_set_text(lbl_width, "Width:");
    lv_obj_set_style_text_color(lbl_width, C_WHITE, 0);
    
    lv_obj_t *slider_width = lv_slider_create(prop_bar);
    lv_obj_set_size(slider_width, 100, 20);
    lv_slider_set_range(slider_width, 1, 20);
    lv_slider_set_value(slider_width, 2, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider_width, stroke_width_event, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(slider_width, C_BG, 0);
    lv_obj_set_style_border_color(slider_width, C_BORDER, 0);
    lv_obj_set_style_border_width(slider_width, 1, 0);
    
    /* Separator */
    lv_obj_t *sep1 = lv_obj_create(prop_bar);
    lv_obj_set_size(sep1, 1, 25);
    lv_obj_set_style_bg_color(sep1, C_BORDER, 0);
    
    /* ────────────────────────────────────
     * COLOR SECTION (Color buttons)
     * ──────────────────────────────────── */
    
    lv_obj_t *lbl_color = lv_label_create(prop_bar);
    lv_label_set_text(lbl_color, "Color:");
    lv_obj_set_style_text_color(lbl_color, C_WHITE, 0);
    
    const lv_color_t colors[] = {C_CYAN, C_WHITE, C_RED, C_GREEN, lv_color_hex(0xFFB81C), lv_color_hex(0x6366F1)};
    for (int i = 0; i < 6; i++) {
        lv_obj_t *btn_color = lv_button_create(prop_bar);
        lv_obj_set_size(btn_color, 24, 24);
        lv_obj_set_style_bg_color(btn_color, colors[i], 0);
        lv_obj_set_style_border_width(btn_color, 2, 0);
        lv_obj_set_style_border_color(btn_color, C_BORDER, 0);
        lv_obj_set_style_radius(btn_color, 3, 0);
        lv_obj_add_event_cb(btn_color, color_btn_event, LV_EVENT_CLICKED, NULL);
        
        if (i == 0) {
            lv_obj_set_style_border_color(btn_color, C_WHITE, 0);
            lv_obj_set_style_border_width(btn_color, 3, 0);
        }
    }
    
    /* Separator */
    lv_obj_t *sep2 = lv_obj_create(prop_bar);
    lv_obj_set_size(sep2, 1, 25);
    lv_obj_set_style_bg_color(sep2, C_BORDER, 0);
    
    /* ────────────────────────────────────
     * GRID/SNAP TOGGLES
     * ──────────────────────────────────── */
    
    lv_obj_t *btn_grid = lv_button_create(prop_bar);
    lv_obj_set_size(btn_grid, 70, 25);
    lv_obj_add_event_cb(btn_grid, grid_toggle_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(btn_grid, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_add_state(btn_grid, LV_STATE_CHECKED);
    
    lv_obj_t *lbl_grid = lv_label_create(btn_grid);
    lv_label_set_text(lbl_grid, "Grid: ON");
    lv_obj_set_style_text_color(lbl_grid, C_BG, 0);
    lv_obj_set_style_text_font(lbl_grid, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_grid);
    lv_obj_set_style_bg_color(btn_grid, C_GREEN, 0);
    lv_obj_set_style_border_width(btn_grid, 1, 0);
    lv_obj_set_style_border_color(btn_grid, C_BORDER, 0);
    lv_obj_set_style_radius(btn_grid, 3, 0);
}


/* =========================================================
 * CANVAS CREATION (Updated positioning)
 * ======================================================== */

static void create_canvas(lv_obj_t *parent)
{
    lv_obj_t *canvas = lv_obj_create(parent);
    g_cnc.canvas_obj = canvas;
    
    /* Canvas positioned below toolbar (45px) and properties bar (35px) */
    lv_obj_set_size(canvas, 600, 375);  /* W x (480 - 45 - 35 - 30) */
    lv_obj_set_pos(canvas, 28, 80);
    lv_obj_clear_flag(canvas, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(canvas, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_add_event_cb(canvas, canvas_event, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(canvas, canvas_event, LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(canvas, canvas_event, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(canvas, canvas_event, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(canvas, canvas_event, LV_EVENT_DRAW_MAIN, NULL);
    
    lv_obj_set_style_bg_color(canvas, C_BG, 0);
    lv_obj_set_style_border_width(canvas, 1, 0);
    lv_obj_set_style_border_color(canvas, C_BORDER, 0);
}


/* =========================================================
 * RIGHT PANEL CREATION
 * ======================================================== */

static void create_left_ruler(lv_obj_t *parent)
{
    lv_obj_t *ruler = lv_obj_create(parent);
    g_cnc.left_ruler_obj = ruler;

    lv_obj_set_size(ruler, 28, 375);  /* Height matches canvas */
    lv_obj_set_pos(ruler, 0, 80);    /* Match canvas Y position */
    lv_obj_set_style_bg_color(ruler, C_PANEL, 0);
    lv_obj_set_style_border_width(ruler, 1, 0);
    lv_obj_set_style_border_color(ruler, C_BORDER, 0);
    lv_obj_clear_flag(ruler, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(ruler, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ruler, ruler_draw_event, LV_EVENT_DRAW_MAIN, NULL);

    lv_obj_t *label = lv_label_create(ruler);
    lv_label_set_text(label, "mm");
    lv_obj_set_style_text_color(label, C_WHITE, 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 2);
}


static void create_right_panel(lv_obj_t *parent)
{
    lv_obj_t *panel = lv_obj_create(parent);
    g_cnc.right_panel_obj = panel;
    
    lv_obj_set_size(panel, 170, 375);  /* Match canvas height */
    lv_obj_set_pos(panel, 630, 80);    /* Position next to canvas */
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(panel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_color(panel, C_PANEL, 0);
    lv_obj_set_style_border_width(panel, 1, 0);
    lv_obj_set_style_border_color(panel, C_BORDER, 0);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_gap(panel, 4, 0);
    
    /* Tool label */
    lv_obj_t *lbl_tool_hdr = lv_label_create(panel);
    lv_label_set_text(lbl_tool_hdr, "Tool");
    lv_obj_set_style_text_color(lbl_tool_hdr, C_CYAN, 0);
    lv_obj_set_style_text_font(lbl_tool_hdr, &lv_font_montserrat_12, 0);
    
    g_cnc.lbl_tool = lv_label_create(panel);
    lv_label_set_text(g_cnc.lbl_tool, "Line");
    lv_obj_set_width(g_cnc.lbl_tool, 150);
    lv_obj_set_style_text_color(g_cnc.lbl_tool, C_WHITE, 0);
    lv_obj_set_style_text_font(g_cnc.lbl_tool, &lv_font_montserrat_14, 0);
    
    /* Separator */
    lv_obj_t *sep1 = lv_obj_create(panel);
    lv_obj_set_size(sep1, 150, 1);
    lv_obj_set_style_bg_color(sep1, C_BORDER, 0);
    
    /* Job Info */
    lv_obj_t *lbl_job_hdr = lv_label_create(panel);
    lv_label_set_text(lbl_job_hdr, "Job Info");
    lv_obj_set_style_text_color(lbl_job_hdr, C_GREEN, 0);
    lv_obj_set_style_text_font(lbl_job_hdr, &lv_font_montserrat_12, 0);
    
    g_cnc.lbl_job_elements = lv_label_create(panel);
    lv_label_set_text(g_cnc.lbl_job_elements, "Objects: 0");
    lv_obj_set_width(g_cnc.lbl_job_elements, 150);
    lv_obj_set_style_text_color(g_cnc.lbl_job_elements, C_WHITE, 0);
    lv_obj_set_style_text_font(g_cnc.lbl_job_elements, &lv_font_montserrat_12, 0);
    
    g_cnc.lbl_dimensions = lv_label_create(panel);
    lv_label_set_text(g_cnc.lbl_dimensions, "Size: 0 x 0");
    lv_obj_set_width(g_cnc.lbl_dimensions, 150);
    lv_obj_set_style_text_color(g_cnc.lbl_dimensions, C_DIM, 0);
    lv_obj_set_style_text_font(g_cnc.lbl_dimensions, &lv_font_montserrat_12, 0);
    
    /* Separator */
    lv_obj_t *sep2 = lv_obj_create(panel);
    lv_obj_set_size(sep2, 150, 1);
    lv_obj_set_style_bg_color(sep2, C_BORDER, 0);
    
    /* View Info */
    lv_obj_t *lbl_view_hdr = lv_label_create(panel);
    lv_label_set_text(lbl_view_hdr, "View");
    lv_obj_set_style_text_color(lbl_view_hdr, C_CYAN, 0);
    lv_obj_set_style_text_font(lbl_view_hdr, &lv_font_montserrat_12, 0);
    
    g_cnc.lbl_zoom = lv_label_create(panel);
    lv_label_set_text(g_cnc.lbl_zoom, "Zoom: 100%");
    lv_obj_set_width(g_cnc.lbl_zoom, 150);
    lv_obj_set_style_text_color(g_cnc.lbl_zoom, C_WHITE, 0);
    lv_obj_set_style_text_font(g_cnc.lbl_zoom, &lv_font_montserrat_12, 0);

    /* Separator */
    lv_obj_t *sep3 = lv_obj_create(panel);
    lv_obj_set_size(sep3, 150, 1);
    lv_obj_set_style_bg_color(sep3, C_BORDER, 0);
}


/* =========================================================
 * STATUS BAR CREATION (Enhanced with real-time info)
 * ======================================================== */

static void create_status_bar(lv_obj_t *parent)
{
    lv_obj_t *status_bar = lv_obj_create(parent);
    g_cnc.status_bar_obj = status_bar;
    
    lv_obj_set_size(status_bar, LV_PCT(100), 30);
    lv_obj_align(status_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(status_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(status_bar, C_TOOLBAR, 0);
    lv_obj_set_style_pad_all(status_bar, 4, 0);
    lv_obj_set_style_pad_gap(status_bar, 12, 0);
    lv_obj_set_style_border_width(status_bar, 1, 0);
    lv_obj_set_style_border_color(status_bar, C_BORDER, 0);
    
    /* Left: Status indicator */
    lv_obj_t *lbl_status = lv_label_create(status_bar);
    lv_label_set_text(lbl_status, "●  Ready");
    lv_obj_set_style_text_color(lbl_status, C_GREEN, 0);
    lv_obj_set_style_text_font(lbl_status, &lv_font_montserrat_12, 0);
    
    /* Separator */
    lv_obj_t *sep1 = lv_obj_create(status_bar);
    lv_obj_set_size(sep1, 1, 20);
    lv_obj_set_style_bg_color(sep1, C_BORDER, 0);
    
    /* Coordinates (real-time) */
    g_cnc.lbl_coords = lv_label_create(status_bar);
    lv_label_set_text(g_cnc.lbl_coords, "X: 0.0  Y: 0.0");
    lv_obj_set_style_text_color(g_cnc.lbl_coords, C_CYAN, 0);
    lv_obj_set_style_text_font(g_cnc.lbl_coords, &lv_font_montserrat_12, 0);
    
    /* Separator */
    lv_obj_t *sep2 = lv_obj_create(status_bar);
    lv_obj_set_size(sep2, 1, 20);
    lv_obj_set_style_bg_color(sep2, C_BORDER, 0);
    
    /* Measurement/Distance */
    lv_obj_t *lbl_measure = lv_label_create(status_bar);
    lv_label_set_text(lbl_measure, "Dist: --");
    lv_obj_set_style_text_color(lbl_measure, C_WHITE, 0);
    lv_obj_set_style_text_font(lbl_measure, &lv_font_montserrat_12, 0);
    
    /* Separator */
    lv_obj_t *sep3 = lv_obj_create(status_bar);
    lv_obj_set_size(sep3, 1, 20);
    lv_obj_set_style_bg_color(sep3, C_BORDER, 0);
    
    /* Grid status */
    g_cnc.lbl_grid = lv_label_create(status_bar);
    lv_label_set_text(g_cnc.lbl_grid, "Grid: 10mm");
    lv_obj_set_style_text_color(g_cnc.lbl_grid, C_GREEN, 0);
    lv_obj_set_style_text_font(g_cnc.lbl_grid, &lv_font_montserrat_12, 0);
}


/* =========================================================
 * PUBLIC API
 * ======================================================== */

void cnc_create_ui(lv_obj_t *parent)
{
    /* Set background */
    lv_obj_set_style_bg_color(parent, C_BG, 0);
    
    /* Initialize CNC state */
    cnc_init();
    
    /* Create UI components in order */
    create_toolbar(parent);              /* 0-45px */
    create_properties_bar(parent);       /* 45-80px */
    create_left_ruler(parent);           /* 80px onward, 28px width */
    create_canvas(parent);               /* Main drawing area */
    create_right_panel(parent);          /* Right side info panel */
    create_status_bar(parent);           /* Bottom status bar */
    
    /* Set default tool */
    cnc_set_tool(CNC_TOOL_LINE);
    cnc_update_job_info();
}
