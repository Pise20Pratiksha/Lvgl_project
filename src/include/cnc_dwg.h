#ifndef CNC_DWG_H
#define CNC_DWG_H

#include <stdbool.h>

/* Saves a DWG alongside the DXF for `filename` (no extension, e.g. "part1").
 * Internally: (1) writes filename.dxf via cnc_sketch_save_dxf(), then
 * (2) shells out to an external DXF->DWG converter (ODA File Converter
 * by default) to produce filename.dwg in the same SKETCH_DIR.
 *
 * IMPORTANT: ODA File Converter is x86_64-only. On Raspberry Pi (ARM)
 * this call will fail cleanly and log why — it is NOT a bug. Either run
 * this on an x86_64 build/host machine, or accept DXF as the deliverable
 * (every mainstream CAD package imports DXF R12 natively).
 *
 * The converter binary name can be overridden at compile time with
 * -DCNC_DWG_CONVERTER_BIN=\"/path/to/ODAFileConverter\".
 */
bool cnc_sketch_save_dwg(const char *filename);

#endif /* CNC_DWG_H */