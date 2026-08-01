# CNC Drawing Tool - LVGL Implementation

## Overview

This is a professional CNC drawing interface built with LVGL for precise technical drawing and job management. The tool allows users to:

- **Draw shapes**: Lines, circles, rectangles
- **Add dimensions**: Measure and annotate drawings
- **Zoom & Pan**: Navigate large designs with precision
- **Job management**: Create, save, and export CNC jobs
- **Grid reference**: Visual grid with major/minor ticks

## Project Structure

```
src/
├── include/
│   ├── cnc_draw.h          # Core CNC drawing definitions
│   └── cnc_ui.h            # UI builder interface
├── cnc/
│   └── cnc_draw.c          # Drawing logic & state management
└── ui/
    └── cnc_ui.c            # LVGL UI components
```

## Features

### Drawing Tools

1. **Line Tool** - Draw straight lines with precision
   - Click start point, drag to end point
   - Automatic line width control
   
2. **Circle Tool** - Draw circles from center point
   - Click to set center, drag to set radius
   - Smooth rendering

3. **Rectangle Tool** - Draw rectangular areas
   - Click and drag to define bounds
   - Perfect for CNC cutting areas

4. **Dimension Tool** - Add measurement annotations
   - Click and drag to create dimension line
   - Automatic length calculation in mm
   - Custom labels supported

5. **Pan Tool** - Navigate the canvas
   - Click and drag to pan viewport
   
6. **Zoom Tool** - Zoom controls
   - Zoom In/Out buttons
   - Fit to View button
   - Current zoom level display

### Additional Features

- **Undo** - Remove last drawn element
- **Clear** - Clear entire canvas
- **Export** - Save job to file format
- **Status Bar** - Real-time coordinate and mode display
- **Right Panel** - Job information and element count
- **Grid Background** - Visual reference grid

## Integration with Main Application

### Basic Usage

```c
#include "cnc_ui.h"
#include "cnc_draw.h"

int main() {
    // ... LVGL initialization code ...
    
    lv_obj_t *scr = lv_screen_active();
    
    // Create the CNC drawing interface
    cnc_create_ui(scr);
    
    // Main event loop
    while (1) {
        uint32_t delay = lv_timer_handler();
        if (delay > 10) delay = 10;
        usleep(delay * 1000);
    }
    
    return 0;
}
```

### Configuration

Edit these constants in `cnc_draw.h` to customize:

```c
// Display geometry
#define TOOLBAR_H    50      // Height of toolbar
#define RULER_SZ     20      // Size of rulers

// Canvas sizing (pixels)
#define VIRTUAL_W    20000   // Virtual canvas width
#define VIRTUAL_H    12000   // Virtual canvas height

// Physical dimensions (mm)
#define RULER_X_MM   100.0f  // X axis range in mm
#define RULER_Y_MM   100.0f  // Y axis range in mm

// Zoom levels
#define ZOOM_MIN     0.01f
#define ZOOM_MAX     10.0f
#define ZOOM_DEFAULT 1.0f
```

### Color Scheme

The CNC interface uses a professional dark theme:

```c
C_BG        = 0x0D0D1A  // Dark background
C_TOOLBAR   = 0x0F1729  // Dark toolbar
C_PANEL     = 0x0A0F1E  // Dark panel
C_BORDER    = 0x1E3A5F  // Blue borders
C_CYAN      = 0x38BDF8  // Cyan accents
C_GREEN     = 0x10B981  // Green accents
C_RED       = 0xEF4444  // Red alerts
C_WHITE     = 0xE2E8F0  // White text
```

## API Reference

### State Management

```c
// Get/Set current tool
void cnc_set_tool(CNCTool tool);

// Clear operations
void cnc_clear_all(void);
void cnc_undo(void);
```

### Drawing Operations

```c
// Draw primitives
void cnc_draw_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void cnc_draw_circle(int32_t cx, int32_t cy, int32_t radius);
void cnc_draw_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void cnc_add_dimension(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const char *label);
```

### View Control

```c
// Zoom operations
void cnc_zoom_in(void);
void cnc_zoom_out(void);
void cnc_fit_to_view(void);
```

### Job Management

