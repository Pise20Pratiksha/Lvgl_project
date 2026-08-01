/*==============================================================
 * src/ui/dxf_review.c
 * Read-only DXF import & review overlay.
 *
 * Additive module — does not alter existing save/export/draw code:
 *  - Lists .dxf files already written by cnc_sketch_save_dxf()
 *    (SKETCH_DIR) and lets the user pick one to preview.
 *  - Reviewing backs up whatever is currently on the canvas, loads
 *    the chosen DXF via the existing cnc_sketch_load_dxf() reader,
 *    and forces read-only (pan/zoom-only) interaction until the
 *    user exits, at which point the original drawing is restored
 *    exactly.
 *==============================================================*/

#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "viewer.h"
#include "viewer_ui.h"
#include "viewer_math.h"
#include "cnc_dxf.h"
#include "cnc_sketch.h"      /* SKETCH_DIR, MAX_FILENAME_LEN */
#include "viewer_dxf_review.h"

#define MAX_DXF_LIST 48

static lv_obj_t *g_import_dialog = NULL;
static char g_dxf_names[MAX_DXF_LIST][MAX_FILENAME_LEN];

static void close_import_dialog(void)
{
    if (g_import_dialog) {
        lv_obj_del(g_import_dialog);
        g_import_dialog = NULL;
    }
}

bool cnc_dxf_review_active(void)
{
    return g.dxf_review_mode;
}

static void exit_review_btn_event(lv_event_t *e)
{
    (void)e;
    cnc_dxf_exit_review();
}

