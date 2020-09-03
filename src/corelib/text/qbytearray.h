/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2016 Intel Corporation.
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

#ifndef QBYTEARRAY_H
#define QBYTEARRAY_H

#include <QtCore/qrefcount.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qarraydata.h>
#include <QtCore/qarraydatapointer.h>
#include <QtCore/qcontainerfwd.h>
#include <QtCore/qbytearrayalgorithms.h>
#include <QtCore/qbytearrayview.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <string>
#include <iterator>

#ifndef QT5_NULL_STRINGS
// ### Should default to 0 in Qt 6.0
#define QT5_NULL_STRINGS 1
#endif

#ifdef truncate
#error qbytearray.h must be included before any header file that defines truncate
#endif

#if defined(Q_OS_DARWIN) || defined(Q_QDOC)
Q_FORWARD_DECLARE_CF_TYPE(CFData);
Q_FORWARD_DECLARE_OBJC_CLASS(NSData);
#endif

QT_BEGIN_NAMESPACE


/*****************************************************************************
  Safe and portable C string functions; extensions to standard string.h
 *****************************************************************************/

Q_CORE_EXPORT char *qstrdup(const char *);

inline size_t qstrlen(const char *str)
{ return str ? strlen(str) : 0; }

inline size_t qstrnlen(const char *str, size_t maxlen)
{
    size_t length = 0;
    if (str) {
        while (length < maxlen && *str++)
            length++;
    }
    return length;
}

Q_CORE_EXPORT char *qstrcpy(char *dst, const char *src);
Q_CORE_EXPORT char *qstrncpy(char *dst, const char *src, size_t len);

Q_CORE_EXPORT int qstrcmp(const char *str1, const char *str2);

inline int qstrncmp(const char *str1, const char *str2, size_t len)
{
    return (str1 && str2) ? strncmp(str1, str2, len)
        : (str1 ? 1 : (str2 ? -1 : 0));
}
Q_CORE_EXPORT int qstricmp(const char *, const char *);
Q_CORE_EXPORT int qstrnicmp(const char *, const char *, size_t len);
Q_CORE_EXPORT int qstrnicmp(const char *, qsizetype, const char *, qsizetype = -1);

// implemented in qvsnprintf.cpp
Q_CORE_EXPORT int qvsnprintf(char *str, size_t n, const char *fmt, va_list ap);
Q_CORE_EXPORT int qsnprintf(char *str, size_t n, const char *fmt, ...);

// qChecksum: Internet checksum
Q_CORE_EXPORT quint16 qChecksum(const char *s, qsizetype len,
                                Qt::ChecksumType standard = Qt::ChecksumIso3309);

class QString;
class QDataStream;

using QByteArrayData = QArrayDataPointer<char>;

#  define QByteArrayLiteral(str) \
    (QByteArray(QByteArrayData(nullptr, const_cast<char *>(str), sizeof(str) - 1))) \
    /**/

class Q_CORE_EXPORT QByteArray
{
public:
    using DataPointer = QByteArrayData;
private:
    typedef QTypedArrayData<char> Data;

    DataPointer d;
    static const char _empty;
public:

    enum Base64Option {
        Base64Encoding = 0,
        Base64UrlEncoding = 1,

        KeepTrailingEquals = 0,
        OmitTrailingEquals = 2,

        IgnoreBase64DecodingErrors = 0,
        AbortOnBase64DecodingErrors = 4,
    };
    Q_DECLARE_FLAGS(Base64Options, Base64Option)

    enum class Base64DecodingStatus {
        Ok,
        IllegalInputLength,
        IllegalCharacter,
        IllegalPadding,
    };

    inline constexpr QByteArray() noexcept;
    QByteArray(const char *, qsizetype size = -1);
    QByteArray(qsizetype size, char c);
    QByteArray(qsizetype size, Qt::Initialization);
    inline QByteArray(const QByteArray &) noexcept;
    inline ~QByteArray();

    QByteArray &operator=(const QByteArray &) noexcept;
    QByteArray &operator=(const char *str);
    inline QByteArray(QByteArray && other) noexcept
    { qSwap(d, other.d); }
    inline QByteArray &operator=(QByteArray &&other) noexcept
    { qSwap(d, other.d); return *this; }
    inline void swap(QByteArray &other) noexcept
    { qSwap(d, other.d); }