```c
// Export to file
void cnc_export_job(const char *filename);

// Update displays
void cnc_update_status(void);
void cnc_update_job_info(void);
```

### Rendering

```c
// Canvas drawing (automatic)
void cnc_canvas_draw_all(lv_obj_t *canvas);
void cnc_render_lines(lv_obj_t *canvas);
void cnc_render_circles(lv_obj_t *canvas);
void cnc_render_rects(lv_obj_t *canvas);
void cnc_render_dimensions(lv_obj_t *canvas);
```

## Data Structures

### CNCState
Main state structure containing:
- Current tool selection
- Drawing parameters (color, width)
- Zoom and pan settings
- Arrays of drawn objects
- UI widget references
- Current job information

### CNCLine
Line drawing data:
- Start and end points (x1, y1, x2, y2)
- Color and width
- Timestamp

### CNCCircle
Circle drawing data:
- Center point (cx, cy)
- Radius
- Color and width

### CNCRect
Rectangle drawing data:
- Bounding box points
- Color and width

### CNCDimension
Measurement annotation:
- Start and end points
- Calculated length in mm
- Custom label

### CNCJob
Job information:
- Name
- Dimensions in mm
- Rotation angle
- Creation timestamp

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `L` | Line tool |
| `C` | Circle tool |
| `R` | Rectangle tool |
| `D` | Dimension tool |
| `Z` | Pan/Zoom tool |
| `Ctrl+Z` | Undo |
| `Del` | Clear all |
| `+` | Zoom in |
| `-` | Zoom out |
| `F` | Fit to view |

(Keyboard support can be added to input event handlers)

## Export Format

Jobs are exported as plain text files with the following format:

```
CNC Job File
Job: New Job
Size: 100.0 x 100.0 mm

Lines: 2
  Line: (100,100) -> (200,200) width=2
  Line: (300,300) -> (400,400) width=2

Circles: 1
  Circle: center=(150,150) radius=50

Rectangles: 1
  Rect: (100,100) -> (300,300)

Dimensions: 2
  Dim: 100.00 mm [Measurement 1]
  Dim: 50.00 mm [Measurement 2]
```

## Performance Considerations

- **Maximum elements**: 256 lines, 64 circles, 64 rectangles, 128 dimensions
- **Rendering**: Real-time canvas updates with grid
- **Memory**: ~50KB for typical job with 100 elements
- **Refresh rate**: Updates on drawing completion

## Extending the Tool

### Adding New Shapes

1. Add shape structure in `cnc_draw.h`
2. Add drawing function in `cnc_draw.c`
3. Add tool enum in `CNCTool`
4. Implement rendering in `cnc_render_*()` function
5. Add toolbar button in `create_toolbar()`

### Adding New Features

- **Snapping**: Implement grid-snapping in canvas_event()
- **Layer support**: Expand draw_objects array to include layers
- **Colors**: Add color picker to right panel
- **Line styles**: Extend line width with dash patterns
- **Import**: Add file loading capability

## Example: Using the CNC Tool

```c
// Initialize
cnc_create_ui(screen);

// Draw a simple shape
cnc_set_tool(CNC_TOOL_LINE);
cnc_draw_line(50, 50, 150, 150);

// Add dimension
cnc_add_dimension(50, 50, 150, 150, "Diagonal");

// Zoom to fit
cnc_fit_to_view();

// Export job
cnc_export_job("my_design.cnc");
```

## Troubleshooting

### Canvas not rendering
- Ensure `lv_obj_invalidate()` is called after drawing operations
- Check that canvas object is properly created in `create_canvas()`

### Tools not responding
- Verify event callbacks are registered
- Check `LV_EVENT_CLICKED` is enabled in lv_conf.h

### Memory issues
- Reduce maximum element counts if needed
- Clear old jobs before creating new ones

## License

Part of LVGL Port Linux project. See project LICENSE file.

## References

- LVGL Documentation: https://docs.lvgl.io
- CNC Machine Specifications: See image reference
- Technical Drawings in Reference Image: Precision measurements for CNC setup

---

**Version**: 1.0  
**Last Updated**: 2026-05-16  
**Author**: LVGL CNC Tool Project
