#include "app.h"
#include "worldmask.h"
#include "map_image.h"

#define WM_SRC_DEPTH 4
#define WM_WATER_PEN 2

static WORD clamp_word(WORD v, WORD lo, WORD hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void coord_to_map_pixel(WORD lat_cd, WORD lon_cd, WORD *x_out, WORD *y_out)
{
    LONG x;
    LONG y;

    if (lat_cd < -9000) lat_cd = -9000;
    if (lat_cd > 9000) lat_cd = 9000;
    while (lon_cd < -18000) lon_cd = (WORD)(lon_cd + 36000);
    while (lon_cd > 18000) lon_cd = (WORD)(lon_cd - 36000);

    x = (((LONG)lon_cd + 18000L) * (LONG)ISS_MAP_SRC_W) / 36000L;
    y = ((9000L - (LONG)lat_cd) * (LONG)ISS_MAP_SRC_H) / 18000L;

    *x_out = clamp_word((WORD)x, 0, (WORD)(ISS_MAP_SRC_W - 1));
    *y_out = clamp_word((WORD)y, 0, (WORD)(ISS_MAP_SRC_H - 1));
}

static UBYTE map_pixel_is_water(WORD x, WORD y)
{
    return (iss_map_get_pen(WM_SRC_DEPTH, x, y) == WM_WATER_PEN) ? 1 : 0;
}

UBYTE worldmask_classify(WORD lat_cd, WORD lon_cd)
{
    WORD x;
    WORD y;
    WORD dx;
    WORD dy;
    UWORD water;
    UWORD land;

    coord_to_map_pixel(lat_cd, lon_cd, &x, &y);

    water = 0;
    land = 0;
    for (dy = -2; dy <= 2; ++dy) {
        for (dx = -2; dx <= 2; ++dx) {
            WORD sx;
            WORD sy;
            sx = clamp_word((WORD)(x + dx), 0, (WORD)(ISS_MAP_SRC_W - 1));
            sy = clamp_word((WORD)(y + dy), 0, (WORD)(ISS_MAP_SRC_H - 1));
            if (map_pixel_is_water(sx, sy))
                ++water;
            else
                ++land;
        }
    }

    if (land >= 18)
        return ISS_SURFACE_LAND;
    if (water >= 18)
        return ISS_SURFACE_WATER;
    return ISS_SURFACE_COAST;
}
