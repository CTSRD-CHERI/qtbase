/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>
#include <QtCore/QString>
#include <QtCore/qarraydata.h>

#include "simplevector.h"

class tst_QArrayData : public QObject
{
    Q_OBJECT

private slots:
    void referenceCounting();
    void simpleVector();
    void simpleVectorReserve_data();
    void simpleVectorReserve();
    void allocate_data();
    void allocate();
    void reallocate_data() { allocate_data(); }
    void reallocate();
    void alignment_data();
    void alignment();
    void typedData();
    void gccBug43247();
    void arrayOps();
    void arrayOps2();
    void fromRawData_data();
    void fromRawData();
    void literals();
    void variadicLiterals();
    void rValueReferences();
    void grow();
};

template <class T> const T &const_(const T &t) { return t; }

void tst_QArrayData::referenceCounting()
{
    {
        // Reference counting initialized to 1 (owned)
        QArrayData array = { Q_BASIC_ATOMIC_INITIALIZER(1), 0, 0 };

        QCOMPARE(array.ref_.loadRelaxed(), 1);

        QVERIFY(array.ref());
        QCOMPARE(array.ref_.loadRelaxed(), 2);

        QVERIFY(array.deref());
        QCOMPARE(array.ref_.loadRelaxed(), 1);

        QVERIFY(array.ref());
        QCOMPARE(array.ref_.loadRelaxed(), 2);

        QVERIFY(array.deref());
        QCOMPARE(array.ref_.loadRelaxed(), 1);

        QVERIFY(!array.deref());
        QCOMPARE(array.ref_.loadRelaxed(), 0);

        // Now would be a good time to free/release allocated data
    }
}

