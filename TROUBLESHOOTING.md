# CNC Drawing Tool - Troubleshooting Guide

## 🔧 Build Issues

### Error: "No such file or directory: src/cnc/cnc_draw.c"

**Cause:** Files created in wrong location or CMakeLists.txt path is wrong

**Solution:**
```bash
# Verify files exist
ls -la /home/user/Desktop/Lvgl_Project/lv_port_linux/src/cnc/
# Should show: cnc_draw.c

ls -la /home/user/Desktop/Lvgl_Project/lv_port_linux/src/include/
# Should show: cnc_draw.h, cnc_ui.h

# If missing, re-create them from the provided code
```

### Error: "undefined reference to 'cnc_init'" or other cnc_* functions

**Cause:** Source files not included in CMakeLists.txt

**Solution:**
1. Open CMakeLists.txt
2. Find the section with source files
3. Add these lines:
```cmake
set(CNC_SOURCES
    src/cnc/cnc_draw.c
    src/ui/cnc_ui.c
)

# Then in add_executable or target_sources:
target_sources(lvglsim PRIVATE ${CNC_SOURCES})
```
4. Rebuild:
```bash
cd build && rm -rf * && cmake .. && make -j4
```

### Error: "fatal error: cnc_draw.h: No such file"

**Cause:** Include path not set or include statement is wrong

**Solution:**
```cmake
# In CMakeLists.txt, ensure this line exists:
target_include_directories(lvglsim PRIVATE src/include)

# Or check the include statement in cnc_ui.c matches:
#include "cnc_draw.h"
```

### Error: "implicit function declaration for 'cnc_create_ui'"

**Cause:** Missing or incorrect header in main file

**Solution:**
```c
// Add this to your main.c or wherever you call cnc_create_ui:
#include "cnc_ui.h"

// Make sure to call from inside your build_ui() function:
void build_ui(void) {
    lv_obj_t *scr = lv_screen_active();
    cnc_create_ui(scr);  // ← Add this line
}
```

### Compilation takes too long or uses lots of memory

**Cause:** Compiling with optimization flags on slow systems

**Solution:**
```bash
# Use lighter compilation flags
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j2  # Use fewer parallel jobs
```

## 🖥️ Runtime Issues

### Canvas appears completely blank (black screen)

**Cause:** UI not initialized or display not working

**Solutions:**

1. **Check if cnc_create_ui() was called:**
   ```c
   void build_ui(void) {
       lv_obj_t *scr = lv_screen_active();
       if (!scr) {
           printf("ERROR: No active screen!\n");
           return;
       }
       cnc_create_ui(scr);  // Must be called
       printf("CNC UI created\n");
   }
   ```

2. **Check display initialization:**
   ```bash
   # Verify fbdev or display driver is initialized
   # Check console for "Screen initialized" message
   ```

3. **Check LVGL is pumping:**
   ```c
   // In main loop, must have:
   while (1) {
       uint32_t delay = lv_timer_handler();
       usleep(delay * 1000);
   }
   ```

### Drawing tools don't respond to clicks

**Cause:** Event handlers not registered or input device not initialized

**Solutions:**

1. **Verify mouse/touch input initialized:**
   ```bash
   # Check console for:
   # "Input device registered"
   # "Touch initialized"
   ```

2. **Check event registration in cnc_ui.c:**
   ```c
   // Verify this is present in create_canvas():
   lv_obj_add_event_cb(canvas, canvas_event, LV_EVENT_ALL, NULL);
   ```

3. **Test with simple button first:**
   ```c
   lv_obj_t *btn = lv_button_create(scr);
   lv_obj_add_event_cb(btn, some_event, LV_EVENT_CLICKED, NULL);
   // If this button doesn't work, input device issue
   ```

### Tools appear in toolbar but drawing doesn't work

**Cause:** Canvas event handler not processing drawing

**Solutions:**

1. **Check canvas_event() function in cnc_ui.c:**
   - Verify `LV_EVENT_PRESSED` and `LV_EVENT_RELEASED` cases
   - Check coordinate reading: `lv_indev_get_point(indev, &p);`

2. **Add debug output:**
   ```c
   static void canvas_event(lv_event_t *e) {
       lv_event_code_t code = lv_event_get_code(e);
       printf("Canvas event: %d\n", code);  // Add this
       // ... rest of code
   }
   ```

3. **Verify canvas is in focus:**
   - Click on canvas area first
   - Try clicking center of canvas
   - Try larger drag motion

### Right panel shows wrong information

**Cause:** Update functions not called or labels not refreshed

