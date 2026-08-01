#ifndef VIEWER_DXF_REVIEW_H
#define VIEWER_DXF_REVIEW_H

#include <stdbool.h>

/* Opens a modal file picker listing saved .dxf sketches (SKETCH_DIR)
 * and, on selection, loads the chosen file onto the canvas in a
 * read-only review mode. Call this from the Export flow. */
void show_dxf_import_dialog(void);

/* Exits DXF review mode, restoring whatever drawing was on the
 * canvas before the review began. Safe to call even if not active. */
void cnc_dxf_exit_review(void);

/* True while a DXF is being reviewed read-only. */
bool cnc_dxf_review_active(void);

#endif /* VIEWER_DXF_REVIEW_H */