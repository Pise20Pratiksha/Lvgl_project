// // #ifndef VIEWER_H
// // #define VIEWER_H

// // #include "lvgl/lvgl.h"
// // #include <stdbool.h>
// // #include <stdint.h>

// // /* =========================================================
// //  * DISPLAY GEOMETRY (set at runtime)
// //  * ======================================================== */
// // extern int32_t SCR_W;
// // extern int32_t SCR_H;

// // #define TOOLBAR_H    28
// // #define RULER_SZ     20
// // #define RPANEL_W     155

// // extern int32_t VP_X;
// // extern int32_t VP_Y;
// // extern int32_t VP_W;
// // extern int32_t VP_H;

// // /* =========================================================
// //  * LARGE VIRTUAL CANVAS + PHYSICAL RULERS (NEW)
// //  * ======================================================== */
// // #define VIRTUAL_W       20005      // logical pixels of your data
// // #define VIRTUAL_H       12505

// // // Physical ruler limits in mm
// // #define RULER_X_MM      25.0f      // X axis ruler goes up to 25 mm
// // #define RULER_Y_MM      100.0f     // Y axis ruler goes up to 100 mm

// // // mm per logical pixel (very small values!)
// // #define MM_PER_PX_X     (RULER_X_MM / VIRTUAL_W)   // ≈ 0.001249 mm/px
// // #define MM_PER_PX_Y     (RULER_Y_MM / VIRTUAL_H)   // ≈ 0.007996 mm/px

// // // Minimum zoom levels (very fine zoom)
// // #define ZOOM_MIN        0.002f     // 0.002 px per screen pixel on X
// // #define ZOOM_MAX        50.0f      // maximum zoom (you can adjust)
// // #define ZOOM_DEFAULT    0.1f       // start at 10% of full resolution

// // /* =========================================================
// //  * COLOUR PALETTE
// //  * ======================================================== */
// // #define C_BG        0x0D0D1A
// // #define C_TOOLBAR   0x0F1729
// // #define C_RULER     0x141428
// // #define C_PANEL     0x0A0F1E
// // #define C_BORDER    0x1E3A5F
// // #define C_CYAN      0x38BDF8
// // #define C_BLUE      0x3B82F6
// // #define C_GREEN     0x10B981
// // #define C_WHITE     0xE2E8F0
// // #define C_DIM       0x64748B
// // #define C_TICK_MAJ  0x90A0B0
// // #define C_TICK_MIN  0x3A4A5E
// // #define C_CHK_A     0x1A1A2E
// // #define C_CHK_B     0x141420

// // #define H(v)  lv_color_hex(v)

// // /* =========================================================
// //  * FONTS
// //  * ======================================================== */
// // #define FONT_SM  (&lv_font_montserrat_12)
// // #define FONT_MD  (&lv_font_montserrat_14)
// // #define FONT_LG  (&lv_font_montserrat_16)

// // /* =========================================================
// //  * VIEWER MODE
// //  * ======================================================== */
// // typedef enum { MODE_PAN = 0, MODE_CROP = 1 } ViewMode;

// // /* =========================================================
// //  * VIEWER STATE - Updated for large canvas
// //  * ======================================================== */
// // typedef struct {
// //     /* Zoom & offset - now using float for sub-pixel precision */
// //     float    zoom;           // pixels per screen pixel (0.002 ~ 50.0)
// //     float    offset_x;       // viewport offset in virtual coordinates
// //     float    offset_y;

// //     /* Pan drag */
// //     bool     dragging;
// //     float    drag_sx, drag_sy;
// //     float    drag_ox, drag_oy;

// //     /* Crop */
// //     bool     has_crop;
// //     bool     cropping;
// //     float    crop_x1, crop_y1;
// //     float    crop_x2, crop_y2;

// //     /* Crosshair cursor (in viewport pixels) */
// //     float    cursor_vx;
// //     float    cursor_vy;

// //     /* Current mode */
// //     ViewMode mode;

