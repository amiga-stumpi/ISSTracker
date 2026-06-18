#include "fixedmath.h"
WORD iss_wrap_lon_cd(LONG lon_cd) { while (lon_cd < -18000) lon_cd += 36000; while (lon_cd > 18000) lon_cd -= 36000; return (WORD)lon_cd; }
WORD iss_abs_word(WORD v) { return (v < 0) ? (WORD)-v : v; }
LONG iss_isqrt32(ULONG v) { ULONG bit; ULONG res; bit = 1UL << 30; while (bit > v) bit >>= 2; res = 0; while (bit != 0) { if (v >= res + bit) { v -= res + bit; res = (res >> 1) + bit; } else { res >>= 1; } bit >>= 2; } return (LONG)res; }
static const WORD sin90[19] = {0,89,178,265,350,432,512,587,658,724,784,839,887,928,962,989,1009,1020,1024};
WORD iss_sin_deg1024(WORD deg) { WORD q; WORD r; WORD v; while (deg < 0) deg += 360; while (deg >= 360) deg -= 360; q = deg / 90; r = deg % 90; if (r > 85) v = 1024; else v = sin90[(r + 2) / 5]; if (q == 0) return v; if (q == 1) return sin90[((90 - r) + 2) / 5]; if (q == 2) return (WORD)-v; return (WORD)-sin90[((90 - r) + 2) / 5]; }
WORD iss_cos_deg1024(WORD deg) { return iss_sin_deg1024((WORD)(deg + 90)); }
WORD iss_cos_lat_cd1024(WORD lat_cd) { WORD deg; if (lat_cd < 0) lat_cd = -lat_cd; deg = (WORD)(lat_cd / 100); if (deg > 90) deg = 90; return iss_cos_deg1024(deg); }
