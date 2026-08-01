/*==============================================================
 * src/ui/color_picker.c
 * Professional Color Picker UI Component
 *
 * Features:
 *  • Professional color palette display
 *  • Recent colors history
 *  • Custom color input
 *  • RGB slider controls
 *  • Color preview
 *  • Hex color input
 *
 *==============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lvgl/lvgl.h"
#include "../include/viewer.h"
#include "../include/viewer_ui.h"
#include "../include/viewer_fill.h"
#include "../include/color_picker.h"

/* Color picker dialog state */
static lv_obj_t *color_picker_dialog = NULL;
static lv_obj_t *preview_obj = NULL;
static lv_obj_t *lbl_r_val = NULL;
static lv_obj_t *lbl_g_val = NULL;
static lv_obj_t *lbl_b_val = NULL;
static uint32_t selected_color = 0x38BDF8;

/* Recent/custom colors (last 8) */
static uint32_t recent_colors[8] = {0};
static int recent_count = 0;

/* =========================================================
 * COLOR UTILITIES
 * ======================================================== */


static void add_recent_color(uint32_t color)
{
    /* Check if color already exists */
    for (int i = 0; i < recent_count; i++) {
        if (recent_colors[i] == color) return;
    }
    
    /* Shift colors and add new one */
    if (recent_count >= 8) {
        for (int i = 0; i < 7; i++) {
            recent_colors[i] = recent_colors[i + 1];
        }
        recent_colors[7] = color;
    } else {
        recent_colors[recent_count++] = color;
    }
}

/* =========================================================
 * COLOR PICKER EVENT HANDLERS
 * ======================================================== */

static void update_color_preview(void)
{
    if (preview_obj) {
        lv_obj_set_style_bg_color(preview_obj, lv_color_hex(selected_color), 0);
    }
}

static void color_palette_cb(lv_event_t *e)
{
    uint32_t color = (uint32_t)(intptr_t)lv_event_get_user_data(e);
    selected_color = color;
    add_recent_color(color);
    update_color_preview();
}

static void rgb_slider_r_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    
    int r = value;
    int g_val = (selected_color >> 8) & 0xFF;
    int b = selected_color & 0xFF;
    
    selected_color = (r << 16) | (g_val << 8) | b;
    update_color_preview();
    if (lbl_r_val) lv_label_set_text_fmt(lbl_r_val, "%d", value);
}

static void rgb_slider_g_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    
    int r = (selected_color >> 16) & 0xFF;
    int g_val = value;
    int b = selected_color & 0xFF;
    
    selected_color = (r << 16) | (g_val << 8) | b;
    update_color_preview();
    if (lbl_g_val) lv_label_set_text_fmt(lbl_g_val, "%d", value);
}

static void rgb_slider_b_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    
    int r = (selected_color >> 16) & 0xFF;
    int g_val = (selected_color >> 8) & 0xFF;
    int b = value;
    
    selected_color = (r << 16) | (g_val << 8) | b;
    update_color_preview();
    if (lbl_b_val) lv_label_set_text_fmt(lbl_b_val, "%d", value);
}

static void hex_input_cb(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    const char *text = lv_textarea_get_text(ta);
    
    if (text && text[0] == '#' && strlen(text) == 7) {
        uint32_t color = 0;
        sscanf(text, "#%x", &color);
        selected_color = color & 0xFFFFFF;
        update_color_preview();
    }
}

static void confirm_cb(lv_event_t *e)
{
    (void)e;
    viewer_set_fill_color(selected_color);
    g.fill_color = selected_color;
    add_recent_color(selected_color);

    if (color_picker_dialog) {
        lv_obj_del(color_picker_dialog);
        color_picker_dialog = NULL;
        preview_obj = NULL;
    }

    set_mode(MODE_FILL);
}

static void cancel_cb(lv_event_t *e)
{
    (void)e;
    if (color_picker_dialog) {
        lv_obj_del(color_picker_dialog);
        color_picker_dialog = NULL;
    }
}

/* =========================================================
 * COLOR PICKER UI CREATION
 * ======================================================== */

