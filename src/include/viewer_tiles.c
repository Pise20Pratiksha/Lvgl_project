// viewer_tiles.c
#include "viewer_tiles.h"
#include "viewer_math.h"
#include <string.h>
#include <stdlib.h>

TileCache tile_cache;

int32_t tile_best_level(float zoom) {
    // Pick smallest mipmap where one tile pixel >= one screen pixel
    for (int l = MIPMAP_LEVELS - 1; l >= 0; l--) {
        float scale = zoom * (float)(1 << l);
        if (scale >= 0.5f) return l;
    }
    return 0;
}

static Tile *cache_find(int32_t level, int32_t tx, int32_t ty) {
    for (int i = 0; i < TILE_CACHE_MAX; i++) {
        Tile *t = &tile_cache.cache[i];
        if (t->valid && t->level == level && t->tx == tx && t->ty == ty)
            return t;
    }
    return NULL;
}

static Tile *cache_evict(void) {
    Tile *oldest = &tile_cache.cache[0];
    for (int i = 1; i < TILE_CACHE_MAX; i++) {
        if (!tile_cache.cache[i].valid) return &tile_cache.cache[i];
        if (tile_cache.cache[i].lru_stamp < oldest->lru_stamp)
            oldest = &tile_cache.cache[i];
    }
    return oldest;
}

/* Stub: replace with your actual image decoder (e.g. lodepng per tile) */
static void decode_tile(int32_t level, int32_t tx, int32_t ty, uint16_t *dst) {
    // In production: decode the tile from your image source.
    // Here we fill with a visible pattern so the UI works.
    uint32_t off = (uint32_t)(level * 17 + tx * 7 + ty * 13);
    for (int i = 0; i < TILE_SZ * TILE_SZ; i++) {
        int r = ((i / TILE_SZ + off) & 0x1F) << 11;
        int g = ((i % TILE_SZ + off) & 0x3F) << 5;
        int b = (off & 0x1F);
        dst[i] = (uint16_t)(r | g | b);
    }
}

uint16_t *tile_get(int32_t level, int32_t tx, int32_t ty) {
    Tile *t = cache_find(level, tx, ty);
    if (t) { t->lru_stamp = ++tile_cache.stamp; return t->pixels; }

    t = cache_evict();
    if (!t->pixels)
        t->pixels = lv_malloc(TILE_SZ * TILE_SZ * sizeof(uint16_t));

    decode_tile(level, tx, ty, t->pixels);
    t->level = level; t->tx = tx; t->ty = ty;
    t->valid = true;  t->lru_stamp = ++tile_cache.stamp;
    return t->pixels;
}

void tile_cache_flush(void) {
    for (int i = 0; i < TILE_CACHE_MAX; i++)
        tile_cache.cache[i].valid = false;
}