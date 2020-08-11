/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Copyright (C) 2019 Intel Corporation
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

#ifndef QLIST_H
#define QLIST_H

#include <QtCore/qarraydatapointer.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qhashfunctions.h>
#include <QtCore/qiterator.h>

#include <functional>
#include <limits>
#include <initializer_list>
#include <type_traits>

QT_BEGIN_NAMESPACE

namespace QtPrivate {
   template <typename V, typename U> qsizetype indexOf(const QList<V> &list, const U &u, qsizetype from);
   template <typename V, typename U> qsizetype lastIndexOf(const QList<V> &list, const U &u, qsizetype from);
}

template <typename T> struct QListSpecialMethods
{
protected:
    ~QListSpecialMethods() = default;
};
template <> struct QListSpecialMethods<QByteArray>;
template <> struct QListSpecialMethods<QString>;

template <typename T>
class QList
#ifndef Q_QDOC
    : public QListSpecialMethods<T>
#endif
{
    typedef QTypedArrayData<T> Data;
    typedef QArrayDataOps<T> DataOps;
    typedef QArrayDataPointer<T> DataPointer;
    class DisableRValueRefs {};

    DataPointer d;

    template <typename V, typename U> friend qsizetype QtPrivate::indexOf(const QList<V> &list, const U &u, qsizetype from);
    template <typename V, typename U> friend qsizetype QtPrivate::lastIndexOf(const QList<V> &list, const U &u, qsizetype from);

public:
    typedef T Type;
    typedef T value_type;
    typedef value_type *pointer;
    typedef const value_type *const_pointer;
    typedef value_type &reference;
    typedef const value_type &const_reference;
    typedef qsizetype size_type;
    typedef qptrdiff difference_type;
    typedef typename Data::iterator iterator;
    typedef typename Data::const_iterator const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
    typedef typename DataPointer::parameter_type parameter_type;
    using rvalue_ref = typename std::conditional<DataPointer::pass_parameter_by_value, DisableRValueRefs, T &&>::type;

private:
    void resize_internal(qsizetype i, Qt::Initialization);
    bool isValidIterator(const_iterator i) const
    {
        const std::less<const T*> less = {};
        return !less(d->end(), i) && !less(i, d->begin());
    }
public:
    QList(DataPointer dd) noexcept
        : d(dd)
    {
    }

public:
    QList() = default;
    explicit QList(qsizetype size)
        : d(Data::allocate(size))
    {
        if (size)
            d->appendInitialize(size);
    }
    QList(qsizetype size, const T &t)
        : d(Data::allocate(size))
    {
        if (size)
            d->copyAppend(size, t);
    }

    inline QList(std::initializer_list<T> args)
        : d(Data::allocate(args.size()))
    {
        if (args.size())
            d->copyAppend(args.begin(), args.end());
    }

    QList<T> &operator=(std::initializer_list<T> args)
    {
        d = DataPointer(Data::allocate(args.size()));
        if (args.size())
            d->copyAppend(args.begin(), args.end());
        return *this;
    }
    template <typename InputIterator, QtPrivate::IfIsForwardIterator<InputIterator> = true>
    QList(InputIterator i1, InputIterator i2)
    {
        const auto distance = std::distance(i1, i2);
        if (distance) {
            d = DataPointer(Data::allocate(distance));
            d->copyAppend(i1, i2);
        }
    }

    template <typename InputIterator, QtPrivate::IfIsNotForwardIterator<InputIterator> = true>
    QList(InputIterator i1, InputIterator i2)
    {
        std::copy(i1, i2, std::back_inserter(*this));
    }

    // compiler-generated special member functions are fine!

    void swap(QList<T> &other) noexcept { qSwap(d, other.d); }

    template <typename U>
    friend QTypeTraits::compare_eq_result<U> operator==(const QList<U> &l, const QList<U> &r);
    template <typename U>
    friend QTypeTraits::compare_eq_result<U> operator!=(const QList<U> &l, const QList<U> &r);

    qsizetype size() const noexcept { return d->size; }
    qsizetype count() const noexcept { return size(); }
    qsizetype length() const noexcept { return size(); }

    inline bool isEmpty() const noexcept { return d->size == 0; }

    void resize(qsizetype size)
    {
        resize_internal(size, Qt::Uninitialized);
        if (size > this->size())
            d->appendInitialize(size);
    }
    void resize(qsizetype size, parameter_type c)
    {
        resize_internal(size, Qt::Uninitialized);
        if (size > this->size())
            d->copyAppend(size - this->size(), c);
    }

    inline qsizetype capacity() const { return qsizetype(d->constAllocatedCapacity()); }
    void reserve(qsizetype size);
    inline void squeeze();

    void detach() { d.detach(); }
    bool isDetached() const noexcept { return !d->isShared(); }

    inline bool isSharedWith(const QList<T> &other) const { return d == other.d; }

    pointer data() { detach(); return d->data(); }
    const_pointer data() const noexcept { return d->data(); }
    const_pointer constData() const noexcept { return d->data(); }
    void clear() {
        if (!size())
            return;
        if (d->needsDetach()) {
            // must allocate memory
            DataPointer detached(Data::allocate(d.allocatedCapacity(), d->detachFlags()));
            d.swap(detached);
        } else {
            d->truncate(0);
        }
    }

    const_reference at(qsizetype i) const noexcept
    {
        Q_ASSERT_X(size_t(i) < size_t(d->size), "QList::at", "index out of range");
        return data()[i];
    }
    reference operator[](qsizetype i)
    {
        Q_ASSERT_X(size_t(i) < size_t(d->size), "QList::operator[]", "index out of range");
        detach();
        return data()[i];
    }
    const_reference operator[](qsizetype i) const noexcept { return at(i); }
    void append(const_reference t)
    { append(const_iterator(std::addressof(t)), const_iterator(std::addressof(t)) + 1); }
    void append(const_iterator i1, const_iterator i2);
    void append(rvalue_ref t) { emplaceBack(std::move(t)); }
    void append(const QList<T> &l) { append(l.constBegin(), l.constEnd()); }
    void append(QList<T> &&l);
    void prepend(rvalue_ref t);
    void prepend(const T &t);

    template <typename ...Args>
    reference emplaceBack(Args&&... args) { return *emplace(count(), std::forward<Args>(args)...); }

    iterator insert(qsizetype i, parameter_type t)
    { return insert(i, 1, t); }
    iterator insert(qsizetype i, qsizetype n, parameter_type t);
    iterator insert(const_iterator before, parameter_type t)
    {
        Q_ASSERT_X(isValidIterator(before),  "QList::insert", "The specified iterator argument 'before' is invalid");
        return insert(before, 1, t);
    }
    iterator insert(const_iterator before, qsizetype n, parameter_type t)
    {
        Q_ASSERT_X(isValidIterator(before),  "QList::insert", "The specified iterator argument 'before' is invalid");
        return insert(std::distance(constBegin(), before), n, t);
    }
    iterator insert(const_iterator before, rvalue_ref t)
    {
        Q_ASSERT_X(isValidIterator(before),  "QList::insert", "The specified iterator argument 'before' is invalid");
        return insert(std::distance(constBegin(), before), std::move(t));
    }
    iterator insert(qsizetype i, rvalue_ref t) { return emplace(i, std::move(t)); }

    template <typename ...Args>
    iterator emplace(const_iterator before, Args&&... args)
    {
        Q_ASSERT_X(isValidIterator(before),  "QList::emplace", "The specified iterator argument 'before' is invalid");
        return emplace(std::distance(constBegin(), before), std::forward<Args>(args)...);
    }

    template <typename ...Args>
    iterator emplace(qsizetype i, Args&&... args);
#if 0
    template< class InputIt >
    iterator insert( const_iterator pos, InputIt first, InputIt last );
    iterator insert( const_iterator pos, std::initializer_list<T> ilist );
#endif
    void replace(qsizetype i, const T &t)
    {
        Q_ASSERT_X(i >= 0 && i < d->size, "QList<T>::replace", "index out of range");
        const T copy(t);
        data()[i] = copy;
    }
    void replace(qsizetype i, rvalue_ref t)
    {
        Q_ASSERT_X(i >= 0 && i < d->size, "QList<T>::replace", "index out of range");
        const T copy(std::move(t));
        data()[i] = std::move(copy);
    }

    void remove(qsizetype i, qsizetype n = 1);
    void removeFirst() { Q_ASSERT(!isEmpty()); remove(0); }
    void removeLast() { Q_ASSERT(!isEmpty()); remove(size() - 1); }
    value_type takeFirst() { Q_ASSERT(!isEmpty()); value_type v = std::move(first()); remove(0); return v; }
    value_type takeLast() { Q_ASSERT(!isEmpty()); value_type v = std::move(last()); remove(size() - 1); return v; }

    QList<T> &fill(parameter_type t, qsizetype size = -1);

    qsizetype indexOf(const T &t, qsizetype from = 0) const noexcept;
    qsizetype lastIndexOf(const T &t, qsizetype from = -1) const noexcept;
    bool contains(const T &t) const noexcept
    {
        return indexOf(t) != -1;
    }
    qsizetype count(const T &t) const noexcept
    {
        return qsizetype(std::count(&*cbegin(), &*cend(), t));
    }

    // QList compatibility
    void removeAt(qsizetype i) { remove(i); }
    qsizetype removeAll(const T &t)
    {
        const const_iterator ce = this->cend(), cit = std::find(this->cbegin(), ce, t);
        if (cit == ce)
            return 0;
        qsizetype index = cit - this->cbegin();
        // next operation detaches, so ce, cit, t may become invalidated:
        const T tCopy = t;
        const iterator e = end(), it = std::remove(begin() + index, e, tCopy);
        const qsizetype result = std::distance(it, e);
        erase(it, e);
        return result;
    }
    bool removeOne(const T &t)
    {
        const qsizetype i = indexOf(t);
        if (i < 0)
            return false;
        remove(i);
        return true;
    }
    T takeAt(qsizetype i) { T t = std::move((*this)[i]); remove(i); return t; }
    void move(qsizetype from, qsizetype to)
    {
        Q_ASSERT_X(from >= 0 && from < size(), "QList::move(qsizetype, qsizetype)", "'from' is out-of-range");
        Q_ASSERT_X(to >= 0 && to < size(), "QList::move(qsizetype, qsizetype)", "'to' is out-of-range");
        if (from == to) // don't detach when no-op
            return;
        detach();
        T * const b = d->begin();
        if (from < to)
            std::rotate(b + from, b + from + 1, b + to + 1);
        else
            std::rotate(b + to, b + from, b + from + 1);
    }

    // STL-style
    iterator begin() { detach(); return d->begin(); }
    iterator end() { detach(); return d->end(); }

    const_iterator begin() const noexcept { return d->constBegin(); }
    const_iterator end() const noexcept { return d->constEnd(); }
    const_iterator cbegin() const noexcept { return d->constBegin(); }
    const_iterator cend() const noexcept { return d->constEnd(); }
    const_iterator constBegin() const noexcept { return d->constBegin(); }
    const_iterator constEnd() const noexcept { return d->constEnd(); }
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

    iterator erase(const_iterator begin, const_iterator end);
    inline iterator erase(const_iterator pos) { return erase(pos, pos+1); }

    // more Qt
    inline T& first() { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T &first() const { Q_ASSERT(!isEmpty()); return *begin(); }
    inline const T &constFirst() const { Q_ASSERT(!isEmpty()); return *begin(); }
    inline T& last() { Q_ASSERT(!isEmpty()); return *(end()-1); }
    inline const T &last() const { Q_ASSERT(!isEmpty()); return *(end()-1); }
    inline const T &constLast() const { Q_ASSERT(!isEmpty()); return *(end()-1); }
    inline bool startsWith(const T &t) const { return !isEmpty() && first() == t; }
    inline bool endsWith(const T &t) const { return !isEmpty() && last() == t; }
    QList<T> mid(qsizetype pos, qsizetype len = -1) const;

    QList<T> first(qsizetype n) const
    {
        Q_ASSERT(size_t(n) <= size_t(size()));
        return QList<T>(begin(), begin() + n);
    }
    QList<T> last(qsizetype n) const
    {
        Q_ASSERT(size_t(n) <= size_t(size()));
        return QList<T>(end() - n, end());
    }
    QList<T> sliced(qsizetype pos) const
    {
        Q_ASSERT(size_t(pos) <= size_t(size()));
        return QList<T>(begin() + pos, end());
    }
    QList<T> sliced(qsizetype pos, qsizetype n) const
    {
        Q_ASSERT(size_t(pos) <= size_t(size()));
        Q_ASSERT(n >= 0);
        Q_ASSERT(pos + n <= size());
        return QList<T>(begin() + pos, begin() + pos + n);
    }

    T value(qsizetype i) const { return value(i, T()); }
    T value(qsizetype i, const T &defaultValue) const;

    void swapItemsAt(qsizetype i, qsizetype j) {
        Q_ASSERT_X(i >= 0 && i < size() && j >= 0 && j < size(),
                    "QList<T>::swap", "index out of range");
        detach();
        qSwap(d->begin()[i], d->begin()[j]);
    }

    // STL compatibility
    inline void push_back(const T &t) { append(t); }
    void push_back(rvalue_ref t) { append(std::move(t)); }
    void push_front(rvalue_ref t) { prepend(std::move(t)); }
    inline void push_front(const T &t) { prepend(t); }
    void pop_back() { removeLast(); }
    void pop_front() { removeFirst(); }

    template <typename ...Args>
    reference emplace_back(Args&&... args) { return emplaceBack(std::forward<Args>(args)...); }

    inline bool empty() const
    { return d->size == 0; }
    inline reference front() { return first(); }
    inline const_reference front() const { return first(); }
    inline reference back() { return last(); }
    inline const_reference back() const { return last(); }
    void shrink_to_fit() { squeeze(); }

    // comfort
    QList<T> &operator+=(const QList<T> &l) { append(l.cbegin(), l.cend()); return *this; }
    QList<T> &operator+=(QList<T> &&l) { append(std::move(l)); return *this; }
    inline QList<T> operator+(const QList<T> &l) const
    { QList n = *this; n += l; return n; }
    inline QList<T> operator+(QList<T> &&l) const
    { QList n = *this; n += std::move(l); return n; }
    inline QList<T> &operator+=(const T &t)
    { append(t); return *this; }
    inline QList<T> &operator<< (const T &t)
    { append(t); return *this; }
    inline QList<T> &operator<<(const QList<T> &l)
    { *this += l; return *this; }
    inline QList<T> &operator<<(QList<T> &&l)
    { *this += std::move(l); return *this; }
    inline QList<T> &operator+=(rvalue_ref t)
    { append(std::move(t)); return *this; }
    inline QList<T> &operator<<(rvalue_ref t)
    { append(std::move(t)); return *this; }

    // Consider deprecating in 6.4 or later
    static QList<T> fromList(const QList<T> &list) { return list; }
    QList<T> toList() const { return *this; }

    static inline QList<T> fromVector(const QList<T> &vector) { return vector; }
    inline QList<T> toVector() const { return *this; }

    template<qsizetype N>
    static QList<T> fromReadOnlyData(const T (&t)[N])
    {
        return QList<T>({ nullptr, const_cast<T *>(t), N });
    }
};

#if defined(__cpp_deduction_guides) && __cpp_deduction_guides >= 201606
template <typename InputIterator,
          typename ValueType = typename std::iterator_traits<InputIterator>::value_type,
          QtPrivate::IfIsInputIterator<InputIterator> = true>
QList(InputIterator, InputIterator) -> QList<ValueType>;
#endif

template <typename T>
inline void QList<T>::resize_internal(qsizetype newSize, Qt::Initialization)
{
    Q_ASSERT(newSize >= 0);

    if (d->needsDetach() || newSize > capacity()) {
        // must allocate memory
        DataPointer detached(Data::allocate(d->detachCapacity(newSize),
                                            d->detachFlags()));
        if (size() && newSize) {
            detached->copyAppend(constBegin(), constBegin() + qMin(newSize, size()));
        }
        d.swap(detached);
    }

    if (newSize < size())
        d->truncate(newSize);
}

template <typename T>
void QList<T>::reserve(qsizetype asize)
{
    // capacity() == 0 for immutable data, so this will force a detaching below
    if (asize <= capacity()) {
        if (d->flags() & Data::CapacityReserved)
            return;  // already reserved, don't shrink
        if (!d->isShared()) {
            // accept current allocation, don't shrink
            d->setFlag(Data::CapacityReserved);
            return;
        }
    }

    DataPointer detached(Data::allocate(qMax(asize, size()),
                                        d->detachFlags() | Data::CapacityReserved));
    detached->copyAppend(constBegin(), constEnd());
    d.swap(detached);
}

template <typename T>
inline void QList<T>::squeeze()
{
    if (d->needsDetach() || size() != capacity()) {
        // must allocate memory
        DataPointer detached(Data::allocate(size(), d->detachFlags() & ~Data::CapacityReserved));
        if (size()) {
            detached->copyAppend(constBegin(), constEnd());
        }
        d.swap(detached);
    }
}

template <typename T>
inline void QList<T>::remove(qsizetype i, qsizetype n)
{
    Q_ASSERT_X(size_t(i) + size_t(n) <= size_t(d->size), "QList::remove", "index out of range");
    Q_ASSERT_X(n >= 0, "QList::remove", "invalid count");

    if (n == 0)
        return;

    const size_t newSize = size() - n;
    if (d->needsDetach() ||
            ((d->flags() & Data::CapacityReserved) == 0
             && newSize < d->allocatedCapacity()/2)) {
        // allocate memory
        DataPointer detached(Data::allocate(d->detachCapacity(newSize),
                             d->detachFlags() & ~(Data::GrowsBackwards | Data::GrowsForward)));
        const_iterator where = constBegin() + i;
        if (newSize) {
            detached->copyAppend(constBegin(), where);
            detached->copyAppend(where + n, constEnd());
        }
        d.swap(detached);
    } else {
        // we're detached and we can just move data around
        d->erase(d->begin() + i, d->begin() + i + n);
    }
}

template <typename T>
inline void QList<T>::prepend(const T &t)
{ insert(0, 1, t); }
template <typename T>
void QList<T>::prepend(rvalue_ref t)
{ insert(0, std::move(t)); }

template<typename T>
inline T QList<T>::value(qsizetype i, const T &defaultValue) const
{
    return size_t(i) < size_t(d->size) ? at(i) : defaultValue;
}

template <typename T>
inline void QList<T>::append(const_iterator i1, const_iterator i2)
{
    if (i1 == i2)
        return;
    const size_t newSize = size() + std::distance(i1, i2);
    if (d->needsDetach() || newSize > d->allocatedCapacity()) {
        DataPointer detached(Data::allocate(d->detachCapacity(newSize),
                                            d->detachFlags() | Data::GrowsForward));
        detached->copyAppend(constBegin(), constEnd());
        detached->copyAppend(i1, i2);
        d.swap(detached);
    } else {
        // we're detached and we can just move data around
        d->copyAppend(i1, i2);
    }
}

template <typename T>
inline void QList<T>::append(QList<T> &&other)
{
    if (other.isEmpty())
        return;
    if (other.d->needsDetach() || !std::is_nothrow_move_constructible_v<T>)
        return append(other);

    const size_t newSize = size() + other.size();
    if (d->needsDetach() || newSize > d->allocatedCapacity()) {
        DataPointer detached(Data::allocate(d->detachCapacity(newSize),
                                            d->detachFlags() | Data::GrowsForward));

        if (!d->needsDetach())
            detached->moveAppend(begin(), end());
        else
            detached->copyAppend(cbegin(), cend());
        detached->moveAppend(other.begin(), other.end());

        d.swap(detached);
    } else {
        // we're detached and we can just move data around
        d->moveAppend(other.begin(), other.end());
    }
}


template <typename T>
inline typename QList<T>::iterator
QList<T>::insert(qsizetype i, qsizetype n, parameter_type t)
{
    Q_ASSERT_X(size_t(i) <= size_t(d->size), "QList<T>::insert", "index out of range");

    // we don't have a quick exit for n == 0
    // it's not worth wasting CPU cycles for that

    const size_t newSize = size() + n;
    if (d->needsDetach() || newSize > d->allocatedCapacity()) {
        typename Data::ArrayOptions flags = d->detachFlags() | Data::GrowsForward;

        DataPointer detached(Data::allocate(d->detachCapacity(newSize), flags));
        const_iterator where = constBegin() + i;
        detached->copyAppend(constBegin(), where);
        detached->copyAppend(n, t);
        detached->copyAppend(where, constEnd());
        d.swap(detached);
    } else {
        // we're detached and we can just move data around
        if (i == size()) {
            d->copyAppend(n, t);
        } else {
            T copy(t);
            d->insert(d.begin() + i, n, copy);
        }
    }
    return d.begin() + i;
}

template <typename T>
template <typename ...Args>
typename QList<T>::iterator
QList<T>::emplace(qsizetype i, Args&&... args)
{
     Q_ASSERT_X(i >= 0 && i <= d->size, "QList<T>::insert", "index out of range");

    const size_t newSize = size() + 1;
    if (d->needsDetach() || newSize > d->allocatedCapacity()) {
        typename Data::ArrayOptions flags = d->detachFlags() | Data::GrowsForward;

        DataPointer detached(Data::allocate(d->detachCapacity(newSize), flags));
        const_iterator where = constBegin() + i;

        // First, create an element to handle cases, when a user moves
        // the element from a container to the same container
        detached->createInPlace(detached.begin() + i, std::forward<Args>(args)...);

        // Then, put the first part of the elements to the new location
        detached->copyAppend(constBegin(), where);

        // After that, increase the actual size, because we created
        // one extra element
        ++detached.size;

        // Finally, put the rest of the elements to the new location
        detached->copyAppend(where, constEnd());

        d.swap(detached);
    } else {
        d->emplace(d.begin() + i, std::forward<Args>(args)...);
    }
    return d.begin() + i;
}

template <typename T>
typename QList<T>::iterator QList<T>::erase(const_iterator abegin, const_iterator aend)
{
    Q_ASSERT_X(isValidIterator(abegin), "QList::erase", "The specified iterator argument 'abegin' is invalid");
    Q_ASSERT_X(isValidIterator(aend), "QList::erase", "The specified iterator argument 'aend' is invalid");
    Q_ASSERT(aend >= abegin);

    qsizetype i = std::distance(d.constBegin(), abegin);
    qsizetype n = std::distance(abegin, aend);
    remove(i, n);

    return d.begin() + i;
}

template <typename T>
inline QList<T> &QList<T>::fill(parameter_type t, qsizetype newSize)
{
    if (newSize == -1)
        newSize = size();
    if (d->needsDetach() || newSize > capacity()) {
        // must allocate memory
        DataPointer detached(Data::allocate(d->detachCapacity(newSize),
                                            d->detachFlags()));
        detached->copyAppend(newSize, t);
        d.swap(detached);
    } else {
        // we're detached
        const T copy(t);
        d->assign(d.begin(), d.begin() + qMin(size(), newSize), t);
        if (newSize > size())
            d->copyAppend(newSize - size(), copy);
    }
    return *this;
}

namespace QtPrivate {
template <typename T, typename U>
qsizetype indexOf(const QList<T> &vector, const U &u, qsizetype from)
{
    if (from < 0)
        from = qMax(from + vector.size(), qsizetype(0));
    if (from < vector.size()) {
        auto n = vector.begin() + from - 1;
        auto e = vector.end();
        while (++n != e)
            if (*n == u)
                return qsizetype(n - vector.begin());
    }
    return -1;
}

template <typename T, typename U>
qsizetype lastIndexOf(const QList<T> &vector, const U &u, qsizetype from)
{
    if (from < 0)
        from += vector.d->size;
    else if (from >= vector.size())
        from = vector.size() - 1;
    if (from >= 0) {
        auto b = vector.begin();
        auto n = vector.begin() + from + 1;
        while (n != b) {
            if (*--n == u)
                return qsizetype(n - b);
        }
    }
    return -1;
}
}

template <typename T>
qsizetype QList<T>::indexOf(const T &t, qsizetype from) const noexcept
{
    return QtPrivate::indexOf<T, T>(*this, t, from);
}

template <typename T>
qsizetype QList<T>::lastIndexOf(const T &t, qsizetype from) const noexcept
{
    return QtPrivate::lastIndexOf(*this, t, from);
}

template <typename T>
inline QList<T> QList<T>::mid(qsizetype pos, qsizetype len) const
{
    qsizetype p = pos;
    qsizetype l = len;
    using namespace QtPrivate;
    switch (QContainerImplHelper::mid(d.size, &p, &l)) {
    case QContainerImplHelper::Null:
    case QContainerImplHelper::Empty:
        return QList();
    case QContainerImplHelper::Full:
        return *this;
    case QContainerImplHelper::Subset:
        break;
    }

    // Allocate memory
    DataPointer copied(Data::allocate(l));
    copied->copyAppend(constBegin() + p, constBegin() + p + l);
    return copied;
}

Q_DECLARE_SEQUENTIAL_ITERATOR(List)
Q_DECLARE_MUTABLE_SEQUENTIAL_ITERATOR(List)

template <typename T>
size_t qHash(const QList<T> &key, size_t seed = 0)
    noexcept(noexcept(qHashRange(key.cbegin(), key.cend(), seed)))
{
    return qHashRange(key.cbegin(), key.cend(), seed);
}

template <typename U>
QTypeTraits::compare_eq_result<U> operator==(const QList<U> &l, const QList<U> &r)
{
    if (l.size() != r.size())
        return false;
    if (l.begin() == r.begin())
        return true;

    // do element-by-element comparison
    return l.d->compare(l.begin(), r.begin(), l.size());
}

template <typename U>
QTypeTraits::compare_eq_result<U> operator!=(const QList<U> &l, const QList<U> &r)
{
    return !(l == r);
}

template <typename T>
auto operator<(const QList<T> &lhs, const QList<T> &rhs)
    noexcept(noexcept(std::lexicographical_compare(lhs.begin(), lhs.end(),
                                                   rhs.begin(), rhs.end())))
    -> decltype(std::declval<T>() < std::declval<T>())
{
    return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                        rhs.begin(), rhs.end());
}

template <typename T>
auto operator>(const QList<T> &lhs, const QList<T> &rhs)
    noexcept(noexcept(lhs < rhs))
    -> decltype(lhs < rhs)
{
    return rhs < lhs;
}

template <typename T>
auto operator<=(const QList<T> &lhs, const QList<T> &rhs)
    noexcept(noexcept(lhs < rhs))
    -> decltype(lhs < rhs)
{
    return !(lhs > rhs);
}

template <typename T>
auto operator>=(const QList<T> &lhs, const QList<T> &rhs)
    noexcept(noexcept(lhs < rhs))
    -> decltype(lhs < rhs)
{
    return !(lhs < rhs);
}

QList<uint> QStringView::toUcs4() const { return QtPrivate::convertToUcs4(*this); }

QT_END_NAMESPACE

#include <QtCore/qbytearraylist.h>
#include <QtCore/qstringlist.h>

#endif // QLIST_H
