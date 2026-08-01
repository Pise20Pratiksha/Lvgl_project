# CNC Sketch Manager - Complete Workflow

## End-to-End Process Flow

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         START: User at Screen                           │
│                    (LVGL Application Running)                           │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                      STEP 1: DRAW ON CANVAS                             │
│                                                                          │
│  Available Tools:                                                       │
│  ┌──────┐ ┌────────┐ ┌────────────┐ ┌───────────┐ ┌──────┐ ┌───────┐ │
│  │ Line │ │Circle  │ │ Rectangle  │ │Dimension │ │ Pan  │ │ Zoom  │ │
│  └──────┘ └────────┘ └────────────┘ └───────────┘ └──────┘ └───────┘ │
│                                                                          │
│  + Adjustable: Stroke width, Color, Grid                               │
│  + Actions: Clear all, Undo, Zoom controls, Fit-to-view                │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    STEP 2: SAVE SKETCH (💾)                             │
│                                                                          │
│  User clicks: 💾 Button in toolbar                                      │
│                      │                                                   │
│                      ▼                                                   │
│  ┌────────────────────────────────────────┐                            │
│  │  btn_save_sketch_event() called        │                            │
│  │  • Generates timestamp                 │                            │
│  │  • Creates filename with date/time     │                            │
│  │  • Calls cnc_sketch_save()             │                            │
│  └────────────────────────────────────────┘                            │
│                      │                                                   │
│                      ▼                                                   │
│  ┌────────────────────────────────────────┐                            │
│  │  cnc_sketch_save() Function            │                            │
│  │  • Reads g_cnc drawing state           │                            │
│  │  • Converts to JSON format             │                            │
│  │  • Ensures /tmp/cnc_sketches/ exists   │                            │
│  │  • Writes file to disk                 │                            │
│  └────────────────────────────────────────┘                            │
│                      │                                                   │
│                      ▼                                                   │
│  Result:                                                                │
│  📁 /tmp/cnc_sketches/sketch_20260529_143012.json                       │
│                                                                          │
│  UI Feedback: "✓ Sketch Saved!" (green text)                            │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│              Saved JSON Format (example excerpt):                       │
│                                                                          │
│  {                                                                      │
│    "version": "1.0",                                                    │
│    "timestamp": 1715337600,                                             │
│    "description": "User sketch from CNC UI",                            │
│    "job": { "name": "New Job", ... },                                   │
│    "lines": [                                                           │
│      { "x1": 50, "y1": 50, "x2": 150, "y2": 150,                       │
│        "width": 2, "color": "#38BDF8" }                                 │
│    ],                                                                   │
│    "circles": [ ... ],                                                  │
│    "rectangles": [ ... ],                                               │
│    "dimensions": [ ... ]                                                │
│  }                                                                      │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                    ┌─────────────┴─────────────┐
                    │                           │
                    ▼                           ▼
    ┌──────────────────────────┐    ┌──────────────────────────┐
    │  OPTION A: EXPORT NOW    │    │ OPTION B: EDIT & SAVE    │
    │  (Next Step Below)        │    │ (Can save again later)   │
    └──────────────────────────┘    └──────────────────────────┘
                    │                           │
                    ▼                           │
