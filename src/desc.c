#include "desc.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <err.h>
#include <archive.h>

#ifndef TRAVIS_CI
#include <alpm_list.h>
#endif

#include "reader.h"
#include "package.h"
#include "util.h"

struct reader {
    const char *buf;
    size_t off;
    size_t len;
};

static ssize_t _getline(struct reader *reader, const char **line, size_t *len)
{
    *line = &reader->buf[reader->off];
    *len = strcspn(*line, "\n");

    if (reader->buf[reader->off + *len] == '\0') {
        reader->off += *len;
        return -1;
    }

    reader->off += *len + 1;
    return 0;
}

static inline int read_desc_list(struct reader *reader, alpm_list_t **list)
{
    const char *line;
    size_t line_len;
    while (_getline(reader, &line, &line_len) == 0) {
        if (line_len == 0)
            break;
        *list = alpm_list_add(*list, strndup(line, line_len));
    }
    return 0;
}

static inline int read_desc_entry(struct reader *reader, char **data)
{
    const char *line;
    size_t line_len;
    _getline(reader, &line, &line_len);
    *data = strndup(line, line_len);
    return 0;
}

static inline int read_desc_size(struct reader *reader, size_t *data)
{
    const char *line;
    size_t line_len;
    _getline(reader, &line, &line_len);
    {
        // TODO: fix
        _cleanup_free_ char *size = strndup(line, line_len);
        fromstr(size, data);
    }
    return 0;
}

static inline int read_desc_time(struct reader *reader, time_t *data)
{
    const char *line;
    size_t line_len;
    _getline(reader, &line, &line_len);
    {
        // TODO: fix
        _cleanup_free_ char *time = strndup(line, line_len);
        fromstr(time, data);
    }
    return 0;
}

ssize_t parse_pkginfo(struct pkginfo_parser *parser, struct pkg *pkg,
                      char *buf, size_t buf_len)
{
    struct reader reader = {.buf = buf, .len = buf_len};

    for (;;) {
        const char *line;
        size_t line_len;
        ssize_t nbytes_r;

        if (_getline(&reader, &line, &line_len) < 0)
            break;
        else if (line_len == 0)
            continue;

        if (strneq(line, "%FILENAME%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->filename);
        } else if (strneq(line, "%NAME%", line_len)) {
            const char *name;
            size_t name_len;
            _getline(&reader, &name, &name_len);
            if (!strneq(name, pkg->name, name_len))
                errx(EXIT_FAILURE, "database entry %%NAME%% and desc record are mismatched!");
        } else if (strneq(line, "%BASE%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->base);
        } else if (strneq(line, "%VERSION%", line_len)) {
            const char *version;
            size_t version_len;
            _getline(&reader, &version, &version_len);
            if (!strneq(version, pkg->version, version_len))
                errx(EXIT_FAILURE, "database entry %%VERSION%% and desc record are mismatched!");
        } else if (strneq(line, "%DESC%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->desc);
        } else if (strneq(line, "%GROUPS%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->groups);
        } else if (strneq(line, "%CSIZE%", line_len)) {
            nbytes_r = read_desc_size(&reader, &pkg->size);
        } else if (strneq(line, "%ISIZE%", line_len)) {
            nbytes_r = read_desc_size(&reader, &pkg->isize);
        } else if (strneq(line, "%SHA256SUM%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->sha256sum);
        } else if(strneq(line, "%PGPSIG%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->base64sig);
        } else if (strneq(line, "%URL%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->url);
        } else if (strneq(line, "%LICENSE%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->licenses);
        } else if (strneq(line, "%ARCH%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->arch);
        } else if (strneq(line, "%BUILDDATE%", line_len)) {
            nbytes_r = read_desc_time(&reader, &pkg->builddate);
        } else if (strneq(line, "%PACKAGER%", line_len)) {
            nbytes_r = read_desc_entry(&reader, &pkg->packager);
        } else if (strneq(line, "%REPLACES%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->replaces);
        } else if (strneq(line, "%DEPENDS%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->depends);
        } else if (strneq(line, "%CONFLICTS%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->conflicts);
        } else if (strneq(line, "%PROVIDES%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->provides);
        } else if (strneq(line, "%OPTDEPENDS%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->optdepends);
        } else if (strneq(line, "%MAKEDEPENDS%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->makedepends);
        } else if (strneq(line, "%CHECKDEPENDS%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->checkdepends);
        } else if (strneq(line, "%FILES%", line_len)) {
            nbytes_r = read_desc_list(&reader, &pkg->files);
        } else {
            printf("UNMATCHED: %.*s\n", (int)line_len, line);
        }
    }

    /* free(reader); */
    return 0;
}

static int archive_read_block(struct archive *archive, char *buf, size_t *buf_len)
{
    for (;;) {
        int status = archive_read_data_block(archive, (void *)&buf,
                                             buf_len, &(int64_t){0});

        if (status == ARCHIVE_RETRY)
            continue;
        if (status <= ARCHIVE_WARN)
            warnx("%s", archive_error_string(archive));

        return status;
    }
}

void read_desc(struct archive *archive, struct pkg *pkg)
{
    struct pkginfo_parser parser = {0};
    char buf[8192];

    for (;;) {
        size_t nbytes_r = sizeof(buf);
        archive_read_block(archive, buf, &nbytes_r);
        parse_pkginfo(&parser, pkg, buf, nbytes_r);
    }
}
