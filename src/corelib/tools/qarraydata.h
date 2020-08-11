/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2019 Intel Corporation.
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

#include <QtCore/qpair.h>
#include <QtCore/qatomic.h>
#include <string.h>

QT_BEGIN_NAMESPACE

template <class T> struct QTypedArrayData;

struct Q_CORE_EXPORT QArrayData
{
    enum ArrayOption {
        /// this option is used by the allocate() function
        DefaultAllocationFlags = 0,
        CapacityReserved     = 0x1,  //!< the capacity was reserved by the user, try to keep it
        GrowsForward         = 0x2,  //!< allocate with eyes towards growing through append()
        GrowsBackwards       = 0x4   //!< allocate with eyes towards growing through prepend()
    };
    Q_DECLARE_FLAGS(ArrayOptions, ArrayOption)

    QBasicAtomicInt ref_;
    uint flags;
    qsizetype alloc;

    inline qsizetype allocatedCapacity() noexcept
    {
        return alloc;
    }

    inline qsizetype constAllocatedCapacity() const noexcept
    {
        return alloc;
    }

    /// Returns true if sharing took place
    bool ref() noexcept
    {
        ref_.ref();
        return true;
    }

    /// Returns false if deallocation is necessary
    bool deref() noexcept
    {
        return ref_.deref();
    }

    bool isShared() const noexcept
    {
        return ref_.loadRelaxed() != 1;
    }

    // Returns true if a detach is necessary before modifying the data
    // This method is intentionally not const: if you want to know whether
    // detaching is necessary, you should be in a non-const function already
    bool needsDetach() const noexcept
    {
        return ref_.loadRelaxed() > 1;
    }

    qsizetype detachCapacity(qsizetype newSize) const noexcept
    {
        if (flags & CapacityReserved && newSize < constAllocatedCapacity())
            return constAllocatedCapacity();
        return newSize;
    }

    ArrayOptions detachFlags() const noexcept
    {
        ArrayOptions result = DefaultAllocationFlags;
        if (flags & CapacityReserved)
            result |= CapacityReserved;
        return result;
    }

    Q_REQUIRED_RESULT
#if defined(Q_CC_GNU)
    __attribute__((__malloc__))
#endif
    static void *allocate(QArrayData **pdata, qsizetype objectSize, qsizetype alignment,
            qsizetype capacity, ArrayOptions options = DefaultAllocationFlags) noexcept;
    Q_REQUIRED_RESULT static QPair<QArrayData *, void *> reallocateUnaligned(QArrayData *data, void *dataPointer,
            qsizetype objectSize, qsizetype newCapacity, ArrayOptions newOptions = DefaultAllocationFlags) noexcept;
    static void deallocate(QArrayData *data, qsizetype objectSize,
            qsizetype alignment) noexcept;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QArrayData::ArrayOptions)