**Solutions:**

1. **Force panel update:**
   ```c
   // In canvas_event() after drawing:
   cnc_update_status();
   cnc_update_job_info();
   lv_obj_invalidate(g_cnc.right_panel_obj);
   ```

2. **Check label creation:**
   ```c
   // In create_right_panel(), verify all labels created:
   g_cnc.lbl_tool != NULL
   g_cnc.lbl_job_name != NULL
   g_cnc.lbl_dimensions != NULL
   // etc.
   ```

### Zoom buttons don't change view

**Cause:** Canvas not invalidated or zoom not applied

**Solutions:**

1. **Ensure invalidation after zoom:**
   ```c
   static void btn_zoom_in_event(lv_event_t *e) {
       if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
           cnc_zoom_in();
           cnc_update_status();
           lv_obj_invalidate(g_cnc.canvas_obj);  // Must be called
       }
   }
   ```

2. **Verify zoom is being calculated:**
   ```c
   // In cnc_zoom_in():
   printf("New zoom: %.2f\n", g_cnc.zoom);
   ```

### Status bar shows wrong coordinates

**Cause:** Coordinates not being tracked or updated

**Solutions:**

1. **Add mouse movement tracking:**
   ```c
   // Can add LV_EVENT_PRESSING to canvas_event:
   else if (code == LV_EVENT_PRESSING) {
       lv_point_t p;
       lv_indev_get_point(indev, &p);
       char buf[32];
       lv_snprintf(buf, sizeof(buf), "X: %d Y: %d", p.x, p.y);
       lv_label_set_text(g_cnc.lbl_coords, buf);
   }
   ```

## 📁 File Issues

### Export button creates file but it's empty

**Cause:** File created but data not written, or wrong path

**Solutions:**

1. **Check file location:**
   ```bash
   # File should be at:
   ls -la /tmp/cnc_job.txt
   
   # Check if file exists and has size:
   ls -lh /tmp/cnc_job.txt
   ```

2. **Check permissions:**
   ```bash
   # /tmp should be writable
   touch /tmp/test.txt
   rm /tmp/test.txt
   ```

3. **Add debug output to export function:**
   ```c
   void cnc_export_job(const char *filename) {
       FILE *f = fopen(filename, "w");
       if (!f) {
           printf("ERROR: Cannot open %s\n", filename);
           return;
       }
       printf("Writing to %s\n", filename);
       fprintf(f, "CNC Job File\n");
       // ... rest of writes ...
       fclose(f);
       printf("File closed successfully\n");
   }
   ```

### Cannot read exported file

**Cause:** File encoding or path issue

**Solutions:**

1. **Read the file:**
   ```bash
   cat /tmp/cnc_job.txt
   # Should show CNC job contents
   ```

2. **Check file format:**
   ```bash
   file /tmp/cnc_job.txt
   # Should show: ASCII text
   ```

3. **Use correct path in code:**
   ```c
   // Use absolute path:
   cnc_export_job("/tmp/cnc_job.txt");
   // Not:
   cnc_export_job("cnc_job.txt");  // Wrong - relative path
   ```

## 🎨 UI/Display Issues

### Colors look wrong or too dark

**Cause:** Color definitions or display color depth issue

**Solutions:**

1. **Check color depth:**
   ```bash
   # Check lv_conf.h for:
   #define LV_COLOR_DEPTH 16  // Should be 16, 32, etc.
   ```

2. **Verify color values:**
   ```c
   // Check in cnc_ui.c:
   #define C_BG        lv_color_hex(0x0D0D1A)
   // Color should be recognizable
   ```

3. **Test with simple color:**
   ```c
   lv_obj_t *test = lv_obj_create(parent);
   lv_obj_set_style_bg_color(test, lv_color_hex(0xFF0000), 0);  // Red
   // If red shows wrong, display driver issue
   ```

### Text is unreadable (too small or invisible)

**Cause:** Font not loaded or text color matches background

**Solutions:**

1. **Check font configuration:**
   ```bash
   # Check lv_conf.h:
   #define LV_FONT_MONTSERRAT_14 1
   #define LV_FONT_MONTSERRAT_16 1
   ```

2. **Verify text color:**
   ```c
   // Ensure text color contrasts with background:
   lv_obj_set_style_text_color(label, C_WHITE, 0);
   // where C_WHITE = #E2E8F0 (bright)
   ```

3. **Test with larger font:**
   ```c
   lv_obj_t *test_lbl = lv_label_create(parent);
   lv_label_set_text(test_lbl, "TEST");
   lv_obj_set_style_text_font(test_lbl, &lv_font_montserrat_22, 0);
   ```

