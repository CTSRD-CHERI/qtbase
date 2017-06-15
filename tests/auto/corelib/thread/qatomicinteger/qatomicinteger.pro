TEMPLATE=subdirs
SUBDIRS=\
    char \
    char16_t \
    char32_t \
    int \
    long \
    qlonglong \
    qulonglong \
    schar \
    short \
    uchar \
    uint \
    ulong \
    ushort \
    wchar_t \

# TODO: FIX these
# TODO: __uintcap_t and __intcap_t for hybrid
# For some reason there is no __atomic_sub_fetch and not __atomic_fetch_add for CHERI in compiler_rt
# SUBDIRS+= qptrdiff quintptr