    inline bool isEmpty() const;
    void resize(qsizetype size);

    QByteArray &fill(char c, qsizetype size = -1);

    inline qsizetype capacity() const;
    inline void reserve(qsizetype size);
    inline void squeeze();

#ifndef QT_NO_CAST_FROM_BYTEARRAY
    inline operator const char *() const;
    inline operator const void *() const;
#endif
    inline char *data();
    inline const char *data() const;
    inline const char *constData() const;
    inline void detach();
    inline bool isDetached() const;
    inline bool isSharedWith(const QByteArray &other) const
    { return data() == other.data() && size() == other.size(); }
    void clear();

    inline char at(qsizetype i) const;
    inline char operator[](qsizetype i) const;
    Q_REQUIRED_RESULT inline char &operator[](qsizetype i);
    Q_REQUIRED_RESULT char front() const { return at(0); }
    Q_REQUIRED_RESULT inline char &front();
    Q_REQUIRED_RESULT char back() const { return at(size() - 1); }
    Q_REQUIRED_RESULT inline char &back();

    qsizetype indexOf(char c, qsizetype from = 0) const;
    qsizetype indexOf(QByteArrayView bv, qsizetype from = 0) const
    { return QtPrivate::findByteArray(qToByteArrayViewIgnoringNull(*this), from, bv); }

    qsizetype lastIndexOf(char c, qsizetype from = -1) const;
    qsizetype lastIndexOf(QByteArrayView bv, qsizetype from = -1) const
    { return QtPrivate::lastIndexOf(qToByteArrayViewIgnoringNull(*this), from, bv); }

    inline bool contains(char c) const;
    inline bool contains(QByteArrayView bv) const;
    qsizetype count(char c) const;
    qsizetype count(QByteArrayView bv) const
    { return QtPrivate::count(qToByteArrayViewIgnoringNull(*this), bv); }

    inline int compare(QByteArrayView a, Qt::CaseSensitivity cs = Qt::CaseSensitive) const noexcept;

    Q_REQUIRED_RESULT QByteArray left(qsizetype len) const;
    Q_REQUIRED_RESULT QByteArray right(qsizetype len) const;
    Q_REQUIRED_RESULT QByteArray mid(qsizetype index, qsizetype len = -1) const;

    Q_REQUIRED_RESULT QByteArray first(qsizetype n) const
    { Q_ASSERT(n >= 0); Q_ASSERT(n <= size()); return QByteArray(data(), n); }
    Q_REQUIRED_RESULT QByteArray last(qsizetype n) const
    { Q_ASSERT(n >= 0); Q_ASSERT(n <= size()); return QByteArray(data() + size() - n, n); }
    Q_REQUIRED_RESULT QByteArray sliced(qsizetype pos) const
    { Q_ASSERT(pos >= 0); Q_ASSERT(pos <= size()); return QByteArray(data() + pos, size() - pos); }
    Q_REQUIRED_RESULT QByteArray sliced(qsizetype pos, qsizetype n) const
    { Q_ASSERT(pos >= 0); Q_ASSERT(n >= 0); Q_ASSERT(size_t(pos) + size_t(n) <= size_t(size())); return QByteArray(data() + pos, n); }
    Q_REQUIRED_RESULT QByteArray chopped(qsizetype len) const
    { Q_ASSERT(len >= 0); Q_ASSERT(len <= size()); return first(size() - len); }

    bool startsWith(QByteArrayView bv) const
    { return QtPrivate::startsWith(qToByteArrayViewIgnoringNull(*this), bv); }
    bool startsWith(char c) const { return size() > 0 && front() == c; }

    bool endsWith(char c) const { return size() > 0 && back() == c; }
    bool endsWith(QByteArrayView bv) const
    { return QtPrivate::endsWith(qToByteArrayViewIgnoringNull(*this), bv); }

    bool isUpper() const;
    bool isLower() const;