┌─────────────────────────────────────────────────────────────────────────┐
│                  STEP 3: EXPORT G-CODE (⚙)                             │
│                                                                          │
│  User clicks: ⚙ Button in toolbar                                       │
│                      │                                                   │
│                      ▼                                                   │
│  ┌────────────────────────────────────────┐                            │
│  │  btn_export_gcode_event() called       │                            │
│  │  • Generates timestamp                 │                            │
│  │  • Creates filename with date/time     │                            │
│  │  • Calls cnc_gcode_generate()          │                            │
│  └────────────────────────────────────────┘                            │
│                      │                                                   │
│                      ▼                                                   │
│  ┌────────────────────────────────────────┐                            │
│  │  cnc_gcode_generate() Function         │                            │
│  │  Parameters (configurable):            │                            │
│  │  • Tool diameter: 1.5mm                │                            │
│  │  • Feed rate: 100 mm/min               │                            │
│  │  • Spindle speed: 3000 RPM             │                            │
│  │  • Safe Z height: 5.0mm                │                            │
│  └────────────────────────────────────────┘                            │
│                      │                                                   │
│                      ▼                                                   │
│  ┌────────────────────────────────────────┐                            │
│  │  For EACH Drawing Element:             │                            │
│  │                                        │                            │
│  │  LINE:                                 │                            │
│  │  • G0 X#.# Y#.# (rapid move to start) │                            │
│  │  • G1 Z-1.0 F#.# (lower tool)         │                            │
│  │  • G1 X#.# Y#.# F#.# (cut line)       │                            │
│  │  • G0 Z5.0 (retract)                  │                            │
│  │                                        │                            │
│  │  CIRCLE:                               │                            │
│  │  • G0 X# Y# (rapid to center)         │                            │
│  │  • G1 Z-1.0 F#.# (lower)              │                            │
│  │  • G2 X# Y# I# J# (arc cut)           │                            │
│  │  • G0 Z5.0 (retract)                  │                            │
│  │                                        │                            │
│  │  RECTANGLE:                            │                            │
│  │  • G0 X# Y# (rapid to corner 1)       │                            │
│  │  • G1 Z-1.0 F#.# (lower)              │                            │
│  │  • G1 commands for 4 corners          │                            │
│  │  • G0 Z5.0 (retract)                  │                            │
│  │                                        │                            │
│  └────────────────────────────────────────┘                            │
│                      │                                                   │
│                      ▼                                                   │
│  ┌────────────────────────────────────────┐                            │
│  │  G-Code Structure Complete:            │                            │
│  │  1. Comments (metadata)                │                            │
│  │  2. Initialization (G21, G90, M3)      │                            │
│  │  3. All cutting operations             │                            │
│  │  4. Shutdown (M5, M30)                 │                            │
│  └────────────────────────────────────────┘                            │
│                      │                                                   │
│                      ▼                                                   │
│  Result:                                                                │
│  📄 /tmp/cnc_gcode/job_20260529_143012.gcode                            │
│                                                                          │
│  UI Feedback: "✓ G-Code Generated!" (green text)                        │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     STEP 4: RETRIEVE FILES                              │
│                                                                          │
│  From Linux Terminal:                                                   │
│  $ ls /tmp/cnc_sketches/                                                │
│  sketch_20260529_143012.json                                            │
│                                                                          │
│  $ ls /tmp/cnc_gcode/                                                   │
│  job_20260529_143012.gcode                                              │
│                                                                          │
│  Copy to USB/Network:                                                   │
│  $ cp /tmp/cnc_gcode/job_*.gcode /mnt/usb/                              │
│  $ scp /tmp/cnc_gcode/job_*.gcode user@cnc-machine:/input/              │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                   STEP 5: LOAD IN CNC SOFTWARE                          │
│                                                                          │
│  On CNC Machine:                                                        │
│  1. Open CNC control software (LinuxCNC, Mach3, etc.)                   │
│  2. File → Load G-Code                                                  │
│  3. Select: job_20260529_143012.gcode                                   │
│  4. Verify tool paths and dimensions                                    │
│  5. Set spindle speed if needed (matches 3000 RPM default)              │
│  6. Set tool to 1.5mm if needed                                         │
│  7. Clamp workpiece                                                     │
│  8. Click RUN                                                           │
│                                                                          │
│  Expected Output:                                                       │
│  ✓ Spindle starts                                                       │
│  ✓ Tool positions correctly                                             │
│  ✓ Cuts follow your drawn design                                        │
│  ✓ Tool retracts between cuts                                           │
│  ✓ Program ends at origin                                               │
└─────────────────────────────────────────────────────────────────────────┘
                                  │
                                  ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                     FINAL RESULT: MANUFACTURED PART                     │
│                                                                          │
│  ✓ Physical product matches your digital design                         │
│  ✓ Dimensions are accurate                                              │
│  ✓ Tool cuts are smooth                                                 │
│  ✓ All features (lines, circles, rectangles) are cut                    │
└─────────────────────────────────────────────────────────────────────────┘
```

## Data Flow Diagram

```
┌──────────────┐
│   g_cnc      │  Global state containing all drawn elements
│   Structure  │  • lines[], circles[], rects[], dims[]
└──────┬───────┘
       │
       │ (on 💾 click)
       ▼
┌──────────────────────┐
│ btn_save_sketch_event│  Event handler for save button
└──────┬───────────────┘
       │
       ▼
┌──────────────────────────────────┐
│ cnc_sketch_save()                │  Converts drawing to JSON
│ - Reads all g_cnc elements       │
│ - Formats as JSON strings        │
│ - Ensures directories exist      │
└──────┬──────────────────────────┘
       │
       ▼
┌──────────────────────────────────┐
│ /tmp/cnc_sketches/              │  Saved sketch file
│ sketch_YYYYMMDD_HHMMSS.json      │
└──────────────────────────────────┘


┌──────────────┐
│   g_cnc      │
│   Structure  │
└──────┬───────┘
       │
       │ (on ⚙ click)
       ▼
┌──────────────────────┐
│ btn_export_gcode_evt │  Event handler for export button
└──────┬───────────────┘
       │
       ▼