// //     /* Widget handles */
// //     lv_obj_t *canvas_obj;     // main drawing area
// //     lv_obj_t *h_ruler_obj;
// //     lv_obj_t *v_ruler_obj;
// //     lv_obj_t *mag_obj;
// //     lv_obj_t *btn_pan;
// //     lv_obj_t *btn_crop;
// //     lv_obj_t *hint_lbl;

// //     /* Right panel labels */
// //     lv_obj_t *lbl_zoom_pct;
// //     lv_obj_t *lbl_level;
// //     lv_obj_t *lbl_px_mm;
// //     lv_obj_t *lbl_mm_px;
// //     lv_obj_t *lbl_eff_dpi;
// //     lv_obj_t *lbl_size_mm;
// //     lv_obj_t *lbl_scale_val;
// //     lv_obj_t *scale_bar_fill;
// //     lv_obj_t *lbl_sel;
// //     lv_obj_t *lbl_mem;

// // } ViewerState;

// // /* Global instance */
// // extern ViewerState g;

// // #endif /* VIEWER_H */



// #ifndef VIEWER_H
// #define VIEWER_H

// #include "lvgl/lvgl.h"
// #include <stdbool.h>
// #include <stdint.h>

// /* Display Geometry */
// extern int32_t SCR_W, SCR_H;
// extern int32_t VP_X, VP_Y, VP_W, VP_H;

// #define TOOLBAR_H    28
// #define RULER_SZ     32
// #define RPANEL_W     155

// #define CHK_CELL     10


// /* Image constants - CHANGE THESE according to your actual image */
// #define IMG_W_PX     1920
// #define IMG_H_PX     1080
// #define IMG_DPI      96

// #define MM_PER_INCH  25.4f

// /* ====================== LARGE VIRTUAL CANVAS ====================== */
// #define VIRTUAL_W       20000      // logical pixels
// #define VIRTUAL_H       5000

// #define RULER_X_MM      25.0f      // X ruler max = 25 mm
// #define RULER_Y_MM      100.0f     // Y ruler max = 100 mm

// #define MM_PER_PX_X     (RULER_X_MM / VIRTUAL_W)
// #define MM_PER_PX_Y     MM_PER_PX_X  /* Use the same mm per pixel for both axes so rulers stay symmetrical */

// #define ZOOM_MIN        0.002f
// #define ZOOM_MAX        60.0f
// #define ZOOM_DEFAULT    0.09f      // start zoomed out a bit

// /* ====================== RULER SETTINGS ====================== */
// #define RULER_X_MIN_MM     0.0f
// #define RULER_X_MAX_MM    25.0f     // Fixed physical range for X ruler
// #define RULER_Y_MIN_MM     0.0f
// #define RULER_Y_MAX_MM   100.0f     // Fixed physical range for Y ruler

// #define MIN_GAP_PX_MICRO   3
// #define MIN_GAP_PX_MINOR   6
// #define MIN_GAP_PX_MAJOR  30

// /* Ruler-specific Colors */
// #define C_RULER_BG      0x1C1C2E
// #define C_RULER_BORDER  0x2A2A3F
// #define C_RULER_MJ      0xE0E0E0     // Major ticks + labels
// #define C_RULER_MN      0xA0A0A0     // Minor ticks
// #define C_RULER_MC      0x606060     // Micro ticks
// #define C_RULER_CURSOR  0x00EEFF     // Current cursor line

// /* Colours */
// #define C_BG        0x0D0D1A
// #define C_TOOLBAR   0x0F1729
// #define C_RULER     0x141428
// #define C_PANEL     0x0A0F1E
// #define C_BORDER    0x1E3A5F
// #define C_CYAN      0x38BDF8
// #define C_BLUE      0x3B82F6
// #define C_GREEN     0x10B981
// #define C_WHITE     0xE2E8F0
// #define C_DIM       0x64748B
// #define C_TICK_MAJ  0x90A0B0
// #define C_TICK_MIN  0x3A4A5E
// #define C_CHK_A     0x1A1A2E
// #define C_CHK_B     0x141420
// #define C_IMG_TINT  0x1E3A5F
// #define C_CROP_SEL  0xF59E0B
// #define C_XH_LINE   0x38BDF8
// #define C_RED_DOT   0xE53E3E
// #define C_MAG_BG    0x08091A