    void truncate(qsizetype pos);
    void chop(qsizetype n);

#if !defined(Q_CLANG_QDOC)
    Q_REQUIRED_RESULT QByteArray toLower() const &
    { return toLower_helper(*this); }
    Q_REQUIRED_RESULT QByteArray toLower() &&
    { return toLower_helper(*this); }
    Q_REQUIRED_RESULT QByteArray toUpper() const &
    { return toUpper_helper(*this); }
    Q_REQUIRED_RESULT QByteArray toUpper() &&
    { return toUpper_helper(*this); }
    Q_REQUIRED_RESULT QByteArray trimmed() const &
    { return trimmed_helper(*this); }
    Q_REQUIRED_RESULT QByteArray trimmed() &&
    { return trimmed_helper(*this); }
    Q_REQUIRED_RESULT QByteArray simplified() const &
    { return simplified_helper(*this); }
    Q_REQUIRED_RESULT QByteArray simplified() &&
    { return simplified_helper(*this); }
#else
    Q_REQUIRED_RESULT QByteArray toLower() const;
    Q_REQUIRED_RESULT QByteArray toUpper() const;
    Q_REQUIRED_RESULT QByteArray trimmed() const;
    Q_REQUIRED_RESULT QByteArray simplified() const;
#endif

    Q_REQUIRED_RESULT QByteArray leftJustified(qsizetype width, char fill = ' ', bool truncate = false) const;
    Q_REQUIRED_RESULT QByteArray rightJustified(qsizetype width, char fill = ' ', bool truncate = false) const;

    QByteArray &prepend(char c);
    inline QByteArray &prepend(qsizetype count, char c);
    QByteArray &prepend(const char *s);
    QByteArray &prepend(const char *s, qsizetype len);
    QByteArray &prepend(const QByteArray &a);
    QByteArray &append(char c);
    inline QByteArray &append(qsizetype count, char c);
    QByteArray &append(const char *s);
    QByteArray &append(const char *s, qsizetype len);
    QByteArray &append(const QByteArray &a);
    QByteArray &insert(qsizetype i, char c);
    QByteArray &insert(qsizetype i, qsizetype count, char c);
    QByteArray &insert(qsizetype i, const char *s);
    QByteArray &insert(qsizetype i, const char *s, qsizetype len);
    QByteArray &insert(qsizetype i, const QByteArray &a);
    QByteArray &remove(qsizetype index, qsizetype len);
    QByteArray &replace(qsizetype index, qsizetype len, const char *s);
    QByteArray &replace(qsizetype index, qsizetype len, const char *s, qsizetype alen);
    QByteArray &replace(qsizetype index, qsizetype len, const QByteArray &s);
    inline QByteArray &replace(char before, const char *after);
    QByteArray &replace(char before, const QByteArray &after);
    inline QByteArray &replace(const char *before, const char *after);
    QByteArray &replace(const char *before, qsizetype bsize, const char *after, qsizetype asize);
    QByteArray &replace(const QByteArray &before, const QByteArray &after);
    inline QByteArray &replace(const QByteArray &before, const char *after);
    QByteArray &replace(const char *before, const QByteArray &after);
    QByteArray &replace(char before, char after);
    inline QByteArray &operator+=(char c);
    inline QByteArray &operator+=(const char *s);
    inline QByteArray &operator+=(const QByteArray &a);

    QList<QByteArray> split(char sep) const;

    Q_REQUIRED_RESULT QByteArray repeated(qsizetype times) const;

#if !defined(QT_NO_CAST_FROM_ASCII) && !defined(QT_RESTRICTED_CAST_FROM_ASCII)
    inline QT_ASCII_CAST_WARN bool operator==(const QString &s2) const;
    inline QT_ASCII_CAST_WARN bool operator!=(const QString &s2) const;
    inline QT_ASCII_CAST_WARN bool operator<(const QString &s2) const;
    inline QT_ASCII_CAST_WARN bool operator>(const QString &s2) const;
    inline QT_ASCII_CAST_WARN bool operator<=(const QString &s2) const;
    inline QT_ASCII_CAST_WARN bool operator>=(const QString &s2) const;
#endif

