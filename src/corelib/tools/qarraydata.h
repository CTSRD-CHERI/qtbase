/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QARRAYDATA_H
#define QARRAYDATA_H

#include <QtCore/qrefcount.h>
#include <string.h>
#include <cstdint>

#ifdef __CHERI_PURE_CAPABILITY__
#undef QARRAYDATA_DEBUG_OUTPUT
#endif
#ifdef QARRAYDATA_DEBUG_OUTPUT
#include <cstdio>
#define qarraydata_dbg(...) fprintf(stderr, __VA_ARGS__)
#else
#define qarraydata_dbg(...) do {} while(false)
#endif

QT_BEGIN_NAMESPACE

struct Q_CORE_EXPORT QArrayData
{
    QtPrivate::RefCount ref;
    int size;
    uint alloc : 31;
    uint capacityReserved : 1;

#ifndef __CHERI_PURE_CAPABILITY__
    qptrdiff offset; // in bytes from beginning of header
    inline qptrdiff dataOffset() const { return offset; }
    void setOffset(qptrdiff newOffset) { offset = newOffset; }
#else
    // FIXME: See if we can make this work somehow without runtime relocs
    // See https://woboq.com/blog/qstringliteral.html
    qintptr _internal_cheri_offset;
    void setOffset(qptrdiff offset) { _internal_cheri_offset = offset; }
    void setPointer(const void *data)
    {
        Q_ASSERT(__builtin_cheri_tag_get(data) && "Setting untagged pointer?");
        _internal_cheri_offset = reinterpret_cast<quintptr>(data);
    }
    inline qptrdiff dataOffset() const
    {
        void *ptr = reinterpret_cast<void *>(_internal_cheri_offset);
        if (__builtin_cheri_tag_get(ptr))
            return static_cast<const char *>(ptr) - reinterpret_cast<const char *>(this);
        // otherwise just return the offset:
        return qptrdiff(_internal_cheri_offset);
    }
    static inline void *cheri_get_arraydata(const QArrayData *obj, quintptr offset, size_t objsize)
    {
        // For CHERI we can't just always add the offset to this since that may be be out-of-bounds
        void *ret = reinterpret_cast<void *>(offset);
        qarraydata_dbg("%s(%#p, %zd): %#p\n", __func__, static_cast<const void *>(obj), objsize,
                       ret);
        if (__builtin_cheri_tag_get(ret))
            return ret; // valid pointer so can just return

        // The generated QMetaObject tables uses large offsets so just assert that the length is
        // sensible.
        Q_ASSERT(offset < __builtin_cheri_length_get(obj));
        ret = const_cast<char *>(reinterpret_cast<const char *>(obj) + qint64(offset));
        qarraydata_dbg("Remaining bytes in buffer: %ld (obj = %#p), obj->alloc=%d, obj->size=%d\n",
                       cheri_bytes_remaining(ret), static_cast<const void *>(obj), obj->alloc,
                       obj->size);

        // If we have an allocation set the bounds of data() to the size of the allocation so that
        // modifying code like QString::append() works.
        // TODO: for the const overload this should probably always be just the size (+1 for the
        // zero terminator) to avoid accidental modification?
        size_t bounds = (obj->alloc ? obj->alloc : obj->size) * objsize;
        // We could set alloc == size + 1 for QByteArrayLiteral/QStringLiteral, but for the
        // shared_null case we have to set alloc==0 since shared_null is also used for
        // types > sizeof(QArrayData) and setting it to one would cause a bounds violation.
        // For QByteArray/Qstring we have to ensure that qrintable(QByteArray()) and
        // qUtf16Printable(QString()) return a single '\0' character and not a zero-size capability,
        // so that that they can be passed to printf()/QString::vasprintf().
        // TO avoid many other changes just assume that alloc==0+static+size<2 means a constant
        // zero-terminated string -> set bounds to size+1
        if (Q_UNLIKELY(obj->alloc == 0 && objsize <= 2 && obj->ref.isStatic())) {
            qarraydata_dbg("%s: Adding nul byte since data is static\n", __func__);
            return __builtin_cheri_bounds_set(ret, bounds + objsize);
        }
#ifndef QT_CHERI_NO_SET_BOUNDS
        ret = __builtin_cheri_bounds_set(ret, bounds);
#endif
        qarraydata_dbg("%s with size(%zd): %#p\n", __func__, bounds, ret);
        return ret;
    }
#endif
    void *boundedData(size_t objsize)
    {
#ifndef __CHERI_PURE_CAPABILITY__
        Q_UNUSED(objsize);
        Q_ASSERT(size == 0 || offset < 0 || size_t(offset) >= sizeof(QArrayData));
        return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(this) + offset);
#else
        Q_ASSERT(size == 0 || reinterpret_cast<void *>(_internal_cheri_offset));
        return cheri_get_arraydata(this, _internal_cheri_offset, objsize);
#endif
    }
    const void *boundedData(size_t objsize) const
    {
#ifndef __CHERI_PURE_CAPABILITY__
        Q_UNUSED(objsize);
        Q_ASSERT(size == 0 || offset < 0 || size_t(offset) >= sizeof(QArrayData));
        return reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(this) + offset);
