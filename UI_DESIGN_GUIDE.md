# CNC Drawing Tool - Visual Interface Guide

## User Interface Layout

```
╔══════════════════════════════════════════════════════════════════════════════╗
║                          CNC DRAWING TOOL v1.0                              ║
╠════════════════════════════════════════════════════════════════════════════╦═╣
║  ✎    │ ◯    │ ▭    │ ⌫    │ ⊕   │ ↶    │ ✕     │ 🔍+  │ 🔍-  │ ⬚ Fit  ║ ║
║ Line  │Circle│ Rect │ Dim  │ Pan │ Undo │ Clear │In    │Out   │ To View ║ ║
╠════════════════════════════════════════════════════════════════════╤═════════╣
║                                                                   │  TOOLBAR ║
║                                                                   │ ═════════╣
║                                                                   │ Current  ║
║                                                                   │ Tool:    ║
║                                                                   │ ▶ Line   ║
║                                                                   │          ║
║                                                                   │ ─────────╣
║                                                                   │ Job Info │
║                                                                   │ ═════════╣
║                                                                   │ New Job  ║
║    CANVAS AREA                                                    │ 100x100mm║
║    ✓ Grid Background (major & minor ticks)                       │ 0 elements
║    ✓ Real-time drawing                                           │          ║
║    ✓ Responsive to mouse/touch                                   │ ─────────╣
║    ✓ Zoom-aware rendering                                        │ Drawing  ║
║    ✓ Dimension annotation support                                │ Data     ║
║    ✓ Multi-shape composition                                     │ ═════════╣
║                                                                   │ Lines: 0 ║
║                                                                   │ Circles:0║
║                                                                   │ Rects: 0 ║
║                                                                   │ Dims:  0 ║
║                                                                   │          ║
║                                                                   │ Zoom:    ║
║                                                                   │ 100%     ║
║                                                                   │          ║
║                                                                   │ [💾 Exp] ║
║                                                                   │ [Export] ║
║                                                                   │ [Job  ]  ║
╠════════════════════════════════════════════════════════════════════════════╦╣
║ CNC Drawing Tool | Ready                    X: 0  Y: 0         Grid: ON   ║║
║ Status Bar - Shows coordinates, grid state, and operation feedback        ║║
╚════════════════════════════════════════════════════════════════════════════╩╝

Colors Used:
  🟦 Dark Navy (Background)    - #0D0D1A
  🟩 Dark Blue (Toolbar/Panel) - #0F1729 / #0A0F1E
  🟦 Cyan (Active/Highlights)  - #38BDF8
  🟩 Green (Success)           - #10B981
  🟥 Red (Alert)               - #EF4444
  ⬜ White (Text)              - #E2E8F0
```

## Tool Icons & Functions

### Drawing Tools

```
┌─ LINE TOOL ───────┐
│ Icon: ✎            │
│ Action: Draw lines │
│ Method: Click-drag │
│ Color: Green       │
└────────────────────┘

┌─ CIRCLE TOOL ─────────────┐
│ Icon: ◯                    │
│ Action: Draw circles       │
│ Method: Click center,      │
│         drag for radius    │
│ Color: Green               │
└────────────────────────────┘

┌─ RECTANGLE TOOL ──────────┐
│ Icon: ▭                    │
│ Action: Draw rectangles    │
│ Method: Click corner,      │
│         drag to opposite   │
│ Color: Green               │
└────────────────────────────┘

┌─ DIMENSION TOOL ──────────────────┐
│ Icon: ⌫                            │
│ Action: Add measurements/dims      │
│ Method: Click point A,             │
│         drag to point B            │
│ Auto-calculates: Length (mm)       │
│ Color: Cyan                        │
└────────────────────────────────────┘

┌─ PAN TOOL ─────────────────┐
│ Icon: ⊕                     │
│ Action: Navigate canvas     │
│ Method: Click-drag to move  │
│ Color: Blue                 │
└─────────────────────────────┘
```

### Control Tools