    short toShort(bool *ok = nullptr, int base = 10) const;
    ushort toUShort(bool *ok = nullptr, int base = 10) const;
    int toInt(bool *ok = nullptr, int base = 10) const;
    uint toUInt(bool *ok = nullptr, int base = 10) const;
    long toLong(bool *ok = nullptr, int base = 10) const;
    ulong toULong(bool *ok = nullptr, int base = 10) const;
    qlonglong toLongLong(bool *ok = nullptr, int base = 10) const;
    qulonglong toULongLong(bool *ok = nullptr, int base = 10) const;
    float toFloat(bool *ok = nullptr) const;
    double toDouble(bool *ok = nullptr) const;
    QByteArray toBase64(Base64Options options = Base64Encoding) const;
    QByteArray toHex(char separator = '\0') const;
    QByteArray toPercentEncoding(const QByteArray &exclude = QByteArray(),
                                 const QByteArray &include = QByteArray(),
                                 char percent = '%') const;

    inline QByteArray &setNum(short, int base = 10);
    inline QByteArray &setNum(ushort, int base = 10);
    inline QByteArray &setNum(int, int base = 10);
    inline QByteArray &setNum(uint, int base = 10);
    inline QByteArray &setNum(long, int base = 10);
    inline QByteArray &setNum(ulong, int base = 10);
    QByteArray &setNum(qlonglong, int base = 10);
    QByteArray &setNum(qulonglong, int base = 10);
    inline QByteArray &setNum(float, char f = 'g', int prec = 6);
    QByteArray &setNum(double, char f = 'g', int prec = 6);
    QByteArray &setRawData(const char *a, qsizetype n);

    Q_REQUIRED_RESULT static QByteArray number(int, int base = 10);
    Q_REQUIRED_RESULT static QByteArray number(uint, int base = 10);
    Q_REQUIRED_RESULT static QByteArray number(long, int base = 10);
    Q_REQUIRED_RESULT static QByteArray number(ulong, int base = 10);
    Q_REQUIRED_RESULT static QByteArray number(qlonglong, int base = 10);
    Q_REQUIRED_RESULT static QByteArray number(qulonglong, int base = 10);
    Q_REQUIRED_RESULT static QByteArray number(double, char f = 'g', int prec = 6);
    Q_REQUIRED_RESULT static QByteArray fromRawData(const char *data, qsizetype size)
    {
        return QByteArray(DataPointer(nullptr, const_cast<char *>(data), size));
    }

    class FromBase64Result;
    Q_REQUIRED_RESULT static FromBase64Result fromBase64Encoding(QByteArray &&base64, Base64Options options = Base64Encoding);
    Q_REQUIRED_RESULT static FromBase64Result fromBase64Encoding(const QByteArray &base64, Base64Options options = Base64Encoding);
    Q_REQUIRED_RESULT static QByteArray fromBase64(const QByteArray &base64, Base64Options options = Base64Encoding);
    Q_REQUIRED_RESULT static QByteArray fromHex(const QByteArray &hexEncoded);
    Q_REQUIRED_RESULT static QByteArray fromPercentEncoding(const QByteArray &pctEncoded, char percent = '%');

#if defined(Q_OS_DARWIN) || defined(Q_QDOC)
    static QByteArray fromCFData(CFDataRef data);
    static QByteArray fromRawCFData(CFDataRef data);
    CFDataRef toCFData() const Q_DECL_CF_RETURNS_RETAINED;
    CFDataRef toRawCFData() const Q_DECL_CF_RETURNS_RETAINED;
    static QByteArray fromNSData(const NSData *data);
    static QByteArray fromRawNSData(const NSData *data);
    NSData *toNSData() const Q_DECL_NS_RETURNS_AUTORELEASED;
    NSData *toRawNSData() const Q_DECL_NS_RETURNS_AUTORELEASED;
#endif

    typedef char *iterator;
    typedef const char *const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    inline iterator begin();
    inline const_iterator begin() const;
    inline const_iterator cbegin() const;
    inline const_iterator constBegin() const;
    inline iterator end();
    inline const_iterator end() const;
    inline const_iterator cend() const;
    inline const_iterator constEnd() const;
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    // stl compatibility
    typedef qsizetype size_type;
    typedef qptrdiff difference_type;
    typedef const char & const_reference;
    typedef char & reference;
    typedef char *pointer;
    typedef const char *const_pointer;
    typedef char value_type;
    inline void push_back(char c);
    inline void push_back(const char *c);
    inline void push_back(const QByteArray &a);
    inline void push_front(char c);
    inline void push_front(const char *c);
    inline void push_front(const QByteArray &a);
    void shrink_to_fit() { squeeze(); }

