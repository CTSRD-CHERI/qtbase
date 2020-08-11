/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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
#include <QCoreApplication>
#include <QDebug>

#define QPROMISE_TEST

#include <QtTest/QtTest>
#include <qfuture.h>
#include <qfuturewatcher.h>
#include <qpromise.h>

#include <algorithm>
#include <memory>
#include <chrono>

class tst_QPromise : public QObject
{
    Q_OBJECT
private slots:
    // simple test cases
    void promise();
    void futureFromPromise();
    void addResult();
    void addResultOutOfOrder();
#ifndef QT_NO_EXCEPTIONS
    void setException();
#endif
    void cancel();
    void progress();

    // complicated test cases
    void addInThread();
    void addInThreadMoveOnlyObject();  // separate test case - QTBUG-84736
    void reportFromMultipleThreads();
    void reportFromMultipleThreadsByMovedPromise();
    void doNotCancelWhenFinished();
#ifndef QT_NO_EXCEPTIONS
    void cancelWhenDestroyed();
#endif
    void cancelWhenReassigned();
    void finishWhenSwapped();
    void cancelWhenMoved();
    void waitUntilResumed();
    void waitUntilCanceled();

    // snippets (external):
    void snippet_basicExample();
    void snippet_multithreadExample();
    void snippet_suspendExample();
};

struct TrivialType { int field = 0; };
struct CopyOnlyType {
    Q_DISABLE_MOVE(CopyOnlyType);
    CopyOnlyType() = default;
    CopyOnlyType(const CopyOnlyType &) = default;
    CopyOnlyType& operator=(const CopyOnlyType &) = default;
    ~CopyOnlyType() = default;

    int field = 0;
};
struct MoveOnlyType {
    Q_DISABLE_COPY(MoveOnlyType);
    MoveOnlyType() = default;
    MoveOnlyType(MoveOnlyType &&) = default;
    MoveOnlyType& operator=(MoveOnlyType &&) = default;
    ~MoveOnlyType() = default;

    int field = 0;
};
bool operator==(const CopyOnlyType &a, const CopyOnlyType &b) { return a.field == b.field; }
bool operator==(const MoveOnlyType &a, const MoveOnlyType &b) { return a.field == b.field; }