// /* Button colors */
// #define C_BTN_NORM   0x383838
// #define C_BTN_PAN    0x0066CC
// #define C_BTN_CROP   0xCC6600
// #define C_BTN_BRD    0x555555

// #define H(v)  lv_color_hex(v)

// #define FONT_SM  (&lv_font_montserrat_12)
// #define FONT_MD  (&lv_font_montserrat_14)
// #define FONT_LG  (&lv_font_montserrat_16)

// typedef enum { MODE_PAN = 0, MODE_CROP = 1 } ViewMode;

// typedef struct {
//     float    zoom;
//     float    offset_x;
//     float    offset_y;

//     bool     dragging;
//     float    drag_sx, drag_sy;
//     float    drag_ox, drag_oy;

//     bool     has_crop;
//     bool     cropping;
//     float    crop_x1, crop_y1;
//     float    crop_x2, crop_y2;

//     float    cursor_vx;
//     float    cursor_vy;

//     ViewMode mode;

//     lv_obj_t *canvas_obj;
//     lv_obj_t *h_ruler_obj;
//     lv_obj_t *v_ruler_obj;
//     lv_obj_t *mag_obj;
//     lv_obj_t *btn_pan;
//     lv_obj_t *btn_crop;
//     lv_obj_t *hint_lbl;

//     lv_obj_t *lbl_zoom_pct;
//     lv_obj_t *lbl_level;
//     lv_obj_t *lbl_px_mm;
//     lv_obj_t *lbl_mm_px;
//     lv_obj_t *lbl_eff_dpi;
//     lv_obj_t *lbl_size_mm;
//     lv_obj_t *lbl_scale_val;
//     lv_obj_t *scale_bar_fill;
//     lv_obj_t *lbl_sel;
//     lv_obj_t *lbl_mem;
// } ViewerState;

// extern ViewerState g;


// #endif



#ifndef VIEWER_H
#define VIEWER_H

#include "lvgl/lvgl.h"
#include <stdbool.h>
#include <stdint.h>

/* =========================================================
 * DISPLAY GEOMETRY (set at runtime)
 * ======================================================== */
extern int32_t SCR_W, SCR_H;
extern int32_t VP_X, VP_Y, VP_W, VP_H;

#define TOOLBAR_H       36      /* top toolbar height */
#define BTOOLBAR_H      52      /* bottom toolbar height */
#define RULER_SZ        32      /* ruler thickness */
#define RPANEL_W        165     /* right panel width */

#define CHK_CELL        10

/* =========================================================
 * IMAGE / VIRTUAL CANVAS CONSTANTS
 * ======================================================== */
#define IMG_W_PX        1920
#define IMG_H_PX        1200
#define IMG_DPI         96
#define MM_PER_INCH     25.4f

#define VIRTUAL_W       20005
#define VIRTUAL_H       12005

#define RULER_X_MM      25.0f
#define RULER_Y_MM      100.0f

#define MM_PER_PX_X     (RULER_X_MM / VIRTUAL_W)
#define MM_PER_PX_Y     MM_PER_PX_X

#define ZOOM_MIN        0.002f
#define ZOOM_MAX        50.0f
#define ZOOM_DEFAULT    0.09f

/* =========================================================
 * RULER SETTINGS
 * ======================================================== */
#define RULER_X_MIN_MM   0.0f
#define RULER_X_MAX_MM   25.0f
#define RULER_Y_MIN_MM   0.0f
#define RULER_Y_MAX_MM   100.0f

