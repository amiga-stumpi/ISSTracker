#ifndef ISS_JSON_MIN_H
#define ISS_JSON_MIN_H
#include <exec/types.h>
LONG json_find_string(const char *json, const char *key, char *out, UWORD out_size);
LONG json_find_long(const char *json, const char *key, ULONG *out);
WORD json_parse_centideg(const char *s, WORD *out);
#endif
