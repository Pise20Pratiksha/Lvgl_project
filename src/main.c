// /*==============================================================
//  * src/main.c
//  *
//  * FIX SUMMARY:
//  *  1. Guard against NULL display / zero resolution after fbdev init.
//  *  2. Pump lv_timer_handler() in a small spin loop BEFORE building
//  *     the UI so the display flushes its first frame and registers
//  *     its active screen — eliminating the "no display registered"
//  *     warning and the NULL-screen segfault.
//  *  3. Cap the timer sleep at 10 ms so touch events stay responsive.
//  *  4. Link every pointer indev to the default display explicitly.
//  *==============================================================*/

// #define _GNU_SOURCE
// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
// #include <unistd.h>        /* usleep */

// #include "lvgl/lvgl.h"


// #include "viewer.h"
// #include "viewer_math.h"
// #include "viewer_ui.h"

// /* ----------------------------------------------------------
//  * pump_timer: run the LVGL timer handler for <ms> milliseconds
//  * so the display driver has time to finish its first-frame
//  * setup and register a valid active screen.
//  * ---------------------------------------------------------- */
// static void pump_timer(uint32_t duration_ms)
// {
//     uint32_t elapsed = 0; // total time spent running timer

//     while (elapsed < duration_ms) {
//         uint32_t sleep_ms = lv_timer_handler();  // LVGL internal task handler (refresh display, input, etc.)
//         if (sleep_ms == 0) sleep_ms = 1;    // Safety: never sleep 0 ms
//         if (sleep_ms > 10) sleep_ms = 10;   // Limit sleep to 10ms → keeps touch responsive
//         usleep(sleep_ms * 1000u);   // Sleep in microseconds
//         elapsed += sleep_ms;
//     }
// }

// /* ----------------------------------------------------------
//  * build_ui: construct the entire LVGL widget tree.
//  * Called only after the display has a valid active screen.
//  * ---------------------------------------------------------- */
// static void build_ui(void)
// {
//     printf("[build_ui] Building UI...\n"); fflush(stdout);

//     /* ---- Sanity-check display ---- */
//     lv_display_t *disp = lv_display_get_default();
//     if (disp == NULL) {
//         fprintf(stderr, "[build_ui] FATAL: no default display\n");
//         exit(1);
//     }

//     /* ---- Get active screen object ---- */
//     lv_obj_t *scr = lv_display_get_screen_active(disp);
//     if (scr == NULL) {
//         fprintf(stderr, "[build_ui] FATAL: active screen is NULL\n");
//         exit(1);
//     }

//     /* ---- Read real resolution from driver ---- */
//     SCR_W = lv_display_get_horizontal_resolution(disp);
//     SCR_H = lv_display_get_vertical_resolution(disp);

//     if (SCR_W == 0 || SCR_H == 0) {
//         fprintf(stderr,
//             "[build_ui] FATAL: display resolution is %dx%d — "
//             "fbdev did not report dimensions yet.\n",
//             (int)SCR_W, (int)SCR_H);
//         exit(1);
//     }

//     VP_X = RULER_SZ;     // start after left ruler
//     VP_Y = TOOLBAR_H + RULER_SZ;    // below toolbar + top ruler
//     VP_W = SCR_W - RULER_SZ - RPANEL_W;    // remove ruler + right panel
//     VP_H = SCR_H - TOOLBAR_H - RULER_SZ;   // remove toolbar + top ruler

//     // Debug print
//     printf("[build_ui] SCR=%dx%d  VP=(%d,%d) %dx%d\n",
//            (int)SCR_W, (int)SCR_H,
//            (int)VP_X,  (int)VP_Y,
//            (int)VP_W,  (int)VP_H);
//     fflush(stdout);

//     /* ---- Validate viewport dimensions ---- */
//     if (VP_W <= 0 || VP_H <= 0) {
//         fprintf(stderr,
//             "[build_ui] FATAL: viewport dimensions invalid (%dx%d). "
//             "Check RULER_SZ=%d RPANEL_W=%d TOOLBAR_H=%d vs SCR=%dx%d\n",
//             (int)VP_W, (int)VP_H,
//             (int)RULER_SZ, (int)RPANEL_W, (int)TOOLBAR_H,
//             (int)SCR_W, (int)SCR_H);
//         exit(1);
//     }

//     /* ---- Initialise global state ---- */
//     lv_memset(&g, 0, sizeof(g));     // reset everything
//     // g.cursor_vx = (float)VP_W * 0.5f;   // cursor X (center of viewport)
//     // g.cursor_vy = (float)VP_H * 0.5f;   // cursor Z (center)
//     g.zoom      = ZOOM_DEFAULT;
//     g.mode      = MODE_PAN;       // default interaction mode