#define MIN_GAP_PX_MICRO  3
#define MIN_GAP_PX_MINOR  6
#define MIN_GAP_PX_MAJOR  30

#define MAX_GAP_MARKERS     16   /* max open-endpoint markers shown */

/* =========================================================
 * COLOUR PALETTE  (dark CNC / industrial theme)
 * ======================================================== */
/* Background layers */
#define C_BG            0x0D0F14
#define C_TOOLBAR       0x161B24
#define C_BTOOLBAR      0x0F1318
#define C_RULER         0x1A1F2B
#define C_PANEL         0x1C2130
#define C_PANEL_HDR     0x242C3D
#define C_BORDER        0x2A3347
#define C_GRID_MAJOR    0x2A3A4A
#define C_GRID_MINOR    0x1C2530
#define C_CHK_A         0x131820
#define C_CHK_B         0x101519

/* Ruler colors */
#define C_RULER_BG      0x1A1F2B
#define C_RULER_BORDER  0x2A3347
#define C_RULER_MJ      0xC8D8E8
#define C_RULER_MN      0x607080
#define C_RULER_MC      0x384858
#define C_RULER_CURSOR  0x00EEFF

/* Accent colors */
#define C_CYAN          0x00D4FF
#define C_BLUE          0x2B7FFF
#define C_GREEN         0x00E5A0
#define C_YELLOW        0xF0B429
#define C_ORANGE        0xFF6B35
#define C_RED           0xFF3B3B
#define C_WHITE         0xE8EEF5
#define C_DIM           0x5A6A7A
#define C_TICK_MAJ      0x90A8C0
#define C_TICK_MIN      0x3A4E62

/* Panel text */
#define C_PNL_TITLE     0xFFFFFF
#define C_PNL_KEY       0x8898AA
#define C_PNL_VAL       0x00D4FF
#define C_PNL_SEC       0xAABBCC

/* Crosshair / overlays */
#define C_XH_LINE       0x00D4FF
#define C_CROP_SEL      0xF0B429
#define C_MAG_BG        0x080C14

/* Button palette */
#define C_BTN_NORM      0x1E2535
#define C_BTN_HOVER     0x28334A
#define C_BTN_PAN       0x1A5FAA
#define C_BTN_CROP      0xAA6010
#define C_BTN_DRAW      0x1A7A5A
#define C_BTN_ACTIVE    0x0066CC
#define C_BTN_BRD       0x3A4A5E

/* Drawing tool colors */
#define C_LINE_DRAW     0x00D4FF
#define C_ARC_DRAW      0x00E5A0
#define C_BOX_DRAW      0xF0B429
#define C_NODE_DOT      0xFF6B35

#define H(v)  lv_color_hex(v)

/* =========================================================
 * FONTS
 * ======================================================== */
#define FONT_SM  (&lv_font_montserrat_12)
#define FONT_MD  (&lv_font_montserrat_14)
#define FONT_LG  (&lv_font_montserrat_16)

#define MAX_FORGED_SEGS   32

/* Forged profile types: segment-based CNC entity (line/arc chain) */
typedef enum {
    FSEG_LINE = 0,
    FSEG_ARC  = 1
} ForgedSegType;

typedef struct ForgedSegment {
    ForgedSegType type;
    float x1, y1;        /* start point, image px            */
    float x2, y2;        /* end point,   image px            */
    float angle_deg;     /* sweep angle, degrees (ARC only)  */
    int   direction;     /* +1 = CW, -1 = CCW (ARC only)     */
    float cx, cy;         /* computed arc center, image px    */
    float radius;          /* computed arc radius, image px    */
} ForgedSegment;

/* Machining pattern applied inside a closed Forged profile.
 * NONE = default cut-away hatch (unchanged existing look). */
typedef enum {
    PATTERN_FILL_NONE   = 0,
    PATTERN_FILL_LINE   = 1,   /* parallel straight-line passes    */
    PATTERN_FILL_ZIGZAG = 2    /* connected serpentine toolpath    */
} PatternFillType;



