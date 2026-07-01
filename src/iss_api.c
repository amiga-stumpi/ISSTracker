#include <exec/libraries.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include "iss_api.h"
#include "http_client.h"
#include "json_min.h"

#define AMITLS13F_INSECURE 1UL
#define HTTPS_TMP_FILE "RAM:isst_https.tmp"

static char g_http_buf[4096];
static struct Library *AmiTLS13Base = 0;
static UBYTE g_tls_checked = 0;
static UBYTE g_tls_ready = 0;

static void set_err(char *err,UWORD err_size,const char *txt){ UWORD i; if(!err || !err_size) return; for(i=0;txt[i] && i+1<err_size;i++) err[i]=txt[i]; err[i]=0; }
static int streq_at(const char *s,const char *k){ while(*k){ if(*s++!=*k++) return 0; } return 1; }
static int is_ws(char c){ return c==' ' || c=='\t' || c=='\r' || c=='\n'; }
static const char *find_key_local(const char *json,const char *key){ const char *p; p=json; while(*p){ if(*p=='\"' && streq_at(p+1,key)){ p++; while(*p && *p!='\"') p++; if(*p=='\"'){ p++; while(is_ws(*p)) p++; if(*p==':') return p+1; } } p++; } return 0; }
static LONG json_find_token(const char *json,const char *key,char *out,UWORD out_size){ const char *p; UWORD n; p=find_key_local(json,key); if(!p) return -1; while(is_ws(*p)) p++; n=0; if(*p=='\"'){ p++; while(*p && *p!='\"' && n+1<out_size) out[n++]=*p++; out[n]=0; return (*p=='\"')?0:-1; } while(*p && *p!=',' && *p!='}' && *p!=']' && !is_ws(*p) && n+1<out_size) out[n++]=*p++; out[n]=0; return n?0:-1; }
static LONG parse_uword_token(const char *json,const char *key,UWORD *out){ char tok[24]; ULONG v; const char *p; if(json_find_token(json,key,tok,sizeof(tok))<0) return -1; p=tok; v=0; while(*p>='0' && *p<='9'){ v=(v*10UL)+(ULONG)(*p-'0'); if(v>65535UL){ *out=65535; return 0; } p++; } *out=(UWORD)v; return 0; }
static LONG read_file_to_buf(const char *path,char *out,UWORD out_size,char *err,UWORD err_size){ BPTR fh; LONG got; fh=Open((STRPTR)path,MODE_OLDFILE); if(!fh){ set_err(err,err_size,"HTTPS read failed"); return -1; } got=Read(fh,out,out_size-1); Close(fh); if(got<0){ set_err(err,err_size,"HTTPS read failed"); return -1; } out[got]=0; return got; }

static LONG tls_init_call(void){ register struct Library *a6 __asm("a6")=AmiTLS13Base; register LONG d0 __asm("d0"); __asm volatile("jsr a6@(-30:W)" : "=r"(d0) : "r"(a6) : "d1","a0","a1","cc","memory"); return d0; }
static LONG tls_http_get_call(const char *url,const char *outfile,ULONG flags){ register ULONG d0 __asm("d0")=flags; register const char *a0 __asm("a0")=url; register const char *a1 __asm("a1")=outfile; register struct Library *a6 __asm("a6")=AmiTLS13Base; __asm volatile("jsr a6@(-84:W)" : "+r"(d0), "+r"(a0), "+r"(a1) : "r"(a6) : "d1","cc","memory"); return (LONG)d0; }
static UBYTE ensure_tls(IssTrackerApp *app){ if(g_tls_checked && g_tls_ready) return 1; g_tls_checked=1; AmiTLS13Base=OpenLibrary((STRPTR)"amitls13.library",0); if(!AmiTLS13Base){ if(app) app->https_available=0; return 0; } if(tls_init_call()!=0){ CloseLibrary(AmiTLS13Base); AmiTLS13Base=0; g_tls_ready=0; if(app) app->https_available=0; return 0; } g_tls_ready=1; if(app) app->https_available=1; return 1; }
UBYTE iss_https_probe(IssTrackerApp *app){ return ensure_tls(app); }
void iss_https_shutdown(void){ AmiTLS13Base=0; g_tls_ready=0; g_tls_checked=0; }

static LONG parse_open_notify(IssPosition *pos,char *err,UWORD err_size)
{
    char lat[24]; char lon[24]; ULONG ts; WORD lat_cd; WORD lon_cd;
    if(json_find_string(g_http_buf,"latitude",lat,sizeof(lat))<0){ set_err(err,err_size,"JSON lat"); return -1; }
    if(json_find_string(g_http_buf,"longitude",lon,sizeof(lon))<0){ set_err(err,err_size,"JSON lon"); return -1; }
    if(json_find_long(g_http_buf,"timestamp",&ts)<0){ set_err(err,err_size,"JSON time"); return -1; }
    if(json_parse_centideg(lat,&lat_cd)<0){ set_err(err,err_size,"Bad lat"); return -1; }
    if(json_parse_centideg(lon,&lon_cd)<0){ set_err(err,err_size,"Bad lon"); return -1; }
    pos->lat_cd=lat_cd; pos->lon_cd=lon_cd; pos->timestamp=ts; pos->valid=1; return 0;
}

