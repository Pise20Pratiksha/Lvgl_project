# CMakeLists.txt Configuration for CNC Tool

## How to Add CNC Tool to Your Build

### Step 1: Locate CMakeLists.txt

Open `/home/user/Desktop/Lvgl_Project/lv_port_linux/CMakeLists.txt`

### Step 2: Find the Source Files Section

Look for where source files are listed. You'll find something like:

```cmake
# Around line 100-150, find the source file definitions
```

### Step 3: Add CNC Source Files

Add the following lines to your CMakeLists.txt:

#### Option A: Using add_executable modifications

Find the line with `add_executable(lvglsim ...)` and add the new files:

```cmake
add_executable(lvglsim
    # ... existing files ...
    src/ui/draw.c
    src/ui/layout.c
    src/ui/panel.c
    src/ui/widgets.c
    
    # ADD THESE NEW LINES:
    src/ui/cnc_ui.c
    src/cnc/cnc_draw.c
    # END NEW LINES
)
```

#### Option B: Using target_sources (Recommended)

If you use `target_sources()`, add:

```cmake
target_sources(lvglsim PRIVATE
    src/ui/cnc_ui.c
    src/cnc/cnc_draw.c
)
```

### Step 4: Verify Include Paths

Make sure include paths are set. Look for:

```cmake
target_include_directories(lvglsim PRIVATE
    src/include
    # ... other paths ...
)
```

If not present, add it:

```cmake
target_include_directories(lvglsim PRIVATE src/include)
```

### Step 5: Build

From the build directory:

```bash
cd /home/user/Desktop/Lvgl_Project/lv_port_linux/build
cmake ..
make -j4
```

### Full CMakeLists.txt Example

Here's what the relevant section should look like:

```cmake
# ... existing cmake code ...

# Source files
set(MAIN_SOURCES
    src/main.c
)

set(UI_SOURCES
    src/ui/draw.c
    src/ui/layout.c
    src/ui/panel.c
    src/ui/widgets.c
    src/ui/cnc_ui.c          # CNC UI Component
)

set(CNC_SOURCES
    src/cnc/cnc_draw.c       # CNC Drawing Logic
)

set(LIB_SOURCES
    # ... any library sources ...
)

# Create executable
add_executable(lvglsim
    ${MAIN_SOURCES}
    ${UI_SOURCES}
    ${CNC_SOURCES}
    ${LIB_SOURCES}
)

# Include directories
target_include_directories(lvglsim PRIVATE
    src/include
    ${CMAKE_SOURCE_DIR}/lvgl
    ${CMAKE_BINARY_DIR}
)

# Link libraries
target_link_libraries(lvglsim PRIVATE
    lvgl_linux
    m
)

# ... rest of cmake ...
```

### Troubleshooting Build Issues

#### Issue: "cnc_draw.c not found"
**Solution**: Make sure you ran:
```bash
mkdir -p src/cnc
# Files should be in: src/cnc/cnc_draw.c
```

#### Issue: "cnc_ui.h: No such file or directory"
**Solution**: Verify include path is set:
```cmake
target_include_directories(lvglsim PRIVATE src/include)
```

#### Issue: "undefined reference to 'cnc_init'"
**Solution**: Ensure `src/cnc/cnc_draw.c` is in cmake sources, then rebuild:
```bash
rm -rf build/*
cmake ..
make -j4
```

#### Issue: "fatal error: cnc_draw.h: No such file or directory" in cnc_ui.c
**Solution**: Check the include statement:
```c
#include "cnc_draw.h"  // This should work with proper include path
```

### Manual Build (Alternative)

If CMake is having issues, you can compile manually:

```bash
cd /home/user/Desktop/Lvgl_Project/lv_port_linux

# Compile CNC files
gcc -I. -Isrc/include -Ilvgl -c src/cnc/cnc_draw.c -o cnc_draw.o
gcc -I. -Isrc/include -Ilvgl -c src/ui/cnc_ui.c -o cnc_ui.o

# Then add these .o files to your link command with other objects
```

### Verification

After building, verify the files compiled:

```bash
# Check for build artifacts
ls -la build/CMakeFiles/lvglsim.dir/src/

# Should show:
# cnc_draw.c.o
# cnc_ui.c.o
```

### Testing the Build

```bash
# Run the executable
./build/bin/lvglsim

# Should see:
# === CNC Drawing Tool ===
# Starting LVGL application...
```

## Alternative: Use Pre-configured CMakeLists.txt

If you want a complete working CMakeLists.txt, here's a minimal version for CNC tool:

```cmake
cmake_minimum_required(VERSION 3.11)
project(lvgl_cnc_tool)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Add LVGL
add_subdirectory(lvgl)

# Source files
set(SOURCES
    src/main.c
    src/ui/cnc_ui.c
    src/cnc/cnc_draw.c
)

# Create executable
add_executable(lvglsim ${SOURCES})

# Include directories
target_include_directories(lvglsim PRIVATE
    src/include
    lvgl
    ${CMAKE_BINARY_DIR}
)

# Link LVGL and system libraries
target_link_libraries(lvglsim PRIVATE lvgl lvgl_linux m)

# Output to bin directory
set_target_properties(lvglsim PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
```

## Next Steps

1. **Update CMakeLists.txt** with the source files
2. **Clean build directory**:
   ```bash
   cd build && rm -rf *
   ```
3. **Rebuild**:
   ```bash
   cmake .. && make -j4
   ```
4. **Run**:
   ```bash
   ./bin/lvglsim
   ```

See QUICKSTART.md for usage instructions once built!