void show_professional_color_picker(void)
{
    lv_obj_t *scr = lv_scr_act();
    
    if (color_picker_dialog) {
        lv_obj_del(color_picker_dialog);
    }
    
    /* Main dialog container */
    lv_obj_t *dlg = lv_obj_create(scr);
    lv_obj_set_size(dlg, 440, 520);
    lv_obj_center(dlg);
    lv_obj_set_style_bg_color(dlg, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_border_color(dlg, lv_color_hex(0x3F51B5), 0);
    lv_obj_set_style_border_width(dlg, 2, 0);
    lv_obj_set_style_radius(dlg, 8, 0);
    lv_obj_set_style_pad_all(dlg, 12, 0);
    
    color_picker_dialog = dlg;
    
    /* Title */
    lv_obj_t *title = lv_label_create(dlg);
    lv_label_set_text(title, "Select Color");
    lv_obj_set_style_text_color(title, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_set_width(title, lv_pct(100));
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
    
    /* Color preview box */
    lv_obj_t *preview = lv_obj_create(dlg);
    lv_obj_set_size(preview, 100, 100);
    lv_obj_set_style_bg_color(preview, lv_color_hex(selected_color), 0);
    lv_obj_set_style_border_color(preview, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(preview, 2, 0);
    lv_obj_set_style_radius(preview, 6, 0);
    lv_obj_align(preview, LV_ALIGN_TOP_RIGHT, -12, 28);
    preview_obj = preview;
    
    lv_obj_t *preview_label = lv_label_create(dlg);
    lv_label_set_text(preview_label, "Current Color");
    lv_obj_set_style_text_color(preview_label, lv_color_hex(0xB0B0B0), 0);
    lv_obj_set_style_text_font(preview_label, &lv_font_montserrat_12, 0);
    lv_obj_align(preview_label, LV_ALIGN_TOP_RIGHT, -12, 132);
    
    /* ─────────────────────────────────────────────────────
     * BASIC COLORS
     * ───────────────────────────────────────────────────── */
    
    lv_obj_t *pal_label = lv_label_create(dlg);
    lv_label_set_text(pal_label, "Basic colors:");
    lv_obj_set_style_text_color(pal_label, lv_color_hex(0xB0B0B0), 0);
    lv_obj_set_style_text_font(pal_label, &lv_font_montserrat_12, 0);
    lv_obj_set_width(pal_label, lv_pct(100));
    lv_obj_align(pal_label, LV_ALIGN_TOP_LEFT, 0, 30);
    
    const uint32_t *palette = viewer_get_palette();
    int cols = 8;
    int box_size = 28;
    int spacing = 4;
    int base_y = 52;
    
    for (int i = 0; i < PALETTE_COUNT; i++) {
        int row = i / cols;
        int col = i % cols;
        
        lv_obj_t *color_box = lv_obj_create(dlg);
        lv_obj_set_size(color_box, box_size, box_size);
        lv_obj_set_pos(color_box, col * (box_size + spacing), base_y + row * (box_size + spacing));
        lv_obj_set_style_bg_color(color_box, lv_color_hex(palette[i]), 0);
        lv_obj_set_style_border_width(color_box, 1, 0);
        lv_obj_set_style_border_color(color_box, lv_color_hex(0x404040), 0);
        lv_obj_set_style_radius(color_box, 4, 0);
        
        lv_obj_add_event_cb(color_box, color_palette_cb, LV_EVENT_CLICKED,
                           (void*)(intptr_t)palette[i]);
    }
    
    /* ─────────────────────────────────────────────────────
     * CUSTOM COLORS
     * ───────────────────────────────────────────────────── */
    
    lv_obj_t *recent_label = lv_label_create(dlg);
    lv_label_set_text(recent_label, "Custom colors:");
    lv_obj_set_style_text_color(recent_label, lv_color_hex(0xB0B0B0), 0);
    lv_obj_set_style_text_font(recent_label, &lv_font_montserrat_12, 0);
    lv_obj_align(recent_label, LV_ALIGN_TOP_LEFT, 0, 180);
    
    for (int i = 0; i < 8; i++) {
        lv_obj_t *recent_box = lv_obj_create(dlg);
        lv_obj_set_size(recent_box, 28, 28);
        lv_obj_set_pos(recent_box, i * 32, 204);
        lv_obj_set_style_bg_color(recent_box,
                                 lv_color_hex(recent_colors[i] ? recent_colors[i] : 0xFFFFFF), 0);
        lv_obj_set_style_border_width(recent_box, 1, 0);
        lv_obj_set_style_border_color(recent_box, lv_color_hex(0x404040), 0);
        lv_obj_set_style_radius(recent_box, 4, 0);
        
        if (recent_colors[i]) {
            lv_obj_add_event_cb(recent_box, color_palette_cb, LV_EVENT_CLICKED,
                               (void*)(intptr_t)recent_colors[i]);
        }
    }
    
    /* ─────────────────────────────────────────────────────
     * RGB SLIDERS
     * ───────────────────────────────────────────────────── */
    
    /* Red slider */
    lv_obj_t *lbl_r = lv_label_create(dlg);
    lv_label_set_text(lbl_r, "R:");
    lv_obj_set_style_text_color(lbl_r, lv_color_hex(0xFF6B6B), 0);
    lv_obj_align(lbl_r, LV_ALIGN_TOP_LEFT, 0, 310);
    
    lv_obj_t *slider_r = lv_slider_create(dlg);
    lv_slider_set_range(slider_r, 0, 255);
    lv_slider_set_value(slider_r, (selected_color >> 16) & 0xFF, false);
    lv_obj_set_size(slider_r, 200, 12);
    lv_obj_align(slider_r, LV_ALIGN_TOP_LEFT, 30, 310);
    lv_obj_add_event_cb(slider_r, rgb_slider_r_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lbl_r_val = lv_label_create(dlg);
    lv_label_set_text_fmt(lbl_r_val, "%d", (selected_color >> 16) & 0xFF);
    lv_obj_set_style_text_color(lbl_r_val, lv_color_hex(0xE0E0E0), 0);
    lv_obj_align(lbl_r_val, LV_ALIGN_TOP_RIGHT, -12, 310);
    
    /* Green slider */
    lv_obj_t *lbl_g = lv_label_create(dlg);
    lv_label_set_text(lbl_g, "G:");
    lv_obj_set_style_text_color(lbl_g, lv_color_hex(0x4CAF50), 0);
    lv_obj_align(lbl_g, LV_ALIGN_TOP_LEFT, 0, 335);
    
    lv_obj_t *slider_g = lv_slider_create(dlg);
    lv_slider_set_range(slider_g, 0, 255);
    lv_slider_set_value(slider_g, (selected_color >> 8) & 0xFF, false);
    lv_obj_set_size(slider_g, 200, 12);
    lv_obj_align(slider_g, LV_ALIGN_TOP_LEFT, 30, 335);
    lv_obj_add_event_cb(slider_g, rgb_slider_g_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lbl_g_val = lv_label_create(dlg);
    lv_label_set_text_fmt(lbl_g_val, "%d", (selected_color >> 8) & 0xFF);
    lv_obj_set_style_text_color(lbl_g_val, lv_color_hex(0xE0E0E0), 0);
    lv_obj_align(lbl_g_val, LV_ALIGN_TOP_RIGHT, -12, 335);
    
    /* Blue slider */
    lv_obj_t *lbl_b = lv_label_create(dlg);
    lv_label_set_text(lbl_b, "B:");
    lv_obj_set_style_text_color(lbl_b, lv_color_hex(0x42A5F5), 0);
    lv_obj_align(lbl_b, LV_ALIGN_TOP_LEFT, 0, 360);
    
    lv_obj_t *slider_b = lv_slider_create(dlg);
    lv_slider_set_range(slider_b, 0, 255);
    lv_slider_set_value(slider_b, selected_color & 0xFF, false);
    lv_obj_set_size(slider_b, 200, 12);
    lv_obj_align(slider_b, LV_ALIGN_TOP_LEFT, 30, 360);
    lv_obj_add_event_cb(slider_b, rgb_slider_b_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lbl_b_val = lv_label_create(dlg);
    lv_label_set_text_fmt(lbl_b_val, "%d", selected_color & 0xFF);
    lv_obj_set_style_text_color(lbl_b_val, lv_color_hex(0xE0E0E0), 0);
    lv_obj_align(lbl_b_val, LV_ALIGN_TOP_RIGHT, -12, 360);
    
    /* ─────────────────────────────────────────────────────
     * HEX INPUT
     * ───────────────────────────────────────────────────── */
    
    lv_obj_t *lbl_hex = lv_label_create(dlg);
    lv_label_set_text(lbl_hex, "Hex:");
    lv_obj_set_style_text_color(lbl_hex, lv_color_hex(0xB0B0B0), 0);
    lv_obj_align(lbl_hex, LV_ALIGN_TOP_LEFT, 0, 390);
    
    lv_obj_t *hex_input = lv_textarea_create(dlg);
    lv_obj_set_size(hex_input, 120, 32);
    lv_textarea_set_text(hex_input, "");
    lv_obj_align(hex_input, LV_ALIGN_TOP_LEFT, 50, 385);
    lv_obj_add_event_cb(hex_input, hex_input_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    /* ─────────────────────────────────────────────────────
     * BUTTONS
     * ───────────────────────────────────────────────────── */
    
    lv_obj_t *btn_confirm = lv_btn_create(dlg);
    lv_obj_set_size(btn_confirm, 100, 36);
    lv_obj_align(btn_confirm, LV_ALIGN_BOTTOM_LEFT, 12, -12);
    lv_obj_set_style_bg_color(btn_confirm, lv_color_hex(0x3F51B5), 0);
    
    lv_obj_t *lbl_confirm = lv_label_create(btn_confirm);
    lv_label_set_text(lbl_confirm, "Apply");
    lv_obj_center(lbl_confirm);
    lv_obj_add_event_cb(btn_confirm, confirm_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_cancel = lv_btn_create(dlg);
    lv_obj_set_size(btn_cancel, 100, 36);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_RIGHT, -12, -12);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x424242), 0);
    
    lv_obj_t *lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, "Cancel");
    lv_obj_center(lbl_cancel);
    lv_obj_add_event_cb(btn_cancel, cancel_cb, LV_EVENT_CLICKED, NULL);
}