static LONG parse_whereiss(IssTrackerApp *app,IssPosition *pos,char *err,UWORD err_size)
{
    char lat[24]; char lon[24]; char vis[24]; ULONG ts; WORD lat_cd; WORD lon_cd;
    if(json_find_token(g_http_buf,"latitude",lat,sizeof(lat))<0){ set_err(err,err_size,"HTTPS JSON lat"); return -1; }
    if(json_find_token(g_http_buf,"longitude",lon,sizeof(lon))<0){ set_err(err,err_size,"HTTPS JSON lon"); return -1; }
    if(json_find_long(g_http_buf,"timestamp",&ts)<0){ set_err(err,err_size,"HTTPS JSON time"); return -1; }
    if(json_parse_centideg(lat,&lat_cd)<0){ set_err(err,err_size,"Bad HTTPS lat"); return -1; }
    if(json_parse_centideg(lon,&lon_cd)<0){ set_err(err,err_size,"Bad HTTPS lon"); return -1; }
    pos->lat_cd=lat_cd; pos->lon_cd=lon_cd; pos->timestamp=ts; pos->valid=1;
    app->enhanced_valid=1;
    app->visibility=2;
    if(json_find_token(g_http_buf,"visibility",vis,sizeof(vis))==0){ if(strcmp(vis,"daylight")==0 || strcmp(vis,"visible")==0 || strcmp(vis,"day")==0) app->visibility=0; else if(strcmp(vis,"eclipsed")==0 || strcmp(vis,"night")==0 || strcmp(vis,"dark")==0) app->visibility=1; }
    parse_uword_token(g_http_buf,"altitude",&app->altitude_km);
    parse_uword_token(g_http_buf,"velocity",&app->velocity_kmh);
    parse_uword_token(g_http_buf,"footprint",&app->footprint_km);
    if(json_find_token(g_http_buf,"solar_lat",lat,sizeof(lat))==0 && json_parse_centideg(lat,&app->solar_lat_cd)<0) app->solar_lat_cd=0;
    if(json_find_token(g_http_buf,"solar_lon",lon,sizeof(lon))==0 && json_parse_centideg(lon,&app->solar_lon_cd)<0) app->solar_lon_cd=0;
    return 0;
}

LONG iss_fetch_now(IssTrackerApp *app,IssPosition *pos,char *err,UWORD err_size)
{
    if(app) app->enhanced_valid=0;
    if(app && ensure_tls(app)){
        DeleteFile((STRPTR)HTTPS_TMP_FILE);
        if(tls_http_get_call("https://api.wheretheiss.at/v1/satellites/25544",HTTPS_TMP_FILE,AMITLS13F_INSECURE)>=0 && read_file_to_buf(HTTPS_TMP_FILE,g_http_buf,sizeof(g_http_buf),err,err_size)>0){
            DeleteFile((STRPTR)HTTPS_TMP_FILE);
            if(parse_whereiss(app,pos,err,err_size)==0) return 0;
        }
        DeleteFile((STRPTR)HTTPS_TMP_FILE);
    }
    if(app) app->enhanced_valid=0;
    if(http_get_plain("api.open-notify.org","/iss-now.json",g_http_buf,sizeof(g_http_buf),err,err_size)<0) return -1;
    return parse_open_notify(pos,err,err_size);
}

static LONG range_find_string(const char *start,const char *end,const char *key,char *out,UWORD out_size)
{
    const char *p; UWORD n; p=start; while(p<end){ if(*p=='\"' && streq_at(p+1,key)){ p++; while(p<end && *p!='\"') p++; if(p<end){ p++; while(p<end && is_ws(*p)) p++; if(p<end && *p==':'){ p++; while(p<end && is_ws(*p)) p++; if(p<end && *p=='\"'){ p++; n=0; while(p<end && *p!='\"' && n+1<out_size) out[n++]=*p++; out[n]=0; return 0; } } } } p++; } return -1;
}

static LONG range_find_ulong(const char *start,const char *end,const char *key,ULONG *out)
{
    const char *p; ULONG v; UBYTE got; p=start; while(p<end){ if(*p=='\"' && streq_at(p+1,key)){ p++; while(p<end && *p!='\"') p++; if(p<end){ p++; while(p<end && is_ws(*p)) p++; if(p<end && *p==':'){ p++; while(p<end && is_ws(*p)) p++; v=0; got=0; while(p<end && *p>='0' && *p<='9'){ got=1; v=(v*10UL)+(ULONG)(*p-'0'); p++; } if(got){ *out=v; return 0; } return -1; } } } p++; } return -1;
}

LONG iss_fetch_crew(IssTrackerApp *app,char *err,UWORD err_size)
{
    const char *p; const char *end; char name[32]; char craft[16]; UWORD count; ULONG number;
    if(!app) return -1;
    if(http_get_plain("api.open-notify.org","/astros.json",g_http_buf,sizeof(g_http_buf),err,err_size)<0) return -1;
    app->crew_count=0; app->people_in_space=0; count=0; number=0;
    if(range_find_ulong(g_http_buf,g_http_buf+strlen(g_http_buf),"number",&number)==0 && number<=64UL) app->people_in_space=(UWORD)number;
    p=strstr(g_http_buf,"\"people\"");
    if(!p) return 0;
    while((p=strchr(p,'{')) && count<ISS_CREW_MAX){
        end=strchr(p,'}');
        if(!end) break;
        if(range_find_string(p,end,"name",name,sizeof(name))==0 && range_find_string(p,end,"craft",craft,sizeof(craft))==0){
            strcpy(app->crew[count].name,name);
            strcpy(app->crew[count].craft,craft);
            count++;
        }
        p=end+1;
    }
    app->crew_count=count;
    if(app->people_in_space==0 || app->people_in_space>64 || app->people_in_space<count) app->people_in_space=count;
    return 0;
}