    static inline QByteArray fromStdString(const std::string &s);
    inline std::string toStdString() const;

    inline qsizetype size() const { return d->size; }
    inline qsizetype count() const { return size(); }
    inline qsizetype length() const { return size(); }
    bool isNull() const;

    inline DataPointer &data_ptr() { return d; }
    explicit inline QByteArray(const DataPointer &dd)
        : d(dd)
    {
    }

private:
    void reallocData(size_t alloc, Data::ArrayOptions options);
    void reallocGrowData(size_t alloc, Data::ArrayOptions options);
    void expand(qsizetype i);
    QByteArray nulTerminated() const;

    static QByteArray toLower_helper(const QByteArray &a);
    static QByteArray toLower_helper(QByteArray &a);
    static QByteArray toUpper_helper(const QByteArray &a);
    static QByteArray toUpper_helper(QByteArray &a);
    static QByteArray trimmed_helper(const QByteArray &a);
    static QByteArray trimmed_helper(QByteArray &a);
    static QByteArray simplified_helper(const QByteArray &a);
    static QByteArray simplified_helper(QByteArray &a);

    friend class QString;
    friend Q_CORE_EXPORT QByteArray qUncompress(const uchar *data, qsizetype nbytes);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QByteArray::Base64Options)

inline constexpr QByteArray::QByteArray() noexcept {}
inline QByteArray::~QByteArray() {}

inline char QByteArray::at(qsizetype i) const
{ Q_ASSERT(size_t(i) < size_t(size())); return d.data()[i]; }
inline char QByteArray::operator[](qsizetype i) const
{ Q_ASSERT(size_t(i) < size_t(size())); return d.data()[i]; }

inline bool QByteArray::isEmpty() const
{ return size() == 0; }
#ifndef QT_NO_CAST_FROM_BYTEARRAY
inline QByteArray::operator const char *() const
{ return data(); }
inline QByteArray::operator const void *() const
{ return data(); }
#endif
inline char *QByteArray::data()
{
    detach();
    Q_ASSERT(d.data());
    return d.data();
}
inline const char *QByteArray::data() const
{
#if QT5_NULL_STRINGS == 1
    return d.data() ? d.data() : &_empty;
#else
    return d.data();
#endif
}
inline const char *QByteArray::constData() const
{ return data(); }
inline void QByteArray::detach()
{ if (d->needsDetach()) reallocData(size_t(size()) + 1u, d->detachFlags()); }
inline bool QByteArray::isDetached() const
{ return !d->isShared(); }
inline QByteArray::QByteArray(const QByteArray &a) noexcept : d(a.d)
{}

inline qsizetype QByteArray::capacity() const
{ const auto realCapacity = d->constAllocatedCapacity(); return realCapacity ? realCapacity - 1 : 0; }

inline void QByteArray::reserve(qsizetype asize)
{
    if (d->needsDetach() || asize > capacity() - d->freeSpaceAtBegin()) {
        reallocData(qMax(size_t(size()), size_t(asize)) + 1u, d->detachFlags() | Data::CapacityReserved);
    } else {
        d->setFlag(Data::CapacityReserved);
    }
}

inline void QByteArray::squeeze()
{
    if ((d->flags() & Data::CapacityReserved) == 0)
        return;
    if (d->needsDetach() || size() < capacity()) {
        reallocData(size_t(size()) + 1u, d->detachFlags() & ~Data::CapacityReserved);
    } else {
        d->clearFlag(Data::CapacityReserved);
    }
}

