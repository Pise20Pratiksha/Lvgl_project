
#ifndef VIEWER_MATH_H
#define VIEWER_MATH_H

#include "viewer.h"
#include <math.h>

/* =========================================================
 * ZOOM & SCALE
 * ======================================================== */
static inline float zoom_scale(void) { return g.zoom; }

static inline float px_per_mm_x(void) { return zoom_scale() / MM_PER_PX_X; }
static inline float px_per_mm_y(void) { return zoom_scale() / MM_PER_PX_Y; }

/* Coordinate transforms */
static inline float vp_to_img_x(float vx) { return (vx - g.offset_x) / zoom_scale(); }
static inline float vp_to_img_y(float vy) { return (vy - g.offset_y) / zoom_scale(); }

static inline float img_to_vp_x(float ix) { return ix * zoom_scale() + g.offset_x; }
static inline float img_to_vp_y(float iy) { return iy * zoom_scale() + g.offset_y; }

/* mm conversion */
static inline float img_to_mm_x(float ix) { return ix * MM_PER_PX_X; }
static inline float img_to_mm_y(float iy) { return iy * MM_PER_PX_Y; }

static inline float vp_to_mm_x(float vx) { return img_to_mm_x(vp_to_img_x(vx)); }
static inline float vp_to_mm_y(float vy) { return img_to_mm_y(vp_to_img_y(vy)); }

bool forged_closed_polygon_image(const DrawObject *o, float *out_x, float *out_y, int *out_n);


static inline float clamp_zoom(float z)
{
    if (z < ZOOM_MIN) return ZOOM_MIN;
    if (z > ZOOM_MAX) return ZOOM_MAX;
    return z;
}

/* =========================================================
 * RULER TICK HELPERS
 * ======================================================== */

#define MIN_GAP_PX_MICRO   3
#define MIN_GAP_PX_MINOR   6
#define MIN_GAP_PX_MAJOR  30

#define RULER_X_MIN_MM     0.0f
#define RULER_X_MAX_MM    25.0f
#define RULER_Y_MIN_MM     0.0f
#define RULER_Y_MAX_MM   100.0f

/* Nice adaptive tick interval */
static inline float nice_interval(float px_per_mm, float base_mm, float min_gap_px)
{
    static const float m[] = {
        0.001f, 0.002f, 0.005f,
        0.01f,  0.02f,  0.05f,
        0.1f,   0.2f,   0.5f,
        1.0f,   2.0f,   5.0f,
        10.0f,  20.0f,  50.0f,
        100.0f
    };

    for (int i = 0; i < (int)(sizeof(m)/sizeof(m[0])); i++) {
        float interval = base_mm * m[i];
        if (interval * px_per_mm >= min_gap_px)
            return interval;
    }
    return base_mm * 100.0f;
}

static inline bool is_on_interval(float mm, float intv)
{
    if (intv <= 0.0f) return false;
    float r = fmodf(mm, intv);
    if (r < 0.0f) r += intv;
    return (r < intv * 0.02f) || (r > intv - intv * 0.02f);
}

/* =========================================================
 * ZOOM FUNCTIONS - DECLARATIONS
 * ======================================================== */
void zoom_at(float new_zoom, float fx, float fy);
void fit_image(void);
void reset_view(void);

#endif /* VIEWER_MATH_H */