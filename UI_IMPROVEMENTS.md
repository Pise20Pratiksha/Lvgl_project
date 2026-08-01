# CNC Drawing UI - Paint-Like Interface Enhancements

## Overview
A professional, smooth paint-like UI for drawing 2D CNC job designs, inspired by the reference sketch provided.

## Layout Structure
```
┌─────────────────────────────────────────────────────┐
│         TOOLBAR (45px)                              │
│  [✎] [◯] [▭] [⌫] | [↶] [✕] | [+] [−] [⬚]         │
├─────────────────────────────────────────────────────┤
│ PROPERTIES BAR (35px)                               │
│ Width: ▬▬▬▬▬ | Color: ⬤⬜🔴🟢🟡🟣 | [Grid: ON]      │
├──────────┬────────────────────────────────┬─────────┤
│  RULER   │                                │ PANEL   │
│  (28px)  │   CANVAS - MAIN DRAWING AREA   │ (170px) │
│  28x375  │         (600x375px)            │         │
│          │                                │ • Tool  │
│          │                                │ • Job   │
│          │                                │ • View  │
├──────────┴────────────────────────────────┴─────────┤
│ STATUS BAR (30px)                                   │
│ ● Ready | X: 0.0  Y: 0.0 | Dist: -- | Grid: 10mm   │
└─────────────────────────────────────────────────────┘
```

## Features Implemented

### 1. **Enhanced Toolbar** (45px)
- **Drawing Tools** (Compact Icon Buttons):
  - ✎ Line
  - ◯ Circle
  - ▭ Rectangle
  - ⌫ Dimension
  
- **Edit Tools**:
  - ↶ Undo
  - ✕ Clear (destructive action - red)
  
- **View Controls**:
  - + Zoom In
  - − Zoom Out
  - ⬚ Fit to View

**Features:**
- Current tool highlighted with cyan background and white border
- Icon-based design for faster visual recognition
- Touch-friendly button sizes (38x33px)
- Smooth color transitions on selection

### 2. **Properties Bar** (35px)
Interactive tool options for real-time adjustments:

- **Stroke Width Slider**:
  - Range: 1-20 pixels
  - Real-time update as you draw
  - Visual feedback with slider position

- **Color Picker**:
  - 6 preset colors: Cyan, White, Red, Green, Yellow, Purple
  - Click to select - automatic border highlight
  - Default color: Cyan
  - Selected color shows with white border (3px)

- **Grid Toggle**:
  - Visual button with ON/OFF state
  - Green background when active
  - Toggle for snap-to-grid functionality
  - Real-time update on status bar

### 3. **Left Ruler** (28px wide, 375px tall)
- Vertical measurement guide aligned with canvas
- Visual ticks: Minor (10px) and Major (50px) marks
- Helps with precise positioning and measurements
- Professional CAD aesthetic

### 4. **Main Canvas** (600x375px)
- Large, responsive drawing area
- Supports multiple drawing tools:
  - Free-hand lines with preview
  - Rectangles with live preview
  - Circles with radius preview
  - Dimension lines
  
- **Preview Mode:**
  - Cyan preview line while dragging
  - Shows final shape before commit
  - Smooth, responsive touch feedback

### 5. **Right Info Panel** (170px wide)
Professional properties display:

- **Tool Section:**
  - Current active tool name
  - Updated in real-time

- **Job Information:**
  - Object count
  - Drawing dimensions
  - Element tracking

- **View Information:**
  - Current zoom level
  - View statistics

### 6. **Status Bar** (30px tall)
Real-time feedback for every action:

- **Status Indicator:** ● Ready (green dot)
- **Live Coordinates:** X: Y: (real-time as you move)
- **Measurement Display:** Distance between points
- **Grid Info:** Current grid spacing (10mm default)

All color-coded for quick visual scanning:
- Green: Status/Grid
- Cyan: Coordinates
- White: Labels

## UI/UX Improvements

### Paint-Like Smoothness
1. **Touch Preview:** See your shape before releasing
2. **Smooth Transitions:** No jarring color changes
3. **Real-Time Feedback:** Coordinates update as you touch
4. **Visual Hierarchy:** Size and color draw attention to important elements

### Professional Appearance
1. **Dark Theme:** Dark blue/gray palette reduces eye strain
2. **Consistent Spacing:** 4-8px gaps create breathing room
3. **Color Coding:**
   - Cyan = Active/Primary
   - Green = Success/Grid
   - Red = Destructive (Clear)
   - White = Labels/Text
4. **Rounded Corners:** 3-4px radius on buttons for modern look

### Responsive Design
1. **Touch-Friendly:** Minimum 38x33px button sizes
2. **Gesture Support:** Multi-touch pinch-to-zoom (when enabled)
3. **Continuous Drawing:** PRESSED → PRESSING → RELEASED state machine
4. **Smooth Zoom:** With focal point preservation

### Accessibility
1. **Color Symbols:** Each tool has a unique icon
2. **Size Contrast:** Important elements are larger
3. **Clear Labels:** All functions clearly labeled
4. **Visual Feedback:** Button states are obvious (highlighted = selected)

## Drawing Workflow

1. **Select Tool** → Toolbar button highlights
2. **Adjust Properties** → Set stroke width, color
3. **Enable Grid** → Toggle in properties bar
4. **Touch Canvas** → See live preview
5. **Drag to Draw** → Shape follows your finger
6. **Release** → Shape commits and updates job info
7. **Undo if Needed** → Click ↶ button
8. **Export When Done** → Job is saved

## Color Palette
- **Background:** #0D0D1A (very dark blue)
- **Toolbar:** #0F1729 (dark blue-gray)
- **Panel:** #0A0F1E (darker blue)
- **Border:** #1E3A5F (medium blue)
- **Primary (Cyan):** #38BDF8 (bright cyan)
- **Success (Green):** #10B981 (bright green)
- **Danger (Red):** #EF4444 (bright red)
- **Text (White):** #E2E8F0 (off-white)
- **Dim:** #64748B (gray)

## Technical Details

### Layout Positioning
- Canvas: Y=80px (below 45px toolbar + 35px properties)
- Ruler: Y=80px, Width=28px (left edge)
- Panel: X=630px (right edge, 170px wide)
- Status: Bottom, Full width

### Canvas Coordinate System
- Local coordinates relative to canvas
- Mapped to image space for drawing
- Touch offset handling for accurate drawing

### Drawing State Machine
- **PRESSED:** Start new shape, store initial point
- **PRESSING:** Update preview, show live shape
- **RELEASED:** Commit shape, update status
- **PRESS_LOST:** Cancel operation if finger lost

## Reference Design
Based on the CNC technical drawing provided, supporting:
- Dimension lines with measurements
- Precise grid alignment
- Professional CAD-like annotations
- Clean, technical aesthetic

---

**Version:** 1.0  
**Date:** May 16, 2026  
**Platform:** Linux Framebuffer with LVGL 9.6.0+
