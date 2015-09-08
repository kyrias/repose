#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <archive.h>
#include <dirent.h>
#include <err.h>

#define _unlikely_(x)       __builtin_expect(!!(x), 1)
#define _unused_            __attribute__((unsused))
#define _noreturn_          __attribute__((noreturn))
#define _cleanup_(x)        __attribute__((cleanup(x)))
#define _printf_(a,b)       __attribute__((format (printf, a, b)))
#define _sentinel_          __attribute__((sentinel))
#define _cleanup_free_      _cleanup_(freep)
#define _cleanup_fclose_    _cleanup_(fclosep)
#define _cleanup_closedir_  _cleanup_(closedirp)
#define _cleanup_close_     _cleanup_(closep)

/* XXX: clang does not have generic builtin */
#if __clang__
#define __builtin_add_overflow(a, b, r) _Generic((a), \
    int: __builtin_sadd_overflow, \
    long int: __builtin_saddl_overflow, \
    long long: __builtin_saddll_overflow, \
    unsigned int: __builtin_uadd_overflow, \
    unsigned long int: __builtin_uaddl_overflow, \
    unsigned long long: __builtin_uaddll_overflow)(a, b, r)
#endif

static inline void freep(void *p)      { free(*(void **)p); }
static inline void fclosep(FILE **fp)  { if (*fp) fclose(*fp); }
static inline void closedirp(DIR **dp) { if (*dp) closedir(*dp); }
static inline void closep(int *fd)     { if (*fd >= 0) close(*fd); }

static inline bool streq(const char *s1, const char *s2) { return strcmp(s1, s2) == 0; }

void trace(const char *fmt, ...) _printf_(1, 2);
void check_posix(intmax_t rc, const char *fmt, ...) _printf_(2, 3);
void check_null(const void *ptr, const char *fmt, ...) _printf_(2, 3);

FILE *fopenat(int dirfd, const char *path, const char *mode);

char *joinstring(const char *root, ...) _sentinel_;

#define fromstr(str, out) _Generic((*out), long: xstrtol, unsigned long: xstrtoul)(str, out)
int xstrtol(const char *str, long *out);
int xstrtoul(const char *str, unsigned long *out);

char *sha256_file(int dirfd, const char *filename);

char *strstrip(char *s);