static void update_review_banner(const char *filename)
{
    if (g.dxf_review_banner) {
        lv_obj_del(g.dxf_review_banner);
        g.dxf_review_banner = NULL;
    }

    lv_obj_t *banner = lv_obj_create(lv_scr_act());
    g.dxf_review_banner = banner;
    lv_obj_set_size(banner, 300, 40);
    lv_obj_align(banner, LV_ALIGN_TOP_MID, 0, TOOLBAR_H + 4);
    lv_obj_set_style_bg_color(banner, lv_color_hex(0x3A2A00), 0);
    lv_obj_set_style_bg_opa(banner, LV_OPA_90, 0);
    lv_obj_set_style_border_color(banner, H(C_YELLOW), 0);
    lv_obj_set_style_border_width(banner, 1, 0);
    lv_obj_set_style_radius(banner, 6, 0);
    lv_obj_set_style_pad_all(banner, 6, 0);
    lv_obj_set_flex_flow(banner, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(banner, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(banner, LV_OBJ_FLAG_SCROLLABLE);

    char buf[96];
    lv_snprintf(buf, sizeof(buf), LV_SYMBOL_EYE_OPEN " Reviewing: %s (read-only)", filename);
    lv_obj_t *lbl = lv_label_create(banner);
    lv_label_set_text(lbl, buf);
    lv_obj_set_style_text_color(lbl, H(C_YELLOW), 0);
    lv_obj_set_style_text_font(lbl, FONT_SM, 0);

    lv_obj_t *btn_exit = lv_button_create(banner);
    lv_obj_set_size(btn_exit, 60, 26);
    lv_obj_add_event_cb(btn_exit, exit_review_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_t *exit_lbl = lv_label_create(btn_exit);
    lv_label_set_text(exit_lbl, "Exit");
    lv_obj_center(exit_lbl);
}

static void enter_review_with_file(const char *filename)
{
    /* Back up the current canvas exactly once — if the user picks a
     * second file mid-review, keep the ORIGINAL pre-review drawing. */
    if (!g.dxf_review_mode) {
        g.dxf_backup_count = g.draw_count;
        for (int i = 0; i < g.draw_count; i++) {
            g.dxf_backup_objects[i] = g.draw_objects[i];
        }
        g.dxf_backup_mode = g.mode;
    }

    if (!cnc_sketch_load_dxf(filename)) {
        printf("[dxf_review] Failed to load %s\n", filename);
        return;
    }

    g.dxf_review_mode = true;
    strncpy(g.dxf_review_name, filename, sizeof(g.dxf_review_name) - 1);
    g.dxf_review_name[sizeof(g.dxf_review_name) - 1] = '\0';

    g.selected_obj = -1;
    set_mode(MODE_PAN);   /* read-only: pan/zoom only, guarded in set_mode() */

    update_review_banner(filename);
    refresh_all();
    update_panel();
}

void cnc_dxf_exit_review(void)
{
    if (!g.dxf_review_mode) return;

    /* Restore the exact drawing that was present before the preview */
    g.draw_count = g.dxf_backup_count;
    for (int i = 0; i < g.dxf_backup_count; i++) {
        g.draw_objects[i] = g.dxf_backup_objects[i];
    }
    g.selected_obj    = -1;
    g.dxf_review_mode = false;
    g.dxf_review_name[0] = '\0';

    if (g.dxf_review_banner) {
        lv_obj_del(g.dxf_review_banner);
        g.dxf_review_banner = NULL;
    }

    set_mode(g.dxf_backup_mode);
    refresh_all();
    update_panel();
}

static void import_item_event(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    intptr_t idx = (intptr_t)lv_obj_get_user_data(btn);
    close_import_dialog();
    if (idx >= 0 && idx < MAX_DXF_LIST) {
        enter_review_with_file(g_dxf_names[idx]);
    }
}

static void import_cancel_event(lv_event_t *e)
{
    (void)e;
    close_import_dialog();
}

void show_dxf_import_dialog(void)
{
    if (g_import_dialog) {
        close_import_dialog();
        return;
    }

    int count = 0;
    DIR *d = opendir(SKETCH_DIR);
    if (d) {
        struct dirent *ent;
        while ((ent = readdir(d)) != NULL && count < MAX_DXF_LIST) {
            size_t len = strlen(ent->d_name);
            if (len > 4 && strcmp(ent->d_name + len - 4, ".dxf") == 0) {
                strncpy(g_dxf_names[count], ent->d_name, sizeof(g_dxf_names[0]) - 1);
                g_dxf_names[count][sizeof(g_dxf_names[0]) - 1] = '\0';
                count++;
            }
        }
        closedir(d);
    }

    lv_obj_t *overlay = lv_obj_create(lv_scr_act());
    g_import_dialog = overlay;
    lv_obj_set_size(overlay, SCR_W, SCR_H);
    lv_obj_set_pos(overlay, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_50, 0);
    lv_obj_set_style_border_width(overlay, 0, 0);
    lv_obj_set_style_radius(overlay, 0, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(overlay, import_cancel_event, LV_EVENT_CLICKED, NULL);

    int32_t card_h = LV_MIN(360, SCR_H - 40);
    lv_obj_t *card = lv_obj_create(overlay);
    lv_obj_set_size(card, 300, card_h);
    lv_obj_align(card, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_bg_color(card, H(C_PANEL), 0);
    lv_obj_set_style_border_color(card, H(C_BORDER), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_pad_all(card, 12, 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(card, 6, 0);
    lv_obj_add_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(card, LV_DIR_VER);
    /* Absorb clicks inside the card so they don't bubble to the overlay's cancel handler */
    lv_obj_add_event_cb(card, import_cancel_event, LV_EVENT_CLICKED, NULL);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Import DXF for Review");
    lv_obj_set_style_text_color(title, H(C_PNL_TITLE), 0);
    lv_obj_set_style_text_font(title, FONT_MD, 0);

    if (count == 0) {
        lv_obj_t *empty = lv_label_create(card);
        lv_label_set_text(empty, "No .dxf files found.\nSave a drawing first.");
        lv_obj_set_style_text_color(empty, H(C_PNL_KEY), 0);
        lv_obj_set_style_text_font(empty, FONT_SM, 0);
        lv_obj_set_style_text_align(empty, LV_TEXT_ALIGN_CENTER, 0);
    } else {
        for (int i = 0; i < count; i++) {
            lv_obj_t *btn = lv_button_create(card);
            lv_obj_set_size(btn, 268, 32);
            lv_obj_set_user_data(btn, (void *)(intptr_t)i);
            lv_obj_add_event_cb(btn, import_item_event, LV_EVENT_CLICKED, NULL);
            lv_obj_set_style_bg_color(btn, H(C_BTN_NORM), 0);
            lv_obj_set_style_border_color(btn, H(C_BTN_BRD), 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_radius(btn, 4, 0);

            lv_obj_t *lbl = lv_label_create(btn);
            lv_label_set_text(lbl, g_dxf_names[i]);
            lv_obj_set_style_text_color(lbl, H(C_WHITE), 0);
            lv_obj_set_style_text_font(lbl, FONT_SM, 0);
            lv_obj_center(lbl);
        }
    }

    lv_obj_t *btn_cancel = lv_button_create(card);
    lv_obj_set_size(btn_cancel, 268, 34);
    lv_obj_add_event_cb(btn_cancel, import_cancel_event, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(btn_cancel, H(C_BTN_NORM), 0);
    lv_obj_set_style_border_color(btn_cancel, H(C_BTN_BRD), 0);
    lv_obj_set_style_border_width(btn_cancel, 1, 0);
    lv_obj_set_style_radius(btn_cancel, 4, 0);
    lv_obj_t *cancel_lbl = lv_label_create(btn_cancel);
    lv_label_set_text(cancel_lbl, "Cancel");
    lv_obj_set_style_text_color(cancel_lbl, H(C_WHITE), 0);
    lv_obj_center(cancel_lbl);
}