void tst_QArrayData::simpleVector()
{
    int data[] = { 0, 1, 2, 3, 4, 5, 6 };
    int array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    SimpleVector<int> v1;
    SimpleVector<int> v2(v1);
    SimpleVector<int> v3(nullptr, nullptr, 0);
    SimpleVector<int> v4(nullptr, data, 0);
    SimpleVector<int> v5(nullptr, data, 1);
    SimpleVector<int> v6(nullptr, data, 7);
    SimpleVector<int> v7(10, 5);
    SimpleVector<int> v8(array, array + sizeof(array)/sizeof(*array));

    v3 = v1;
    v1.swap(v3);
    v4.clear();

    QVERIFY(v1.isNull());
    QVERIFY(v2.isNull());
    QVERIFY(v3.isNull());
    QVERIFY(v4.isNull());
    QVERIFY(!v5.isNull());
    QVERIFY(!v6.isNull());
    QVERIFY(!v7.isNull());
    QVERIFY(!v8.isNull());

    QVERIFY(v1.isEmpty());
    QVERIFY(v2.isEmpty());
    QVERIFY(v3.isEmpty());
    QVERIFY(v4.isEmpty());
    QVERIFY(!v5.isEmpty());
    QVERIFY(!v6.isEmpty());
    QVERIFY(!v7.isEmpty());
    QVERIFY(!v8.isEmpty());

    QCOMPARE(v1.size(), size_t(0));
    QCOMPARE(v2.size(), size_t(0));
    QCOMPARE(v3.size(), size_t(0));
    QCOMPARE(v4.size(), size_t(0));
    QCOMPARE(v5.size(), size_t(1));
    QCOMPARE(v6.size(), size_t(7));
    QCOMPARE(v7.size(), size_t(10));
    QCOMPARE(v8.size(), size_t(10));

    QCOMPARE(v1.capacity(), size_t(0));
    QCOMPARE(v2.capacity(), size_t(0));
    QCOMPARE(v3.capacity(), size_t(0));
    QCOMPARE(v4.capacity(), size_t(0));
    QCOMPARE(v5.capacity(), size_t(0));
    // v6.capacity() is unspecified, for now
    QVERIFY(v7.capacity() >= size_t(10));
    QVERIFY(v8.capacity() >= size_t(10));

    QVERIFY(v1.isStatic());
    QVERIFY(v2.isStatic());
    QVERIFY(v3.isStatic());
    QVERIFY(v4.isStatic());
    QVERIFY(v5.isStatic());
    QVERIFY(v6.isStatic());
    QVERIFY(!v7.isStatic());
    QVERIFY(!v8.isStatic());

    QVERIFY(v1.isShared());
    QVERIFY(v2.isShared());
    QVERIFY(v3.isShared());
    QVERIFY(v4.isShared());
    QVERIFY(v5.isShared());
    QVERIFY(v6.isShared());
    QVERIFY(!v7.isShared());
    QVERIFY((SimpleVector<int>(v7), v7.isShared()));
    QVERIFY(!v7.isShared());
    QVERIFY(!v8.isShared());

    QVERIFY(v1.isSharedWith(v2));
    QVERIFY(v1.isSharedWith(v3));
    QVERIFY(v1.isSharedWith(v4));
    QVERIFY(!v1.isSharedWith(v5));
    QVERIFY(!v1.isSharedWith(v6));

    QCOMPARE(v1.constBegin(), v1.constEnd());
    QCOMPARE(v4.constBegin(), v4.constEnd());
    QCOMPARE((v6.constBegin() + v6.size()), v6.constEnd());
    QCOMPARE((v7.constBegin() + v7.size()), v7.constEnd());
    QCOMPARE((v8.constBegin() + v8.size()), v8.constEnd());

    QVERIFY(v1 == v2);
    QVERIFY(v1 == v3);
    QVERIFY(v1 == v4);
    QVERIFY(v1 != v5);
    QVERIFY(!(v1 == v6));

    QVERIFY(v1 != v6);
    QVERIFY(v4 != v6);
    QVERIFY(v5 != v6);
    QVERIFY(!(v1 == v5));

    QVERIFY(v1 < v6);
    QVERIFY(!(v6 < v1));
    QVERIFY(v6 > v1);
    QVERIFY(!(v1 > v6));
    QVERIFY(v1 <= v6);
    QVERIFY(!(v6 <= v1));
    QVERIFY(v6 >= v1);
    QVERIFY(!(v1 >= v6));

    {
        SimpleVector<int> temp(v6);

        QCOMPARE(const_(v6).front(), 0);
        QCOMPARE(const_(v6).back(), 6);

        QVERIFY(temp.isShared());
        QVERIFY(temp.isSharedWith(v6));

        QCOMPARE(temp.front(), 0);
        QCOMPARE(temp.back(), 6);

        // Detached
        QVERIFY(!temp.isShared());
        const int *const tempBegin = temp.begin();

        for (size_t i = 0; i < v6.size(); ++i) {
            QCOMPARE(const_(v6)[i], int(i));
            QCOMPARE(const_(v6).at(i), int(i));
            QCOMPARE(&const_(v6)[i], &const_(v6).at(i));

            QCOMPARE(const_(v8)[i], const_(v6)[i]);

            QCOMPARE(temp[i], int(i));
            QCOMPARE(temp.at(i), int(i));
            QCOMPARE(&temp[i], &temp.at(i));
        }

        // A single detach should do
        QCOMPARE((const int *)temp.begin(), tempBegin);
    }

    {
        int count = 0;
        Q_FOREACH (int value, v7) {
            QCOMPARE(value, 5);
            ++count;
        }

        QCOMPARE(count, 10);
    }

    {
        int count = 0;
        Q_FOREACH (int value, v8) {
            QCOMPARE(value, count);
            ++count;
        }

        QCOMPARE(count, 10);
    }

    v5 = v6;
    QVERIFY(v5.isSharedWith(v6));
    QVERIFY(!v1.isSharedWith(v5));

    v1.swap(v6);
    QVERIFY(v6.isNull());
    QVERIFY(v1.isSharedWith(v5));

    {
        using std::swap;
        swap(v1, v6);
        QVERIFY(v5.isSharedWith(v6));
        QVERIFY(!v1.isSharedWith(v5));
    }

    v1.prepend(array, array + sizeof(array)/sizeof(array[0]));
    QCOMPARE(v1.size(), size_t(10));
    QVERIFY(v1 == v8);

    v6 = v1;
    QVERIFY(v1.isSharedWith(v6));

    v1.prepend(array, array + sizeof(array)/sizeof(array[0]));
    QVERIFY(!v1.isSharedWith(v6));
    QCOMPARE(v1.size(), size_t(20));
    QCOMPARE(v6.size(), size_t(10));

    for (int i = 0; i < 20; ++i)
        QCOMPARE(v1[i], v6[i % 10]);

    v1.clear();

    v1.append(array, array + sizeof(array)/sizeof(array[0]));
    // v1 is now [0 .. 9]
    QCOMPARE(v1.size(), size_t(10));
    QVERIFY(v1 == v8);

    v6 = v1;
    QVERIFY(v1.isSharedWith(v6));

    v1.append(array, array + sizeof(array)/sizeof(array[0]));
    // v1 is now [0 .. 9, 0 .. 9]
    QVERIFY(!v1.isSharedWith(v6));
    QCOMPARE(v1.size(), size_t(20));
    QCOMPARE(v6.size(), size_t(10));

    for (int i = 0; i < 20; ++i)
        QCOMPARE(v1[i], v6[i % 10]);

    v1.insert(0, v6.constBegin(), v6.constEnd());
    // v1 is now [ 0 .. 9, 0 .. 9, 0 .. 9]
    QCOMPARE(v1.size(), size_t(30));

    for (int i = 0; i < 30; ++i)
        QCOMPARE(v1[i], v8[i % 10]);

    v6 = v1;
    QVERIFY(v1.isSharedWith(v6));

    v1.insert(10, v6.constBegin(), v6.constEnd());
    // v1 is now [ 0..9, <new data>0..9, 0..9, 0..9</new data>, 0..9, 0..9 ]
    QVERIFY(!v1.isSharedWith(v6));
    QCOMPARE(v1.size(), size_t(60));
    QCOMPARE(v6.size(), size_t(30));

    for (int i = 0; i < 30; ++i)
        QCOMPARE(v6[i], v8[i % 10]);
    for (int i = 0; i < 60; ++i)
        QCOMPARE(v1[i], v8[i % 10]);

    v1.insert(int(v1.size()), v6.constBegin(), v6.constEnd());
    // v1 is now [ 0..9 x 6, <new data>0..9 x 3</new data> ]
    QCOMPARE(v1.size(), size_t(90));

    for (int i = 0; i < 90; ++i)
        QCOMPARE(v1[i], v8[i % 10]);

    v1.insert(-1, v8.constBegin(), v8.constEnd());
    // v1 is now [ 0..9 x 9, <new data>0..9</new data> ]
    QCOMPARE(v1.size(), size_t(100));

    for (int i = 0; i < 100; ++i)
        QCOMPARE(v1[i], v8[i % 10]);

    v1.insert(-11, v8.constBegin(), v8.constEnd());
    // v1 is now [ 0..9 x 9, <new data>0..9</new data>, 0..9 ]
    QCOMPARE(v1.size(), size_t(110));

    for (int i = 0; i < 110; ++i)
        QCOMPARE(v1[i], v8[i % 10]);

    v1.insert(-200, v8.constBegin(), v8.constEnd());
    // v1 is now [ <new data>0..9</new data>, 0..9 x 11 ]
    QCOMPARE(v1.size(), size_t(120));

    for (int i = 0; i < 120; ++i)
        QCOMPARE(v1[i], v8[i % 10]);
}