inline char &QByteArray::operator[](qsizetype i)
{ Q_ASSERT(i >= 0 && i < size()); return data()[i]; }
inline char &QByteArray::front() { return operator[](0); }
inline char &QByteArray::back() { return operator[](size() - 1); }
inline QByteArray::iterator QByteArray::begin()
{ return data(); }
inline QByteArray::const_iterator QByteArray::begin() const
{ return data(); }
inline QByteArray::const_iterator QByteArray::cbegin() const
{ return data(); }
inline QByteArray::const_iterator QByteArray::constBegin() const
{ return data(); }
inline QByteArray::iterator QByteArray::end()
{ return data() + size(); }
inline QByteArray::const_iterator QByteArray::end() const
{ return data() + size(); }
inline QByteArray::const_iterator QByteArray::cend() const
{ return data() + size(); }
inline QByteArray::const_iterator QByteArray::constEnd() const
{ return data() + size(); }
inline QByteArray &QByteArray::append(qsizetype n, char ch)
{ return insert(size(), n, ch); }
inline QByteArray &QByteArray::prepend(qsizetype n, char ch)
{ return insert(0, n, ch); }
inline QByteArray &QByteArray::operator+=(char c)
{ return append(c); }
inline QByteArray &QByteArray::operator+=(const char *s)
{ return append(s); }
inline QByteArray &QByteArray::operator+=(const QByteArray &a)
{ return append(a); }
inline void QByteArray::push_back(char c)
{ append(c); }
inline void QByteArray::push_back(const char *c)
{ append(c); }
inline void QByteArray::push_back(const QByteArray &a)
{ append(a); }
inline void QByteArray::push_front(char c)
{ prepend(c); }
inline void QByteArray::push_front(const char *c)
{ prepend(c); }
inline void QByteArray::push_front(const QByteArray &a)
{ prepend(a); }
inline bool QByteArray::contains(char c) const
{ return indexOf(c) != -1; }
inline bool QByteArray::contains(QByteArrayView bv) const
{ return indexOf(bv) != -1; }
inline int QByteArray::compare(QByteArrayView a, Qt::CaseSensitivity cs) const noexcept
{
    return cs == Qt::CaseSensitive ? QtPrivate::compareMemory(*this, a) :
                                     qstrnicmp(data(), size(), a.data(), a.size());
}
inline bool operator==(const QByteArray &a1, const QByteArray &a2) noexcept
{ return QByteArrayView(a1) == QByteArrayView(a2); }
inline bool operator==(const QByteArray &a1, const char *a2) noexcept
{ return a2 ? QtPrivate::compareMemory(a1, a2) == 0 : a1.isEmpty(); }
inline bool operator==(const char *a1, const QByteArray &a2) noexcept
{ return a1 ? QtPrivate::compareMemory(a1, a2) == 0 : a2.isEmpty(); }
inline bool operator!=(const QByteArray &a1, const QByteArray &a2) noexcept
{ return !(a1==a2); }
inline bool operator!=(const QByteArray &a1, const char *a2) noexcept
{ return a2 ? QtPrivate::compareMemory(a1, a2) != 0 : !a1.isEmpty(); }
inline bool operator!=(const char *a1, const QByteArray &a2) noexcept
{ return a1 ? QtPrivate::compareMemory(a1, a2) != 0 : !a2.isEmpty(); }
inline bool operator<(const QByteArray &a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(QByteArrayView(a1), QByteArrayView(a2)) < 0; }
 inline bool operator<(const QByteArray &a1, const char *a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) < 0; }