### Toolbar buttons overlap or are misaligned

**Cause:** Widget sizing or flex layout issue

**Solutions:**

1. **Check toolbar size:**
   ```c
   // In create_toolbar():
   lv_obj_set_size(toolbar, LV_PCT(100), 50);  // 50px height
   ```

2. **Verify button sizes:**
   ```c
   lv_obj_t *btn = lv_button_create(toolbar);
   lv_obj_set_width(btn, 80);   // 80px width
   lv_obj_set_height(btn, 40);  // 40px height
   ```

3. **Check flex layout:**
   ```c
   lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
   lv_obj_set_flex_align(toolbar, LV_FLEX_ALIGN_START, 
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
   ```

## 🧮 Calculation Issues

### Dimensions show 0.00 mm or wrong values

**Cause:** Unit conversion or calculation error

**Solutions:**

1. **Check mm per pixel:**
   ```c
   // In cnc_draw.h, verify:
   #define MM_PER_PX_X (RULER_X_MM / VIRTUAL_W)
   #define MM_PER_PX_Y (RULER_Y_MM / VIRTUAL_H)
   
   // Should not be 0
   printf("MM/px X: %.6f\n", MM_PER_PX_X);
   printf("MM/px Y: %.6f\n", MM_PER_PX_Y);
   ```

2. **Check dimension calculation:**
   ```c
   // In cnc_add_dimension():
   float dx = (float)(x2 - x1) * 0.1f;  // Pixel to mm
   float dy = (float)(y2 - y1) * 0.1f;
   dim->length_mm = sqrtf(dx*dx + dy*dy);
   
   printf("Calculated dimension: %.2f mm\n", dim->length_mm);
   ```

3. **Verify math library:**
   ```cmake
   # In CMakeLists.txt, ensure -lm is linked:
   target_link_libraries(lvglsim PRIVATE m)
   ```

## 🔍 Debugging Steps

### General Troubleshooting

1. **Enable verbose output:**
   ```c
   // Add printf() statements throughout code
   // Especially in event handlers and drawing functions
   ```

2. **Check global state:**
   ```c
   // Print CNC state periodically:
   printf("Tool: %d, Line count: %d, Zoom: %.2f\n",
          g_cnc.current_tool, g_cnc.line_count, g_cnc.zoom);
   ```

3. **Verify object creation:**
   ```c
   // Check if UI objects created properly:
   printf("Canvas: %p\n", g_cnc.canvas_obj);
   printf("Toolbar: %p\n", g_cnc.toolbar_obj);
   printf("Panel: %p\n", g_cnc.right_panel_obj);
   ```

4. **Test components independently:**
   ```c
   // Create simple test version with just canvas
   // Then gradually add features
   ```

## 📊 Performance Issues

### Application runs slow or is unresponsive

**Causes & Solutions:**

1. **Too many elements drawn:**
   ```c
   // Clear old drawings:
   cnc_clear_all();
   // Reduces: g_cnc.line_count, circle_count, etc.
   ```

2. **Zoom level too extreme:**
   ```c
   // Reset zoom:
   cnc_fit_to_view();
   // Or manually:
   g_cnc.zoom = 1.0f;
   ```

3. **Missing event filtering:**
   ```c
   // Only redraw on actual changes, not every frame
   // Use lv_obj_invalidate() sparingly
   ```

4. **Check main loop timing:**
   ```c
   while (1) {
       uint32_t delay = lv_timer_handler();
       if (delay > 50) delay = 50;  // Cap delay
       usleep(delay * 1000);
   }
   ```

## 🆘 Getting Help

If you're still stuck:

1. **Check console output** - Look for error messages
2. **Read source comments** - Code has explanations
3. **Try the example** - `example/main_cnc.c` shows correct usage
4. **Verify file structure** - All files in correct locations
5. **Review documentation** - Read CNC_TOOL_README.md

## 📋 Diagnostic Checklist

- [ ] All files created in correct locations?
- [ ] CMakeLists.txt updated with new sources?
- [ ] Build completes without errors?
- [ ] Application starts without crashes?
- [ ] CNC interface appears on screen?
- [ ] Toolbar buttons are clickable?
- [ ] Canvas responds to drawing?
- [ ] Right panel shows information?
- [ ] Status bar updates correctly?
- [ ] Export creates file with content?

If all check out, the tool is working! 🎉

---

**Remember:** Start simple, add features gradually, and test each part before moving on.