template <class T>
struct QTypedArrayData
    : QArrayData
{
    class iterator {
        T *i = nullptr;
    public:
        typedef std::random_access_iterator_tag  iterator_category;
        typedef qsizetype difference_type;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;

        inline constexpr iterator() = default;
        inline iterator(T *n) : i(n) {}
        inline T &operator*() const { return *i; }
        inline T *operator->() const { return i; }
        inline T &operator[](qsizetype j) const { return *(i + j); }
        inline constexpr bool operator==(iterator o) const { return i == o.i; }
        inline constexpr bool operator!=(iterator o) const { return i != o.i; }
        inline constexpr bool operator<(iterator other) const { return i < other.i; }
        inline constexpr bool operator<=(iterator other) const { return i <= other.i; }
        inline constexpr bool operator>(iterator other) const { return i > other.i; }
        inline constexpr bool operator>=(iterator other) const { return i >= other.i; }
        inline constexpr bool operator==(pointer p) const { return i == p; }
        inline constexpr bool operator!=(pointer p) const { return i != p; }
        inline iterator &operator++() { ++i; return *this; }
        inline iterator operator++(int) { T *n = i; ++i; return n; }
        inline iterator &operator--() { i--; return *this; }
        inline iterator operator--(int) { T *n = i; i--; return n; }
        inline iterator &operator+=(qsizetype j) { i+=j; return *this; }
        inline iterator &operator-=(qsizetype j) { i-=j; return *this; }
        inline iterator operator+(qsizetype j) const { return iterator(i+j); }
        inline iterator operator-(qsizetype j) const { return iterator(i-j); }
        friend inline iterator operator+(qsizetype j, iterator k) { return k + j; }
        inline qsizetype operator-(iterator j) const { return i - j.i; }
        inline operator T*() const { return i; }
    };

    class const_iterator {
        const T *i = nullptr;
    public:
        typedef std::random_access_iterator_tag  iterator_category;
        typedef qsizetype difference_type;
        typedef T value_type;
        typedef const T *pointer;
        typedef const T &reference;

        inline constexpr const_iterator() = default;
        inline const_iterator(const T *n) : i(n) {}
        inline constexpr const_iterator(iterator o): i(o) {}
        inline const T &operator*() const { return *i; }
        inline const T *operator->() const { return i; }
        inline const T &operator[](qsizetype j) const { return *(i + j); }
        inline constexpr bool operator==(const_iterator o) const { return i == o.i; }
        inline constexpr bool operator!=(const_iterator o) const { return i != o.i; }
        inline constexpr bool operator<(const_iterator other) const { return i < other.i; }
        inline constexpr bool operator<=(const_iterator other) const { return i <= other.i; }
        inline constexpr bool operator>(const_iterator other) const { return i > other.i; }
        inline constexpr bool operator>=(const_iterator other) const { return i >= other.i; }
        inline constexpr bool operator==(iterator o) const { return i == const_iterator(o).i; }
        inline constexpr bool operator!=(iterator o) const { return i != const_iterator(o).i; }
        inline constexpr bool operator==(pointer p) const { return i == p; }
        inline constexpr bool operator!=(pointer p) const { return i != p; }
        inline const_iterator &operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const T *n = i; ++i; return n; }
        inline const_iterator &operator--() { i--; return *this; }
        inline const_iterator operator--(int) { const T *n = i; i--; return n; }
        inline const_iterator &operator+=(qsizetype j) { i+=j; return *this; }
        inline const_iterator &operator-=(qsizetype j) { i-=j; return *this; }
        inline const_iterator operator+(qsizetype j) const { return const_iterator(i+j); }
        inline const_iterator operator-(qsizetype j) const { return const_iterator(i-j); }
        friend inline const_iterator operator+(qsizetype j, const_iterator k) { return k + j; }
        inline qsizetype operator-(const_iterator j) const { return i - j.i; }
        inline operator const T*() const { return i; }
    };

    class AlignmentDummy { QArrayData header; T data; };

    Q_REQUIRED_RESULT static QPair<QTypedArrayData *, T *> allocate(qsizetype capacity,
            ArrayOptions options = DefaultAllocationFlags)
    {
        static_assert(sizeof(QTypedArrayData) == sizeof(QArrayData));
        QArrayData *d;
        void *result = QArrayData::allocate(&d, sizeof(T), alignof(AlignmentDummy), capacity, options);
#if (defined(Q_CC_GNU) && Q_CC_GNU >= 407) || QT_HAS_BUILTIN(__builtin_assume_aligned)
        result = __builtin_assume_aligned(result, Q_ALIGNOF(AlignmentDummy));
#endif
        return qMakePair(static_cast<QTypedArrayData *>(d), static_cast<T *>(result));
    }

    static QPair<QTypedArrayData *, T *>
    reallocateUnaligned(QTypedArrayData *data, T *dataPointer, qsizetype capacity,
            ArrayOptions options = DefaultAllocationFlags)
    {
        static_assert(sizeof(QTypedArrayData) == sizeof(QArrayData));
        QPair<QArrayData *, void *> pair =
                QArrayData::reallocateUnaligned(data, dataPointer, sizeof(T), capacity, options);
        return qMakePair(static_cast<QTypedArrayData *>(pair.first), static_cast<T *>(pair.second));
    }

    static void deallocate(QArrayData *data) noexcept
    {
        static_assert(sizeof(QTypedArrayData) == sizeof(QArrayData));
        QArrayData::deallocate(data, sizeof(T), alignof(AlignmentDummy));
    }
};

namespace QtPrivate {
struct Q_CORE_EXPORT QContainerImplHelper
{
    enum CutResult { Null, Empty, Full, Subset };
    static constexpr CutResult mid(qsizetype originalLength, qsizetype *_position, qsizetype *_length)
    {
        qsizetype &position = *_position;
        qsizetype &length = *_length;
        if (position > originalLength) {
            position = 0;
            length = 0;
            return Null;
        }

        if (position < 0) {
            if (length < 0 || length + position >= originalLength) {
                position = 0;
                length = originalLength;
                return Full;
            }
            if (length + position <= 0) {
                position = length = 0;
                return Null;
            }
            length += position;
            position = 0;
        } else if (size_t(length) > size_t(originalLength - position)) {
            length = originalLength - position;
        }

        if (position == 0 && length == originalLength)
            return Full;

        return length > 0 ? Subset : Empty;
    }
};
}

QT_END_NAMESPACE

#endif // include guard
