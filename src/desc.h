#pragma once

#include <sys/types.h>

struct pkg;
struct archive;

enum state {
    PKGINFO_INITIAL = 0,
    PKGINFO_FILENAME,
    PKGINFO_NAME,
    PKGINFO_BASE,
    PKGINFO_VERSION,
    PKGINFO_DESC,
    PKGINFO_GROUPS,
    PKGINFO_CSIZE,
    PKGINFO_ISIZE,
    PKGINFO_SHA256SUM,
    PKGINFO_PGPSIG,
    PKGINFO_URL,
    PKGINFO_LICENSE,
    PKGINFO_ARCH,
    PKGINFO_BUILDDATE,
    PKGINFO_PACKAGER,
    PKGINFO_REPLACES,
    PKGINFO_DEPENDS,
    PKGINFO_CONFLICTS,
    PKGINFO_PROVIDES,
    PKGINFO_OPTDEPENDS,
    PKGINFO_MAKEDEPENDS,
    PKGINFO_CHECKDEPENDS,
    PKGINFO_FILES
};

struct pkginfo_parser {
    enum state state;
};

void read_desc(struct archive *archive, struct pkg *pkg);
ssize_t parse_pkginfo(struct pkginfo_parser *parser, struct pkg *pkg,
                      char *buf, size_t buf_len);