/* =========================================================
 * VIEWER / DRAW MODES
 * ======================================================== */
typedef enum {
    MODE_PAN        = 0,
    MODE_CROP       = 1,
    MODE_LINE       = 2,    /* straight line drawing */
    MODE_ARC        = 3,    /* curve / arc drawing   */
    MODE_BOX        = 4,    /* box / rectangle       */
    MODE_EDIT       = 5,    /* node editing          */
    MODE_FILL       = 6,    /* flood fill tool       */
    MODE_CHECK      = 7,     /* validation check      */
    MODE_BAR        = 8,
    MODE_PIPE       = 9,
    MODE_FORGED     = 10,
    MODE_FORGED_CENTER = 11,
    MODE_PATTERN_FILL  = 12
} ViewMode;

/* =========================================================
 * DRAWING PRIMITIVES
 * ======================================================== */
#define MAX_DRAW_OBJECTS  256
#define MAX_ARC_CTRL_PTS    3   /* start, control, end */
#define MAX_POLY_VERTS     32



typedef enum {
    OBJ_LINE = 0,
    OBJ_ARC  = 1,
    OBJ_BOX  = 2,
    OBJ_POLY = 3,
    OBJ_BAR    = 4,
    OBJ_PIPE   = 5,
    OBJ_FORGED = 6
} DrawObjType;


typedef struct {
    DrawObjType type;
    /* For LINE: p1=start, p2=end (image coords)
       For BOX : p1=TL,    p2=BR
       For ARC : p1=start, ctrl=control, p2=end
       For POLY: verts[0..vert_count-1] are image coords. */
    float x1, y1;
    float x2, y2;
    float cx, cy;   /* control point for arc */
    int   width;    /* stroke width */
    uint32_t color; /* ARGB/RGB hex color for filled objects */
    int   vert_count;
    float verts_x[MAX_POLY_VERTS];
    float verts_y[MAX_POLY_VERTS];
    bool  selected;
    float pipe_diameter; 

    /* NEW: Forged-part segment chain (lines + arcs), kept alongside
     * verts_x/verts_y (legacy) so old saved profiles still render. */
    ForgedSegment seg_data[MAX_FORGED_SEGS];
    int           seg_count;

    bool  has_centerline;
    float cl_x1, cl_y1, cl_x2, cl_y2;   /* image-space endpoints, freely movable */

    PatternFillType pattern_fill;
    int32_t         pattern_spacing;   /* 0 = use default spacing */
    bool             fill_visible;

} DrawObject;

/* =========================================================
 * VIEWER STATE
 * ======================================================== */