Q_DECLARE_METATYPE(SimpleVector<int>)

void tst_QArrayData::simpleVectorReserve_data()
{
    QTest::addColumn<SimpleVector<int> >("vector");
    QTest::addColumn<size_t>("capacity");
    QTest::addColumn<size_t>("size");

    QTest::newRow("null") << SimpleVector<int>() << size_t(0) << size_t(0);
    QTest::newRow("empty") << SimpleVector<int>(0, 42) << size_t(0) << size_t(0);
    QTest::newRow("non-empty") << SimpleVector<int>(5, 42) << size_t(5) << size_t(5);

    static const int array[] =
        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

    QTest::newRow("raw-data") << SimpleVector<int>::fromRawData(array, 15) << size_t(0) << size_t(15);
}

void tst_QArrayData::simpleVectorReserve()
{
    QFETCH(SimpleVector<int>, vector);
    QFETCH(size_t, capacity);
    QFETCH(size_t, size);

    QVERIFY(!capacity || capacity >= size);

    QCOMPARE(vector.capacity(), capacity);
    QCOMPARE(vector.size(), size);

    const SimpleVector<int> copy(vector);

    vector.reserve(0);
    QCOMPARE(vector.capacity(), capacity);
    QCOMPARE(vector.size(), size);

    vector.reserve(10);

    // zero-capacity (immutable) resets with detach
    if (!capacity)
        capacity = size;

    QCOMPARE(vector.capacity(), qMax(size_t(10), capacity));
    QCOMPARE(vector.size(), size);

    vector.reserve(20);
    QCOMPARE(vector.capacity(), size_t(20));
    QCOMPARE(vector.size(), size);

    vector.reserve(30);
    QCOMPARE(vector.capacity(), size_t(30));
    QCOMPARE(vector.size(), size);

    QVERIFY(vector == copy);
}

struct Deallocator
{
    Deallocator(size_t objectSize, size_t alignment)
        : objectSize(objectSize)
        , alignment(alignment)
    {
    }

    ~Deallocator()
    {
        Q_FOREACH (QArrayData *data, headers)
            QArrayData::deallocate(data, objectSize, alignment);
    }

    size_t objectSize;
    size_t alignment;
    QList<QArrayData *> headers;
};

Q_DECLARE_METATYPE(const QArrayData *)
Q_DECLARE_METATYPE(QArrayData::ArrayOptions)

void tst_QArrayData::allocate_data()
{
    QTest::addColumn<size_t>("objectSize");
    QTest::addColumn<size_t>("alignment");
    QTest::addColumn<QArrayData::ArrayOptions>("allocateOptions");
    QTest::addColumn<bool>("isCapacityReserved");

    struct {
        char const *typeName;
        size_t objectSize;
        size_t alignment;
    } types[] = {
        { "char", sizeof(char), alignof(char) },
        { "short", sizeof(short), alignof(short) },
        { "void *", sizeof(void *), alignof(void *) }
    };

    struct {
        char const *description;
        QArrayData::ArrayOptions allocateOptions;
        bool isCapacityReserved;
    } options[] = {
        { "Default", QArrayData::DefaultAllocationFlags, false },
        { "Reserved", QArrayData::CapacityReserved, true },
        { "Grow", QArrayData::GrowsForward, false }
    };

    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i)
        for (size_t j = 0; j < sizeof(options)/sizeof(options[0]); ++j)
            QTest::newRow(qPrintable(
                        QLatin1String(types[i].typeName)
                        + QLatin1String(": ")
                        + QLatin1String(options[j].description)))
                << types[i].objectSize << types[i].alignment
                << options[j].allocateOptions << options[j].isCapacityReserved;
}

void tst_QArrayData::allocate()
{
    QFETCH(size_t, objectSize);
    QFETCH(size_t, alignment);
    QFETCH(QArrayData::ArrayOptions, allocateOptions);
    QFETCH(bool, isCapacityReserved);

    // Minimum alignment that can be requested is that of QArrayData.
    // Typically, this alignment is sizeof(void *) and ensured by malloc.
    size_t minAlignment = qMax(alignment, alignof(QArrayData));

    Deallocator keeper(objectSize, minAlignment);
    keeper.headers.reserve(1024);

    for (qsizetype capacity = 1; capacity <= 1024; capacity <<= 1) {
        QArrayData *data;
        void *dataPointer = QArrayData::allocate(&data, objectSize, minAlignment,
                capacity, QArrayData::ArrayOptions(allocateOptions));

        keeper.headers.append(data);

        if (allocateOptions & QArrayData::GrowsForward)
            QVERIFY(data->allocatedCapacity() > capacity);
        else
            QCOMPARE(data->allocatedCapacity(), capacity);
        QCOMPARE(bool(data->flags & QArrayData::CapacityReserved), isCapacityReserved);

        // Check that the allocated array can be used. Best tested with a
        // memory checker, such as valgrind, running.
        ::memset(dataPointer, 'A', objectSize * capacity);
    }
}

