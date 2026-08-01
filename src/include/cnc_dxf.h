#ifndef CNC_DXF_H
#define CNC_DXF_H

#include <stdbool.h>

/* Save the current canvas drawing (g.draw_objects[]) to a DXF R12 file.
 * Standard geometry entities (LINE / POLYLINE) are written for visual
 * compatibility with other CAD tools, and lossless custom XDATA
 * (APPID "CNCVIEWER") is attached to every entity so
 * cnc_sketch_load_dxf() can reconstruct the exact original DrawObject —
 * including Bar/Pipe-specific fields (pipe_diameter) and full Forged
 * segment chains (type / angle / direction / center / radius).
 *
 * 'filename' is resolved the same way as cnc_sketch_save() (SKETCH_DIR),
 * e.g. pass "part1.dxf".
 */
bool cnc_sketch_save_dxf(const char *filename);

/* Load a DXF file previously written by cnc_sketch_save_dxf() back into
 * g.draw_objects[], replacing whatever is currently on the canvas — lets
 * you re-check a Bar/Pipe/Forged drawing that was saved as DXF.
 */
bool cnc_sketch_load_dxf(const char *filename);

#endif /* CNC_DXF_H */