typedef struct {
    /* Zoom & pan */
    float    zoom;
    float    offset_x;
    float    offset_y;

    /* Pan drag */
    bool     dragging;
    float    drag_sx, drag_sy;
    float    drag_ox, drag_oy;

    /* Crop selection */
    bool     has_crop;
    bool     cropping;
    float    crop_x1, crop_y1;
    float    crop_x2, crop_y2;

    /* Crosshair cursor (viewport pixels) */
    float    cursor_vx;
    float    cursor_vy;

    /* Current mode */
    ViewMode mode;

    /* Drawing state */
    bool     drawing;           /* mid-draw stroke     */
    int      draw_phase;        /* 0=idle, 1=drawing      */
    float    draw_x1, draw_y1;  /* first point (image) */
    float    draw_x2, draw_y2;  /* current end point      */
    float    draw_cx, draw_cy;  /* control point          */
    float    brush_size;        /* active stroke width   */
    bool     curve_dragging;    /* dragging control point */
    bool     edit_dragging;     /* dragging a selected handle */
    int      edit_drag_point;   /* 1=start, 2=control, 3=end */

    /* Drawing objects store */
    DrawObject  draw_objects[MAX_DRAW_OBJECTS];
    int         draw_count;
    int         selected_obj;   /* -1 if none */

    /* Undo stack (simple: index of last added) */
    int         undo_stack[MAX_DRAW_OBJECTS];
    int         undo_top;
    bool        undo_is_seg[MAX_DRAW_OBJECTS]; 

    /* Fill tool state */
    uint32_t    fill_color;
    int         fill_tolerance;    /* tolerance (px) used by flood fill */
    /* Palette UI */
    lv_obj_t   *palette_obj;
    bool        has_gaps;           /* gap detection result */
    int         gap_count;          /* number of gaps found */
    
    /* Validation state */
    bool        is_valid;           /* drawing passes validation */
    char        validation_msg[96]; /* validation status message */

    /* Widget handles */
    lv_obj_t *canvas_obj;
    lv_obj_t *h_ruler_obj;
    lv_obj_t *v_ruler_obj;
    lv_obj_t *mag_obj;

    /* Top toolbar buttons */
    lv_obj_t *btn_pan;
    lv_obj_t *btn_crop;
    lv_obj_t *hint_lbl;

    /* Bottom toolbar buttons */
    lv_obj_t *btn_line;
    lv_obj_t *btn_arc;
    lv_obj_t *btn_box;
    lv_obj_t *btn_edit;
    lv_obj_t *btn_fill;
    lv_obj_t *btn_check;
    lv_obj_t *btn_undo;
    lv_obj_t *btn_redo;

    lv_obj_t *btn_draw_object;
    lv_obj_t *draw_menu;            /* popup container, NULL when closed */
    ViewMode  last_draw_submode;    /* remembers active submode for icon state */

    /* NEW: forged-part segment-chain build state (dialog-driven) */
    ForgedSegment forged_segs[MAX_FORGED_SEGS];
    int           forged_seg_count;
    float         forged_cur_x, forged_cur_y;   /* pen position, image px */
    bool          forged_active;                /* profile build in progress */
    /* Backwards-compatible legacy forged point storage */
    int           forged_vcount;                /* number of legacy verts */
    float         forged_vx[MAX_POLY_VERTS];
    float         forged_vy[MAX_POLY_VERTS];


    /* Right panel labels */
    lv_obj_t *lbl_zoom_pct;
    lv_obj_t *lbl_array_idx;
    lv_obj_t *lbl_array_sz;
    lv_obj_t *lbl_resolution;
    lv_obj_t *lbl_coord;
    lv_obj_t *lbl_precision;
    lv_obj_t *lbl_sel;
    lv_obj_t *lbl_mem;
    lv_obj_t *lbl_draw_count;
    lv_obj_t *scale_bar_fill;
    lv_obj_t *lbl_scale_val;
    lv_obj_t *btn_pattern;

    float    gap_pts_x[MAX_GAP_MARKERS];
    float    gap_pts_y[MAX_GAP_MARKERS];
    int      gap_pts_count;   /* 0 = nothing to render            */

    /* ── DXF Review (read-only import), additive — see dxf_review.c ── */
    bool        dxf_review_mode;              /* true while previewing an imported DXF */
    char        dxf_review_name[64];          /* filename currently being reviewed */
    DrawObject  dxf_backup_objects[MAX_DRAW_OBJECTS];
    int         dxf_backup_count;
    ViewMode    dxf_backup_mode;
    lv_obj_t   *dxf_review_banner;

    int   forged_centerline_target;   /* index into g.draw_objects, or -1 */
    bool  centerline_dragging;
    int   centerline_drag_point;      /* 1 = first endpoint, 2 = second endpoint */

    /* Forged segment editing (MODE_EDIT) */
    int   edit_forged_seg;      /* index of seg_data[] node being dragged, -1 if none */

    /* Forged build-time undo/redo (before Finish commits it) */
    ForgedSegment forged_undo_stack[MAX_FORGED_SEGS];
    int           forged_undo_top;

} ViewerState;




/* Global instance */
extern ViewerState g;

#endif /* VIEWER_H */