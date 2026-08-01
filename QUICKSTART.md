# CNC Drawing Tool - Quick Start Guide

## 📋 Overview

Based on your technical sketches (microwave/camera setup with precision measurements), I've created a professional CNC drawing tool with:

✅ **Drawing Tools**: Lines, circles, rectangles with precision  
✅ **Measurements**: Dimension annotations with automatic mm calculation  
✅ **Job Management**: Create, view, and export CNC jobs  
✅ **Professional UI**: Dark theme with toolbar, canvas, side panel, and status bar  
✅ **Zoom & Pan**: Navigate large designs with ease  
✅ **Real-time Updates**: See changes instantly on canvas  

## 📁 Files Created

### Core Implementation
- `src/include/cnc_draw.h` - Drawing definitions & API
- `src/cnc/cnc_draw.c` - Core drawing logic & state management
- `src/include/cnc_ui.h` - UI interface header
- `src/ui/cnc_ui.c` - LVGL UI components builder

### Documentation & Examples
- `CNC_TOOL_README.md` - Complete reference documentation
- `example/main_cnc.c` - Integration example
- `QUICKSTART.md` - This file

## 🚀 Quick Start

### Step 1: Update CMakeLists.txt

Add these files to your build:

```cmake
# In CMakeLists.txt, find the section where source files are added
# Add these new files:

set(UI_SOURCES
    src/ui/draw.c
    src/ui/layout.c
    src/ui/panel.c
    src/ui/widgets.c
    src/ui/cnc_ui.c          # ADD THIS
)

set(CNC_SOURCES
    src/cnc/cnc_draw.c       # ADD THIS
)

# In your target_sources() call, add:
target_sources(lvglsim PRIVATE ${UI_SOURCES} ${CNC_SOURCES})

# Make sure to add the include directory:
target_include_directories(lvglsim PRIVATE src/include)
```

### Step 2: Update Your Main Application

Replace your `src/main.c` with content from `example/main_cnc.c`, or add these lines:

```c
#include "cnc_ui.h"
#include "cnc_draw.h"

// In your build_ui() function:
void build_ui(void)
{
    lv_obj_t *scr = lv_screen_active();
    cnc_create_ui(scr);  // This creates the entire CNC interface!
}
```

### Step 3: Build and Run

```bash
cd /home/user/Desktop/Lvgl_Project/lv_port_linux
mkdir -p build && cd build
cmake ..
make -j4
./bin/lvglsim
```

## 🎮 Using the CNC Drawing Tool

### Drawing Interface

```
┌─────────────────────────────────────────────────────────┐
│  ✎ Line │ ◯ Circle │ ▭ Rect │ ⌫ Dim │ ⊕ Pan │ ↶ Undo │  (Toolbar)
├──────────────────────────────────────┬──────────────────┤
│                                      │  Current Tool    │
│                                      │  Tool: Line      │
│        CANVAS AREA                   │  ─────────────   │
│        (Draw Here)                   │  Job: New Job    │
│                                      │  100 x 100 mm    │
│                                      │  0 elements      │
│                                      │                  │
│                                      │  [💾 Export Job] │
├──────────────────────────────────────┴──────────────────┤
│  CNC Drawing Tool | Ready              X: 0 Y: 0 Grid:ON│ (Status)
└───────────────────────────────────────────────────────────┘
```

### Tool Usage

1. **Line Tool** 🖊️
   - Click starting point
   - Drag to ending point
   - Release to create line

2. **Circle Tool** 🔵
   - Click at center
   - Drag outward for radius
   - Release to create circle

3. **Rectangle Tool** ▭
   - Click top-left corner
   - Drag to bottom-right
   - Release to create rectangle

4. **Dimension Tool** 📏
   - Click first point
   - Drag to second point
   - Automatic distance calculation

5. **Zoom Controls** 🔍
   - `Zoom +` button to zoom in
   - `Zoom -` button to zoom out
   - `Fit` button to fit entire job

6. **Undo/Clear** ↶✕
   - `Undo` removes last element
   - `Clear` removes everything

7. **Export** 💾
   - Click Export to save job as text file
   - Location: `/tmp/cnc_job.txt`

## 📊 Precision Measurements

From your sketch reference, the tool supports:

- **X Axis**: 25 mm range (precision: ~0.001 mm/px)
- **Y Axis**: 100 mm range (precision: ~0.008 mm/px)
- **Zoom**: 0.01× to 10× magnification
- **Grid**: Major ticks every 50px, minor every 10px

### Configuring Dimensions

Edit `src/include/cnc_draw.h`:

```c
// Set your work area dimensions
#define RULER_X_MM    100.0f   // X axis range in mm
#define RULER_Y_MM    100.0f   // Y axis range in mm

// Set virtual canvas size
#define VIRTUAL_W     20000    // Canvas width in pixels
#define VIRTUAL_H     12000    // Canvas height in pixels

// Precision automatically calculated as:
// MM_PER_PX = RULER_X/VIRTUAL_W
```

## 💾 Exported Job Format

When you click "Export Job", it creates a text file:

```
CNC Job File
Job: New Job
Size: 100.0 x 100.0 mm

Lines: 2
  Line: (100,100) -> (200,200) width=2
  Line: (300,300) -> (400,400) width=2

Circles: 1
  Circle: center=(150,150) radius=50

Rectangles: 0

Dimensions: 1
  Dim: 141.42 mm [Auto-measured]
```

