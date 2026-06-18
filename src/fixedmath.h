#ifndef ISS_FIXEDMATH_H
#define ISS_FIXEDMATH_H
#include <exec/types.h>
WORD iss_wrap_lon_cd(LONG lon_cd);
WORD iss_abs_word(WORD v);
LONG iss_isqrt32(ULONG v);
WORD iss_sin_deg1024(WORD deg);
WORD iss_cos_deg1024(WORD deg);
WORD iss_cos_lat_cd1024(WORD lat_cd);
#endif
