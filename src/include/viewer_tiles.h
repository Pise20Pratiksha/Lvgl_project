// viewer_tiles.h
#ifndef VIEWER_TILES_H
#define VIEWER_TILES_H

#include "viewer.h"
#include <stdint.h>
#include <stdbool.h>

#define TILE_SZ         256          // pixels per tile edge
#define TILE_CACHE_MAX  64           // LRU cache slots
#define MIPMAP_LEVELS   7            // L0=full, L6=20000>>6=312px wide

typedef struct {
    int32_t  level;          // mipmap level (0=full res)
    int32_t  tx, ty;         // tile grid coordinates
    uint16_t *pixels;        // RGB565 pixel data (256×256 × 2 bytes)
    uint32_t  lru_stamp;     // for eviction
    bool      valid;
} Tile;

typedef struct {
    Tile     cache[TILE_CACHE_MAX];
    uint32_t stamp;
} TileCache;

extern TileCache tile_cache;

/* Returns cached or freshly decoded tile pixels. Never NULL. */
uint16_t *tile_get(int32_t level, int32_t tx, int32_t ty);

/* Best mipmap level for current zoom (zoom_scale() → level) */
int32_t   tile_best_level(float zoom);

/* Invalidate all cached tiles (call after image source changes) */
void      tile_cache_flush(void);

#endif