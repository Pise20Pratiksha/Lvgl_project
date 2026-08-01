/*==============================================================
 * Example: CNC Drawing Tool Integration
 * 
 * This file demonstrates how to integrate the CNC drawing
 * tool into your LVGL application.
 *
 * To use this, replace the contents of src/main.c with this
 * code and rebuild the project.
 *==============================================================*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lvgl/lvgl.h"
#include "cnc_ui.h"
#include "cnc_draw.h"

/* =========================================================
 * DISPLAY INITIALIZATION (Keep existing code)
 * ======================================================== */

extern void fbdev_init(void);
extern void mouse_init(void);
extern void keyboard_init(void);

/* =========================================================
 * PUMP TIMER (Keep existing code)
 * ======================================================== */

static void pump_timer(uint32_t duration_ms)
{
    uint32_t elapsed = 0;

    while (elapsed < duration_ms) {
        uint32_t sleep_ms = lv_timer_handler();
        if (sleep_ms == 0) sleep_ms = 1;
        if (sleep_ms > 10) sleep_ms = 10;
        usleep(sleep_ms * 1000u);
        elapsed += sleep_ms;
    }
}

/* =========================================================
 * BUILD UI - CNC Drawing Tool
 * ======================================================== */

static void build_ui(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0D0D1A), 0);
    
    /* Create the CNC drawing interface */
    cnc_create_ui(scr);
    
    printf("CNC Drawing Interface Loaded!\n");
}

/* =========================================================
 * MAIN APPLICATION
 * ======================================================== */

int main(int argc, char *argv[])
{
    printf("=== CNC Drawing Tool ===\n");
    printf("Starting LVGL application...\n");

    /* Initialize LVGL */
    lv_init();

    /* Initialize display driver (fbdev) */
    fbdev_init();
    
    /* Pump timer for 100ms to let display initialize */
    pump_timer(100);

    /* Get screen dimensions */
    lv_obj_t *scr = lv_screen_active();
    if (!scr) {
        fprintf(stderr, "FATAL: No active screen after initialization\n");
        return 1;
    }

    printf("Screen initialized successfully\n");

    /* Initialize input devices */
    mouse_init();
    keyboard_init();

    /* Build the UI */
    build_ui();

    printf("UI built. Entering main loop...\n");
    printf("\nCNC Tool Controls:\n");
    printf("  Line Tool    - Click and drag to draw lines\n");
    printf("  Circle Tool  - Click center, drag for radius\n");
    printf("  Rectangle    - Click and drag to define bounds\n");
    printf("  Dimension    - Click and drag to measure\n");
    printf("  Zoom +/-     - Use toolbar buttons\n");
    printf("  Undo/Clear   - Remove last element or clear all\n");
    printf("  Export       - Save job to /tmp/cnc_job.txt\n\n");

    /* Main event loop */
    while (1) {
        uint32_t delay = lv_timer_handler();
        if (delay > 10) delay = 10;
        usleep(delay * 1000);
    }

    return 0;
}

/* =========================================================
 * OPTIONAL: Command-line Interface for Testing
 * ======================================================== */

/*
Example test commands (can be added to main loop for development):

// Draw test pattern
void test_draw_pattern(void)
{
    cnc_set_tool(CNC_TOOL_LINE);
    cnc_draw_line(100, 100, 200, 100);
    cnc_draw_line(200, 100, 200, 200);
    cnc_draw_line(200, 200, 100, 200);
    cnc_draw_line(100, 200, 100, 100);
    
    cnc_set_tool(CNC_TOOL_CIRCLE);
    cnc_draw_circle(150, 150, 50);
    
    cnc_set_tool(CNC_TOOL_RECTANGLE);
    cnc_draw_rect(250, 100, 350, 200);
    
    cnc_set_tool(CNC_TOOL_DIMENSION);
    cnc_add_dimension(100, 100, 200, 100, "Width");
    cnc_add_dimension(100, 100, 100, 200, "Height");
    
    lv_obj_invalidate(g_cnc.canvas_obj);
}

// To use in main loop after build_ui():
// test_draw_pattern();
*/
