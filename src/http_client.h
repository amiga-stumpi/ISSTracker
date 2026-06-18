#ifndef ISS_HTTP_CLIENT_H
#define ISS_HTTP_CLIENT_H
#include <exec/types.h>
LONG http_get_plain(const char *host, const char *path, char *out, UWORD out_size, char *err, UWORD err_size);
#endif