//     float sc    = zoom_scale();     // convert zoom → scale factor

//     // Center image inside viewport
//     g.offset_x  = (VP_W - VIRTUAL_W * sc) * 0.5f;
//     g.offset_y  = (VP_H - VIRTUAL_H * sc) * 0.5f;

//     g.cursor_vx = (float)VP_W * 0.5f;
//     g.cursor_vy = (float)VP_H * 0.5f;

//     /* ---- Style the root screen ---- */
//     lv_obj_set_style_bg_color(scr, H(C_BG), 0);  // background color
//     lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
//     lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

//     /* ---- Build widget tree ---- */
//     build_canvas_area(scr);   // main image + rulers
//     build_right_panel(scr);    // info/settings panel
//     build_toolbar(scr);       // top toolbar buttons

//     set_mode(MODE_PAN);  // start in pan mode
//     update_panel();       // refresh right panel values
//     lv_obj_invalidate(scr);  // force redraw

//     printf("[build_ui] Done\n"); fflush(stdout);
// }


// int main(void)
// {
//     memset(&g, 0, sizeof(g));

//     printf("[main] lv_init...\n"); fflush(stdout);
//     lv_init();
//     lv_fs_stdio_init();

//     /* ================================================
//      * DISPLAY: Use fbdev instead of DRM on Raspberry Pi
//      * ================================================ */
//     printf("[main] Initializing lv_linux_fbdev...\n"); fflush(stdout);

//     lv_display_t *disp = lv_linux_fbdev_create();
//     if (disp == NULL) {
//         fprintf(stderr, "[main] FATAL: lv_linux_fbdev_create() failed\n");
//         return -1;
//     }

//     if (lv_linux_fbdev_set_file(disp, "/dev/fb0") != LV_RESULT_OK) {
//         fprintf(stderr, "[main] FATAL: lv_linux_fbdev_set_file(/dev/fb0) failed\n");
//         // Try fb1 as fallback
//         if (lv_linux_fbdev_set_file(disp, "/dev/fb1") != LV_RESULT_OK) {
//             fprintf(stderr, "[main] FATAL: Failed to open both /dev/fb0 and /dev/fb1\n");
//             return -1;
//         }
//         printf("[main] Using /dev/fb1\n");
//     } else {
//         printf("[main] Using /dev/fb0 successfully\n");
//     }

//     /* ================================================
//      * INPUT: evdev touch / mouse
//      * ================================================ */
//     printf("[main] Initializing evdev touch...\n"); fflush(stdout);

//     const char *evdev_path = "/dev/input/event5";   // ← change if needed

//     lv_indev_t *touch_indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, evdev_path);
//     if (touch_indev == NULL) {
//         fprintf(stderr, "[main] WARNING: lv_evdev_create failed for %s\n", evdev_path);
//     } else {
//         lv_indev_set_display(touch_indev, disp);
//         printf("[main] Touch input linked: %s\n", evdev_path);
//     }

//     /* Warm up LVGL + display driver */
//     printf("[main] Warming up display...\n"); fflush(stdout);
//     pump_timer(300);        // Give fbdev time to report resolution

//     /* Build UI */
//     build_ui();

//     printf("[main] Entering main loop...\n"); fflush(stdout);

//     /* Main loop */
//     while (1) {
//         uint32_t ms = lv_timer_handler();
//         if (ms == 0) ms = 1;
//         if (ms > 10) ms = 10;
//         usleep(ms * 1000u);
//     }

//     return 0;
// }

/*==============================================================
 * src/main.c
 *
 * Changes vs original:
 *  1. VP_H now accounts for BTOOLBAR_H (bottom toolbar).
 *  2. build_bottom_toolbar() called after build_canvas_area().
 *  3. g.selected_obj initialised to -1 (no selection).
 *==============================================================*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lvgl/lvgl.h"
#include "viewer.h"
#include "viewer_math.h"
#include "viewer_ui.h"
#include "viewer_fill.h"

/* ----------------------------------------------------------
 * pump_timer: run LVGL timer handler for <duration_ms> ms
 * ---------------------------------------------------------- */
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

/* ----------------------------------------------------------
 * build_ui: construct the full LVGL widget tree
 * ---------------------------------------------------------- */