void tst_QArrayData::reallocate()
{
    QFETCH(size_t, objectSize);
    QFETCH(size_t, alignment);
    QFETCH(QArrayData::ArrayOptions, allocateOptions);
    QFETCH(bool, isCapacityReserved);

    // Minimum alignment that can be requested is that of QArrayData.
    // Typically, this alignment is sizeof(void *) and ensured by malloc.
    size_t minAlignment = qMax(alignment, alignof(QArrayData));

    int capacity = 10;
    Deallocator keeper(objectSize, minAlignment);
    QArrayData *data;
    void *dataPointer = QArrayData::allocate(&data, objectSize, minAlignment, capacity,
                                             QArrayData::ArrayOptions(allocateOptions) & ~QArrayData::GrowsForward);
    keeper.headers.append(data);

    memset(dataPointer, 'A', objectSize * capacity);

    // now try to reallocate
    int newCapacity = 40;
    auto pair = QArrayData::reallocateUnaligned(data, dataPointer, objectSize, newCapacity,
                                                QArrayData::ArrayOptions(allocateOptions));
    data = pair.first;
    dataPointer = pair.second;
    QVERIFY(data);
    keeper.headers.clear();
    keeper.headers.append(data);

    if (allocateOptions & QArrayData::GrowsForward)
        QVERIFY(data->allocatedCapacity() > newCapacity);
    else
        QCOMPARE(data->allocatedCapacity(), newCapacity);
    QCOMPARE(!(data->flags & QArrayData::CapacityReserved), !isCapacityReserved);

    for (int i = 0; i < capacity; ++i)
        QCOMPARE(static_cast<char *>(dataPointer)[i], 'A');
}

class Unaligned
{
    Q_DECL_UNUSED_MEMBER char dummy[8];
};

void tst_QArrayData::alignment_data()
{
    QTest::addColumn<size_t>("alignment");

    for (size_t i = 1; i < 10; ++i) {
        size_t alignment = size_t(1u) << i;
        QTest::newRow(qPrintable(QString::number(alignment))) << alignment;
    }
}

void tst_QArrayData::alignment()
{
    QFETCH(size_t, alignment);

    // Minimum alignment that can be requested is that of QArrayData.
    // Typically, this alignment is sizeof(void *) and ensured by malloc.
    size_t minAlignment = qMax(alignment, alignof(QArrayData));

    Deallocator keeper(sizeof(Unaligned), minAlignment);
    keeper.headers.reserve(100);

    for (int i = 0; i < 100; ++i) {
        QArrayData *data;
        void *dataPointer = QArrayData::allocate(&data, sizeof(Unaligned),
                minAlignment, 8, QArrayData::DefaultAllocationFlags);
        keeper.headers.append(data);

        QVERIFY(data);
        QVERIFY(data->allocatedCapacity() >= uint(8));

        // These conditions should hold as long as header and array are
        // allocated together
        qptrdiff offset = reinterpret_cast<char *>(dataPointer) -
                reinterpret_cast<char *>(data);
        QVERIFY(offset >= qptrdiff(sizeof(QArrayData)));
        QVERIFY(offset <= qptrdiff(sizeof(QArrayData)
                    + minAlignment - alignof(QArrayData)));

        // Data is aligned
        QCOMPARE(quintptr(quintptr(dataPointer) % alignment), quintptr(0u));

        // Check that the allocated array can be used. Best tested with a
        // memory checker, such as valgrind, running.
        ::memset(dataPointer, 'A', sizeof(Unaligned) * 8);
    }
}

void tst_QArrayData::typedData()
{
    {
        Deallocator keeper(sizeof(char),
                alignof(QTypedArrayData<char>::AlignmentDummy));
        QPair<QTypedArrayData<char> *, char *> pair = QTypedArrayData<char>::allocate(10);
        QArrayData *array = pair.first;
        keeper.headers.append(array);

        QVERIFY(array);
        QCOMPARE(array->allocatedCapacity(), qsizetype(10));

        // Check that the allocated array can be used. Best tested with a
        // memory checker, such as valgrind, running.
        ::memset(pair.second, 0, 10 * sizeof(char));

        keeper.headers.clear();
        QTypedArrayData<short>::deallocate(array);

        QVERIFY(true);
    }

    {
        Deallocator keeper(sizeof(short),
                alignof(QTypedArrayData<short>::AlignmentDummy));
        QPair<QTypedArrayData<short> *, short *> pair = QTypedArrayData<short>::allocate(10);
        QArrayData *array = pair.first;
        keeper.headers.append(array);

        QVERIFY(array);
        QCOMPARE(array->allocatedCapacity(), qsizetype(10));

        // Check that the allocated array can be used. Best tested with a
        // memory checker, such as valgrind, running.
        ::memset(pair.second, 0, 10 * sizeof(short));

        keeper.headers.clear();
        QTypedArrayData<short>::deallocate(array);

        QVERIFY(true);
    }

    {
        Deallocator keeper(sizeof(double),
                alignof(QTypedArrayData<double>::AlignmentDummy));
        QPair<QTypedArrayData<double> *, double *> pair = QTypedArrayData<double>::allocate(10);
        QArrayData *array = pair.first;
        keeper.headers.append(array);

        QVERIFY(array);
        QCOMPARE(array->allocatedCapacity(), qsizetype(10));

        // Check that the allocated array can be used. Best tested with a
        // memory checker, such as valgrind, running.
        ::memset(pair.second, 0, 10 * sizeof(double));

        keeper.headers.clear();
        QTypedArrayData<double>::deallocate(array);

        QVERIFY(true);
    }
}