```
┌─ ZOOM IN ──────┐        ┌─ ZOOM OUT ──────┐        ┌─ FIT VIEW ──────┐
│ Icon: 🔍+      │        │ Icon: 🔍-       │        │ Icon: ⬚         │
│ Action: Magnify│        │ Action: Reduce  │        │ Action: Fit all │
│ Max: 10×       │        │ Min: 0.01×      │        │ work area       │
└────────────────┘        └─────────────────┘        └─────────────────┘

┌─ UNDO ──────────────────┐        ┌─ CLEAR ────────────────┐
│ Icon: ↶                  │        │ Icon: ✕                │
│ Action: Remove last item │        │ Action: Remove all     │
│ Level: 1 element max     │        │ Confirm: Yes/No modal  │
└──────────────────────────┘        └────────────────────────┘

┌─ EXPORT JOB ──────────────────────────────────────┐
│ Icon: 💾                                          │
│ Action: Save drawing to file                      │
│ Format: Plain text with dimensions                │
│ Location: /tmp/cnc_job.txt (configurable)         │
│ Includes: Lines, circles, rects, dimensions      │
└────────────────────────────────────────────────────┘
```

## Right Panel Sections

### 1. Current Tool Display
```
┌──────────────────┐
│ Current Tool     │
├──────────────────┤
│ Tool: Line       │
│                  │
│ (Updates as you  │
│  switch tools)   │
└──────────────────┘
```

### 2. Job Information
```
┌──────────────────┐
│ Job Information  │
├──────────────────┤
│ New Job          │ (Name)
│ 100x100 mm       │ (Dimensions)
│ 5 elements       │ (Count)
└──────────────────┘
```

### 3. Drawing Data Statistics
```
┌──────────────────┐
│ Drawing Data     │
├──────────────────┤
│ Lines: 3         │
│ Circles: 1       │
│ Rects: 2         │
│ Dims: 4          │
├──────────────────┤
│ Zoom: 125%       │
└──────────────────┘
```

### 4. Export Button
```
┌──────────────────┐
│  💾 Export Job   │
│                  │
│  [Click to save] │
│  to /tmp/cnc_... │
└──────────────────┘
```

## Canvas Area Features

```
Grid Pattern (Dark Theme):
┌─────────────────────────────────┐
│ ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ │  Major grid
│ • • • • • • • • • • • • • • • • │  Minor grid points
│ ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ │
│ • • • • • • • • • • • • • • • • │
│ ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░ │
│ • • • • • • • • • • • • • • • • │
└─────────────────────────────────┘

Drawn Elements (Examples):
  ━━━ Lines (Green)
  ⭕ Circles (Green outline)
  ▭ Rectangles (Green outline)
  ┈┈ Dimensions (Cyan with endpoint dots)
```

## Status Bar Information

```
┌─────────────────────────────────────────────────────────────┐
│ CNC Drawing Tool | Ready   X: 150  Y: 200    Grid: ON      │
├─────────────────────────────────────────────────────────────┤
│ • Tool Status: Shows current operation
│ • Coordinates: X & Y position of cursor
│ • Grid State: Whether grid is visible
│ • Operation feedback (e.g., "Job Exported!")
└─────────────────────────────────────────────────────────────┘
```

## Workflow Visualization

```
START
  │
  ├─→ [CNC Tool Loads]
  │     • UI initializes
  │     • Canvas ready
  │     • All tools available
  │
  ├─→ [Select Tool]
  │     • Line, Circle, Rectangle, Dimension
  │     • Pan, Zoom
  │
  ├─→ [Draw on Canvas]
  │     • Click-drag to create shape
  │     • Grid background for reference
  │     • Real-time preview
  │
  ├─→ [View Results]
  │     • Right panel shows statistics
  │     • Status bar shows info
  │     • All shapes visible
  │
  ├─→ [Refine Drawing]
  │     • Undo last action
  │     • Add more shapes
  │     • Add dimensions
  │
  ├─→ [Zoom/Pan]
  │     • Zoom in for detail
  │     • Zoom out for overview
  │     • Pan to navigate
  │
  ├─→ [Export Job]
  │     • Click Export button
  │     • File saved to /tmp/cnc_job.txt
  │     • Status shows "Job Exported!"
  │
  └─→ END (Use with CNC software)
```

## Keyboard Shortcut Reference

