#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MAX(V,M)       ((V)>(M)?(M):(V))
#define MIN(V,M)       ((V)<(M)?(M):(V))
#define CLAMP(V,H,L)   (MIN(MAX(V,H),L))
#define CHKSUB(A,B)    (((A)-(B))>(A)?0:((A)-(B)))
#define UNUSED(VAR)    ((void) (VAR))
#define ARRAY_LEN(ARR) ((size_t)(sizeof(ARR)/sizeof(*(ARR))))
#define BITSET(V,B)    (((V) & (B)) == (B))

void *ecalloc(size_t nmemb, size_t size);

/* a reimplementation of assert(3) that calls die() instead of abort(3) */
#define ENSURE(EXPR) (__ensure((EXPR), #EXPR, __FILE__, __LINE__, __func__))
void __ensure(_Bool expr, char *str, char *file, size_t line, const char *fn);

void die(const char *fmt, ...);
char *format(const char *format, ...);

#endif
