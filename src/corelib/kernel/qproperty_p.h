/****************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
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

#ifndef QPROPERTY_P_H
#define QPROPERTY_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include <qglobal.h>
#include <qproperty.h>

#include <qvarlengtharray.h>
#include <qscopedpointer.h>
#include <vector>


QT_BEGIN_NAMESPACE

// Keep all classes related to QProperty in one compilation unit. Performance of this code is crucial and
// we need to allow the compiler to inline where it makes sense.

// This is a helper "namespace"
struct Q_AUTOTEST_EXPORT QPropertyBasePointer
{
    const QtPrivate::QPropertyBase *ptr = nullptr;

    QPropertyBindingPrivate *bindingPtr() const
    {
        if (ptr->d_ptr & QtPrivate::QPropertyBase::BindingBit)
            return reinterpret_cast<QPropertyBindingPrivate*>(ptr->d_ptr & ~QtPrivate::QPropertyBase::FlagMask);
        return nullptr;
    }

    void setObservers(QPropertyObserver *observer)
    {
        observer->prev = reinterpret_cast<QPropertyObserver**>(&(ptr->d_ptr));
        ptr->d_ptr = (reinterpret_cast<quintptr>(observer) & ~QtPrivate::QPropertyBase::FlagMask);
    }
    void addObserver(QPropertyObserver *observer);
    void setFirstObserver(QPropertyObserver *observer);
    QPropertyObserverPointer firstObserver() const;

    int observerCount() const;

    template <typename T>
    static QPropertyBasePointer get(QProperty<T> &property)
    {
        return QPropertyBasePointer{&property.d.priv};
    }
};

// This is a helper "namespace"
struct QPropertyObserverPointer
{
    QPropertyObserver *ptr = nullptr;

    void unlink();

    void setBindingToMarkDirty(QPropertyBindingPrivate *binding);
    void setChangeHandler(void (*changeHandler)(QPropertyObserver *, void *));
    void setAliasedProperty(void *propertyPtr);

    void notify(QPropertyBindingPrivate *triggeringBinding, void *propertyDataPtr);
    void observeProperty(QPropertyBasePointer property);

    explicit operator bool() const { return ptr != nullptr; }

    QPropertyObserverPointer nextObserver() const { return {ptr->next.data()}; }
};

class QPropertyBindingErrorPrivate : public QSharedData
{
public:
    QPropertyBindingError::Type type = QPropertyBindingError::NoError;
    QString description;
};

struct BindingEvaluationState
{
    BindingEvaluationState(QPropertyBindingPrivate *binding);
    ~BindingEvaluationState();
    QPropertyBindingPrivate *binding;
    BindingEvaluationState *previousState = nullptr;
    BindingEvaluationState **currentState = nullptr;
};

class Q_CORE_EXPORT QPropertyBindingPrivate : public QSharedData
{
private:
    friend struct QPropertyBasePointer;

    using ObserverArray = std::array<QPropertyObserver, 4>;

    // QSharedData is 4 bytes. Use the padding for the bools as we need 8 byte alignment below.
    bool dirty = false;
    bool updating = false;
    bool hasStaticObserver = false;
    bool isBool = false;

    QUntypedPropertyBinding::BindingEvaluationFunction evaluationFunction;

    QPropertyObserverPointer firstObserver;
    union {
        ObserverArray inlineDependencyObservers;
        struct {
            void *staticObserver;
            QtPrivate::QPropertyObserverCallback staticObserverCallback;
            QtPrivate::QPropertyGuardFunction staticGuardCallback;
        };
    };
    QScopedPointer<std::vector<QPropertyObserver>> heapObservers;

    void *propertyDataPtr = nullptr;

    QPropertyBindingSourceLocation location;
    QPropertyBindingError error;

    QMetaType metaType;

public:
    // public because the auto-tests access it, too.
    size_t dependencyObserverCount = 0;

    QPropertyBindingPrivate(const QMetaType &metaType, QUntypedPropertyBinding::BindingEvaluationFunction evaluationFunction,
                            const QPropertyBindingSourceLocation &location)
        : isBool(metaType.id() == QMetaType::Bool)
        , evaluationFunction(std::move(evaluationFunction))
        , inlineDependencyObservers() // Explicit initialization required because of union
        , location(location)
        , metaType(metaType)
    {}
    virtual ~QPropertyBindingPrivate();

    void setDirty(bool d) { dirty = d; }
    void setProperty(void *propertyPtr) { propertyDataPtr = propertyPtr; }
    void setStaticObserver(void *observer, QtPrivate::QPropertyObserverCallback callback,
                           QtPrivate::QPropertyGuardFunction guardCallback)
    {
        if (observer) {
            if (!hasStaticObserver) {
                if (dependencyObserverCount > 0) {
                    if (!heapObservers)
                        heapObservers.reset(new std::vector<QPropertyObserver>());
                    for (size_t i = 0, end = qMin(dependencyObserverCount, inlineDependencyObservers.size()); i < end; ++i)
                        heapObservers->push_back(std::move(inlineDependencyObservers[i]));
                }
                inlineDependencyObservers.~ObserverArray();
            }

            hasStaticObserver = true;
            staticObserver = observer;
            staticObserverCallback = callback;
            staticGuardCallback = guardCallback;
        } else if (hasStaticObserver) {
            hasStaticObserver = false;
            new (&inlineDependencyObservers) ObserverArray();
            for (size_t i = 0, end = qMin(dependencyObserverCount, inlineDependencyObservers.size()); i < end; ++i) {
                inlineDependencyObservers[i] = std::move(heapObservers->back());
                heapObservers->pop_back();
            }
        }
    }
    void prependObserver(QPropertyObserverPointer observer) {
        observer.ptr->prev = const_cast<QPropertyObserver **>(&firstObserver.ptr);
        firstObserver = observer;
    }

    QPropertyObserverPointer takeObservers()
    {
        auto observers = firstObserver;
        firstObserver.ptr = nullptr;
        return observers;
    }

    void clearDependencyObservers() {
        if (!hasStaticObserver) {
            for (size_t i = 0; i < qMin(dependencyObserverCount, inlineDependencyObservers.size()); ++i) {
                QPropertyObserverPointer p{&inlineDependencyObservers[i]};
                p.unlink();
            }
        }
        if (heapObservers)
            heapObservers->clear();
        dependencyObserverCount = 0;
    }
    QPropertyObserverPointer allocateDependencyObserver() {
        if (!hasStaticObserver && dependencyObserverCount < inlineDependencyObservers.size()) {
            ++dependencyObserverCount;
            return {&inlineDependencyObservers[dependencyObserverCount - 1]};
        }
        ++dependencyObserverCount;
        if (!heapObservers)
            heapObservers.reset(new std::vector<QPropertyObserver>());
        return {&heapObservers->emplace_back()};
    }

    QPropertyBindingSourceLocation sourceLocation() const { return location; }
    QPropertyBindingError bindingError() const { return error; }
    QMetaType valueMetaType() const { return metaType; }

    void unlinkAndDeref();

    void markDirtyAndNotifyObservers();
    bool evaluateIfDirtyAndReturnTrueIfValueChanged();

    static QPropertyBindingPrivate *get(const QUntypedPropertyBinding &binding)
    { return binding.d.data(); }

    void setError(QPropertyBindingError &&e)
    { error = std::move(e); }

    static QPropertyBindingPrivate *currentlyEvaluatingBinding();
};

inline void QPropertyBasePointer::setFirstObserver(QPropertyObserver *observer)
{
    if (auto *binding = bindingPtr()) {
        binding->firstObserver.ptr = observer;
        return;
    }
    ptr->d_ptr = reinterpret_cast<quintptr>(observer)
            | (qptraddr(ptr->d_ptr) & QtPrivate::QPropertyBase::FlagMask);
}

inline QPropertyObserverPointer QPropertyBasePointer::firstObserver() const
{
    if (auto *binding = bindingPtr())
        return binding->firstObserver;
    return {reinterpret_cast<QPropertyObserver*>(ptr->d_ptr & ~QtPrivate::QPropertyBase::FlagMask)};
}

QT_END_NAMESPACE

#endif // QPROPERTY_P_H