// A wrapper for a test function, calls the function, if it fails, reports failure
#define RUN_TEST_FUNC(test, ...) \
do { \
    test(__VA_ARGS__); \
    if (QTest::currentTestFailed()) \
        QFAIL("Test case " #test "(" #__VA_ARGS__ ") failed"); \
} while (false)

// std::thread-like wrapper that ensures that the thread is joined at the end of
// a scope to prevent potential std::terminate
struct ThreadWrapper
{
    std::unique_ptr<QThread> t;
    template<typename Function>
    ThreadWrapper(Function &&f) : t(QThread::create(std::forward<Function>(f)))
    {
        t->start();
    }
    void join() { t->wait(); }
    ~ThreadWrapper()
    {
        t->wait();
    }
};

void tst_QPromise::promise()
{
    const auto testCanCreatePromise = [] (auto promise) {
        promise.reportStarted();
        promise.suspendIfRequested();  // should not block on its own
        promise.reportFinished();
    };

    RUN_TEST_FUNC(testCanCreatePromise, QPromise<void>());
    RUN_TEST_FUNC(testCanCreatePromise, QPromise<int>());
    RUN_TEST_FUNC(testCanCreatePromise, QPromise<QList<float>>());
    RUN_TEST_FUNC(testCanCreatePromise, QPromise<TrivialType>());
    RUN_TEST_FUNC(testCanCreatePromise, QPromise<CopyOnlyType>());
    RUN_TEST_FUNC(testCanCreatePromise, QPromise<MoveOnlyType>());
}

void tst_QPromise::futureFromPromise()
{
    const auto testCanCreateFutureFromPromise = [] (auto promise) {
        auto future = promise.future();
        QVERIFY(!future.isValid());

        promise.reportStarted();
        QCOMPARE(future.isStarted(), true);
        QVERIFY(future.isValid());

        promise.reportFinished();
        QCOMPARE(future.isFinished(), true);
        QVERIFY(future.isValid());

        future.waitForFinished();
    };

    RUN_TEST_FUNC(testCanCreateFutureFromPromise, QPromise<void>());
    RUN_TEST_FUNC(testCanCreateFutureFromPromise, QPromise<double>());
    RUN_TEST_FUNC(testCanCreateFutureFromPromise, QPromise<QList<int>>());
    RUN_TEST_FUNC(testCanCreateFutureFromPromise, QPromise<TrivialType>());
    RUN_TEST_FUNC(testCanCreateFutureFromPromise, QPromise<CopyOnlyType>());
    RUN_TEST_FUNC(testCanCreateFutureFromPromise, QPromise<MoveOnlyType>());
}

void tst_QPromise::addResult()
{
    QPromise<int> promise;
    auto f = promise.future();

    // add as lvalue
    {
        int result = 456;
        promise.addResult(result);
        QCOMPARE(f.resultCount(), 1);
        QCOMPARE(f.result(), result);
        QCOMPARE(f.resultAt(0), result);
    }
    // add as rvalue
    {
        int result = 789;
        promise.addResult(789);
        QCOMPARE(f.resultCount(), 2);
        QCOMPARE(f.resultAt(1), result);
    }
    // add at position
    {
        int result = 56238;
        promise.addResult(result, 2);
        QCOMPARE(f.resultCount(), 3);
        QCOMPARE(f.resultAt(2), result);
    }
    // add at position and overwrite
    {
        int result = -1;
        const auto originalCount = f.resultCount();
        promise.addResult(result, 0);
        QCOMPARE(f.resultCount(), originalCount);
        QCOMPARE(f.resultAt(0), result);
    }
}

void tst_QPromise::addResultOutOfOrder()
{
    // Compare results available in QFuture to expected results
    const auto compareResults = [] (const auto &future, auto expected) {
        QCOMPARE(future.resultCount(), expected.size());
        // index based loop
        for (int i = 0; i < future.resultCount(); ++i)
            QCOMPARE(future.resultAt(i), expected.at(i));
        // iterator based loop
        QVERIFY(std::equal(future.begin(), future.end(), expected.begin()));
    };

    // out of order results without a gap
    {
        QPromise<int> promise;
        auto f = promise.future();
        promise.addResult(456, 1);
        QCOMPARE(f.resultCount(), 0);
        promise.addResult(123, 0);

        QList<int> expected({123, 456});
        RUN_TEST_FUNC(compareResults, f, expected);
        QCOMPARE(f.results(), expected);
    }

    // out of order results with a gap that is closed "later"
    {
        QPromise<int> promise;
        auto f = promise.future();
        promise.addResult(0, 0);
        promise.addResult(1, 1);
        promise.addResult(3, 3);  // intentional gap here

        QList<int> expectedWhenGapExists({0, 1});
        RUN_TEST_FUNC(compareResults, f, expectedWhenGapExists);
        QCOMPARE(f.resultAt(3), 3);

        QList<int> expectedWhenNoGap({0, 1, 2, 3});
        promise.addResult(2, 2);  // fill a gap with a value
        RUN_TEST_FUNC(compareResults, f, expectedWhenNoGap);
        QCOMPARE(f.results(), expectedWhenNoGap);
    }
}

#ifndef QT_NO_EXCEPTIONS
void tst_QPromise::setException()
{
    struct TestException {};  // custom exception class
    const auto testExceptionCaught = [] (auto promise, const auto& exception) {
        auto f = promise.future();
        promise.reportStarted();
        promise.setException(exception);
        promise.reportFinished();

        bool caught = false;
        try {
            f.waitForFinished();
        } catch (const QException&) {
            caught = true;
        } catch (const TestException&) {
            caught = true;
        }
        QVERIFY(caught);
    };

    RUN_TEST_FUNC(testExceptionCaught, QPromise<void>(), QException());
    RUN_TEST_FUNC(testExceptionCaught, QPromise<int>(), QException());
    RUN_TEST_FUNC(testExceptionCaught, QPromise<void>(),
                       std::make_exception_ptr(TestException()));
    RUN_TEST_FUNC(testExceptionCaught, QPromise<int>(),
                       std::make_exception_ptr(TestException()));
}
#endif

void tst_QPromise::cancel()
{
    const auto testCancel = [] (auto promise) {
        auto f = promise.future();
        f.cancel();
        QCOMPARE(promise.isCanceled(), true);
    };

    testCancel(QPromise<void>());
    testCancel(QPromise<int>());
}

void tst_QPromise::progress()
{
    const auto testProgress = [] (auto promise) {
        auto f = promise.future();

        promise.setProgressRange(0, 2);
        QCOMPARE(f.progressMinimum(), 0);
        QCOMPARE(f.progressMaximum(), 2);

        QCOMPARE(f.progressValue(), 0);
        promise.setProgressValue(1);
        QCOMPARE(f.progressValue(), 1);
        promise.setProgressValue(0);  // decrement
        QCOMPARE(f.progressValue(), 1);
        promise.setProgressValue(10);  // out of range
        QEXPECT_FAIL("", "Out of range value is set - QTBUG-84729", Continue);
        QCOMPARE(f.progressValue(), 1);

        promise.setProgressRange(0, 100);
        promise.setProgressValueAndText(50, u8"50%");
        QCOMPARE(f.progressValue(), 50);
        QCOMPARE(f.progressText(), u8"50%");
    };

    RUN_TEST_FUNC(testProgress, QPromise<void>());
    RUN_TEST_FUNC(testProgress, QPromise<int>());
}

void tst_QPromise::addInThread()
{
    const auto testAddResult = [] (auto promise, const auto &result) {
        promise.reportStarted();
        auto f = promise.future();
        // move construct QPromise
        ThreadWrapper thr([p = std::move(promise), &result] () mutable {
            p.addResult(result);
        });
        // Waits for result first
        QCOMPARE(f.result(), result);
        QCOMPARE(f.resultAt(0), result);
    };

    RUN_TEST_FUNC(testAddResult, QPromise<int>(), 42);
    RUN_TEST_FUNC(testAddResult, QPromise<QString>(), u8"42");
    RUN_TEST_FUNC(testAddResult, QPromise<CopyOnlyType>(), CopyOnlyType{99});
}

void tst_QPromise::addInThreadMoveOnlyObject()
{
    QPromise<MoveOnlyType> promise;
    promise.reportStarted();
    auto f = promise.future();

    ThreadWrapper thr([p = std::move(promise)] () mutable {
        p.addResult(MoveOnlyType{-11});
    });

    // Iterators wait for result first
    for (auto& result : f)
        QCOMPARE(result, MoveOnlyType{-11});
}

void tst_QPromise::reportFromMultipleThreads()
{
    QPromise<int> promise;
    auto f = promise.future();
    promise.reportStarted();

    ThreadWrapper threads[] = {
        ThreadWrapper([&promise] () mutable { promise.addResult(42); }),
        ThreadWrapper([&promise] () mutable { promise.addResult(43); }),
        ThreadWrapper([&promise] () mutable { promise.addResult(44); }),
    };
    for (auto& t : threads)
        t.join();
    promise.reportFinished();

    QList<int> expected = {42, 43, 44};
    for (auto actual : f.results()) {
        QVERIFY(std::find(expected.begin(), expected.end(), actual) != expected.end());
        expected.removeOne(actual);
    }
}

void tst_QPromise::reportFromMultipleThreadsByMovedPromise()
{
    QPromise<int> initialPromise;
    auto f = initialPromise.future();
    {
        // Move QPromise into local scope: local QPromise (as being
        // move-constructed) must be able to set results, QFuture must still
        // hold correct references to results.
        auto promise = std::move(initialPromise);
        promise.reportStarted();
        ThreadWrapper threads[] = {
            ThreadWrapper([&promise] () mutable { promise.addResult(42); }),
            ThreadWrapper([&promise] () mutable { promise.addResult(43); }),
            ThreadWrapper([&promise] () mutable { promise.addResult(44); }),
        };
        for (auto& t : threads)
            t.join();
        promise.reportFinished();
    }

    QCOMPARE(f.isFinished(), true);
    QCOMPARE(f.isValid(), true);

    QList<int> expected = {42, 43, 44};
    for (auto actual : f.results()) {
        QVERIFY(std::find(expected.begin(), expected.end(), actual) != expected.end());
        expected.removeOne(actual);
    }
}

void tst_QPromise::doNotCancelWhenFinished()
{
    const auto testFinishedPromise = [] (auto promise) {
        auto f = promise.future();
        promise.reportStarted();

        // Finish QPromise inside thread, destructor must not call cancel()
        ThreadWrapper([p = std::move(promise)] () mutable { p.reportFinished(); }).join();

        f.waitForFinished();

        QCOMPARE(f.isFinished(), true);
        QCOMPARE(f.isCanceled(), false);
    };

    RUN_TEST_FUNC(testFinishedPromise, QPromise<void>());
    RUN_TEST_FUNC(testFinishedPromise, QPromise<int>());
    RUN_TEST_FUNC(testFinishedPromise, QPromise<QString>());
}

#ifndef QT_NO_EXCEPTIONS
void tst_QPromise::cancelWhenDestroyed()
{
    QPromise<int> initialPromise;
    auto f = initialPromise.future();

    try {
        // Move QPromise to local scope. On destruction, it must call cancel().
        auto promise = std::move(initialPromise);
        promise.reportStarted();
        ThreadWrapper threads[] = {
            ThreadWrapper([&promise] () mutable { promise.addResult(42); }),
            ThreadWrapper([&promise] () mutable { promise.addResult(43); }),
            ThreadWrapper([&promise] () mutable { promise.addResult(44); }),
        };
        for (auto& t : threads)
            t.join();
        throw "Throw in the middle, we lose our promise here, reportFinished() not called!";
        promise.reportFinished();
    } catch (...) {}

    QCOMPARE(f.isFinished(), true);
    QCOMPARE(f.isCanceled(), true);

    // Results are still available despite throw
    QList<int> expected = {42, 43, 44};
    for (auto actual : f.results()) {
        QVERIFY(std::find(expected.begin(), expected.end(), actual) != expected.end());
        expected.removeOne(actual);
    }
}
#endif

void tst_QPromise::cancelWhenReassigned()
{
    QPromise<int> promise;
    auto f = promise.future();
    promise.reportStarted();

    ThreadWrapper thr([p = std::move(promise)] () mutable {
        QThread::msleep(100);
        p = QPromise<int>();  // assign new promise, old must be correctly destroyed
    });

    f.waitForFinished();  // wait for the old promise

    QCOMPARE(f.isFinished(), true);
    QCOMPARE(f.isCanceled(), true);
}

void tst_QPromise::finishWhenSwapped()
{
    QPromise<int> promise1;
    auto f1 = promise1.future();
    promise1.reportStarted();

    QPromise<int> promise2;
    auto f2 = promise2.future();
    promise2.reportStarted();

    ThreadWrapper thr([&promise1, &promise2] () mutable {
        QThread::msleep(100);
        promise1.addResult(0);
        promise2.addResult(1);
        swap(promise1, promise2);  // ADL must resolve this
        promise1.addResult(2);
        promise2.addResult(3);
        promise1.reportFinished();  // this finish is for future #2
        promise2.reportFinished();  // this finish is for future #1
    });

    f1.waitForFinished();
    f2.waitForFinished();

    // Future #1 and #2 are finished inside thread
    QCOMPARE(f1.isFinished(), true);
    QCOMPARE(f1.isCanceled(), false);

    QCOMPARE(f2.isFinished(), true);
    QCOMPARE(f2.isCanceled(), false);

    QCOMPARE(f1.resultAt(0), 0);
    QCOMPARE(f1.resultAt(1), 3);

    QCOMPARE(f2.resultAt(0), 1);
    QCOMPARE(f2.resultAt(1), 2);
}

void tst_QPromise::cancelWhenMoved()
{
    QPromise<int> promise1;
    auto f1 = promise1.future();
    promise1.reportStarted();

    QPromise<int> promise2;
    auto f2 = promise2.future();
    promise2.reportStarted();

    // Move promises to local scope to test cancellation behavior
    ThreadWrapper thr([p1 = std::move(promise1), p2 = std::move(promise2)] () mutable {
        QThread::msleep(100);
        p1 = std::move(p2);
        p1.reportFinished();  // this finish is for future #2
    });

    f1.waitForFinished();
    f2.waitForFinished();

    // Future #1 is implicitly cancelled inside thread
    QCOMPARE(f1.isFinished(), true);
    QCOMPARE(f1.isCanceled(), true);

    // Future #2 is explicitly finished inside thread
    QCOMPARE(f2.isFinished(), true);
    QCOMPARE(f2.isCanceled(), false);
}

void tst_QPromise::waitUntilResumed()
{
    QPromise<int> promise;
    promise.reportStarted();
    auto f = promise.future();
    f.suspend();

    ThreadWrapper thr([p = std::move(promise)] () mutable {
        p.suspendIfRequested();
        p.addResult(42);  // result added after suspend
        p.reportFinished();
    });

    while (!f.isSuspended()) {  // busy wait until worker thread suspends
        QCOMPARE(f.isFinished(), false);  // exit condition in case of failure
        QThread::msleep(50);  // allow another thread to actually carry on
    }

    f.resume();
    f.waitForFinished();

    QCOMPARE(f.resultCount(), 1);
    QCOMPARE(f.result(), 42);
}

void tst_QPromise::waitUntilCanceled()
{
    QPromise<int> promise;
    promise.reportStarted();
    auto f = promise.future();
    f.suspend();

    ThreadWrapper thr([p = std::move(promise)] () mutable {
        p.suspendIfRequested();
        p.addResult(42);  // result not added due to QFuture::cancel()
        p.reportFinished();
    });

    while (!f.isSuspended()) {  // busy wait until worker thread suspends
        QCOMPARE(f.isFinished(), false);  // exit condition in case of failure
        QThread::msleep(50);  // allow another thread to actually carry on
    }

    f.cancel();
    f.waitForFinished();

    QCOMPARE(f.resultCount(), 0);
}

// Below is a quick and dirty hack to make snippets a part of a test suite
#include "snippet_qpromise.cpp"
void tst_QPromise::snippet_basicExample()
{
    snippet_QPromise::basicExample();
}

void tst_QPromise::snippet_multithreadExample()
{
    snippet_QPromise::multithreadExample();
}

void tst_QPromise::snippet_suspendExample()
{
    snippet_QPromise::suspendExample();
}

QTEST_MAIN(tst_QPromise)
#include "tst_qpromise.moc"