static void build_ui(void)
{
    printf("[build_ui] Building UI...\n"); fflush(stdout);

    lv_display_t *disp = lv_display_get_default();
    if (!disp) { fprintf(stderr, "[build_ui] FATAL: no default display\n"); exit(1); }

    lv_obj_t *scr = lv_display_get_screen_active(disp);
    if (!scr) { fprintf(stderr, "[build_ui] FATAL: active screen is NULL\n"); exit(1); }

    SCR_W = lv_display_get_horizontal_resolution(disp);
    SCR_H = lv_display_get_vertical_resolution(disp);
    if (SCR_W == 0 || SCR_H == 0) {
        fprintf(stderr, "[build_ui] FATAL: display resolution %dx%d\n",
                (int)SCR_W, (int)SCR_H);
        exit(1);
    }

    /* ---- Viewport geometry ----
     *  Top:    TOOLBAR_H
     *  Left:   RULER_SZ
     *  Right:  RPANEL_W
     *  Bottom: BTOOLBAR_H
     */
    VP_X = RULER_SZ;
    VP_Y = TOOLBAR_H + RULER_SZ;
    VP_W = SCR_W - RULER_SZ - RPANEL_W;
    VP_H = SCR_H - TOOLBAR_H - RULER_SZ - BTOOLBAR_H;

    printf("[build_ui] SCR=%dx%d  VP=(%d,%d) %dx%d\n",
           (int)SCR_W, (int)SCR_H,
           (int)VP_X, (int)VP_Y, (int)VP_W, (int)VP_H);
    fflush(stdout);

    if (VP_W <= 0 || VP_H <= 0) {
        fprintf(stderr, "[build_ui] FATAL: VP invalid (%dx%d)\n",
                (int)VP_W, (int)VP_H);
        exit(1);
    }

    /* ---- Initialise global state ---- */
    lv_memset(&g, 0, sizeof(g));
    g.zoom         = ZOOM_DEFAULT;
    g.mode         = MODE_PAN;
    g.brush_size   = 4.0f;
    g.selected_obj = -1;
    g.fill_color   = 0x38BDF8; /* default fill color */
    
    /* Initialize fill module with professional features */
    viewer_fill_init();
    viewer_set_fill_color(0x38BDF8);

    /* Start at the true origin: top-left of the virtual canvas.
     * This means users open at (0,0) and do not need Reset to see it. */
    g.offset_x = 0.0f;
    g.offset_y = 0.0f;
    g.cursor_vx = 0.0f;
    g.cursor_vy = 0.0f;
    // g.cursor_vx = (float)VP_W * 0.5f;
    // g.cursor_vy = (float)VP_H * 0.5f;

    /* ---- Style root screen ---- */
    lv_obj_set_style_bg_color(scr, H(C_BG), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_scrollbar_mode(scr, LV_SCROLLBAR_MODE_OFF);

    /* ---- Build widget tree ---- */
    build_canvas_area(scr);       /* canvas + rulers */
    build_right_panel(scr);       /* CNC JOB DETAILS */
    build_toolbar(scr);           /* top toolbar     */
    build_bottom_toolbar(scr);    /* drawing tools   */

    set_mode(MODE_PAN);
    update_panel();
    lv_obj_invalidate(scr);

    printf("[build_ui] Done\n"); fflush(stdout);
}

/* ----------------------------------------------------------
 * main
 * ---------------------------------------------------------- */
int main(void)
{
    memset(&g, 0, sizeof(g));

    printf("[main] lv_init...\n"); fflush(stdout);
    lv_init();
    lv_fs_stdio_init();

    printf("[main] Initializing lv_linux_fbdev...\n"); fflush(stdout);

    lv_display_t *disp = lv_linux_fbdev_create();
    if (!disp) {
        fprintf(stderr, "[main] FATAL: lv_linux_fbdev_create() failed\n");
        return -1;
    }

    if (lv_linux_fbdev_set_file(disp, "/dev/fb0") != LV_RESULT_OK) {
        fprintf(stderr, "[main] Trying /dev/fb1...\n");
        if (lv_linux_fbdev_set_file(disp, "/dev/fb1") != LV_RESULT_OK) {
            fprintf(stderr, "[main] FATAL: Cannot open fb0 or fb1\n");
            return -1;
        }
        printf("[main] Using /dev/fb1\n");
    } else {
        printf("[main] Using /dev/fb0\n");
    }

    /* Touch input */
    const char *evdev_path = "/dev/input/event5";
    lv_indev_t *touch_indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, evdev_path);
    if (!touch_indev) {
        fprintf(stderr, "[main] WARNING: lv_evdev_create failed for %s\n", evdev_path);
    } else {
        lv_indev_set_display(touch_indev, disp);
        printf("[main] Touch input linked: %s\n", evdev_path);
    }

    /* Warm up display */
    printf("[main] Warming up display...\n"); fflush(stdout);
    pump_timer(300);

    build_ui();

    printf("[main] Entering main loop...\n"); fflush(stdout);

    while (1) {
        uint32_t ms = lv_timer_handler();
        if (ms == 0) ms = 1;
        if (ms > 10) ms = 10;
        usleep(ms * 1000u);
    }

    return 0;
}