void tst_QArrayData::gccBug43247()
{
    // This test tries to verify QArrayData is not affected by GCC optimizer
    // bug #43247.
    // Reported on GCC 4.4.3, Linux, affects QList

    QTest::ignoreMessage(QtDebugMsg, "GCC Optimization bug #43247 not triggered (3)");
    QTest::ignoreMessage(QtDebugMsg, "GCC Optimization bug #43247 not triggered (4)");
    QTest::ignoreMessage(QtDebugMsg, "GCC Optimization bug #43247 not triggered (5)");
    QTest::ignoreMessage(QtDebugMsg, "GCC Optimization bug #43247 not triggered (6)");
    QTest::ignoreMessage(QtDebugMsg, "GCC Optimization bug #43247 not triggered (7)");

    SimpleVector<int> array(10, 0);
    // QList<int> list(10, 0);

    for (int i = 0; i < 10; ++i) {
        if (i >= 3 && i < 8)
            qDebug("GCC Optimization bug #43247 not triggered (%i)", i);

        // When access to data is implemented through an array of size 1, this
        // line lets the compiler assume i == 0, and the conditional above is
        // skipped.
        QVERIFY(array.at(i) == 0);
        // QVERIFY(list.at(i) == 0);
    }
}

struct CountedObject
{
    CountedObject()
        : id(liveCount++)
        , flags(DefaultConstructed)
    {
    }

    CountedObject(const CountedObject &other)
        : id(other.id)
        , flags(other.flags == DefaultConstructed
                ? ObjectFlags(CopyConstructed | DefaultConstructed)
                : CopyConstructed)
    {
        ++liveCount;
    }

    ~CountedObject()
    {
        --liveCount;
    }

    CountedObject &operator=(const CountedObject &other)
    {
        flags = ObjectFlags(other.flags | CopyAssigned);
        id = other.id;
        return *this;
    }

    struct LeakChecker
    {
        LeakChecker()
            : previousLiveCount(liveCount)
        {
        }

        ~LeakChecker()
        {
            QCOMPARE(liveCount, previousLiveCount);
        }

    private:
        const size_t previousLiveCount;
    };

    enum ObjectFlags {
        DefaultConstructed  = 1,
        CopyConstructed     = 2,
        CopyAssigned        = 4
    };

    size_t id; // not unique
    ObjectFlags flags;

    static size_t liveCount;
};

size_t CountedObject::liveCount = 0;

