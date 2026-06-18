#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include "amitcp13/bsdsocket.h"
#include "http_client.h"
extern struct Library *SocketBase;
static LONG Sock_socket(LONG d,LONG t,LONG p){ register struct Library *a6 __asm("a6")=SocketBase; register LONG d0 __asm("d0")=d; register LONG d1 __asm("d1")=t; register LONG d2 __asm("d2")=p; __asm volatile("jsr a6@(-30)" : "+r"(d0) : "r"(a6),"r"(d1),"r"(d2) : "d3","d4","d5","a0","a1","cc","memory"); return d0; }
static LONG Sock_connect(LONG s,struct sockaddr *a,LONG l){ register struct Library *a6 __asm("a6")=SocketBase; register LONG d0 __asm("d0")=s; register struct sockaddr *a0 __asm("a0")=a; register LONG d1 __asm("d1")=l; __asm volatile("jsr a6@(-36)" : "+r"(d0) : "r"(a6),"r"(a0),"r"(d1) : "d2","d3","d4","d5","a1","cc","memory"); return d0; }
static LONG Sock_send(LONG s,const char *b,LONG l,LONG f){ register struct Library *a6 __asm("a6")=SocketBase; register LONG d0 __asm("d0")=s; register const char *a0 __asm("a0")=b; register LONG d1 __asm("d1")=l; register LONG d2 __asm("d2")=f; __asm volatile("jsr a6@(-42)" : "+r"(d0) : "r"(a6),"r"(a0),"r"(d1),"r"(d2) : "d3","d4","d5","a1","cc","memory"); return d0; }
static LONG Sock_recv(LONG s,char *b,LONG l,LONG f){ register struct Library *a6 __asm("a6")=SocketBase; register LONG d0 __asm("d0")=s; register char *a0 __asm("a0")=b; register LONG d1 __asm("d1")=l; register LONG d2 __asm("d2")=f; __asm volatile("jsr a6@(-48)" : "+r"(d0) : "r"(a6),"r"(a0),"r"(d1),"r"(d2) : "d3","d4","d5","a1","cc","memory"); return d0; }
static LONG Sock_close(LONG s){ register struct Library *a6 __asm("a6")=SocketBase; register LONG d0 __asm("d0")=s; __asm volatile("jsr a6@(-72)" : "+r"(d0) : "r"(a6) : "d1","d2","d3","d4","d5","a0","a1","cc","memory"); return d0; }
static struct hostent *Sock_gethostbyname(const char *n){ register struct Library *a6 __asm("a6")=SocketBase; register const char *a0 __asm("a0")=n; register struct hostent *d0 __asm("d0"); __asm volatile("jsr a6@(-78)" : "=r"(d0) : "r"(a6),"r"(a0) : "d1","d2","d3","d4","d5","a1","cc","memory"); return d0; }
static LONG Sock_WaitSelect(LONG n,fd_set *r,fd_set *w,fd_set *e,struct timeval *tv,ULONG *m){ register struct Library *a6 __asm("a6")=SocketBase; register LONG d0 __asm("d0")=n; register fd_set *a0 __asm("a0")=r; register fd_set *a1 __asm("a1")=w; register fd_set *a2 __asm("a2")=e; register struct timeval *a3 __asm("a3")=tv; register ULONG *d1 __asm("d1")=m; __asm volatile("jsr a6@(-132)" : "+r"(d0) : "r"(a6),"r"(a0),"r"(a1),"r"(a2),"r"(a3),"r"(d1) : "d2","d3","d4","d5","cc","memory"); return d0; }
static void errset(char *err,UWORD sz,const char *s){ UWORD i; if(!err||!sz) return; for(i=0;s[i]&&i+1<sz;i++) err[i]=s[i]; err[i]=0; }
static UWORD slen(const char *s){ UWORD n; n=0; while(s[n]) n++; return n; }
static void add(char *d,UWORD *i,UWORD max,const char *s){ while(*s && *i+1<max) d[(*i)++]=*s++; d[*i]=0; }
LONG http_get_plain(const char *host,const char *path,char *out,UWORD out_size,char *err,UWORD err_size){
    struct hostent *he; LONG fd; struct sockaddr_in sa; char req[192]; char chunk[128]; LONG got; LONG total; LONG hdr; UWORD i; UWORD rq;
    if(!SocketBase){ errset(err,err_size,"socket.library missing"); return -1; }
    he=Sock_gethostbyname(host); if(!he || !he->h_addr){ errset(err,err_size,"DNS failed"); return -1; }
    fd=Sock_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP); if(fd<0){ errset(err,err_size,"socket failed"); return -1; }
    memset(&sa,0,sizeof(sa)); sa.sin_family=AF_INET; sa.sin_port=80; sa.sin_addr.s_addr=*((ULONG*)he->h_addr);
    if(Sock_connect(fd,(struct sockaddr*)&sa,sizeof(sa))<0){ errset(err,err_size,"connect failed"); Sock_close(fd); return -1; }
    rq=0; add(req,&rq,sizeof(req),"GET "); add(req,&rq,sizeof(req),path); add(req,&rq,sizeof(req)," HTTP/1.0\r\nHost: "); add(req,&rq,sizeof(req),host); add(req,&rq,sizeof(req),"\r\nConnection: close\r\nUser-Agent: ISSTracker13\r\n\r\n");
    if(Sock_send(fd,req,slen(req),0)<0){ errset(err,err_size,"send failed"); Sock_close(fd); return -1; }
    total=0; hdr=0;
    while(1){ fd_set rf; struct timeval tv; FD_ZERO(&rf); FD_SET(fd,&rf); tv.tv_sec=8; tv.tv_usec=0; if(Sock_WaitSelect(fd+1,&rf,0,0,&tv,0)<=0){ errset(err,err_size,"receive timeout"); Sock_close(fd); return -1; } got=Sock_recv(fd,chunk,sizeof(chunk),0); if(got==0) break; if(got<0){ errset(err,err_size,"receive failed"); Sock_close(fd); return -1; } for(i=0;i<(UWORD)got;i++){ if(!hdr){ if(total>=3 && out[total-3]=='\r' && out[total-2]=='\n' && out[total-1]=='\r' && chunk[i]=='\n'){ hdr=1; total=0; } else if(total+1<out_size) out[total++]=chunk[i]; } else if(total+1<out_size) out[total++]=chunk[i]; } }
    out[total]=0; Sock_close(fd); if(!hdr){ errset(err,err_size,"bad HTTP"); return -1; } return total;
}
