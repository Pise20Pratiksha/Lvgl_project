/*==============================================================
 * src/include/cnc_sketch.h
 * CNC Sketch Management - Save/Load/Export
 *
 * Features:
 *  • Save sketches to JSON format
 *  • Load sketches from JSON
 *  • Export sketches to G-code for CNC machines
 *  • Job history management
 *==============================================================*/

#ifndef CNC_SKETCH_H
#define CNC_SKETCH_H

#include <stdint.h>
#include <stdbool.h>
#include "cnc_draw.h"

/* =========================================================
 * SKETCH FILE DEFINITIONS
 * ======================================================== */

#define SKETCH_DIR          "/home/user/CNC_Sketches/"
#define GCODE_DIR            "/home/user/CNC_GCode/"
#define MAX_FILENAME_LEN    128

typedef struct {
    char filename[MAX_FILENAME_LEN];
    char description[256];
    uint32_t timestamp;
    uint32_t element_count;
} SketchInfo;

/* =========================================================
 * SKETCH MANAGEMENT FUNCTIONS
 * ======================================================== */

/**
 * cnc_sketch_save() - Save current drawing as a sketch file
 * @filename: Output filename (without path)
 * @description: Optional sketch description
 * Returns: true on success
 */
bool cnc_sketch_save(const char *filename, const char *description);

/**
 * cnc_sketch_load() - Load a sketch file into the drawing canvas
 * @filename: Sketch filename to load
 * Returns: true on success
 */
bool cnc_sketch_load(const char *filename);

/**
 * cnc_sketch_delete() - Delete a saved sketch
 * @filename: Sketch filename to delete
 * Returns: true on success
 */
bool cnc_sketch_delete(const char *filename);

/**
 * cnc_sketch_list() - List all saved sketches
 * @sketches: Output array for sketch info
 * @max_count: Maximum sketches to return
 * Returns: Number of sketches found
 */
int cnc_sketch_list(SketchInfo *sketches, int max_count);

/* =========================================================
 * G-CODE GENERATION FUNCTIONS
 * ======================================================== */

/**
 * cnc_gcode_generate() - Generate G-code from current sketch
 * @output_file: Path to output G-code file
 * @tool_diameter_mm: Cutting tool diameter in mm
 * @feed_rate: Tool feed rate (units/min)
 * @spindle_speed: Spindle RPM
 * Returns: true on success
 */
bool cnc_gcode_generate(const char *output_file, 
                        float tool_diameter_mm, 
                        float feed_rate, 
                        uint32_t spindle_speed);

/**
 * cnc_gcode_from_file() - Generate G-code from a sketch file
 * @sketch_file: Input sketch filename
 * @output_file: Path to output G-code file
 * @tool_diameter_mm: Cutting tool diameter in mm
 * @feed_rate: Tool feed rate (units/min)
 * @spindle_speed: Spindle RPM
 * Returns: true on success
 */
bool cnc_gcode_from_file(const char *sketch_file, 
                         const char *output_file,
                         float tool_diameter_mm, 
                         float feed_rate, 
                         uint32_t spindle_speed);

/**
 * cnc_sketch_export_image() - Export current canvas as a PNG image
 * @filename: Output filename (without path)
 * @width: Image width in pixels
 * @height: Image height in pixels
 * Returns: true on success
 */
bool cnc_sketch_export_image(const char *filename, int width, int height);

/**
 * cnc_get_sketch_path() - Get full path for a sketch file
 * @filename: Just the filename
 * @fullpath: Output buffer for full path
 * @bufsize: Size of fullpath buffer
 */
void cnc_get_sketch_path(const char *filename, char *fullpath, int bufsize);

/**
 * cnc_get_gcode_path() - Get full path for a G-code file
 * @filename: Just the filename
 * @fullpath: Output buffer for full path
 * @bufsize: Size of fullpath buffer
 */
void cnc_get_gcode_path(const char *filename, char *fullpath, int bufsize);

#define SKETCH_IMG_DIR      "/home/user/CNC_Images/"

#endif /* CNC_SKETCH_H */