void tst_QArrayData::arrayOps()
{
    CountedObject::LeakChecker leakChecker; Q_UNUSED(leakChecker);

    const int intArray[5] = { 80, 101, 100, 114, 111 };
    const QString stringArray[5] = {
        QLatin1String("just"),
        QLatin1String("for"),
        QLatin1String("testing"),
        QLatin1String("a"),
        QLatin1String("vector")
    };
    const CountedObject objArray[5];

    QVERIFY(!QTypeInfo<int>::isComplex && !QTypeInfo<int>::isStatic);
    QVERIFY(QTypeInfo<QString>::isComplex && !QTypeInfo<QString>::isStatic);
    QVERIFY(QTypeInfo<CountedObject>::isComplex && QTypeInfo<CountedObject>::isStatic);

    QCOMPARE(CountedObject::liveCount, size_t(5));
    for (size_t i = 0; i < 5; ++i)
        QCOMPARE(objArray[i].id, i);

    ////////////////////////////////////////////////////////////////////////////
    // copyAppend (I)
    SimpleVector<int> vi(intArray, intArray + 5);
    SimpleVector<QString> vs(stringArray, stringArray + 5);
    SimpleVector<CountedObject> vo(objArray, objArray + 5);

    QCOMPARE(CountedObject::liveCount, size_t(10));
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(vi[i], intArray[i]);
        QVERIFY(vs[i].isSharedWith(stringArray[i]));

        QCOMPARE(vo[i].id, objArray[i].id);
        QCOMPARE(int(vo[i].flags), CountedObject::CopyConstructed
                | CountedObject::DefaultConstructed);
    }

    ////////////////////////////////////////////////////////////////////////////
    // destroyAll
    vi.clear();
    vs.clear();
    vo.clear();

    QCOMPARE(CountedObject::liveCount, size_t(5));

    ////////////////////////////////////////////////////////////////////////////
    // copyAppend (II)
    int referenceInt = 7;
    QString referenceString = QLatin1String("reference");
    CountedObject referenceObject;

    vi = SimpleVector<int>(5, referenceInt);
    vs = SimpleVector<QString>(5, referenceString);
    vo = SimpleVector<CountedObject>(5, referenceObject);

    QCOMPARE(vi.size(), size_t(5));
    QCOMPARE(vs.size(), size_t(5));
    QCOMPARE(vo.size(), size_t(5));

    QCOMPARE(CountedObject::liveCount, size_t(11));
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(vi[i], referenceInt);
        QVERIFY(vs[i].isSharedWith(referenceString));

        QCOMPARE(vo[i].id, referenceObject.id);
        QCOMPARE(int(vo[i].flags), CountedObject::CopyConstructed
                | CountedObject::DefaultConstructed);
    }

    ////////////////////////////////////////////////////////////////////////////
    // insert
    vi.reserve(30);
    vs.reserve(30);
    vo.reserve(30);

    QCOMPARE(vi.size(), size_t(5));
    QCOMPARE(vs.size(), size_t(5));
    QCOMPARE(vo.size(), size_t(5));

    QVERIFY(vi.capacity() >= 30);
    QVERIFY(vs.capacity() >= 30);
    QVERIFY(vo.capacity() >= 30);

    // Displace as many elements as array is extended by
    vi.insert(0, intArray, intArray + 5);
    vs.insert(0, stringArray, stringArray + 5);
    vo.insert(0, objArray, objArray + 5);

    QCOMPARE(vi.size(), size_t(10));
    QCOMPARE(vs.size(), size_t(10));
    QCOMPARE(vo.size(), size_t(10));

    // Displace more elements than array is extended by
    vi.insert(0, intArray, intArray + 5);
    vs.insert(0, stringArray, stringArray + 5);
    vo.insert(0, objArray, objArray + 5);

    QCOMPARE(vi.size(), size_t(15));
    QCOMPARE(vs.size(), size_t(15));
    QCOMPARE(vo.size(), size_t(15));

    // Displace less elements than array is extended by
    vi.insert(5, vi.constBegin(), vi.constEnd());
    vs.insert(5, vs.constBegin(), vs.constEnd());
    vo.insert(5, vo.constBegin(), vo.constEnd());

    QCOMPARE(vi.size(), size_t(30));
    QCOMPARE(vs.size(), size_t(30));
    QCOMPARE(vo.size(), size_t(30));

    QCOMPARE(CountedObject::liveCount, size_t(36));
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(vi[i], intArray[i % 5]);
        QVERIFY(vs[i].isSharedWith(stringArray[i % 5]));

        QCOMPARE(vo[i].id, objArray[i % 5].id);
        QCOMPARE(int(vo[i].flags), CountedObject::DefaultConstructed
                | CountedObject::CopyAssigned);
    }

    for (int i = 5; i < 15; ++i) {
        QCOMPARE(vi[i], intArray[i % 5]);
        QVERIFY(vs[i].isSharedWith(stringArray[i % 5]));

        QCOMPARE(vo[i].id, objArray[i % 5].id);
        QCOMPARE(int(vo[i].flags), CountedObject::CopyConstructed
                | CountedObject::CopyAssigned);
    }

    for (int i = 15; i < 20; ++i) {
        QCOMPARE(vi[i], referenceInt);
        QVERIFY(vs[i].isSharedWith(referenceString));

        QCOMPARE(vo[i].id, referenceObject.id);
        QCOMPARE(int(vo[i].flags), CountedObject::CopyConstructed
                | CountedObject::CopyAssigned);
    }

    for (int i = 20; i < 25; ++i) {
        QCOMPARE(vi[i], intArray[i % 5]);
        QVERIFY(vs[i].isSharedWith(stringArray[i % 5]));

        QCOMPARE(vo[i].id, objArray[i % 5].id);

        //  Originally inserted as (DefaultConstructed | CopyAssigned), later
        //  get shuffled into place by std::rotate (SimpleVector::insert,
        //  overlapping mode).
        //  Depending on implementation of rotate, final assignment can be:
        //     - straight from source: DefaultConstructed | CopyAssigned
        //     - through a temporary: CopyConstructed | CopyAssigned
        QCOMPARE(vo[i].flags & CountedObject::CopyAssigned,
                int(CountedObject::CopyAssigned));
    }

    for (int i = 25; i < 30; ++i) {
        QCOMPARE(vi[i], referenceInt);
        QVERIFY(vs[i].isSharedWith(referenceString));

        QCOMPARE(vo[i].id, referenceObject.id);
        QCOMPARE(int(vo[i].flags), CountedObject::CopyConstructed
                | CountedObject::CopyAssigned);
    }
}