#else
        Q_ASSERT(size == 0 || reinterpret_cast<void *>(_internal_cheri_offset));
        return cheri_get_arraydata(this, _internal_cheri_offset, objsize);
#endif
    }

    // This refers to array data mutability, not "header data" represented by
    // data members in QArrayData. Shared data (array and header) must still
    // follow COW principles.
    bool isMutable() const
    {
        return alloc != 0;
    }

    enum AllocationOption {
        CapacityReserved    = 0x1,
#if !defined(QT_NO_UNSHARABLE_CONTAINERS)
        Unsharable          = 0x2,
#endif
        RawData             = 0x4,
        Grow                = 0x8,
        WithNulTerminator   = 0x10,

        Default = 0
    };

    Q_DECLARE_FLAGS(AllocationOptions, AllocationOption)

    size_t detachCapacity(size_t newSize) const
    {
        if (capacityReserved && newSize < alloc)
            return alloc;
        return newSize;
    }

    AllocationOptions detachFlags() const
    {
        AllocationOptions result;
        if (capacityReserved)
            result |= CapacityReserved;
        return result;
    }

    AllocationOptions cloneFlags() const
    {
        AllocationOptions result;
        if (capacityReserved)
            result |= CapacityReserved;
        return result;
    }

    Q_REQUIRED_RESULT static QArrayData *allocate(size_t objectSize, size_t alignment,
            size_t capacity, AllocationOptions options = Default) noexcept;
    Q_REQUIRED_RESULT static QArrayData *reallocateUnaligned(QArrayData *data, size_t objectSize,
            size_t newCapacity, AllocationOptions newOptions = Default) noexcept;
    static void deallocate(QArrayData *data, size_t objectSize,
            size_t alignment) noexcept;

    static const QArrayData shared_null[2];
    static QArrayData *sharedNull() noexcept { return const_cast<QArrayData*>(shared_null); }
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QArrayData::AllocationOptions)