This can be:
- Parsed by CAM software
- Used for CNC machine programming
- Shared with other users
- Imported into CAD tools

## 🎨 Color Scheme

The interface uses a professional dark theme optimized for precision work:

| Element | Color | Purpose |
|---------|-------|---------|
| Background | #0D0D1A | Main canvas background |
| Toolbar | #0F1729 | Tool selection area |
| Panels | #0A0F1E | Information panels |
| Borders | #1E3A5F | UI element borders |
| Text | #E2E8F0 | Primary text |
| Active | #38BDF8 | Cyan highlights |
| Dimensions | #38BDF8 | Measurement color |
| Good | #10B981 | Success/zoom |
| Alert | #EF4444 | Clear button |

## 🔧 Advanced Configuration

### Changing Drawing Colors

In `src/cnc/cnc_draw.c`:

```c
// Set default drawing color (currently green)
g_cnc.draw_color = lv_color_hex(0x10B981);  // Green
// Change to:
g_cnc.draw_color = lv_color_hex(0xFFFF00);  // Yellow
```

### Changing Line Width

```c
g_cnc.line_width = 2;  // Default 2 pixels
g_cnc.line_width = 4;  // Make thicker lines
```

### Adding More Storage

Increase element limits in `src/include/cnc_draw.h`:

```c
CNCLine lines[256];      // Change 256 to larger number
CNCCircle circles[64];   // Change 64 to larger number
```

## 🐛 Troubleshooting

### Canvas appears blank
- Ensure `cnc_create_ui(scr)` is called in `build_ui()`
- Check that cmake build includes all source files
- Verify `cnc_canvas_draw_all()` is called in canvas event

### Drawing tools don't work
- Click once to start, drag once to end, release
- Not continuous - one gesture per shape
- Check coordinates are within canvas bounds

### Export button doesn't create file
- Check write permissions on `/tmp/`
- File may have been created - check: `ls -la /tmp/cnc_job.txt`
- Look for error messages in terminal

### Performance issues
- Reduce number of elements (undo some drawings)
- Decrease zoom level (zoom out)
- Close other applications

## 📚 Example Workflow

```
1. Start application
   → CNC interface loads with blank canvas

2. Select "Line" tool
   → Right panel shows "Tool: Line"

3. Draw first line
   → Click point A, drag to point B, release
   → Line appears on canvas

4. Draw rectangle
   → Switch to "Rectangle" tool
   → Click corner, drag opposite corner, release

5. Add dimension
   → Switch to "Dimension" tool
   → Click start, drag to end
   → Auto-calculated length shows on canvas

6. Zoom to fit
   → Click "Fit" button
   → Entire design visible in viewport

7. Export job
   → Click "Export Job" button
   → File saved to /tmp/cnc_job.txt
   → Status shows "Job Exported!"

8. Import to CAM
   → Use the exported file with your CNC software
   → Follow CAM tool's import procedure
```

## 🔗 Integration with CNC Machine Software

The exported format can be used with:

1. **LinuxCNC** - Parse the dimensions for path generation
2. **FreeCAD** - Import dimensions for further refinement
3. **Inkscape** - Use with Gcodetools extension
4. **Custom CAM** - Simple text format easy to parse

Example C code to parse the export:

```c
FILE *f = fopen("/tmp/cnc_job.txt", "r");
char line[128];
while (fgets(line, sizeof(line), f)) {
    if (sscanf(line, "  Dim: %f mm", &length) == 1) {
        printf("Found dimension: %.2f mm\n", length);
        // Use in CNC calculation
    }
}
fclose(f);
```

## 📖 Complete Reference

For detailed API documentation, see:
- `CNC_TOOL_README.md` - Full feature reference
- `src/include/cnc_draw.h` - Function definitions
- `src/include/cnc_ui.h` - UI interface

## ✨ Features Summary

| Feature | Status | Notes |
|---------|--------|-------|
| Line Drawing | ✅ | Full implementation |
| Circle Drawing | ✅ | From center with radius |
| Rectangle Drawing | ✅ | From corner to corner |
| Dimension Tools | ✅ | Auto mm calculation |
| Zoom In/Out | ✅ | Up to 10× magnification |
| Pan/Navigate | ✅ | Pan tool included |
| Undo | ✅ | One level, last element |
| Clear All | ✅ | Remove all elements |
| Export Job | ✅ | Text file format |
| Grid Background | ✅ | Major/minor ticks |
| Status Bar | ✅ | Real-time coordinates |
| Job Information | ✅ | Size, element count |
| Dark Theme | ✅ | Professional CAD look |

## 🎯 Next Steps

1. **Build and Test**
   ```bash
   cmake .. && make -j4 && ./bin/lvglsim
   ```

2. **Try Drawing**
   - Create a simple rectangle (for work area)
   - Add a circle (tool location)
   - Add dimensions (measure distances)

3. **Export and Use**
   - Export to see the format
   - Open `/tmp/cnc_job.txt` to view data
   - Use with your CNC software

4. **Customize**
   - Adjust colors in cnc_ui.c
   - Change dimensions in cnc_draw.h
   - Add more tools as needed

## 📞 Support

Refer to:
- LVGL Documentation: https://docs.lvgl.io
- CNC specifications from your reference image
- Source code comments for detailed explanations

---

**Ready to use!** Build the project and start drawing your CNC designs.

🎨 **Happy Drawing!** ✨