void tst_QArrayData::arrayOps2()
{
    CountedObject::LeakChecker leakChecker; Q_UNUSED(leakChecker);

    ////////////////////////////////////////////////////////////////////////////
    // appendInitialize
    SimpleVector<int> vi(5);
    SimpleVector<QString> vs(5);
    SimpleVector<CountedObject> vo(5);

    QCOMPARE(vi.size(), size_t(5));
    QCOMPARE(vs.size(), size_t(5));
    QCOMPARE(vo.size(), size_t(5));

    QCOMPARE(CountedObject::liveCount, size_t(5));
    for (size_t i = 0; i < 5; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i);
        QCOMPARE(int(vo[i].flags), int(CountedObject::DefaultConstructed));
    }

    ////////////////////////////////////////////////////////////////////////////
    // appendInitialize, again

    // These will detach
    vi.resize(10);
    vs.resize(10);
    vo.resize(10);

    QCOMPARE(vi.size(), size_t(10));
    QCOMPARE(vs.size(), size_t(10));
    QCOMPARE(vo.size(), size_t(10));

    QCOMPARE(CountedObject::liveCount, size_t(10));
    for (size_t i = 0; i < 5; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i);
        QCOMPARE(int(vo[i].flags), CountedObject::DefaultConstructed
                | CountedObject::CopyConstructed);
    }

    for (size_t i = 5; i < 10; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i + 5);
        QCOMPARE(int(vo[i].flags), int(CountedObject::DefaultConstructed));
    }

    ////////////////////////////////////////////////////////////////////////////
    // truncate
    QVERIFY(!vi.isShared());
    QVERIFY(!vs.isShared());
    QVERIFY(!vo.isShared());

    // These shouldn't detach
    vi.resize(7);
    vs.resize(7);
    vo.resize(7);

    QCOMPARE(vi.size(), size_t(7));
    QCOMPARE(vs.size(), size_t(7));
    QCOMPARE(vo.size(), size_t(7));

    QCOMPARE(CountedObject::liveCount, size_t(7));
    for (size_t i = 0; i < 5; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i);
        QCOMPARE(int(vo[i].flags), CountedObject::DefaultConstructed
                | CountedObject::CopyConstructed);
    }

    for (size_t i = 5; i < 7; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i + 5);
        QCOMPARE(int(vo[i].flags), int(CountedObject::DefaultConstructed));
    }

    ////////////////////////////////////////////////////////////////////////////
    vi.resize(10);
    vs.resize(10);
    vo.resize(10);

    for (size_t i = 7; i < 10; ++i) {
        vi[i] = int(i);
        vs[i] = QString::number(i);

        QCOMPARE(vo[i].id, i);
        QCOMPARE(int(vo[i].flags), int(CountedObject::DefaultConstructed));
    }

    QCOMPARE(CountedObject::liveCount, size_t(10));

    ////////////////////////////////////////////////////////////////////////////
    // erase
    vi.erase(vi.begin() + 2, vi.begin() + 5);
    vs.erase(vs.begin() + 2, vs.begin() + 5);
    vo.erase(vo.begin() + 2, vo.begin() + 5);

    QCOMPARE(vi.size(), size_t(7));
    QCOMPARE(vs.size(), size_t(7));
    QCOMPARE(vo.size(), size_t(7));

    QCOMPARE(CountedObject::liveCount, size_t(7));
    for (size_t i = 0; i < 2; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i);
        QCOMPARE(int(vo[i].flags), CountedObject::DefaultConstructed
                | CountedObject::CopyConstructed);
    }

    for (size_t i = 2; i < 4; ++i) {
        QCOMPARE(vi[i], 0);
        QVERIFY(vs[i].isNull());

        QCOMPARE(vo[i].id, i + 8);
        QCOMPARE(int(vo[i].flags), int(CountedObject::DefaultConstructed)
                | CountedObject::CopyAssigned);
    }

    for (size_t i = 4; i < 7; ++i) {
        QCOMPARE(vi[i], int(i + 3));
        QCOMPARE(vs[i], QString::number(i + 3));

        QCOMPARE(vo[i].id, i + 3);
        QCOMPARE(int(vo[i].flags), CountedObject::DefaultConstructed
                | CountedObject::CopyAssigned);
    }
}

Q_DECLARE_METATYPE(QArrayDataPointer<int>)

struct ResetOnDtor
{
    ResetOnDtor()
        : value_()
    {
    }

    ResetOnDtor(int value)
        : value_(value)
    {
    }

    ~ResetOnDtor()
    {
        value_ = 0;
    }

    int value_;
};

bool operator==(const ResetOnDtor &lhs, const ResetOnDtor &rhs)
{
    return lhs.value_ == rhs.value_;
}