template <class T>
struct QTypedArrayData
    : QArrayData
{
#ifdef QT_STRICT_ITERATORS
    class iterator {
    public:
        T *i;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef int difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline iterator() : i(nullptr) {}
        inline iterator(T *n) : i(n) {}
        inline iterator(const iterator &o): i(o.i){} // #### Qt 6: remove, the implicit version is fine
        inline T &operator*() const { return *i; }
        inline T *operator->() const { return i; }
        inline T &operator[](int j) const { return *(i + j); }
        inline bool operator==(const iterator &o) const { return i == o.i; }
        inline bool operator!=(const iterator &o) const { return i != o.i; }
        inline bool operator<(const iterator& other) const { return i < other.i; }
        inline bool operator<=(const iterator& other) const { return i <= other.i; }
        inline bool operator>(const iterator& other) const { return i > other.i; }
        inline bool operator>=(const iterator& other) const { return i >= other.i; }
        inline iterator &operator++() { ++i; return *this; }
        inline iterator operator++(int) { T *n = i; ++i; return n; }
        inline iterator &operator--() { i--; return *this; }
        inline iterator operator--(int) { T *n = i; i--; return n; }
        inline iterator &operator+=(int j) { i+=j; return *this; }
        inline iterator &operator-=(int j) { i-=j; return *this; }
        inline iterator operator+(int j) const { return iterator(i+j); }
        inline iterator operator-(int j) const { return iterator(i-j); }
        friend inline iterator operator+(int j, iterator k) { return k + j; }
        inline int operator-(iterator j) const { return i - j.i; }
        inline operator T*() const { return i; }
    };
    friend class iterator;

    class const_iterator {
    public:
        const T *i;
        typedef std::random_access_iterator_tag  iterator_category;
        typedef int difference_type;
        typedef T value_type;
        typedef const T *pointer;
        typedef const T &reference;

        inline const_iterator() : i(nullptr) {}
        inline const_iterator(const T *n) : i(n) {}
        inline const_iterator(const const_iterator &o): i(o.i) {} // #### Qt 6: remove, the default version is fine
        inline explicit const_iterator(const iterator &o): i(o.i) {}
        inline const T &operator*() const { return *i; }
        inline const T *operator->() const { return i; }
        inline const T &operator[](int j) const { return *(i + j); }
        inline bool operator==(const const_iterator &o) const { return i == o.i; }
        inline bool operator!=(const const_iterator &o) const { return i != o.i; }
        inline bool operator<(const const_iterator& other) const { return i < other.i; }
        inline bool operator<=(const const_iterator& other) const { return i <= other.i; }
        inline bool operator>(const const_iterator& other) const { return i > other.i; }
        inline bool operator>=(const const_iterator& other) const { return i >= other.i; }
        inline const_iterator &operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const T *n = i; ++i; return n; }
        inline const_iterator &operator--() { i--; return *this; }
        inline const_iterator operator--(int) { const T *n = i; i--; return n; }
        inline const_iterator &operator+=(int j) { i+=j; return *this; }
        inline const_iterator &operator-=(int j) { i-=j; return *this; }
        inline const_iterator operator+(int j) const { return const_iterator(i+j); }
        inline const_iterator operator-(int j) const { return const_iterator(i-j); }
        friend inline const_iterator operator+(int j, const_iterator k) { return k + j; }
        inline int operator-(const_iterator j) const { return i - j.i; }
        inline operator const T*() const { return i; }
    };
    friend class const_iterator;
#else
    typedef T* iterator;
    typedef const T* const_iterator;
#endif

    T *data() { return static_cast<T *>(QArrayData::boundedData(sizeof(T))); }
    const T *data() const { return static_cast<const T *>(QArrayData::boundedData(sizeof(T))); }

    iterator begin(iterator = iterator()) { return data(); }
    iterator end(iterator = iterator()) { return data() + size; }
    const_iterator begin(const_iterator = const_iterator()) const { return data(); }
    const_iterator end(const_iterator = const_iterator()) const { return data() + size; }
    const_iterator constBegin(const_iterator = const_iterator()) const { return data(); }
    const_iterator constEnd(const_iterator = const_iterator()) const { return data() + size; }

    class AlignmentDummy { QArrayData header; T data; };

    Q_REQUIRED_RESULT static QTypedArrayData *allocate(size_t capacity,
            AllocationOptions options = Default)
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        return static_cast<QTypedArrayData *>(QArrayData::allocate(sizeof(T),
                    Q_ALIGNOF(AlignmentDummy), capacity, options));
    }

    static QTypedArrayData *reallocateUnaligned(QTypedArrayData *data, size_t capacity,
            AllocationOptions options = Default)
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        return static_cast<QTypedArrayData *>(QArrayData::reallocateUnaligned(data, sizeof(T),
                    capacity, options));
    }

    static void deallocate(QArrayData *data)
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        QArrayData::deallocate(data, sizeof(T), Q_ALIGNOF(AlignmentDummy));
    }

    static QTypedArrayData *fromRawData(const T *data, size_t n,
            AllocationOptions options = Default)
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        Q_ASSERT(data && "Creating rawData from NULL");
        QTypedArrayData *result = allocate(0, options | RawData);
        if (result) {
            Q_ASSERT(!result->ref.isShared()); // No shared empty, please!
#ifndef __CHERI_PURE_CAPABILITY__
            result->offset = reinterpret_cast<const char *>(data)
                - reinterpret_cast<const char *>(result);
#else
            result->setPointer(__builtin_cheri_bounds_set(data, n * sizeof(T)));
#endif
            if (options & WithNulTerminator) {
                // The raw capability includes a nul terminator, but the size
                // should not include it.
                Q_ASSERT(data[n - 1] == '\0');
                result->size = int(n - 1);
            } else {
                result->size = int(n);
            }
        }
        return result;
    }

    static QTypedArrayData *sharedNull() noexcept
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        return static_cast<QTypedArrayData *>(QArrayData::sharedNull());
    }

    static QTypedArrayData *sharedEmpty()
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        return allocate(/* capacity */ 0);
    }

