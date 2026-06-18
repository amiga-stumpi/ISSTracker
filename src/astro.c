#include "app.h"
#include "astro.h"
#include "fixedmath.h"
static int leap(ULONG y){ return ((y%4)==0 && ((y%100)!=0 || (y%400)==0)); }
static UWORD doy_from_unix(ULONG t){ ULONG days; ULONG y; static const UBYTE mdays[12]={31,28,31,30,31,30,31,31,30,31,30,31}; UWORD m; days=t/86400UL; y=1970; while(days>=(ULONG)(leap(y)?366:365)){ days-=(ULONG)(leap(y)?366:365); y++; } for(m=0;m<12;m++){ UWORD d; d=mdays[m]; if(m==1 && leap(y)) d++; if(days<d) break; days-=d; } return (UWORD)(days+1); }
UBYTE astro_light_state(ULONG timestamp, WORD lat_cd, WORD lon_cd){ UWORD doy; ULONG sod; WORD angle; WORD decl; WORD sublon; WORD dlat; WORD dlon; LONG score; doy=doy_from_unix(timestamp); sod=timestamp%86400UL; angle=(WORD)(((LONG)(doy+10)*360L)/365L); decl=(WORD)((-2344L*(LONG)iss_cos_deg1024(angle))/1024L); sublon=iss_wrap_lon_cd(18000L-((LONG)sod*36000L)/86400L); dlat=iss_abs_word((WORD)(lat_cd-decl)); dlon=iss_abs_word(iss_wrap_lon_cd((LONG)lon_cd-(LONG)sublon)); score=9000L-(LONG)dlat-((LONG)dlon/2L); if(score>0) return ISS_LIGHT_DAY; if(score>-1200) return ISS_LIGHT_TWILIGHT; return ISS_LIGHT_NIGHT; }