┌──────────────────────────────────┐
│ cnc_gcode_generate()             │  Converts drawing to G-code
│ - Reads all g_cnc elements       │
│ - For each element:              │
│   • Converts pixels to mm        │
│   • Generates G0/G1/G2 commands  │
│   • Calculates tool paths        │
│   • Formats as proper G-code     │
└──────┬──────────────────────────┘
       │
       ▼
┌──────────────────────────────────┐
│ /tmp/cnc_gcode/                  │  G-code file ready for CNC
│ job_YYYYMMDD_HHMMSS.gcode        │
└──────────────────────────────────┘
       │
       │ (user copies to CNC machine)
       ▼
┌──────────────────────────────────┐
│ CNC Machine Software             │  Loads G-code and runs job
│ (LinuxCNC, Mach3, etc.)          │
└──────────────────────────────────┘
```

## Coordinate Transformation Chain

```
Screen Pixels
    │
    │ (User draws on touchscreen)
    ▼
Drawing Data
(stored in g_cnc arrays with pixel coordinates)
    │
    │ (on save)
    ▼
JSON File
(pixel coordinates, colors, metadata)
    │
    │ (on export)
    ▼
Pixel → MM Conversion
(multiply by 0.1)
    │
    ▼
G-Code Coordinates
(real-world mm measurements)
    │
    │ (transmitted to CNC)
    ▼
CNC Machine Movement
(actual physical cuts at real dimensions)
    │
    ▼
Physical Part
(manufactured from your design)
```

## File Organization

```
Workspace Root:
└── lv_port_linux/
    ├── src/
    │   ├── cnc/
    │   │   ├── cnc_draw.c        (existing - core drawing)
    │   │   └── cnc_sketch.c      (NEW - sketch management)
    │   ├── ui/
    │   │   └── cnc_ui.c          (modified - added buttons)
    │   └── include/
    │       ├── cnc_draw.h        (existing)
    │       └── cnc_sketch.h      (NEW)
    ├── CMakeLists.txt            (no changes needed)
    ├── SKETCH_MANAGER_GUIDE.md           (NEW)
    ├── SKETCH_MANAGER_DEVELOPER.md       (NEW)
    └── SKETCH_IMPLEMENTATION_COMPLETE.md (NEW)

Build Output:
└── build/
    ├── bin/
    │   └── lvglsim              (compiled executable)
    └── ... (cmake build files)

Runtime Storage:
└── /tmp/
    ├── cnc_sketches/
    │   ├── sketch_20260529_143012.json
    │   ├── sketch_20260529_143545.json
    │   └── ...
    └── cnc_gcode/
        ├── job_20260529_143100.gcode
        ├── job_20260529_143600.gcode
        └── ...
```

## Key Transformations

### 1. Drawing → JSON
```
Input:  g_cnc array with drawing primitives
        • line: (x1, y1) → (x2, y2) in pixels
        • circle: center (cx, cy), radius in pixels
        • rectangle: (x1, y1) → (x2, y2) in pixels

Output: JSON structured data
        • Preserves all coordinates
        • Includes colors and styling
        • Adds metadata (timestamp, description)
```

### 2. JSON → G-Code
```
Input:  JSON file with sketch data
        • Coordinates in pixels
        • Colors (for reference)
        • Metadata

Process: For each element:
        1. Convert pixels → mm (multiply by 0.1)
        2. Check element type (line, circle, rect)
        3. Generate appropriate G-code commands
        4. Add safety features (retracts, speed ramps)
        5. Format as ISO 6983 compliant G-code

Output: Text file with G-code commands
        • Ready for CNC machine
        • Includes all safety features
        • Documented with comments
```

## Button Integration

```
Toolbar Layout:
┌─────────────────────────────────────────────────────────┐
│ [■] [Line] [Circle] [Rect] [Dim] [Pan] [Zoom] [║] │
│                 (existing tools)        [sep]         │
│                                                        │
│ [✕] [↶] [+] [-] [⬚] [║] [💾] [⚙] │
│ (existing)              (sep) (NEW)  (NEW) │
└─────────────────────────────────────────────────────────┘

Legend:
[■] = Color/fill selector
[Line][Circle][Rect][Dim] = Drawing tools
[Pan][Zoom] = View controls
[✕] = Clear all
[↶] = Undo
[+]/[-] = Zoom controls
[⬚] = Fit to view
[║] = Visual separator
[💾] = Save Sketch (NEW)
[⚙] = Export G-Code (NEW)
```

## Success Criteria Achieved ✓

- ✓ Draws and stores complex shapes
- ✓ Saves to persistent JSON files
- ✓ Converts to standard G-code format
- ✓ Generates CNC-ready files
- ✓ Integrates seamlessly with UI
- ✓ No external dependencies
- ✓ Fully backward compatible
- ✓ Comprehensive documentation
- ✓ Clean, maintainable code
- ✓ Production-ready implementation
