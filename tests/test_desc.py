import pytest
import weakref


REPOSE_DESC = b'''%FILENAME%
repose-git-5.19.g82c3d4a-1-x86_64.pkg.tar.xz

%NAME%
repose-git

%VERSION%
5.19.g82c3d4a-1

%DESC%
A archlinux repo building tool

%CSIZE%
18804

%ISIZE%
51200

%SHA256SUM%
4045b3b24bae8a2d811323e5dd3727345e9e6f81788c65d5935d07b2ee06b505

%URL%
http://github.com/vodik/repose

%LICENSE%
GPL

%ARCH%
x86_64

%BUILDDATE%
1448690669

%PACKAGER%
Simon Gomizelj <simongmzlj@gmail.com>
'''

REPOSE_DEPENDS = b'''%DEPENDS%
pacman
libarchive
gnupg

%CONFLICTS%
repose

%PROVIDES%
repose

%MAKEDEPENDS%
git
'''


global_weakkeydict = weakref.WeakKeyDictionary()
def new_pkg(ffi, name, version):
    name = ffi.new('char[]', name)
    version = ffi.new('char[]', version)
    pkg = ffi.new('struct pkg*', {'name': name,
                                  'version': version})

    global_weakkeydict[pkg] = (name, version)
    return pkg


def read_alpm_string_list(ffi, data):
    def worker(data):
        while data != ffi.NULL:
            yield ffi.string(ffi.cast('char*', data.data))
            data = data.next
    return list(worker(data))


@pytest.fixture
def parser(ffi, lib):
    parser = ffi.new('struct pkginfo_parser*', {'state': 0})
    return parser


def test_parse_desc(ffi, lib, parser):
    pkg = new_pkg(ffi, b'repose-git', b'5.19.g82c3d4a-1')
    lib.parse_pkginfo(parser, pkg, REPOSE_DESC, len(REPOSE_DESC))
    assert parser.state == lib.PKGINFO_INITIAL

    assert pkg.base == ffi.NULL
    assert pkg.base64sig == ffi.NULL
    assert ffi.string(pkg.filename) == b'repose-git-5.19.g82c3d4a-1-x86_64.pkg.tar.xz'
    assert ffi.string(pkg.desc) == b'A archlinux repo building tool'
    assert pkg.size == 18804
    assert pkg.isize == 51200
    assert ffi.string(pkg.sha256sum) == b'4045b3b24bae8a2d811323e5dd3727345e9e6f81788c65d5935d07b2ee06b505'
    assert ffi.string(pkg.url) == b'http://github.com/vodik/repose'
    assert ffi.string(pkg.arch) == b'x86_64'
    assert pkg.builddate == 1448690669
    assert ffi.string(pkg.packager) == b'Simon Gomizelj <simongmzlj@gmail.com>'

    licenses = read_alpm_string_list(ffi, pkg.licenses)
    assert licenses == [b'GPL']


def test_parse_depends(ffi, lib, parser):
    pkg = new_pkg(ffi, b'repose-git', b'5.19.g82c3d4a-1')
    lib.parse_pkginfo(parser, pkg, REPOSE_DEPENDS, len(REPOSE_DESC))
    assert parser.state == lib.PKGINFO_INITIAL

    depends = read_alpm_string_list(ffi, pkg.depends)
    conflicts = read_alpm_string_list(ffi, pkg.conflicts)
    provides = read_alpm_string_list(ffi, pkg.provides)
    makedepends = read_alpm_string_list(ffi, pkg.makedepends)
    assert depends == [b'pacman', b'libarchive', b'gnupg']
    assert conflicts == [b'repose']
    assert provides == [b'repose']
    assert makedepends == [b'git']