#if !defined(QT_NO_UNSHARABLE_CONTAINERS)
    static QTypedArrayData *unsharableEmpty()
    {
        Q_STATIC_ASSERT(sizeof(QTypedArrayData) == sizeof(QArrayData));
        return allocate(/* capacity */ 0, Unsharable);
    }
#endif
};

template <class T, size_t N>
struct QStaticArrayData
{
    QArrayData header;
    T data[N];
};

// Support for returning QArrayDataPointer<T> from functions
template <class T>
struct QArrayDataPointerRef
{
    QTypedArrayData<T> *ptr;
};

#define Q_STATIC_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size, offset) \
    { Q_REFCOUNT_INITIALIZE_STATIC, size, 0, 0, offset } \
    /**/

#define Q_STATIC_ARRAY_DATA_HEADER_INITIALIZER(type, size) \
    Q_STATIC_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(size,\
        ((sizeof(QArrayData) + (Q_ALIGNOF(type) - 1)) & ~(Q_ALIGNOF(type) - 1) )) \
    /**/

////////////////////////////////////////////////////////////////////////////////
//  Q_ARRAY_LITERAL

// The idea here is to place a (read-only) copy of header and array data in an
// mmappable portion of the executable (typically, .rodata section). This is
// accomplished by hiding a static const instance of QStaticArrayData, which is
// POD.

// Hide array inside a lambda
#define Q_ARRAY_LITERAL(Type, ...)                                              \
    ([]() -> QArrayDataPointerRef<Type> {                                       \
            /* MSVC 2010 Doesn't support static variables in a lambda, but */   \
            /* happily accepts them in a static function of a lambda-local */   \
            /* struct :-) */                                                    \
            struct StaticWrapper {                                              \
                static QArrayDataPointerRef<Type> get()                         \
                {                                                               \
                    Q_ARRAY_LITERAL_IMPL(Type, __VA_ARGS__)                     \
                    return ref;                                                 \
                }                                                               \
            };                                                                  \
            return StaticWrapper::get();                                        \
        }())                                                                    \
    /**/

#ifdef Q_COMPILER_CONSTEXPR
#define Q_ARRAY_LITERAL_CHECK_LITERAL_TYPE(Type) Q_STATIC_ASSERT(std::is_literal_type<Type>::value)
#else
#define Q_ARRAY_LITERAL_CHECK_LITERAL_TYPE(Type) do {} while (0)
#endif

#define Q_ARRAY_LITERAL_IMPL(Type, ...)                                         \
    Q_ARRAY_LITERAL_CHECK_LITERAL_TYPE(Type);                                   \
                                                                                \
    /* Portable compile-time array size computation */                          \
    Q_CONSTEXPR Type data[] = { __VA_ARGS__ }; Q_UNUSED(data);                  \
    enum { Size = sizeof(data) / sizeof(data[0]) };                             \
                                                                                \
    static const QStaticArrayData<Type, Size> literal = {                       \
        Q_STATIC_ARRAY_DATA_HEADER_INITIALIZER(Type, Size), { __VA_ARGS__ } };  \
                                                                                \
    QArrayDataPointerRef<Type> ref =                                            \
        { static_cast<QTypedArrayData<Type> *>(                                 \
            const_cast<QArrayData *>(&literal.header)) };                       \
    /**/

namespace QtPrivate {
struct Q_CORE_EXPORT QContainerImplHelper
{
    enum CutResult { Null, Empty, Full, Subset };
    static CutResult mid(int originalLength, int *position, int *length);
};
}

QT_END_NAMESPACE

#endif // include guard