template <class T>
void fromRawData_impl()
{
    static const T array[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    {
        // Default: Immutable, sharable
        SimpleVector<T> raw = SimpleVector<T>::fromRawData(array,
                sizeof(array)/sizeof(array[0]));

        QCOMPARE(raw.size(), size_t(11));
        QCOMPARE((const T *)raw.constBegin(), array);
        QCOMPARE((const T *)raw.constEnd(), (const T *)(array + sizeof(array)/sizeof(array[0])));

        QVERIFY(raw.isShared());
        QVERIFY(SimpleVector<T>(raw).isSharedWith(raw));
        QVERIFY(raw.isShared());

        // Detach
        QCOMPARE(raw.back(), T(11));
        QVERIFY((const T *)raw.constBegin() != array);
    }
}

void tst_QArrayData::fromRawData_data()
{
    QTest::addColumn<int>("type");

    QTest::newRow("int") << 0;
    QTest::newRow("ResetOnDtor") << 1;
}
void tst_QArrayData::fromRawData()
{
    QFETCH(int, type);

    switch (type)
    {
        case 0:
            fromRawData_impl<int>();
            break;
        case 1:
            fromRawData_impl<ResetOnDtor>();
            break;

        default:
            QFAIL("Unexpected type data");
    }
}

void tst_QArrayData::literals()
{
    {
        QArrayDataPointer<char> d = Q_ARRAY_LITERAL(char, "ABCDEFGHIJ");
        QCOMPARE(d.size, 10u + 1u);
        for (int i = 0; i < 10; ++i)
            QCOMPARE(d.data()[i], char('A' + i));
    }

    {
        QList<char> l(Q_ARRAY_LITERAL(char, "ABCDEFGHIJ"));
        QCOMPARE(l.size(), 11);
        QCOMPARE(l.capacity(), 0);
        for (int i = 0; i < 10; ++i)
            QCOMPARE(l.at(i), char('A' + i));

        (void)l.begin(); // "detach"

        QCOMPARE(l.size(), 11);
        QVERIFY(l.capacity() >= l.size());
        for (int i = 0; i < 10; ++i)
            QCOMPARE(l[i], char('A' + i));
    }

    {
        // wchar_t is not necessarily 2-bytes
        QArrayDataPointer<wchar_t> d = Q_ARRAY_LITERAL(wchar_t, L"ABCDEFGHIJ");
        QCOMPARE(d.size, 10u + 1u);
        for (int i = 0; i < 10; ++i)
            QCOMPARE(d.data()[i], wchar_t('A' + i));
    }

    struct LiteralType {
        int value;
        Q_DECL_CONSTEXPR LiteralType(int v = 0) : value(v) {}
    };

    {
        QArrayDataPointer<LiteralType> d = Q_ARRAY_LITERAL(LiteralType, LiteralType(0), LiteralType(1), LiteralType(2));
        QCOMPARE(d->size, 3);
        for (int i = 0; i < 3; ++i)
            QCOMPARE(d->data()[i].value, i);
    }

    {
        QList<LiteralType> l(Q_ARRAY_LITERAL(LiteralType, LiteralType(0), LiteralType(1), LiteralType(2)));
        QCOMPARE(l.size(), 3);
        QCOMPARE(l.capacity(), 0);
        for (int i = 0; i < 3; ++i)
            QCOMPARE(l.at(i).value, i);

        (void)l.begin(); // "detach"

        QCOMPARE(l.size(), 3);
        QVERIFY(l.capacity() >= l.size());
        for (int i = 0; i < 3; ++i)
            QCOMPARE(l[i].value, i);
    }
}

// Variadic Q_ARRAY_LITERAL need to be available in the current configuration.
void tst_QArrayData::variadicLiterals()
{
    {
        QArrayDataPointer<int> d =
            Q_ARRAY_LITERAL(int, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
        QCOMPARE(d.size, 10u);
        for (int i = 0; i < 10; ++i)
            QCOMPARE(d.data()[i], i);
    }

    {
        QArrayDataPointer<char> d = Q_ARRAY_LITERAL(char,
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J');
        QCOMPARE(d.size, 10u);
        for (int i = 0; i < 10; ++i)
            QCOMPARE(d.data()[i], char('A' + i));
    }

    {
        QArrayDataPointer<const char *> d = Q_ARRAY_LITERAL(const char *,
                "A", "B", "C", "D", "E", "F", "G", "H", "I", "J");
        QCOMPARE(d.size, 10u);
        for (int i = 0; i < 10; ++i) {
            QCOMPARE(d.data()[i][0], char('A' + i));
            QCOMPARE(d.data()[i][1], '\0');
        }
    }
}

// std::remove_reference is in C++11, but requires library support
template <class T> struct RemoveReference { typedef T Type; };
template <class T> struct RemoveReference<T &> { typedef T Type; };

// single-argument std::move is in C++11, but requires library support
template <class T>
typename RemoveReference<T>::Type &&cxx11Move(T &&t)
{
    return static_cast<typename RemoveReference<T>::Type &&>(t);
}

struct CompilerHasCxx11ImplicitMoves
{
    static bool value()
    {
        DetectImplicitMove d(cxx11Move(DetectImplicitMove()));
        return d.constructor == DetectConstructor::MoveConstructor;
    }

    struct DetectConstructor
    {
        Q_DECL_CONSTEXPR DetectConstructor()
            : constructor(DefaultConstructor)
        {
        }

        Q_DECL_CONSTEXPR DetectConstructor(const DetectConstructor &)
            : constructor(CopyConstructor)
        {
        }

        Q_DECL_CONSTEXPR DetectConstructor(DetectConstructor &&)
            : constructor(MoveConstructor)
        {
        }

        enum Constructor {
            DefaultConstructor,
            CopyConstructor,
            MoveConstructor
        };

        Constructor constructor;
    };

    struct DetectImplicitMove
        : DetectConstructor
    {
    };
};

// RValue references need to be supported in the current configuration
void tst_QArrayData::rValueReferences()
{
    if (!CompilerHasCxx11ImplicitMoves::value())
        QSKIP("Implicit move ctor not supported in current configuration");

    SimpleVector<int> v1(1, 42);
    SimpleVector<int> v2;

    const SimpleVector<int>::const_iterator begin = v1.constBegin();

    QVERIFY(!v1.isNull());
    QVERIFY(v2.isNull());

    // move-assign
    v2 = cxx11Move(v1);

    QVERIFY(v1.isNull());
    QVERIFY(!v2.isNull());
    QCOMPARE(v2.constBegin(), begin);

    SimpleVector<int> v3(cxx11Move(v2));

    QVERIFY(v1.isNull());
    QVERIFY(v2.isNull());
    QVERIFY(!v3.isNull());
    QCOMPARE(v3.constBegin(), begin);

    QCOMPARE(v3.size(), size_t(1));
    QCOMPARE(v3.front(), 42);
}

void tst_QArrayData::grow()
{
    SimpleVector<int> vector;

    QCOMPARE(vector.size(), size_t(0));
    QCOMPARE(vector.capacity(), size_t(0));

    size_t previousCapacity = 0;
    size_t allocations = 0;
    for (size_t i = 1; i < (1 << 20); ++i) {
        int source[1] = { int(i) };
        vector.append(source, source + 1);
        QCOMPARE(vector.size(), i);
        if (vector.capacity() != previousCapacity) {
            // Don't re-allocate until necessary
            QVERIFY(previousCapacity < i);

            previousCapacity = vector.capacity();
            ++allocations;

            // Going element-wise is slow under valgrind
            if (previousCapacity - i > 10) {
                i = previousCapacity - 5;
                vector.back() = -int(i);
                vector.resize(i);

                // It's still not the time to re-allocate
                QCOMPARE(vector.capacity(), previousCapacity);
            }
        }
    }
    QVERIFY(vector.size() >= size_t(1 << 20));

    // QArrayData::Grow prevents excessive allocations on a growing container
    QVERIFY(allocations > 20 / 2);
    QVERIFY(allocations < 20 * 2);

    for (size_t i = 0; i < vector.size(); ++i) {
        int value = const_(vector).at(i);
        if (value < 0) {
            i = -value;
            continue;
        }

        QCOMPARE(value, int(i + 1));
    }
}

QTEST_APPLESS_MAIN(tst_QArrayData)
#include "tst_qarraydata.moc"