| Key | Function | Notes |
|-----|----------|-------|
| `L` | Line tool | (Can be implemented) |
| `C` | Circle tool | (Can be implemented) |
| `R` | Rectangle tool | (Can be implemented) |
| `D` | Dimension tool | (Can be implemented) |
| `Z` | Pan/Zoom tool | (Can be implemented) |
| `Ctrl+Z` | Undo | (Can be implemented) |
| `Delete` | Clear all | (Can be implemented) |
| `+` / `=` | Zoom in | (Can be implemented) |
| `-` / `_` | Zoom out | (Can be implemented) |
| `F` | Fit to view | (Can be implemented) |
| `G` | Toggle grid | (Can be implemented) |

*Note: Keyboard support can be added by handling key events in the main loop*

## Mouse/Touch Interactions

```
ACTION                          EFFECT
─────────────────────────────────────────────────────────
Tool Button Click               Switch active tool
Canvas Click-Drag               Draw shape (depends on tool)
Undo Button Click               Remove last element
Clear Button Click              Remove all elements
Zoom +/- Button Click           Magnify/reduce view
Fit Button Click                Fit all content
Export Button Click             Save job file
Canvas Pan Tool + Drag          Move viewport
Canvas Scroll (if enabled)      Zoom in/out
```

## Data Flow

```
User Input
   │
   ├─ Click on tool button
   │  └─→ cnc_set_tool()
   │      └─→ Update g_cnc.current_tool
   │
   ├─ Draw on canvas
   │  └─→ canvas_event() handler
   │      ├─→ Get mouse position
   │      ├─→ Call appropriate drawing function
   │      │  (cnc_draw_line, cnc_draw_circle, etc.)
   │      └─→ Update g_cnc arrays
   │
   ├─ Click Undo/Clear
   │  └─→ cnc_undo() / cnc_clear_all()
   │      └─→ Modify element counts
   │
   ├─ Click Zoom buttons
   │  └─→ cnc_zoom_in() / cnc_zoom_out()
   │      └─→ Update g_cnc.zoom
   │
   └─ Click Export
      └─→ cnc_export_job()
          └─→ Write file with current data
            
Canvas Rendering
   │
   ├─ cnc_canvas_draw_all()
   │  ├─ Draw grid background
   │  ├─ Render all lines
   │  ├─ Render all circles
   │  ├─ Render all rectangles
   │  └─ Render all dimensions
   │
   └─ lv_obj_invalidate() → Screen updates
      └─→ User sees changes
```

## Performance Metrics

```
Component          Max Count  Memory/Item  Total Memory
────────────────────────────────────────────────────────
Lines              256        ~20 bytes    ~5 KB
Circles            64         ~20 bytes    ~1 KB
Rectangles         64         ~20 bytes    ~1 KB
Dimensions         128        ~50 bytes    ~6 KB
────────────────────────────────────────────────────────
Total Objects      512                     ~13 KB
UI Components      ~20                     ~30 KB
─────────────────────────────────────────────────────────
Total Memory                               ~50 KB
```

## Color Legend

```
ELEMENT              COLOR           HEX VALUE
─────────────────────────────────────────────────
Background           Dark Navy       #0D0D1A
Toolbar              Dark Blue       #0F1729
Panels               Darker Blue     #0A0F1E
Borders              Medium Blue     #1E3A5F
Primary Text         Light Gray      #E2E8F0
Secondary Text       Gray            #64748B
Active Elements      Cyan            #38BDF8
Grid Major           Dark Cyan       #3A4A5E
Grid Minor           Very Dark Blue  #1E2A3A
Draw Color (default) Green           #10B981
Dimension Color      Cyan            #38BDF8
Success Indicator    Green           #10B981
Alert/Error          Red             #EF4444
```

---

**UI Design Specifications**
- **Theme**: Dark CAD-style interface
- **Resolution**: Works at any screen size
- **Responsiveness**: Touch and mouse compatible
- **Theme Style**: Professional engineering software aesthetic
- **Font**: System fonts via LVGL (14-16px)
- **Layout**: Flexible with toolbar top, canvas center, panel right

This design provides an intuitive interface for precision CNC drawing work while maintaining
a professional appearance suitable for engineering and manufacturing applications.