inline bool operator<(const char *a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) < 0; }
inline bool operator<=(const QByteArray &a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(QByteArrayView(a1), QByteArrayView(a2)) <= 0; }
inline bool operator<=(const QByteArray &a1, const char *a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) <= 0; }
inline bool operator<=(const char *a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) <= 0; }
inline bool operator>(const QByteArray &a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(QByteArrayView(a1), QByteArrayView(a2)) > 0; }
inline bool operator>(const QByteArray &a1, const char *a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) > 0; }
inline bool operator>(const char *a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) > 0; }
inline bool operator>=(const QByteArray &a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(QByteArrayView(a1), QByteArrayView(a2)) >= 0; }
inline bool operator>=(const QByteArray &a1, const char *a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) >= 0; }
inline bool operator>=(const char *a1, const QByteArray &a2) noexcept
{ return QtPrivate::compareMemory(a1, a2) >= 0; }
#if !defined(QT_USE_QSTRINGBUILDER)
inline const QByteArray operator+(const QByteArray &a1, const QByteArray &a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const QByteArray &a1, const char *a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const QByteArray &a1, char a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(const char *a1, const QByteArray &a2)
{ return QByteArray(a1) += a2; }
inline const QByteArray operator+(char a1, const QByteArray &a2)
{ return QByteArray(&a1, 1) += a2; }
#endif // QT_USE_QSTRINGBUILDER
inline QByteArray &QByteArray::replace(char before, const char *c)
{ return replace(&before, 1, c, qstrlen(c)); }
inline QByteArray &QByteArray::replace(const QByteArray &before, const char *c)
{ return replace(before.constData(), before.size(), c, qstrlen(c)); }
inline QByteArray &QByteArray::replace(const char *before, const char *after)
{ return replace(before, qstrlen(before), after, qstrlen(after)); }

inline QByteArray &QByteArray::setNum(short n, int base)
{ return base == 10 ? setNum(qlonglong(n), base) : setNum(qulonglong(ushort(n)), base); }
inline QByteArray &QByteArray::setNum(ushort n, int base)
{ return setNum(qulonglong(n), base); }
inline QByteArray &QByteArray::setNum(int n, int base)
{ return base == 10 ? setNum(qlonglong(n), base) : setNum(qulonglong(uint(n)), base); }
inline QByteArray &QByteArray::setNum(uint n, int base)
{ return setNum(qulonglong(n), base); }
inline QByteArray &QByteArray::setNum(long n, int base)
{ return base == 10 ? setNum(qlonglong(n), base) : setNum(qulonglong(ulong(n)), base); }
inline QByteArray &QByteArray::setNum(ulong n, int base)
{ return setNum(qulonglong(n), base); }
inline QByteArray &QByteArray::setNum(float n, char f, int prec)
{ return setNum(double(n),f,prec); }

inline std::string QByteArray::toStdString() const
{ return std::string(constData(), length()); }

inline QByteArray QByteArray::fromStdString(const std::string &s)
{ return QByteArray(s.data(), qsizetype(s.size())); }

#if !defined(QT_NO_DATASTREAM) || (defined(QT_BOOTSTRAPPED) && !defined(QT_BUILD_QMAKE))
Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QByteArray &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QByteArray &);
#endif

#ifndef QT_NO_COMPRESS
Q_CORE_EXPORT QByteArray qCompress(const uchar* data, qsizetype nbytes, int compressionLevel = -1);
Q_CORE_EXPORT QByteArray qUncompress(const uchar* data, qsizetype nbytes);
inline QByteArray qCompress(const QByteArray& data, int compressionLevel = -1)
{ return qCompress(reinterpret_cast<const uchar *>(data.constData()), data.size(), compressionLevel); }
inline QByteArray qUncompress(const QByteArray& data)
{ return qUncompress(reinterpret_cast<const uchar*>(data.constData()), data.size()); }
#endif

Q_DECLARE_SHARED(QByteArray)

class QByteArray::FromBase64Result
{
public:
    QByteArray decoded;
    QByteArray::Base64DecodingStatus decodingStatus;

    void swap(QByteArray::FromBase64Result &other) noexcept
    {
        qSwap(decoded, other.decoded);
        qSwap(decodingStatus, other.decodingStatus);
    }

    explicit operator bool() const noexcept { return decodingStatus == QByteArray::Base64DecodingStatus::Ok; }

#if defined(Q_COMPILER_REF_QUALIFIERS) && !defined(Q_QDOC)
    QByteArray &operator*() & noexcept { return decoded; }
    const QByteArray &operator*() const & noexcept { return decoded; }
    QByteArray &&operator*() && noexcept { return std::move(decoded); }
#else
    QByteArray &operator*() noexcept { return decoded; }
    const QByteArray &operator*() const noexcept { return decoded; }
#endif
};

Q_DECLARE_SHARED(QByteArray::FromBase64Result)

inline bool operator==(const QByteArray::FromBase64Result &lhs, const QByteArray::FromBase64Result &rhs) noexcept
{
    if (lhs.decodingStatus != rhs.decodingStatus)
        return false;

    if (lhs.decodingStatus == QByteArray::Base64DecodingStatus::Ok && lhs.decoded != rhs.decoded)
        return false;

    return true;
}

inline bool operator!=(const QByteArray::FromBase64Result &lhs, const QByteArray::FromBase64Result &rhs) noexcept
{
    return !operator==(lhs, rhs);
}

Q_CORE_EXPORT Q_DECL_PURE_FUNCTION size_t qHash(const QByteArray::FromBase64Result &key, size_t seed = 0) noexcept;

//
// QByteArrayView members that require QByteArray:
//
QByteArray QByteArrayView::toByteArray() const
{
    return QByteArray(data(), size());
}

QT_END_NAMESPACE

#endif // QBYTEARRAY_H
