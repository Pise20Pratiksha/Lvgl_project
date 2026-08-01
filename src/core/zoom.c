/*==============================================================
 * src/core/zoom.c
 * zoom_at(), fit_image(), reset_view() implementations.
 * Declared in viewer_math.h
 *==============================================================*/

// #include "viewer.h"
// #include "viewer_math.h"

// void zoom_at(int32_t new_zoom, float fx, float fy)
// {
//     if(new_zoom < ZOOM_MIN) new_zoom = ZOOM_MIN;  
//     if(new_zoom > ZOOM_MAX) new_zoom = ZOOM_MAX;
//     float ix   = vp_to_img_x(fx);   /* image X (pixels) under the focal point */
//     float iy   = vp_to_img_y(fy);
//     g.zoom     = new_zoom;  /* store new fixed-point zoom in the global state */
//     float sc   = zoom_scale();
//     g.offset_x = fx - ix * sc;  /* shift image so focal point stays at fx */
//     g.offset_y = fy - iy * sc;
// }

// //Scale the image so it fits entirely inside the viewport
// void fit_image(void)
// {
//     float sx = (float)VP_W / (float)IMG_W_PX;  /* scale needed to fill width  */
//     float sy = (float)VP_H / (float)IMG_H_PX;  /* for height */
//     float s  = (sx < sy) ? sx : sy;  // fit scale
//     int32_t nz = (int32_t)(s * ZOOM_100);  /* fixed-point zoom for scale s */

//     // Clamp to valid zoom range 
//     if(nz < ZOOM_MIN) nz = ZOOM_MIN;
//     if(nz > ZOOM_MAX) nz = ZOOM_MAX;

//     g.zoom     = nz;   /* store the fitted zoom level */

//     float sc   = zoom_scale();
//     g.offset_x = (VP_W - IMG_W_PX * sc) * 0.5f;
//     g.offset_y = (VP_H - IMG_H_PX * sc) * 0.5f;
// }

// void reset_view(void)
// {
//     g.zoom     = ZOOM_100;  //100 % zoom — one image pixel = one screen pixel
//     float sc   = zoom_scale();
//     g.offset_x = (VP_W - IMG_W_PX * sc) * 0.5f;  /* centre horizontally at 1:1 */
//     g.offset_y = (VP_H - IMG_H_PX * sc) * 0.5f;
// }


#include "viewer.h"
#include "viewer_math.h"

void zoom_at(float new_zoom, float fx, float fy)
{
    new_zoom = clamp_zoom(new_zoom);

    float ix = vp_to_img_x(fx);
    float iy = vp_to_img_y(fy);

    g.zoom = new_zoom;

    float sc = zoom_scale();
    g.offset_x = fx - ix * sc;
    g.offset_y = fy - iy * sc;
}

void fit_image(void)
{
    /* Fit the full virtual canvas inside viewport with 5% margin */
    float sx = (float)VP_W / (float)VIRTUAL_W;
    float sy = (float)VP_H / (float)VIRTUAL_H;
    float s  = (sx < sy ? sx : sy) * 0.95f;

    g.zoom     = clamp_zoom(s);
    (void)zoom_scale();  /* computed but not currently used */
    g.offset_x = 0.0f;
    g.offset_y = 0.0f;
}

void reset_view(void)
{
    g.zoom     = ZOOM_DEFAULT;
    (void)zoom_scale();  /* computed but not currently used */
    g.offset_x = 0.0f;
    g.offset_y = 0.0f;
}