/*==============================================================
 * src/include/viewer_fill.h
 * Advanced Fill & Color Management
 *
 * Features:
 *  • Flood fill algorithm (paint bucket)
 *  • Gap detection in closed shapes
 *  • Enhanced color palette
 *  • Color history for undo/redo
 *  • Custom color picker
 *
 *==============================================================*/

#ifndef VIEWER_FILL_H
#define VIEWER_FILL_H

#include <stdint.h>
#include <stdbool.h>

/* =========================================================
 * PROFESSIONAL COLOR PALETTE (Material Design inspired)
 * ======================================================== */
#define PALETTE_COUNT 32

/* Basic Paint-style color palette */
static const uint32_t professional_palette[PALETTE_COUNT] = {
    0x000000,  /* Black */
    0x7F7F7F,  /* Gray */
    0xC0C0C0,  /* Silver */
    0xFFFFFF,  /* White */
    0x800000,  /* Maroon */
    0xFF0000,  /* Red */
    0xFF7F7F,  /* Light Red */
    0xFFA500,  /* Orange */
    0xFFFF00,  /* Yellow */
    0x9ACD32,  /* Yellow Green */
    0x00FF00,  /* Lime */
    0x00FFFF,  /* Cyan */
    0x0000FF,  /* Blue */
    0x7F00FF,  /* Purple */
    0xFF00FF,  /* Magenta */
    0xFFC0CB,  /* Pink */
    0x8B4513,  /* SaddleBrown */
    0xA52A2A,  /* Brown */
    0xD2691E,  /* Chocolate */
    0xDAA520,  /* GoldenRod */
    0xB8860B,  /* DarkGoldenRod */
    0x228B22,  /* ForestGreen */
    0x40E0D0,  /* Turquoise */
    0x1E90FF,  /* DodgerBlue */
    0x000080,  /* Navy */
    0x4B0082,  /* Indigo */
    0x8B008B,  /* DarkMagenta */
    0xFFC107,  /* Amber */
    0xFFB300,  /* Vivid Yellow */
    0x4CAF50,  /* MediumSeaGreen */
    0x00BCD4,  /* Cyan Blue */
    0x9E9E9E,  /* Medium Gray */
};

/* =========================================================
 * COLOR HISTORY (for undo/redo)
 * ======================================================== */
#define MAX_COLOR_HISTORY 32

typedef struct {
    uint32_t color;
    uint32_t target_color;  /* color being replaced */
    float    fill_x, fill_y;/* fill position */
} ColorFillAction;

typedef struct {
    ColorFillAction actions[MAX_COLOR_HISTORY];
    int             top;     /* current position in history */
    int             max;     /* max used slots */
} ColorHistory;

/* =========================================================
 * GAP DETECTION STRUCTURES
 * ======================================================== */
typedef struct {
    float   x, y;
    int     size;
} GapInfo;

#define MAX_DETECTED_GAPS 16

typedef struct {
    GapInfo gaps[MAX_DETECTED_GAPS];
    int     count;
    bool    has_gaps;
    float   min_gap_size_px;  /* pixels */
} GapDetectionResult;

/* =========================================================
 * FILL STATE
 * ======================================================== */
typedef struct {
    uint32_t current_color;
    uint32_t highlight_color;  /* for gap detection */
    bool     show_gaps;
    ColorHistory color_history;
    GapDetectionResult last_gaps;
} FillState;

/* =========================================================
 * FLOOD FILL ALGORITHM
 * ======================================================== */

/**
 * Flood Fill (Paint Bucket)
 * Fills a connected region with the target color.
 * Only fills within closed boundaries or the entire canvas if no boundaries.
 *
 * @param x, y      Start position in image coordinates
 * @param new_color Color to fill with
 * @param tolerance Color tolerance for matching
 * @return          TRUE if fill was performed, FALSE if cancelled/invalid
 */
bool viewer_flood_fill(float x, float y, uint32_t new_color, int tolerance);

/**
 * Detect Gaps in Closed Shape
 * Scans for unclosed areas in a drawn shape and returns their locations.
 * Used to highlight incomplete drawings.
 *
 * @param x1, y1, x2, y2  Bounding box of region to check
 * @param min_gap_px      Minimum gap size in pixels to detect
 * @return                GapDetectionResult with found gaps
 */
GapDetectionResult viewer_detect_gaps(float x1, float y1, float x2, float y2, 
                                       float min_gap_px);

/**
 * Highlight Detected Gaps
 * Renders visual indicators at gap locations
 *
 * @param result  Gap detection result from viewer_detect_gaps()
 * @param color   Highlight color
 */
void viewer_highlight_gaps(const GapDetectionResult *result, uint32_t color);

/**
 * Check if Point is Inside Closed Shape
 * Determines if a point lies within a closed boundary using ray casting
 *
 * @param x, y    Point in image coordinates
 * @return        TRUE if inside, FALSE if outside
 */
bool viewer_point_in_closed_shape(float x, float y);

/* =========================================================
 * COLOR MANAGEMENT
 * ======================================================== */

/**
 * Get Professional Color Palette
 * @return Pointer to palette array of PALETTE_COUNT colors
 */
const uint32_t *viewer_get_palette(void);

/**
 * Add Color to History
 * Stores a color fill action for undo/redo
 *
 * @param color   Fill color used
 * @param target  Original color being replaced
 * @param x, y    Fill position
 */
void viewer_color_history_add(uint32_t color, uint32_t target, float x, float y);

/**
 * Undo Color Fill
 * Reverts to previous color state
 */
bool viewer_color_undo(void);

/**
 * Redo Color Fill
 * Redoes a color operation
 */
bool viewer_color_redo(void);

/**
 * Remove Color from Area
 * Clears/removes fill color from a region
 *
 * @param x, y    Position
 * @param size_px Region size in pixels
 */
void viewer_remove_color(float x, float y, float size_px);

/* =========================================================
 * FILL STATE MANAGEMENT
 * ======================================================== */

/**
 * Initialize Fill Module
 * Call once at startup
 */
void viewer_fill_init(void);

/**
 * Set Current Fill Color
 * @param color ARGB/RGB hex color
 */
void viewer_set_fill_color(uint32_t color);

/**
 * Get Current Fill Color
 * @return Current fill color
 */
uint32_t viewer_get_fill_color(void);

/**
 * Enable/Disable Gap Detection Display
 * @param enabled TRUE to show gaps, FALSE to hide
 */
void viewer_set_gap_display(bool enabled);

/**
 * Perform Validation Check
 * Checks for unclosed areas and returns result
 * 
 * @param x1, y1, x2, y2  Region to check
 * @return                Result with gap information
 */
GapDetectionResult viewer_validate_closed_shape(float x1, float y1, float x2, float y2);

#endif /* VIEWER_FILL_H */
