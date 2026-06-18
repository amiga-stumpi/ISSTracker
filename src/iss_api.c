#include "iss_api.h"
#include "http_client.h"
#include "json_min.h"
static char g_http_buf[1024];
LONG iss_fetch_now(IssPosition *pos,char *err,UWORD err_size){ char lat[24]; char lon[24]; ULONG ts; WORD lat_cd; WORD lon_cd; if(http_get_plain("api.open-notify.org","/iss-now.json",g_http_buf,sizeof(g_http_buf),err,err_size)<0) return -1; if(json_find_string(g_http_buf,"latitude",lat,sizeof(lat))<0) return -1; if(json_find_string(g_http_buf,"longitude",lon,sizeof(lon))<0) return -1; if(json_find_long(g_http_buf,"timestamp",&ts)<0) return -1; if(json_parse_centideg(lat,&lat_cd)<0) return -1; if(json_parse_centideg(lon,&lon_cd)<0) return -1; pos->lat_cd=lat_cd; pos->lon_cd=lon_cd; pos->timestamp=ts; pos->valid=1; return 0; }
