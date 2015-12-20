#define SIZE_MAX ...

typedef struct __alpm_list_t {
    void *data;
    struct __alpm_list_t *next;
    ...;
} alpm_list_t;

typedef int... time_t;

char *strptime(const char *s, const char *format, struct tm *tm);

struct pkg {
    unsigned long name_hash;
    char *filename;
    char *name;
    char *base;
    char *version;
    char *desc;
    char *url;
    char *packager;
    char *sha256sum;
    char *base64sig;
    char *arch;
    size_t size;
    size_t isize;
    time_t builddate;
    time_t mtime;

    alpm_list_t *groups;
    alpm_list_t *licenses;
    alpm_list_t *replaces;
    alpm_list_t *depends;
    alpm_list_t *conflicts;
    alpm_list_t *provides;
    alpm_list_t *optdepends;
    alpm_list_t *makedepends;
    alpm_list_t *checkdepends;
    alpm_list_t *files;
};

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

// utils
char *joinstring(const char *root, ...);
int str_to_size(const char *str, size_t *out);
int str_to_time(const char *size, time_t *out);
char *strstrip(char *s);

// desc
ssize_t parse_pkginfo(struct pkginfo_parser *parser, struct pkg *pkg,
                      char *buf, size_t buf_len);
