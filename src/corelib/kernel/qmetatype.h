/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Copyright (C) 2018 Intel Corporation.
** Copyright (C) 2014 Olivier Goffart <ogoffart@woboq.com>
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

#ifndef QMETATYPE_H
#define QMETATYPE_H

#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qvarlengtharray.h>
#include <QtCore/qrefcount.h>
#include <QtCore/qdatastream.h>
#ifndef QT_NO_QOBJECT
#include <QtCore/qobjectdefs.h>
#endif

#include <array>
#include <new>
#include <vector>
#include <list>
#include <map>
#include <optional>
#include <functional>

#ifdef Bool
#error qmetatype.h must be included before any header file that defines Bool
#endif

QT_BEGIN_NAMESPACE

// from qcborcommon.h
enum class QCborSimpleType : quint8;

template <typename T>
struct QMetaTypeId2;

template <typename T>
inline constexpr int qMetaTypeId();

// F is a tuple: (QMetaType::TypeName, QMetaType::TypeNameID, RealType)
// ### Qt6: reorder the types to match the C++ integral type ranking
#define QT_FOR_EACH_STATIC_PRIMITIVE_TYPE(F)\
    F(Void, 43, void) \
    F(Bool, 1, bool) \
    F(Int, 2, int) \
    F(UInt, 3, uint) \
    F(LongLong, 4, qlonglong) \
    F(ULongLong, 5, qulonglong) \
    F(Double, 6, double) \
    F(Long, 32, long) \
    F(Short, 33, short) \
    F(Char, 34, char) \
    F(Char16, 56, char16_t) \
    F(Char32, 57, char32_t) \
    F(ULong, 35, ulong) \
    F(UShort, 36, ushort) \
    F(UChar, 37, uchar) \
    F(Float, 38, float) \
    F(SChar, 40, signed char) \
    F(Nullptr, 51, std::nullptr_t) \
    F(QCborSimpleType, 52, QCborSimpleType) \

#ifdef __CHERI__
// XXXAR: which numbers are safe to use?
#define QT_FOR_EACH_STATIC_PRIMITIVE_CAPABILITY_TYPE(F) \
    F(IntCap, 59, __intcap_t) \
    F(UIntCap, 60, __uintcap_t)
#else
#define QT_FOR_EACH_STATIC_PRIMITIVE_CAPABILITY_TYPE(F)
#endif

#define QT_FOR_EACH_STATIC_PRIMITIVE_POINTER(F)\
    F(VoidStar, 31, void*) \
    QT_FOR_EACH_STATIC_PRIMITIVE_CAPABILITY_TYPE(F) \

#if QT_CONFIG(easingcurve)
#define QT_FOR_EACH_STATIC_EASINGCURVE(F)\
    F(QEasingCurve, 29, QEasingCurve)
#else
#define QT_FOR_EACH_STATIC_EASINGCURVE(F)
#endif

#if QT_CONFIG(itemmodel)
#define QT_FOR_EACH_STATIC_ITEMMODEL_CLASS(F)\
    F(QModelIndex, 42, QModelIndex) \
    F(QPersistentModelIndex, 50, QPersistentModelIndex)
#else
#define QT_FOR_EACH_STATIC_ITEMMODEL_CLASS(F)
#endif

#if QT_CONFIG(regularexpression)
#  define QT_FOR_EACH_STATIC_REGULAR_EXPRESSION(F) \
    F(QRegularExpression, 44, QRegularExpression)
#else
#  define QT_FOR_EACH_STATIC_REGULAR_EXPRESSION(F)
#endif

#define QT_FOR_EACH_STATIC_CORE_CLASS(F)\
    F(QChar, 7, QChar) \
    F(QString, 10, QString) \
    F(QStringList, 11, QStringList) \
    F(QByteArray, 12, QByteArray) \
    F(QBitArray, 13, QBitArray) \
    F(QDate, 14, QDate) \
    F(QTime, 15, QTime) \
    F(QDateTime, 16, QDateTime) \
    F(QUrl, 17, QUrl) \
    F(QLocale, 18, QLocale) \
    F(QRect, 19, QRect) \
    F(QRectF, 20, QRectF) \
    F(QSize, 21, QSize) \
    F(QSizeF, 22, QSizeF) \
    F(QLine, 23, QLine) \
    F(QLineF, 24, QLineF) \
    F(QPoint, 25, QPoint) \
    F(QPointF, 26, QPointF) \
    QT_FOR_EACH_STATIC_EASINGCURVE(F) \
    F(QUuid, 30, QUuid) \
    F(QVariant, 41, QVariant) \
    QT_FOR_EACH_STATIC_REGULAR_EXPRESSION(F) \
    F(QJsonValue, 45, QJsonValue) \
    F(QJsonObject, 46, QJsonObject) \
    F(QJsonArray, 47, QJsonArray) \
    F(QJsonDocument, 48, QJsonDocument) \
    F(QCborValue, 53, QCborValue) \
    F(QCborArray, 54, QCborArray) \
    F(QCborMap, 55, QCborMap) \
    QT_FOR_EACH_STATIC_ITEMMODEL_CLASS(F)

#define QT_FOR_EACH_STATIC_CORE_POINTER(F)\
    F(QObjectStar, 39, QObject*)

#define QT_FOR_EACH_STATIC_CORE_TEMPLATE(F)\
    F(QVariantMap, 8, QVariantMap) \
    F(QVariantList, 9, QVariantList) \
    F(QVariantHash, 28, QVariantHash) \
    F(QVariantPair, 58, QVariantPair) \
    F(QByteArrayList, 49, QByteArrayList) \

#if QT_CONFIG(shortcut)
#define QT_FOR_EACH_STATIC_KEYSEQUENCE_CLASS(F)\
    F(QKeySequence, 75, QKeySequence)
#else
#define QT_FOR_EACH_STATIC_KEYSEQUENCE_CLASS(F)
#endif

#define QT_FOR_EACH_STATIC_GUI_CLASS(F)\
    F(QFont, 64, QFont) \
    F(QPixmap, 65, QPixmap) \
    F(QBrush, 66, QBrush) \
    F(QColor, 67, QColor) \
    F(QPalette, 68, QPalette) \
    F(QIcon, 69, QIcon) \
    F(QImage, 70, QImage) \
    F(QPolygon, 71, QPolygon) \
    F(QRegion, 72, QRegion) \
    F(QBitmap, 73, QBitmap) \
    F(QCursor, 74, QCursor) \
    QT_FOR_EACH_STATIC_KEYSEQUENCE_CLASS(F) \
    F(QPen, 76, QPen) \
    F(QTextLength, 77, QTextLength) \
    F(QTextFormat, 78, QTextFormat) \
    F(QTransform, 80, QTransform) \
    F(QMatrix4x4, 81, QMatrix4x4) \
    F(QVector2D, 82, QVector2D) \
    F(QVector3D, 83, QVector3D) \
    F(QVector4D, 84, QVector4D) \
    F(QQuaternion, 85, QQuaternion) \
    F(QPolygonF, 86, QPolygonF) \
    F(QColorSpace, 87, QColorSpace) \


#define QT_FOR_EACH_STATIC_WIDGETS_CLASS(F)\
    F(QSizePolicy, 121, QSizePolicy) \

// F is a tuple: (QMetaType::TypeName, QMetaType::TypeNameID, AliasingType, "RealType")
#define QT_FOR_EACH_STATIC_ALIAS_TYPE(F)\
    F(ULong, -1, ulong, "unsigned long") \
    F(UInt, -1, uint, "unsigned int") \
    F(UShort, -1, ushort, "unsigned short") \
    F(UChar, -1, uchar, "unsigned char") \
    F(LongLong, -1, qlonglong, "long long") \
    F(ULongLong, -1, qulonglong, "unsigned long long") \
    F(SChar, -1, signed char, "qint8") \
    F(UChar, -1, uchar, "quint8") \
    F(Short, -1, short, "qint16") \
    F(UShort, -1, ushort, "quint16") \
    F(Int, -1, int, "qint32") \
    F(UInt, -1, uint, "quint32") \
    F(LongLong, -1, qlonglong, "qint64") \
    F(ULongLong, -1, qulonglong, "quint64") \
    F(QVariantList, -1, QVariantList, "QList<QVariant>") \
    F(QVariantMap, -1, QVariantMap, "QMap<QString,QVariant>") \
    F(QVariantHash, -1, QVariantHash, "QHash<QString,QVariant>") \
    F(QVariantPair, -1, QVariantPair, "QPair<QVariant,QVariant>") \
    F(QByteArrayList, -1, QByteArrayList, "QList<QByteArray>") \

#define QT_FOR_EACH_STATIC_TYPE(F)\
    QT_FOR_EACH_STATIC_PRIMITIVE_TYPE(F)\
    QT_FOR_EACH_STATIC_PRIMITIVE_POINTER(F)\
    QT_FOR_EACH_STATIC_CORE_CLASS(F)\
    QT_FOR_EACH_STATIC_CORE_POINTER(F)\
    QT_FOR_EACH_STATIC_CORE_TEMPLATE(F)\
    QT_FOR_EACH_STATIC_GUI_CLASS(F)\
    QT_FOR_EACH_STATIC_WIDGETS_CLASS(F)\

#define QT_DEFINE_METATYPE_ID(TypeName, Id, Name) \
    TypeName = Id,

#define QT_FOR_EACH_AUTOMATIC_TEMPLATE_1ARG(F) \
    F(QList) \
    F(QQueue) \
    F(QStack) \
    F(QSet) \
    /*end*/

#define QT_FOR_EACH_AUTOMATIC_TEMPLATE_2ARG(F) \
    F(QHash, class) \
    F(QMap, class)

#define QT_FOR_EACH_AUTOMATIC_TEMPLATE_SMART_POINTER(F) \
    F(QSharedPointer) \
    F(QWeakPointer) \
    F(QPointer)

class QDataStream;
struct QMetaObject;

namespace QtPrivate
{

class QMetaTypeInterface;

/*!
    This template is used for implicit conversion from type From to type To.
    \internal
*/
template<typename From, typename To>
To convertImplicit(const From& from)
{
    return from;
}

    template<typename T, bool>
    struct SequentialValueTypeIsMetaType;
    template<typename T, bool>
    struct AssociativeValueTypeIsMetaType;
    template<typename T, bool>
    struct IsMetaTypePair;
    template<typename, typename>
    struct MetaTypeSmartPointerHelper;

    template<typename T>
    struct IsQFlags : std::false_type {};

    template<typename Enum>
    struct IsQFlags<QFlags<Enum>> : std::true_type {};

    template<typename T>
    struct IsEnumOrFlags : std::disjunction<std::is_enum<T>, IsQFlags<T>> {};
}  // namespace QtPrivate

class Q_CORE_EXPORT QMetaType {
public:
#ifndef Q_CLANG_QDOC
    // The code that actually gets compiled.
    enum Type {
        // these are merged with QVariant
        QT_FOR_EACH_STATIC_TYPE(QT_DEFINE_METATYPE_ID)

        FirstCoreType = Bool,
#ifdef __CHERI__
        LastCoreType = UIntCap,
#else
        LastCoreType = QVariantPair,
#endif
        FirstGuiType = QFont,
        LastGuiType = QColorSpace,
        FirstWidgetsType = QSizePolicy,
        LastWidgetsType = QSizePolicy,
        HighestInternalId = LastWidgetsType,

        QReal = sizeof(qreal) == sizeof(double) ? Double : Float,

        UnknownType = 0,
        User = 1024
    };
#else
    // If we are using QDoc it fakes the Type enum looks like this.
    enum Type {
        UnknownType = 0, Bool = 1, Int = 2, UInt = 3, LongLong = 4, ULongLong = 5,
        Double = 6, Long = 32, Short = 33, Char = 34, ULong = 35, UShort = 36,
        UChar = 37, Float = 38,
        VoidStar = 31,
        QChar = 7, QString = 10, QStringList = 11, QByteArray = 12,
        QBitArray = 13, QDate = 14, QTime = 15, QDateTime = 16, QUrl = 17,
        QLocale = 18, QRect = 19, QRectF = 20, QSize = 21, QSizeF = 22,
        QLine = 23, QLineF = 24, QPoint = 25, QPointF = 26,
        QEasingCurve = 29, QUuid = 30, QVariant = 41, QModelIndex = 42,
        QPersistentModelIndex = 50, QRegularExpression = 44,
        QJsonValue = 45, QJsonObject = 46, QJsonArray = 47, QJsonDocument = 48,
        QByteArrayList = 49, QObjectStar = 39, SChar = 40,
        Void = 43,
        Nullptr = 51,
        QVariantMap = 8, QVariantList = 9, QVariantHash = 28,
        QCborSimpleType = 52, QCborValue = 53, QCborArray = 54, QCborMap = 55,
        Char16 = 56, Char32 = 57,
        IntCap = 59, UIntCap = 60,
        // Gui types
        QFont = 64, QPixmap = 65, QBrush = 66, QColor = 67, QPalette = 68,
        QIcon = 69, QImage = 70, QPolygon = 71, QRegion = 72, QBitmap = 73,
        QCursor = 74, QKeySequence = 75, QPen = 76, QTextLength = 77, QTextFormat = 78,
        QTransform = 80, QMatrix4x4 = 81, QVector2D = 82,
        QVector3D = 83, QVector4D = 84, QQuaternion = 85, QPolygonF = 86, QColorSpace = 87,

        // Widget types
        QSizePolicy = 121,
        LastCoreType = Char32,
        LastGuiType = QColorSpace,
        User = 1024
    };
#endif

    enum TypeFlag {
        NeedsConstruction = 0x1,
        NeedsDestruction = 0x2,
        MovableType = 0x4,
        PointerToQObject = 0x8,
        IsEnumeration = 0x10,
        SharedPointerToQObject = 0x20,
        WeakPointerToQObject = 0x40,
        TrackingPointerToQObject = 0x80,
        IsUnsignedEnumeration = 0x100,
        IsGadget = 0x200,
        PointerToGadget = 0x400,
        IsPointer = 0x800,
    };
    Q_DECLARE_FLAGS(TypeFlags, TypeFlag)

    static void registerNormalizedTypedef(const QT_PREPEND_NAMESPACE(QByteArray) &normalizedTypeName, QMetaType type);

#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_6_0
    static int type(const char *typeName)
    { return QMetaType::fromName(typeName).id(); }
    QT_DEPRECATED_VERSION_6_0
    static int type(const QT_PREPEND_NAMESPACE(QByteArray) &typeName)
    { return QMetaType::fromName(typeName).id(); }
    QT_DEPRECATED_VERSION_6_0
    static const char *typeName(int type)
    { return QMetaType(type).name(); }
    QT_DEPRECATED_VERSION_6_0
    static int sizeOf(int type)
    { return QMetaType(type).sizeOf(); }
    QT_DEPRECATED_VERSION_6_0
    static TypeFlags typeFlags(int type)
    { return QMetaType(type).flags(); }
    QT_DEPRECATED_VERSION_6_0
    static const QMetaObject *metaObjectForType(int type)
    { return QMetaType(type).metaObject(); }
    QT_DEPRECATED_VERSION_6_0
    static void *create(int type, const void *copy = nullptr)
    { return QMetaType(type).create(copy); }
    QT_DEPRECATED_VERSION_6_0
    static void destroy(int type, void *data)
    { return QMetaType(type).destroy(data); }
    QT_DEPRECATED_VERSION_6_0
    static void *construct(int type, void *where, const void *copy)
    { return QMetaType(type).construct(where, copy); }
    QT_DEPRECATED_VERSION_6_0
    static void destruct(int type, void *where)
    { return QMetaType(type).destruct(where); }
#endif
    static bool isRegistered(int type);

    explicit QMetaType(int type);
    explicit constexpr QMetaType(QtPrivate::QMetaTypeInterface *d) : d_ptr(d) {}
    constexpr QMetaType() = default;

    bool isValid() const;
    bool isRegistered() const;
    int id() const;
    qsizetype sizeOf() const;
    qsizetype alignOf() const;
    TypeFlags flags() const;
    const QMetaObject *metaObject() const;
    const char *name() const;

    void *create(const void *copy = nullptr) const;
    void destroy(void *data) const;
    void *construct(void *where, const void *copy = nullptr) const;
    void destruct(void *data) const;
    std::optional<int> compare(const void *lhs, const void *rhs) const;
    bool equals(const void *lhs, const void *rhs) const;

    bool isEqualityComparable() const;
    bool isOrdered() const;

#ifndef QT_NO_DATASTREAM
    bool save(QDataStream &stream, const void *data) const;
    bool load(QDataStream &stream, void *data) const;

#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_6_0
    static bool save(QDataStream &stream, int type, const void *data)
    { return QMetaType(type).save(stream, data); }
    QT_DEPRECATED_VERSION_6_0
    static bool load(QDataStream &stream, int type, void *data)
    { return QMetaType(type).load(stream, data); }
#endif
#endif

    template<typename T>
    static QMetaType fromType();
    static QMetaType fromName(QByteArrayView name);

    friend bool operator==(QMetaType a, QMetaType b) { return a.id() == b.id(); }
    friend bool operator!=(QMetaType a, QMetaType b) { return !(a == b); }

public:

#ifndef QT_NO_DEBUG_STREAM
    bool debugStream(QDebug& dbg, const void *rhs);
    bool hasRegisteredDebugStreamOperator() const;

#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_6_0
    static bool debugStream(QDebug& dbg, const void *rhs, int typeId)
    { return QMetaType(typeId).debugStream(dbg, rhs); }
    template<typename T>
    QT_DEPRECATED_VERSION_6_0
    static bool hasRegisteredDebugStreamOperator()
    { return QMetaType::fromType<T>().hasRegisteredDebugStreamOperator(); }
    QT_DEPRECATED_VERSION_6_0
    static bool hasRegisteredDebugStreamOperator(int typeId)
    { return QMetaType(typeId).hasRegisteredDebugStreamOperator(); }
#endif
#endif

    // type erased converter function
    using ConverterFunction = std::function<bool(const void *src, void *target)>;

    // implicit conversion supported like double -> float
    template<typename From, typename To>
    static bool registerConverter()
    {
        return registerConverter<From, To>(QtPrivate::convertImplicit<From, To>);
    }

#ifdef Q_CLANG_QDOC
    template<typename MemberFunction, int>
    static bool registerConverter(MemberFunction function);
    template<typename MemberFunctionOk, char>
    static bool registerConverter(MemberFunctionOk function);
    template<typename UnaryFunction>
    static bool registerConverter(UnaryFunction function);
#else
    // member function as in "QString QFont::toString() const"
    template<typename From, typename To>
    static bool registerConverter(To(From::*function)() const)
    {
        static_assert((!QMetaTypeId2<To>::IsBuiltIn || !QMetaTypeId2<From>::IsBuiltIn),
            "QMetaType::registerConverter: At least one of the types must be a custom type.");

        const int fromTypeId = qMetaTypeId<From>();
        const int toTypeId = qMetaTypeId<To>();
        auto converter = [function](const void *from, void *to) -> bool {
            const From *f = static_cast<const From *>(from);
            To *t = static_cast<To *>(to);
            *t = (f->*function)();
            return true;
        };
        return registerConverterFunction(converter, fromTypeId, toTypeId);
    }

    // member function as in "double QString::toDouble(bool *ok = nullptr) const"
    template<typename From, typename To>
    static bool registerConverter(To(From::*function)(bool*) const)
    {
        static_assert((!QMetaTypeId2<To>::IsBuiltIn || !QMetaTypeId2<From>::IsBuiltIn),
            "QMetaType::registerConverter: At least one of the types must be a custom type.");

        const int fromTypeId = qMetaTypeId<From>();
        const int toTypeId = qMetaTypeId<To>();
        auto converter = [function](const void *from, void *to) -> bool {
            const From *f = static_cast<const From *>(from);
            To *t = static_cast<To *>(to);
            bool result = true;
            *t = (f->*function)(&result);
            if (!result)
                *t = To();
            return result;
        };
        return registerConverterFunction(converter, fromTypeId, toTypeId);
    }

    // functor or function pointer
    template<typename From, typename To, typename UnaryFunction>
    static bool registerConverter(UnaryFunction function)
    {
        static_assert((!QMetaTypeId2<To>::IsBuiltIn || !QMetaTypeId2<From>::IsBuiltIn),
            "QMetaType::registerConverter: At least one of the types must be a custom type.");

        const int fromTypeId = qMetaTypeId<From>();
        const int toTypeId = qMetaTypeId<To>();
        auto converter = [function](const void *from, void *to) -> bool {
            const From *f = static_cast<const From *>(from);
            To *t = static_cast<To *>(to);
            *t = function(*f);
            return true;
        };
        return registerConverterFunction(converter, fromTypeId, toTypeId);
    }
#endif

    static bool convert(QMetaType fromType, const void *from, QMetaType toType, void *to);
    static bool canConvert(QMetaType fromType, QMetaType toType);
#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_6_0
    static bool convert(const void *from, int fromTypeId, void *to, int toTypeId)
    { return convert(QMetaType(fromTypeId), from, QMetaType(toTypeId), to); }
    QT_DEPRECATED_VERSION_6_0
    static bool compare(const void *lhs, const void *rhs, int typeId, int *result)
    {
        QMetaType t(typeId);
        auto c = t.compare(lhs, rhs);
        if (!c) {
            *result = 0;
            return false;
        }
        *result = *c;
        return true;
    }
    QT_DEPRECATED_VERSION_6_0
    static bool equals(const void *lhs, const void *rhs, int typeId, int *result)
    {
        QMetaType t(typeId);
        if (!t.isEqualityComparable())
            return false;
        *result = t.equals(lhs, rhs) ? 0 : -1;
        return true;
    }
#endif

    template<typename From, typename To>
    static bool hasRegisteredConverterFunction()
    {
        return hasRegisteredConverterFunction(qMetaTypeId<From>(), qMetaTypeId<To>());
    }

    static bool hasRegisteredConverterFunction(int fromTypeId, int toTypeId);

#ifndef Q_CLANG_QDOC
    template<typename, bool> friend struct QtPrivate::SequentialValueTypeIsMetaType;
    template<typename, bool> friend struct QtPrivate::AssociativeValueTypeIsMetaType;
    template<typename, bool> friend struct QtPrivate::IsMetaTypePair;
    template<typename, typename> friend struct QtPrivate::MetaTypeSmartPointerHelper;
#endif
    static bool registerConverterFunction(const ConverterFunction &f, int from, int to);
    static void unregisterConverterFunction(int from, int to);

    static void unregisterMetaType(QMetaType type);
    QtPrivate::QMetaTypeInterface *iface() { return d_ptr; }

private:
    friend class QVariant;
    QtPrivate::QMetaTypeInterface *d_ptr = nullptr;
};

#undef QT_DEFINE_METATYPE_ID

Q_DECLARE_OPERATORS_FOR_FLAGS(QMetaType::TypeFlags)

#define QT_METATYPE_PRIVATE_DECLARE_TYPEINFO(C, F)  \
    }                                               \
    Q_DECLARE_TYPEINFO(QtMetaTypePrivate:: C, (F)); \
    namespace QtMetaTypePrivate {

namespace QtMetaTypePrivate {

template<typename const_iterator>
struct IteratorOwnerCommon
{
    static void assign(void **ptr, const_iterator iterator)
    {
        *ptr = new const_iterator(iterator);
    }
    static void assign(void **ptr, void * const * src)
    {
        *ptr = new const_iterator(*static_cast<const_iterator*>(*src));
    }

    static void advance(void **iterator, int step)
    {
        const_iterator &it = *static_cast<const_iterator*>(*iterator);
        std::advance(it, step);
    }

    static void destroy(void **ptr)
    {
        delete static_cast<const_iterator*>(*ptr);
    }

    static bool equal(void * const *it, void * const *other)
    {
        return *static_cast<const_iterator*>(*it) == *static_cast<const_iterator*>(*other);
    }
};

template<typename const_iterator>
struct IteratorOwner : IteratorOwnerCommon<const_iterator>
{
    static void getData(void * const *iterator, void *dataPtr)
    {
        const_iterator *it = static_cast<const_iterator*>(*iterator);
        typename const_iterator::value_type *data = static_cast<typename const_iterator::value_type *>(dataPtr);
        *data = **it;
    }

    static void getData(const_iterator it, void *dataPtr)
    {
        typename const_iterator::value_type *data = static_cast<typename const_iterator::value_type *>(dataPtr);
        *data = *it;
    }
};

template<typename value_type>
struct IteratorOwner<const value_type*>
{
private:
    // We need to disable typed overloads of assign() and getData() if the value_type
    // is void* to avoid overloads conflicts. We do it by injecting unaccessible Dummy
    // type as part of the overload signature.
    struct Dummy {};
    typedef typename std::conditional<std::is_same<value_type, void*>::value, Dummy, value_type>::type value_type_OR_Dummy;
public:
    static void assign(void **ptr, const value_type_OR_Dummy *iterator )
    {
        *ptr = const_cast<value_type*>(iterator);
    }
    static void assign(void **ptr, void * const * src)
    {
        *ptr = static_cast<value_type*>(*src);
    }

    static void advance(void **iterator, int step)
    {
        value_type *it = static_cast<value_type*>(*iterator);
        std::advance(it, step);
        *iterator = it;
    }

    static void destroy(void **)
    {
    }

    static void getData(void * const *iterator, void *dataPtr)
    {
        *static_cast<value_type *>(dataPtr) = *static_cast<value_type * const>(*iterator);
    }

    static void getData(const value_type_OR_Dummy *it, void *dataPtr)
    {
        *static_cast<value_type *>(dataPtr) = *static_cast<value_type const *>(it);
    }

    static bool equal(void * const *it, void * const *other)
    {
        return static_cast<value_type const *>(*it) == static_cast<value_type const *>(*other);
    }
};

enum IteratorCapability
{
    ForwardCapability = 1,
    BiDirectionalCapability = 2,
    RandomAccessCapability = 4
};

enum ContainerCapability
{
    ContainerIsAppendable = 1
};

template<typename Container, typename T = void>
struct ContainerCapabilitiesImpl
{
    enum {ContainerCapabilities = 0};
    using appendFunction = void(*)(const void *container, const void *newElement);
    static constexpr const appendFunction appendImpl = nullptr;
};

template<typename Container>
struct ContainerCapabilitiesImpl<Container, decltype(std::declval<Container>().push_back(std::declval<typename Container::value_type>()))>
{
    enum {ContainerCapabilities = ContainerIsAppendable};

    // The code below invokes undefined behavior if and only if the pointer passed into QSequentialIterableImpl
    // pointed to a const object to begin with
    static void appendImpl(const void *container, const void *value)
    { static_cast<Container *>(const_cast<void *>(container))->push_back(*static_cast<const typename Container::value_type *>(value)); }
};

template<typename Container>
struct ContainerCapabilitiesImpl<Container,
        std::void_t<decltype(std::declval<Container>().insert(std::declval<typename Container::value_type>())),
                    decltype(std::declval<typename Container::value_type>() == std::declval<typename Container::value_type>())>>
{
    enum {ContainerCapabilities = ContainerIsAppendable};

    // The code below invokes undefined behavior if and only if the pointer passed into QSequentialIterableImpl
    // pointed to a const object to begin with
    static void appendImpl(const void *container, const void *value)
    { static_cast<Container *>(const_cast<void *>(container))->insert(*static_cast<const typename Container::value_type *>(value)); }
};

template<typename T, typename Category = typename std::iterator_traits<typename T::const_iterator>::iterator_category>
struct CapabilitiesImpl;

template<typename T>
struct CapabilitiesImpl<T, std::forward_iterator_tag>
{ enum { IteratorCapabilities = ForwardCapability }; };
template<typename T>
struct CapabilitiesImpl<T, std::bidirectional_iterator_tag>
{ enum { IteratorCapabilities = BiDirectionalCapability | ForwardCapability }; };
template<typename T>
struct CapabilitiesImpl<T, std::random_access_iterator_tag>
{ enum { IteratorCapabilities = RandomAccessCapability | BiDirectionalCapability | ForwardCapability }; };

template<typename T>
struct ContainerAPI : CapabilitiesImpl<T>
{
    static int size(const T *t) { return int(std::distance(t->begin(), t->end())); }
};

template<typename T>
struct ContainerAPI<QList<T>> : CapabilitiesImpl<QList<T>>
{ static int size(const QList<T> *t) { return t->size(); } };

template<typename T>
struct ContainerAPI<std::vector<T> > : CapabilitiesImpl<std::vector<T> >
{ static int size(const std::vector<T> *t) { return int(t->size()); } };

template<typename T>
struct ContainerAPI<std::list<T> > : CapabilitiesImpl<std::list<T> >
{ static int size(const std::list<T> *t) { return int(t->size()); } };

/*
 revision 0: _iteratorCapabilties is treated as a bitfield, the remaining bits are used to introduce
             _revision, _containerCapabilities and _unused. The latter contains 21 bits that are
             not used yet
*/
class QSequentialIterableImpl
{
public:
    const void * _iterable;
    void *_iterator;
    QMetaType _metaType;
    uint _iteratorCapabilities;
    // Iterator capabilities looks actually like
    // uint _iteratorCapabilities:4;
    // uint _revision:3;
    // uint _containerCapabilities:4;
    // uint _unused:21;
    typedef int(*sizeFunc)(const void *p);
    typedef void (*atFunc)(const void *p, int, void *);
    enum Position { ToBegin, ToEnd };
    typedef void (*moveIteratorFunc)(const void *p, void **, Position position);
    typedef void (*advanceFunc)(void **p, int);
    typedef void (*getFunc)( void * const *p, void *dataPtr);
    typedef void (*destroyIterFunc)(void **p);
    typedef bool (*equalIterFunc)(void * const *p, void * const *other);
    typedef void (*copyIterFunc)(void **, void * const *);
    typedef void(*appendFunction)(const void *container, const void *newElement);

    IteratorCapability iteratorCapabilities() {return static_cast<IteratorCapability>(_iteratorCapabilities & 0xF);}
    uint revision() {return _iteratorCapabilities >> 4 & 0x7;}
    uint containerCapabilities() {return _iteratorCapabilities >> 7 & 0xF;}

    sizeFunc _size;
    atFunc _at;
    moveIteratorFunc _moveTo;
    appendFunction _append;
    advanceFunc _advance;
    getFunc _get;
    destroyIterFunc _destroyIter;
    equalIterFunc _equalIter;
    copyIterFunc _copyIter;

    template<class T>
    static int sizeImpl(const void *p)
    { return ContainerAPI<T>::size(static_cast<const T*>(p)); }

    template<class T>
    static void atImpl(const void *p, int idx, void *dataPtr)
    {
        typename T::const_iterator i = static_cast<const T*>(p)->begin();
        std::advance(i, idx);
        IteratorOwner<typename T::const_iterator>::getData(i, dataPtr);
    }

    template<class Container>
    static void moveToImpl(const void *container, void **iterator, Position position)
    {
        auto it = (position == ToBegin) ?
                    static_cast<const Container *>(container)->begin() :
                    static_cast<const Container *>(container)->end();
        IteratorOwner<typename Container::const_iterator>::assign(iterator, it);
    }

public:
    template<class T> QSequentialIterableImpl(const T*p)
      : _iterable(p)
      , _iterator(nullptr)
      , _metaType(QMetaType::fromType<typename T::value_type>())
      , _iteratorCapabilities(ContainerAPI<T>::IteratorCapabilities | (0 << 4) | (ContainerCapabilitiesImpl<T>::ContainerCapabilities << (4+3)))
      , _size(sizeImpl<T>)
      , _at(atImpl<T>)
      , _moveTo(moveToImpl<T>)
      , _append(ContainerCapabilitiesImpl<T>::appendImpl)
      , _advance(IteratorOwner<typename T::const_iterator>::advance)
      , _get(IteratorOwner<typename T::const_iterator>::getData)
      , _destroyIter(IteratorOwner<typename T::const_iterator>::destroy)
      , _equalIter(IteratorOwner<typename T::const_iterator>::equal)
      , _copyIter(IteratorOwner<typename T::const_iterator>::assign)
    {
    }

    QSequentialIterableImpl()
      : _iterable(nullptr)
      , _iterator(nullptr)
      , _iteratorCapabilities(0 | (0 << 4) ) // no iterator capabilities, revision 0
      , _size(nullptr)
      , _at(nullptr)
      , _moveTo(nullptr)
      , _append(nullptr)
      , _advance(nullptr)
      , _get(nullptr)
      , _destroyIter(nullptr)
      , _equalIter(nullptr)
      , _copyIter(nullptr)
    {
    }

    inline void moveToBegin() {
        _moveTo(_iterable, &_iterator, ToBegin);
    }
    inline void moveToEnd() {
        _moveTo(_iterable, &_iterator, ToEnd);
    }
    inline bool equal(const QSequentialIterableImpl&other) const { return _equalIter(&_iterator, &other._iterator); }
    inline QSequentialIterableImpl &advance(int i) {
      Q_ASSERT(i > 0 || _iteratorCapabilities & BiDirectionalCapability);
      _advance(&_iterator, i);
      return *this;
    }

    inline void append(const void *newElement) {
        if (containerCapabilities() & ContainerIsAppendable)
            _append(_iterable, newElement);
    }

    inline void getCurrent(void *dataPtr) const { _get(&_iterator, dataPtr); }

    void at(int idx, void *dataPtr) const
    { return _at(_iterable, idx, dataPtr); }

    int size() const { Q_ASSERT(_iterable); return _size(_iterable); }

    inline void destroyIter() { _destroyIter(&_iterator); }

    void copy(const QSequentialIterableImpl &other)
    {
      *this = other;
      _copyIter(&_iterator, &other._iterator);
    }
};
QT_METATYPE_PRIVATE_DECLARE_TYPEINFO(QSequentialIterableImpl, Q_MOVABLE_TYPE)

template<typename From>
struct QSequentialIterableConvertFunctor
{
    QSequentialIterableImpl operator()(const From &f) const
    {
        return QSequentialIterableImpl(&f);
    }
};
}

namespace QtMetaTypePrivate {
template<typename T, bool = std::is_same<typename T::const_iterator::value_type, typename T::mapped_type>::value>
struct AssociativeContainerAccessor
{
    static const typename T::key_type& getKey(const typename T::const_iterator &it)
    {
        return it.key();
    }

    static const typename T::mapped_type& getValue(const typename T::const_iterator &it)
    {
        return it.value();
    }
};

template<typename T, bool = std::is_same<typename T::const_iterator::value_type, std::pair<const typename T::key_type, typename T::mapped_type> >::value>
struct StlStyleAssociativeContainerAccessor;

template<typename T>
struct StlStyleAssociativeContainerAccessor<T, true>
{
    static const typename T::key_type& getKey(const typename T::const_iterator &it)
    {
        return it->first;
    }

    static const typename T::mapped_type& getValue(const typename T::const_iterator &it)
    {
        return it->second;
    }
};

template<typename T>
struct AssociativeContainerAccessor<T, false> : public StlStyleAssociativeContainerAccessor<T>
{
};

class QAssociativeIterableImpl
{
public:
    const void *_iterable;
    void *_iterator;
    QMetaType _metaType_key;
    QMetaType _metaType_value;
    typedef int(*sizeFunc)(const void *p);
    typedef void (*findFunc)(const void *container, const void *p, void **iterator);
    typedef void (*beginFunc)(const void *p, void **);
    typedef void (*advanceFunc)(void **p, int);
    typedef void (*getFunc)(void * const *p, void *dataPtr);
    typedef void (*destroyIterFunc)(void **p);
    typedef bool (*equalIterFunc)(void * const *p, void * const *other);
    typedef void (*copyIterFunc)(void **, void * const *);

    sizeFunc _size;
    findFunc _find;
    beginFunc _begin;
    beginFunc _end;
    advanceFunc _advance;
    getFunc _getKey;
    getFunc _getValue;
    destroyIterFunc _destroyIter;
    equalIterFunc _equalIter;
    copyIterFunc _copyIter;

    template<class T>
    static int sizeImpl(const void *p)
    { return int(std::distance(static_cast<const T*>(p)->begin(),
                               static_cast<const T*>(p)->end())); }

    template<class T>
    static void findImpl(const void *container, const void *p, void **iterator)
    { IteratorOwner<typename T::const_iterator>::assign(iterator,
                                                        static_cast<const T*>(container)->find(*static_cast<const typename T::key_type*>(p))); }

    template<class T>
    static void advanceImpl(void **p, int step)
    { std::advance(*static_cast<typename T::const_iterator*>(*p), step); }

    template<class T>
    static void beginImpl(const void *container, void **iterator)
    { IteratorOwner<typename T::const_iterator>::assign(iterator, static_cast<const T*>(container)->begin()); }

    template<class T>
    static void endImpl(const void *container, void **iterator)
    { IteratorOwner<typename T::const_iterator>::assign(iterator, static_cast<const T*>(container)->end()); }

    template<class T>
    static void getKeyImpl(void * const *iterator, void *dataPtr)
    {
        auto *data = static_cast<typename T::key_type *>(dataPtr);
        *data = AssociativeContainerAccessor<T>::getKey(*static_cast<typename T::const_iterator*>(*iterator));
    }

    template<class T>
    static void getValueImpl(void * const *iterator, void *dataPtr)
    {
        auto *data = static_cast<typename T::mapped_type *>(dataPtr);
        *data = AssociativeContainerAccessor<T>::getValue(*static_cast<typename T::const_iterator*>(*iterator));
    }

public:
    template<class T> QAssociativeIterableImpl(const T*p)
      : _iterable(p)
      , _iterator(nullptr)
      , _metaType_key(QMetaType::fromType<typename T::key_type>())
      , _metaType_value(QMetaType::fromType<typename T::mapped_type>())
      , _size(sizeImpl<T>)
      , _find(findImpl<T>)
      , _begin(beginImpl<T>)
      , _end(endImpl<T>)
      , _advance(advanceImpl<T>)
      , _getKey(getKeyImpl<T>)
      , _getValue(getValueImpl<T>)
      , _destroyIter(IteratorOwner<typename T::const_iterator>::destroy)
      , _equalIter(IteratorOwner<typename T::const_iterator>::equal)
      , _copyIter(IteratorOwner<typename T::const_iterator>::assign)
    {
    }

    QAssociativeIterableImpl()
      : _iterable(nullptr)
      , _iterator(nullptr)
      , _size(nullptr)
      , _find(nullptr)
      , _begin(nullptr)
      , _end(nullptr)
      , _advance(nullptr)
      , _getKey(nullptr)
      , _getValue(nullptr)
      , _destroyIter(nullptr)
      , _equalIter(nullptr)
      , _copyIter(nullptr)
    {
    }

    inline void begin() { _begin(_iterable, &_iterator); }
    inline void end() { _end(_iterable, &_iterator); }
    inline bool equal(const QAssociativeIterableImpl&other) const { return _equalIter(&_iterator, &other._iterator); }
    inline QAssociativeIterableImpl &advance(int i) { _advance(&_iterator, i); return *this; }

    inline void destroyIter() { _destroyIter(&_iterator); }

    inline void getCurrentKey(void *dataPtr) const { return _getKey(&_iterator, dataPtr); }
    inline void getCurrentValue(void *dataPtr) const { return _getValue(&_iterator, dataPtr); }

    inline void find(const void *key)
    { _find(_iterable, key, &_iterator); }

    int size() const { Q_ASSERT(_iterable); return _size(_iterable); }

    void copy(const QAssociativeIterableImpl &other)
    {
      *this = other;
      _copyIter(&_iterator, &other._iterator);
    }
};
QT_METATYPE_PRIVATE_DECLARE_TYPEINFO(QAssociativeIterableImpl, Q_MOVABLE_TYPE)

template<typename From>
struct QAssociativeIterableConvertFunctor
{
    QAssociativeIterableImpl operator()(const From& f) const
    {
        return QAssociativeIterableImpl(&f);
    }
};

class QPairVariantInterfaceImpl
{
public:
    const void *_pair;
    QMetaType _metaType_first;
    QMetaType _metaType_second;

    typedef void (*getFunc)(const void * const *p, void *);

    getFunc _getFirst;
    getFunc _getSecond;

    template<class T>
    static void getFirstImpl(const void * const *pair, void *dataPtr)
    { *static_cast<typename T::first_type *>(dataPtr) = static_cast<const T*>(*pair)->first; }
    template<class T>
    static void getSecondImpl(const void * const *pair, void *dataPtr)
    { *static_cast<typename T::second_type *>(dataPtr) = static_cast<const T*>(*pair)->second; }

public:
    template<class T> QPairVariantInterfaceImpl(const T*p)
      : _pair(p)
      , _metaType_first(QMetaType::fromType<typename T::first_type>())
      , _metaType_second(QMetaType::fromType<typename T::second_type>())
      , _getFirst(getFirstImpl<T>)
      , _getSecond(getSecondImpl<T>)
    {
    }

    QPairVariantInterfaceImpl()
      : _pair(nullptr)
      , _getFirst(nullptr)
      , _getSecond(nullptr)
    {
    }

    inline void first(void *dataPtr) const { _getFirst(&_pair, dataPtr); }
    inline void second(void *dataPtr) const { _getSecond(&_pair, dataPtr); }
};
QT_METATYPE_PRIVATE_DECLARE_TYPEINFO(QPairVariantInterfaceImpl, Q_MOVABLE_TYPE)

template<typename From>
struct QPairVariantInterfaceConvertFunctor;

template<typename T, typename U>
struct QPairVariantInterfaceConvertFunctor<std::pair<T, U> >
{
    QPairVariantInterfaceImpl operator()(const std::pair<T, U>& f) const
    {
        return QPairVariantInterfaceImpl(&f);
    }
};

}

class QObject;

#define QT_FORWARD_DECLARE_SHARED_POINTER_TYPES_ITER(Name) \
    template <class T> class Name; \

QT_FOR_EACH_AUTOMATIC_TEMPLATE_SMART_POINTER(QT_FORWARD_DECLARE_SHARED_POINTER_TYPES_ITER)

namespace QtPrivate
{
    template<typename T>
    struct IsPointerToTypeDerivedFromQObject
    {
        enum { Value = false };
    };

    // Specialize to avoid sizeof(void) warning
    template<>
    struct IsPointerToTypeDerivedFromQObject<void*>
    {
        enum { Value = false };
    };
    template<>
    struct IsPointerToTypeDerivedFromQObject<const void*>
    {
        enum { Value = false };
    };
    template<>
    struct IsPointerToTypeDerivedFromQObject<QObject*>
    {
        enum { Value = true };
    };

    template<typename T>
    struct IsPointerToTypeDerivedFromQObject<T*>
    {
        typedef qint8 yes_type;
        typedef qint64 no_type;

#ifndef QT_NO_QOBJECT
        static yes_type checkType(QObject* );
#endif
        static no_type checkType(...);
        static_assert(sizeof(T), "Type argument of Q_DECLARE_METATYPE(T*) must be fully defined");
        enum { Value = sizeof(checkType(static_cast<T*>(nullptr))) == sizeof(yes_type) };
    };

    template<typename T, typename Enable = void>
    struct IsGadgetHelper { enum { IsRealGadget = false, IsGadgetOrDerivedFrom = false }; };

    template<typename T>
    struct IsGadgetHelper<T, typename T::QtGadgetHelper>
    {
        template <typename X>
        static char checkType(void (X::*)());
        static void *checkType(void (T::*)());
        enum {
            IsRealGadget = sizeof(checkType(&T::qt_check_for_QGADGET_macro)) == sizeof(void *),
            IsGadgetOrDerivedFrom = true
        };
    };

    template<typename T, typename Enable = void>
    struct IsPointerToGadgetHelper { enum { IsRealGadget = false, IsGadgetOrDerivedFrom = false }; };

    template<typename T>
    struct IsPointerToGadgetHelper<T*, typename T::QtGadgetHelper>
    {
        using BaseType = T;
        template <typename X>
        static char checkType(void (X::*)());
        static void *checkType(void (T::*)());
        enum {
            IsRealGadget = !IsPointerToTypeDerivedFromQObject<T*>::Value && sizeof(checkType(&T::qt_check_for_QGADGET_macro)) == sizeof(void *),
            IsGadgetOrDerivedFrom = !IsPointerToTypeDerivedFromQObject<T*>::Value
        };
    };


    template<typename T> char qt_getEnumMetaObject(const T&);

    template<typename T>
    struct IsQEnumHelper {
        static const T &declval();
        // If the type was declared with Q_ENUM, the friend qt_getEnumMetaObject() declared in the
        // Q_ENUM macro will be chosen by ADL, and the return type will be QMetaObject*.
        // Otherwise the chosen overload will be the catch all template function
        // qt_getEnumMetaObject(T) which returns 'char'
        enum { Value = sizeof(qt_getEnumMetaObject(declval())) == sizeof(QMetaObject*) };
    };
    template<> struct IsQEnumHelper<void> { enum { Value = false }; };

    template<typename T, typename Enable = void>
    struct MetaObjectForType
    {
        static constexpr inline const QMetaObject *value() { return nullptr; }
    };
#ifndef QT_NO_QOBJECT
    template<>
    struct MetaObjectForType<void>
    {
        static constexpr inline const QMetaObject *value() { return nullptr; }
    };
    template<typename T>
    struct MetaObjectForType<T*, typename std::enable_if<IsPointerToTypeDerivedFromQObject<T*>::Value>::type>
    {
        static constexpr inline const QMetaObject *value() { return &T::staticMetaObject; }
    };
    template<typename T>
    struct MetaObjectForType<T, typename std::enable_if<IsGadgetHelper<T>::IsGadgetOrDerivedFrom>::type>
    {
        static constexpr inline const QMetaObject *value() { return &T::staticMetaObject; }
    };
    template<typename T>
    struct MetaObjectForType<T, typename std::enable_if<IsPointerToGadgetHelper<T>::IsGadgetOrDerivedFrom>::type>
    {
        static constexpr inline const QMetaObject *value()
        {
            return &IsPointerToGadgetHelper<T>::BaseType::staticMetaObject;
        }
    };
    template<typename T>
    struct MetaObjectForType<T, typename std::enable_if<IsQEnumHelper<T>::Value>::type >
    {
        static constexpr inline const QMetaObject *value() { return qt_getEnumMetaObject(T()); }
    };
#endif

    template<typename T>
    struct IsSharedPointerToTypeDerivedFromQObject
    {
        enum { Value = false };
    };

    template<typename T>
    struct IsSharedPointerToTypeDerivedFromQObject<QSharedPointer<T> > : IsPointerToTypeDerivedFromQObject<T*>
    {
    };

    template<typename T>
    struct IsWeakPointerToTypeDerivedFromQObject
    {
        enum { Value = false };
    };

    template<typename T>
    struct IsWeakPointerToTypeDerivedFromQObject<QWeakPointer<T> > : IsPointerToTypeDerivedFromQObject<T*>
    {
    };

    template<typename T>
    struct IsTrackingPointerToTypeDerivedFromQObject
    {
        enum { Value = false };
    };

    template<typename T>
    struct IsTrackingPointerToTypeDerivedFromQObject<QPointer<T> >
    {
        enum { Value = true };
    };

    template<typename T>
    struct IsSequentialContainer
    {
        enum { Value = false };
    };

    template<typename T>
    struct IsAssociativeContainer
    {
        enum { Value = false };
    };

    template<typename T, bool = QtPrivate::IsSequentialContainer<T>::Value>
    struct SequentialContainerConverterHelper
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };

    template<typename T, bool = QMetaTypeId2<typename T::value_type>::Defined>
    struct SequentialValueTypeIsMetaType
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };

    template<typename T>
    struct SequentialContainerConverterHelper<T, true> : SequentialValueTypeIsMetaType<T>
    {
    };

    template<typename T, bool = QtPrivate::IsAssociativeContainer<T>::Value>
    struct AssociativeContainerConverterHelper
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };

    template<typename T, bool = QMetaTypeId2<typename T::mapped_type>::Defined>
    struct AssociativeValueTypeIsMetaType
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };

    template<typename T, bool = QMetaTypeId2<typename T::key_type>::Defined>
    struct KeyAndValueTypeIsMetaType
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };

    template<typename T>
    struct KeyAndValueTypeIsMetaType<T, true> : AssociativeValueTypeIsMetaType<T>
    {
    };

    template<typename T>
    struct AssociativeContainerConverterHelper<T, true> : KeyAndValueTypeIsMetaType<T>
    {
    };

    template<typename T, bool = QMetaTypeId2<typename T::first_type>::Defined
                                && QMetaTypeId2<typename T::second_type>::Defined>
    struct IsMetaTypePair
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };

    template<typename T>
    struct IsMetaTypePair<T, true>
    {
        inline static bool registerConverter(int id);
    };

    template<typename T>
    struct IsPair
    {
        static bool registerConverter(int)
        {
            return false;
        }
    };
    template<typename T, typename U>
    struct IsPair<std::pair<T, U> > : IsMetaTypePair<std::pair<T, U> > {};

    template<typename T>
    struct MetaTypePairHelper : IsPair<T> {};

    template<typename T, typename = void>
    struct MetaTypeSmartPointerHelper
    {
        static bool registerConverter(int) { return false; }
    };

    Q_CORE_EXPORT bool isBuiltinType(const QByteArray &type);
} // namespace QtPrivate

template <typename T, int =
    QtPrivate::IsPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::PointerToQObject :
    QtPrivate::IsGadgetHelper<T>::IsRealGadget             ? QMetaType::IsGadget :
    QtPrivate::IsPointerToGadgetHelper<T>::IsRealGadget    ? QMetaType::PointerToGadget :
    QtPrivate::IsQEnumHelper<T>::Value                     ? QMetaType::IsEnumeration : 0>
struct QMetaTypeIdQObject
{
    enum {
        Defined = 0
    };
};

template <typename T>
struct QMetaTypeId : public QMetaTypeIdQObject<T>
{
};

template <typename T>
struct QMetaTypeId2
{
    using NameAsArrayType = void;
    enum { Defined = QMetaTypeId<T>::Defined, IsBuiltIn=false };
    static inline constexpr int qt_metatype_id() { return QMetaTypeId<T>::qt_metatype_id(); }
};

template <typename T>
struct QMetaTypeId2<const T&> : QMetaTypeId2<T> {};

template <typename T>
struct QMetaTypeId2<T&> { enum {Defined = false }; };

namespace QtPrivate {
    template <typename T, bool Defined = QMetaTypeId2<T>::Defined>
    struct QMetaTypeIdHelper {
        static inline constexpr int qt_metatype_id()
        { return QMetaTypeId2<T>::qt_metatype_id(); }
    };
    template <typename T> struct QMetaTypeIdHelper<T, false> {
        static inline constexpr int qt_metatype_id()
        { return -1; }
    };

    // Function pointers don't derive from QObject
    template <typename Result, typename... Args>
    struct IsPointerToTypeDerivedFromQObject<Result(*)(Args...)> { enum { Value = false }; };

    template<typename T, bool = std::is_enum<T>::value>
    constexpr bool IsUnsignedEnum = false;
    template<typename T>
    constexpr bool IsUnsignedEnum<T, true> = !std::is_signed_v<std::underlying_type_t<T>>;

    template<typename T>
    struct QMetaTypeTypeFlags
    {
        enum { Flags = (QTypeInfo<T>::isRelocatable ? QMetaType::MovableType : 0)
                     | (QTypeInfo<T>::isComplex ? QMetaType::NeedsConstruction : 0)
                     | (QTypeInfo<T>::isComplex ? QMetaType::NeedsDestruction : 0)
                     | (IsPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::PointerToQObject : 0)
                     | (IsSharedPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::SharedPointerToQObject : 0)
                     | (IsWeakPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::WeakPointerToQObject : 0)
                     | (IsTrackingPointerToTypeDerivedFromQObject<T>::Value ? QMetaType::TrackingPointerToQObject : 0)
                     | (IsEnumOrFlags<T>::value ? QMetaType::IsEnumeration : 0)
                     | (IsGadgetHelper<T>::IsGadgetOrDerivedFrom ? QMetaType::IsGadget : 0)
                     | (IsPointerToGadgetHelper<T>::IsGadgetOrDerivedFrom ? QMetaType::PointerToGadget : 0)
                     | (QTypeInfo<T>::isPointer ? QMetaType::IsPointer : 0)
                     | (IsUnsignedEnum<T> ? QMetaType::IsUnsignedEnumeration : 0)
             };
    };

    template<typename T, bool defined>
    struct MetaTypeDefinedHelper
    {
        enum DefinedType { Defined = defined };
    };

    template<typename SmartPointer>
    struct QSmartPointerConvertFunctor
    {
        QObject* operator()(const SmartPointer &p) const
        {
            return p.operator->();
        }
    };

    // hack to delay name lookup to instantiation time by making
    // EnableInternalData a dependent name:
    template <typename T>
    struct EnableInternalDataWrap;

    template<typename T>
    struct QSmartPointerConvertFunctor<QWeakPointer<T> >
    {
        QObject* operator()(const QWeakPointer<T> &p) const
        {
            return QtPrivate::EnableInternalDataWrap<T>::internalData(p);
        }
    };
}

template <typename T>
int qRegisterNormalizedMetaType(const QT_PREPEND_NAMESPACE(QByteArray) &normalizedTypeName)
{
#ifndef QT_NO_QOBJECT
    Q_ASSERT_X(normalizedTypeName == QMetaObject::normalizedType(normalizedTypeName.constData()), "qRegisterNormalizedMetaType", "qRegisterNormalizedMetaType was called with a not normalized type name, please call qRegisterMetaType instead.");
#endif

    const QMetaType metaType = QMetaType::fromType<T>();
    const int id = metaType.id();

    if (id > 0) {
        QMetaType::registerNormalizedTypedef(normalizedTypeName, metaType);
        QtPrivate::SequentialContainerConverterHelper<T>::registerConverter(id);
        QtPrivate::AssociativeContainerConverterHelper<T>::registerConverter(id);
        QtPrivate::MetaTypePairHelper<T>::registerConverter(id);
        QtPrivate::MetaTypeSmartPointerHelper<T>::registerConverter(id);
    }

    return id;
}

template <typename T>
int qRegisterMetaType(const char *typeName)
{
#ifdef QT_NO_QOBJECT
    QT_PREPEND_NAMESPACE(QByteArray) normalizedTypeName = typeName;
#else
    QT_PREPEND_NAMESPACE(QByteArray) normalizedTypeName = QMetaObject::normalizedType(typeName);
#endif
    return qRegisterNormalizedMetaType<T>(normalizedTypeName);
}

template <typename T>
inline constexpr int qMetaTypeId()
{
    if constexpr (bool(QMetaTypeId2<T>::IsBuiltIn)) {
        return QMetaTypeId2<T>::MetaType;
    } else {
        return QMetaType::fromType<T>().id();
    }
}

template <typename T>
inline constexpr int qRegisterMetaType()
{
    int id = qMetaTypeId<T>();
    return id;
}

#ifndef QT_NO_QOBJECT
template <typename T>
struct QMetaTypeIdQObject<T*, QMetaType::PointerToQObject>
{
    enum {
        Defined = 1
    };

    static int qt_metatype_id()
    {
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0);
        if (const int id = metatype_id.loadAcquire())
            return id;
        const char * const cName = T::staticMetaObject.className();
        QByteArray typeName;
        typeName.reserve(int(strlen(cName)) + 1);
        typeName.append(cName).append('*');
        const int newId = qRegisterNormalizedMetaType<T*>(typeName);
        metatype_id.storeRelease(newId);
        return newId;
    }
};

template <typename T>
struct QMetaTypeIdQObject<T, QMetaType::IsGadget>
{
    enum {
        Defined = std::is_default_constructible<T>::value
    };

    static int qt_metatype_id()
    {
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0);
        if (const int id = metatype_id.loadAcquire())
            return id;
        const char * const cName = T::staticMetaObject.className();
        const int newId = qRegisterNormalizedMetaType<T>(cName);
        metatype_id.storeRelease(newId);
        return newId;
    }
};

template <typename T>
struct QMetaTypeIdQObject<T*, QMetaType::PointerToGadget>
{
    enum {
        Defined = 1
    };

    static int qt_metatype_id()
    {
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0);
        if (const int id = metatype_id.loadAcquire())
            return id;
        const char * const cName = T::staticMetaObject.className();
        QByteArray typeName;
        typeName.reserve(int(strlen(cName)) + 1);
        typeName.append(cName).append('*');
        const int newId = qRegisterNormalizedMetaType<T*>(typeName);
        metatype_id.storeRelease(newId);
        return newId;
    }
};

template <typename T>
struct QMetaTypeIdQObject<T, QMetaType::IsEnumeration>
{
    enum {
        Defined = 1
    };

    static int qt_metatype_id()
    {
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0);
        if (const int id = metatype_id.loadAcquire())
            return id;
        const char *eName = qt_getEnumName(T());
        const char *cName = qt_getEnumMetaObject(T())->className();
        QByteArray typeName;
        typeName.reserve(int(strlen(cName) + 2 + strlen(eName)));
        typeName.append(cName).append("::").append(eName);
        const int newId = qRegisterNormalizedMetaType<T>(typeName);
        metatype_id.storeRelease(newId);
        return newId;
    }
};
#endif

#define Q_DECLARE_OPAQUE_POINTER(POINTER)                               \
    QT_BEGIN_NAMESPACE namespace QtPrivate {                            \
        template <>                                                     \
        struct IsPointerToTypeDerivedFromQObject<POINTER >              \
        {                                                               \
            enum { Value = false };                                     \
        };                                                              \
    } QT_END_NAMESPACE                                                  \
    /**/

#ifndef Q_MOC_RUN
#define Q_DECLARE_METATYPE(TYPE) Q_DECLARE_METATYPE_IMPL(TYPE)
#define Q_DECLARE_METATYPE_IMPL(TYPE)                                   \
    QT_BEGIN_NAMESPACE                                                  \
    template <>                                                         \
    struct QMetaTypeId< TYPE >                                          \
    {                                                                   \
        enum { Defined = 1 };                                           \
        static int qt_metatype_id()                                     \
            {                                                           \
                static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
                if (const int id = metatype_id.loadAcquire())           \
                    return id;                                          \
                const int newId = qRegisterMetaType< TYPE >(#TYPE);     \
                metatype_id.storeRelease(newId);                        \
                return newId;                                           \
            }                                                           \
    };                                                                  \
    QT_END_NAMESPACE
#endif // Q_MOC_RUN

#define Q_DECLARE_BUILTIN_METATYPE(TYPE, METATYPEID, NAME) \
    QT_BEGIN_NAMESPACE \
    template<> struct QMetaTypeId2<NAME> \
    { \
        using NameAsArrayType = std::array<char, sizeof(#NAME)>; \
        enum { Defined = 1, IsBuiltIn = true, MetaType = METATYPEID };   \
        static inline constexpr int qt_metatype_id() { return METATYPEID; } \
        static constexpr NameAsArrayType nameAsArray = { #NAME }; \
    }; \
    QT_END_NAMESPACE

#define QT_FORWARD_DECLARE_STATIC_TYPES_ITER(TypeName, TypeId, Name) \
    class Name;

QT_FOR_EACH_STATIC_CORE_CLASS(QT_FORWARD_DECLARE_STATIC_TYPES_ITER)
QT_FOR_EACH_STATIC_GUI_CLASS(QT_FORWARD_DECLARE_STATIC_TYPES_ITER)
QT_FOR_EACH_STATIC_WIDGETS_CLASS(QT_FORWARD_DECLARE_STATIC_TYPES_ITER)

#undef QT_FORWARD_DECLARE_STATIC_TYPES_ITER

typedef QList<QVariant> QVariantList;
typedef QMap<QString, QVariant> QVariantMap;
typedef QHash<QString, QVariant> QVariantHash;
typedef QPair<QVariant, QVariant> QVariantPair;
#ifdef Q_CLANG_QDOC
class QByteArrayList;
#else
using QByteArrayList = QList<QByteArray>;
#endif

#define Q_DECLARE_METATYPE_TEMPLATE_1ARG(SINGLE_ARG_TEMPLATE) \
QT_BEGIN_NAMESPACE \
template <typename T> \
struct QMetaTypeId< SINGLE_ARG_TEMPLATE<T> > \
{ \
    enum { \
        Defined = QMetaTypeId2<T>::Defined \
    }; \
    static int qt_metatype_id() \
    { \
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
        if (const int id = metatype_id.loadRelaxed()) \
            return id; \
        const char *tName = QMetaType::fromType<T>().name(); \
        Q_ASSERT(tName); \
        const int tNameLen = int(qstrlen(tName)); \
        QByteArray typeName; \
        typeName.reserve(int(sizeof(#SINGLE_ARG_TEMPLATE)) + 1 + tNameLen + 1 + 1); \
        typeName.append(#SINGLE_ARG_TEMPLATE, int(sizeof(#SINGLE_ARG_TEMPLATE)) - 1) \
            .append('<').append(tName, tNameLen); \
        typeName.append('>'); \
        const int newId = qRegisterNormalizedMetaType< SINGLE_ARG_TEMPLATE<T> >(typeName); \
        metatype_id.storeRelease(newId); \
        return newId; \
    } \
}; \
namespace QtPrivate { \
template<typename T> \
struct IsSequentialContainer<SINGLE_ARG_TEMPLATE<T> > \
{ \
    enum { Value = true }; \
}; \
} \
QT_END_NAMESPACE

#define Q_DECLARE_METATYPE_TEMPLATE_2ARG(DOUBLE_ARG_TEMPLATE) \
QT_BEGIN_NAMESPACE \
template<typename T, typename U> \
struct QMetaTypeId< DOUBLE_ARG_TEMPLATE<T, U> > \
{ \
    enum { \
        Defined = QMetaTypeId2<T>::Defined && QMetaTypeId2<U>::Defined \
    }; \
    static int qt_metatype_id() \
    { \
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
        if (const int id = metatype_id.loadAcquire()) \
            return id; \
        const char *tName = QMetaType::fromType<T>().name(); \
        const char *uName = QMetaType::fromType<U>().name(); \
        Q_ASSERT(tName); \
        Q_ASSERT(uName); \
        const int tNameLen = int(qstrlen(tName)); \
        const int uNameLen = int(qstrlen(uName)); \
        QByteArray typeName; \
        typeName.reserve(int(sizeof(#DOUBLE_ARG_TEMPLATE)) + 1 + tNameLen + 1 + uNameLen + 1 + 1); \
        typeName.append(#DOUBLE_ARG_TEMPLATE, int(sizeof(#DOUBLE_ARG_TEMPLATE)) - 1) \
            .append('<').append(tName, tNameLen).append(',').append(uName, uNameLen); \
        typeName.append('>'); \
        const int newId = qRegisterNormalizedMetaType< DOUBLE_ARG_TEMPLATE<T, U> >(typeName); \
        metatype_id.storeRelease(newId); \
        return newId; \
    } \
}; \
QT_END_NAMESPACE

namespace QtPrivate {

template<typename T, bool /* isSharedPointerToQObjectDerived */ = false>
struct SharedPointerMetaTypeIdHelper
{
    enum {
        Defined = 0
    };
    static int qt_metatype_id()
    {
        return -1;
    }
};

}

#define Q_DECLARE_SMART_POINTER_METATYPE(SMART_POINTER) \
QT_BEGIN_NAMESPACE \
namespace QtPrivate { \
template<typename T> \
struct SharedPointerMetaTypeIdHelper<SMART_POINTER<T>, true> \
{ \
    enum { \
        Defined = 1 \
    }; \
    static int qt_metatype_id() \
    { \
        static QBasicAtomicInt metatype_id = Q_BASIC_ATOMIC_INITIALIZER(0); \
        if (const int id = metatype_id.loadAcquire()) \
            return id; \
        const char * const cName = T::staticMetaObject.className(); \
        QByteArray typeName; \
        typeName.reserve(int(sizeof(#SMART_POINTER) + 1 + strlen(cName) + 1)); \
        typeName.append(#SMART_POINTER, int(sizeof(#SMART_POINTER)) - 1) \
            .append('<').append(cName).append('>'); \
        const int newId = qRegisterNormalizedMetaType< SMART_POINTER<T> >(typeName); \
        metatype_id.storeRelease(newId); \
        return newId; \
    } \
}; \
template<typename T> \
struct MetaTypeSmartPointerHelper<SMART_POINTER<T> , \
        typename std::enable_if<IsPointerToTypeDerivedFromQObject<T*>::Value>::type> \
{ \
    static bool registerConverter(int id) \
    { \
        const int toId = QMetaType::QObjectStar; \
        if (!QMetaType::hasRegisteredConverterFunction(id, toId)) { \
            QtPrivate::QSmartPointerConvertFunctor<SMART_POINTER<T> > o; \
            return QMetaType::registerConverter<SMART_POINTER<T>, QObject*>(o); \
        } \
        return true; \
    } \
}; \
} \
template <typename T> \
struct QMetaTypeId< SMART_POINTER<T> > \
    : QtPrivate::SharedPointerMetaTypeIdHelper< SMART_POINTER<T>, \
                                                QtPrivate::IsPointerToTypeDerivedFromQObject<T*>::Value> \
{ \
};\
QT_END_NAMESPACE

#define Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE_ITER(TEMPLATENAME) \
    QT_BEGIN_NAMESPACE \
    template <class T> class TEMPLATENAME; \
    QT_END_NAMESPACE \
    Q_DECLARE_METATYPE_TEMPLATE_1ARG(TEMPLATENAME)

QT_END_NAMESPACE

QT_FOR_EACH_AUTOMATIC_TEMPLATE_1ARG(Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE_ITER)

#undef Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE_ITER

#define Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE Q_DECLARE_METATYPE_TEMPLATE_1ARG

Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE(std::vector)
Q_DECLARE_SEQUENTIAL_CONTAINER_METATYPE(std::list)

#define Q_FORWARD_DECLARE_METATYPE_TEMPLATE_2ARG_ITER(TEMPLATENAME, CPPTYPE) \
    QT_BEGIN_NAMESPACE \
    template <class T1, class T2> CPPTYPE TEMPLATENAME; \
    QT_END_NAMESPACE \

QT_FOR_EACH_AUTOMATIC_TEMPLATE_2ARG(Q_FORWARD_DECLARE_METATYPE_TEMPLATE_2ARG_ITER)

#undef Q_DECLARE_METATYPE_TEMPLATE_2ARG_ITER

#define Q_DECLARE_ASSOCIATIVE_CONTAINER_METATYPE(TEMPLATENAME) \
    QT_BEGIN_NAMESPACE \
    namespace QtPrivate { \
    template<typename T, typename U> \
    struct IsAssociativeContainer<TEMPLATENAME<T, U> > \
    { \
        enum { Value = true }; \
    }; \
    } \
    QT_END_NAMESPACE \
    Q_DECLARE_METATYPE_TEMPLATE_2ARG(TEMPLATENAME)

Q_DECLARE_ASSOCIATIVE_CONTAINER_METATYPE(QHash)
Q_DECLARE_ASSOCIATIVE_CONTAINER_METATYPE(QMap)
Q_DECLARE_ASSOCIATIVE_CONTAINER_METATYPE(std::map)

Q_DECLARE_METATYPE_TEMPLATE_2ARG(std::pair)

#define Q_DECLARE_METATYPE_TEMPLATE_SMART_POINTER_ITER(TEMPLATENAME) \
    Q_DECLARE_SMART_POINTER_METATYPE(TEMPLATENAME)


QT_FOR_EACH_AUTOMATIC_TEMPLATE_SMART_POINTER(Q_DECLARE_METATYPE_TEMPLATE_SMART_POINTER_ITER)

QT_BEGIN_NAMESPACE

#undef Q_DECLARE_METATYPE_TEMPLATE_SMART_POINTER_ITER

QT_END_NAMESPACE


QT_FOR_EACH_STATIC_TYPE(Q_DECLARE_BUILTIN_METATYPE)

Q_DECLARE_METATYPE(QtMetaTypePrivate::QSequentialIterableImpl)
Q_DECLARE_METATYPE(QtMetaTypePrivate::QAssociativeIterableImpl)
Q_DECLARE_METATYPE(QtMetaTypePrivate::QPairVariantInterfaceImpl)

QT_BEGIN_NAMESPACE

template <typename T>
inline bool QtPrivate::IsMetaTypePair<T, true>::registerConverter(int id)
{
    const int toId = qMetaTypeId<QtMetaTypePrivate::QPairVariantInterfaceImpl>();
    if (!QMetaType::hasRegisteredConverterFunction(id, toId)) {
        QtMetaTypePrivate::QPairVariantInterfaceConvertFunctor<T> o;
        return QMetaType::registerConverter<T, QtMetaTypePrivate::QPairVariantInterfaceImpl>(o);
    }
    return true;
}

namespace QtPrivate {
    template<typename T>
    struct SequentialValueTypeIsMetaType<T, true>
    {
        static bool registerConverter(int id)
        {
            const int toId = qMetaTypeId<QtMetaTypePrivate::QSequentialIterableImpl>();
            if (!QMetaType::hasRegisteredConverterFunction(id, toId)) {
                QtMetaTypePrivate::QSequentialIterableConvertFunctor<T> o;
                return QMetaType::registerConverter<T, QtMetaTypePrivate::QSequentialIterableImpl>(o);
            }
            return true;
        }
    };

    template<typename T>
    struct AssociativeValueTypeIsMetaType<T, true>
    {
        static bool registerConverter(int id)
        {
            const int toId = qMetaTypeId<QtMetaTypePrivate::QAssociativeIterableImpl>();
            if (!QMetaType::hasRegisteredConverterFunction(id, toId)) {
                QtMetaTypePrivate::QAssociativeIterableConvertFunctor<T> o;
                return QMetaType::registerConverter<T, QtMetaTypePrivate::QAssociativeIterableImpl>(o);
            }
            return true;
        }
    };
}

namespace QtPrivate {

class QMetaTypeInterface
{
public:
    ushort revision; // 0 in Qt 6.0. Can increase if new field are added
    ushort alignment;
    uint size;
    uint flags;
    QBasicAtomicInt typeId;
    const QMetaObject *metaObject;
    const char *name;

    using DefaultCtrFn = void (*)(const QMetaTypeInterface *, void *);
    DefaultCtrFn defaultCtr;
    using CopyCtrFn = void (*)(const QMetaTypeInterface *, void *, const void *);
    CopyCtrFn copyCtr;
    using MoveCtrFn = void (*)(const QMetaTypeInterface *, void *, void *);
    MoveCtrFn moveCtr;
    using DtorFn = void (*)(const QMetaTypeInterface *, void *);
    DtorFn dtor;
    using EqualsFn = bool (*)(const QMetaTypeInterface *, const void *, const void *);
    EqualsFn equals;
    using LessThanFn = bool (*)(const QMetaTypeInterface *, const void *, const void *);
    LessThanFn lessThan;
    using DebugStreamFn = void (*)(const QMetaTypeInterface *, QDebug &, const void *);
    DebugStreamFn debugStream;
    using DataStreamOutFn = void (*)(const QMetaTypeInterface *, QDataStream &, const void *);
    DataStreamOutFn dataStreamOut;
    using DataStreamInFn = void (*)(const QMetaTypeInterface *, QDataStream &, void *);
    DataStreamInFn dataStreamIn;

    using LegacyRegisterOp = void (*)();
    LegacyRegisterOp legacyRegisterOp;
};

struct QTypeNormalizer
{
    char *output;
    int len = 0;
    char last = 0;

private:
    static constexpr bool is_ident_char(char s)
    {
        return ((s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z') || (s >= '0' && s <= '9')
                || s == '_');
    }
    static constexpr bool is_space(char s) { return (s == ' ' || s == '\t' || s == '\n'); }
    static constexpr bool is_number(char s) { return s >= '0' && s <= '9'; }
    static constexpr bool starts_with_token(const char *b, const char *e, const char *token,
                                            bool msvcKw = false)
    {
        while (b != e && *token && *b == *token) {
            b++;
            token++;
        }
        if (*token)
            return false;
#ifdef Q_CC_MSVC
        /// On MSVC, keywords like class or struct are not separated with spaces in constexpr
        /// context
        if (msvcKw)
            return true;
#endif
        Q_UNUSED(msvcKw);
        return b == e || !is_ident_char(*b);
    }
    static constexpr bool skipToken(const char *&x, const char *e, const char *token,
                                    bool msvcKw = false)
    {
        if (!starts_with_token(x, e, token, msvcKw))
            return false;
        while (*token++)
            x++;
        while (x != e && is_space(*x))
            x++;
        return true;
    }
    static constexpr const char *skipString(const char *x, const char *e)
    {
        char delim = *x;
        x++;
        while (x != e && *x != delim) {
            if (*x == '\\') {
                x++;
                if (x == e)
                    return e;
            }
            x++;
        }
        if (x != e)
            x++;
        return x;
    };
    static constexpr const char *skipTemplate(const char *x, const char *e, bool stopAtComa = false)
    {
        int scopeDepth = 0;
        int templateDepth = 0;
        while (x != e) {
            switch (*x) {
            case '<':
                if (!scopeDepth)
                    templateDepth++;
                break;
            case ',':
                if (stopAtComa && !scopeDepth && !templateDepth)
                    return x;
                break;
            case '>':
                if (!scopeDepth)
                    if (--templateDepth < 0)
                        return x;
                break;
            case '(':
            case '[':
            case '{':
                scopeDepth++;
                break;
            case '}':
            case ']':
            case ')':
                scopeDepth--;
                break;
            case '\'':
                if (is_number(x[-1]))
                    break;
                Q_FALLTHROUGH();
            case '\"':
                x = skipString(x, e);
                continue;
            }
            x++;
        }
        return x;
    };

    constexpr void append(char x)
    {
        last = x;
        len++;
        if (output)
            *output++ = x;
    }

    constexpr void appendStr(const char *x)
    {
        while (*x)
            append(*x++);
    };

public:
    constexpr int normalizeType(const char *begin, const char *end, bool adjustConst = true)
    {
        // Trim spaces
        while (begin != end && is_space(*begin))
            begin++;
        while (begin != end && is_space(*(end - 1)))
            end--;

        // Convert 'char const *' into 'const char *'. Start at index 1,
        // not 0, because 'const char *' is already OK.
        const char *cst = begin + 1;
        if (*begin == '\'' || *begin == '"')
            cst = skipString(begin, end);
        bool seenStar = false;
        bool hasMiddleConst = false;
        while (cst < end) {
            if (*cst == '\"' || (*cst == '\'' && !is_number(cst[-1]))) {
                cst = skipString(cst, end);
                if (cst == end)
                    break;
            }

            // We mustn't convert 'char * const *' into 'const char **'
            // and we must beware of 'Bar<const Bla>'.
            if (*cst == '&' || *cst == '*' || *cst == '[') {
                seenStar = *cst != '&' || cst != (end - 1);
                break;
            }
            if (*cst == '<') {
                cst = skipTemplate(cst + 1, end);
                if (cst == end)
                    break;
            }
            cst++;
            const char *skipedCst = cst;
            if (!is_ident_char(*(cst - 1)) && skipToken(skipedCst, end, "const")) {
                const char *testEnd = end;
                while (skipedCst < testEnd--) {
                    if (*testEnd == '*' || *testEnd == '['
                        || (*testEnd == '&' && testEnd != (end - 1))) {
                        seenStar = true;
                        break;
                    }
                    if (*testEnd == '>')
                        break;
                }
                if (adjustConst && !seenStar) {
                    if (*(end - 1) == '&')
                        end--;
                } else {
                    appendStr("const ");
                }
                normalizeType(begin, cst, false);
                begin = skipedCst;
                hasMiddleConst = true;
                break;
            }
        }
        if (skipToken(begin, end, "const")) {
            if (adjustConst && !seenStar) {
                if (*(end - 1) == '&')
                    end--;
            } else {
                appendStr("const ");
            }
        }
        if (seenStar && adjustConst) {
            const char *e = end;
            if (*(end - 1) == '&' && *(end - 2) != '&')
                e--;
            while (begin != e && is_space(*(e - 1)))
                e--;
            const char *token = "tsnoc"; // 'const' reverse, to check if it ends with const
            while (*token && begin != e && *(--e) == *token++)
                ;
            if (!*token && begin != e && !is_ident_char(*(e - 1))) {
                while (begin != e && is_space(*(e - 1)))
                    e--;
                end = e;
            }
        }

        // discard 'struct', 'class', and 'enum'; they are optional
        // and we don't want them in the normalized signature
        skipToken(begin, end, "struct", true) || skipToken(begin, end, "class", true)
                || skipToken(begin, end, "enum", true);

#ifdef QT_NAMESPACE
        const char *nsbeg = begin;
        if (skipToken(nsbeg, end, QT_STRINGIFY(QT_NAMESPACE)) && nsbeg + 2 < end && nsbeg[0] == ':'
            && nsbeg[1] == ':') {
            begin = nsbeg + 2;
            while (begin != end && is_space(*begin))
                begin++;
        }
#endif

        if (skipToken(begin, end, "QVector")) {
            // Replace QVector by QList
            appendStr("QList");
        }

        if (skipToken(begin, end, "QPair")) {
            // replace QPair by std::pair
            appendStr("std::pair");
        }

        if (!hasMiddleConst) {
            // Normalize the integer types
            int numLong = 0;
            int numSigned = 0;
            int numUnsigned = 0;
            int numInt = 0;
            int numShort = 0;
            int numChar = 0;
            while (begin < end) {
                if (skipToken(begin, end, "long")) {
                    numLong++;
                    continue;
                }
                if (skipToken(begin, end, "int")) {
                    numInt++;
                    continue;
                }
                if (skipToken(begin, end, "short")) {
                    numShort++;
                    continue;
                }
                if (skipToken(begin, end, "unsigned")) {
                    numUnsigned++;
                    continue;
                }
                if (skipToken(begin, end, "signed")) {
                    numSigned++;
                    continue;
                }
                if (skipToken(begin, end, "char")) {
                    numChar++;
                    continue;
                }
                break;
            }
            if (numChar || numShort) {
                if (numSigned && numChar)
                    appendStr("signed ");
                if (numUnsigned)
                    appendStr("unsigned ");
                if (numChar)
                    appendStr("char");
                else
                    appendStr("short");
            } else if (numLong) {
                if (numLong == 1) {
                    if (numUnsigned)
                        append('u');
                    appendStr("long");
                } else {
                    if (numUnsigned)
                        appendStr("unsigned ");
                    appendStr("long long");
                }
            } else if (numUnsigned || numSigned || numInt) {
                if (numUnsigned)
                    append('u');
                appendStr("int");
            }
        }

        bool spaceSkiped = true;
        while (begin != end) {
            char c = *begin++;
            if (is_space(c)) {
                spaceSkiped = true;
            } else if ((c == '\'' && !is_number(last)) || c == '\"') {
                begin--;
                auto x = skipString(begin, end);
                while (begin < x)
                    append(*begin++);
            } else {
                if (spaceSkiped && is_ident_char(last) && is_ident_char(c))
                    append(' ');
                append(c);
                spaceSkiped = false;
                if (c == '<') {
                    do {
                        // template recursion
                        const char *tpl = skipTemplate(begin, end, true);
                        normalizeType(begin, tpl, false);
                        if (tpl == end)
                            return len;
                        append(*tpl);
                        begin = tpl;
                    } while (*begin++ == ',');
                }
            }
        }
        return len;
    }
};

// Normalize the type between begin and end, and store the data in the output. Returns the length.
// The idea is to first run this function with nullptr as output to allocate the output with the
// size
constexpr int qNormalizeType(const char *begin, const char *end, char *output)
{
    return QTypeNormalizer { output }.normalizeType(begin, end);
}

template<typename T>
struct is_std_pair : std::false_type {};

template <typename T1_, typename T2_>
struct is_std_pair<std::pair<T1_, T2_>> : std::true_type {
    using T1 = T1_;
    using T2 = T2_;
};

template<typename T>
constexpr auto typenameHelper()
{
    if constexpr (is_std_pair<T>::value) {
        using T1 = typename is_std_pair<T>::T1;
        using T2 = typename is_std_pair<T>::T2;
        std::remove_const_t<std::conditional_t<bool (QMetaTypeId2<T1>::IsBuiltIn), typename QMetaTypeId2<T1>::NameAsArrayType, decltype(typenameHelper<T1>())>> t1Name {};
        std::remove_const_t<std::conditional_t<bool (QMetaTypeId2<T2>::IsBuiltIn), typename QMetaTypeId2<T2>::NameAsArrayType, decltype(typenameHelper<T2>())>> t2Name {};
        if constexpr (bool (QMetaTypeId2<T1>::IsBuiltIn) ) {
            t1Name = QMetaTypeId2<T1>::nameAsArray;
        } else {
            t1Name = typenameHelper<T1>();
        }
        if constexpr (bool (QMetaTypeId2<T2>::IsBuiltIn) ) {
            t2Name = QMetaTypeId2<T2>::nameAsArray;
        } else {
            t2Name = typenameHelper<T2>();
        }
        constexpr auto nonTypeDependentLen = sizeof("std::pair<,>");
        constexpr auto t1Len = t1Name.size() - 1;
        constexpr auto t2Len = t2Name.size() - 1;
        constexpr auto length = nonTypeDependentLen + t1Len + t2Len;
        std::array<char, length + 1> result {};
        constexpr auto prefix = "std::pair<";
        int currentLength = 0;
        for (; currentLength < int(sizeof("std::pair<")-1); ++currentLength)
            result[currentLength] = prefix[currentLength];
        for (int i = 0; i < int(t1Len); ++currentLength, ++i)
            result[currentLength] = t1Name[i];
        result[currentLength++] = ',';
        for (int i = 0; i < int(t2Len); ++currentLength, ++i)
            result[currentLength] = t2Name[i];
        result[currentLength++] = '>';
        result[currentLength++] = '\0';
        return result;
    } else {
        constexpr auto prefix = sizeof(
#ifdef QT_NAMESPACE
            QT_STRINGIFY(QT_NAMESPACE) "::"
#endif
#ifdef Q_CC_MSVC
            "auto __cdecl QtPrivate::typenameHelper<"
#elif defined(Q_CC_CLANG)
            "auto QtPrivate::typenameHelper() [T = "
#else
            "constexpr auto QtPrivate::typenameHelper() [with T = "
#endif
            ) - 1;
#ifdef Q_CC_MSVC
        constexpr int suffix = sizeof(">(void)");
#else
        constexpr int suffix = sizeof("]");
#endif

#if !(defined(Q_CC_GNU) && !defined(Q_CC_INTEL) && !defined(Q_CC_CLANG))
        constexpr auto func = Q_FUNC_INFO;
        constexpr const char *begin = func + prefix;
        constexpr const char *end = func + sizeof(Q_FUNC_INFO) - suffix;
        constexpr int len = qNormalizeType(begin, end, nullptr);
#else // GCC < 8.1 did not have Q_FUNC_INFO as constexpr, and GCC 9 has a precompiled header bug
        auto func = Q_FUNC_INFO;
        const char *begin = func + prefix;
        const char *end = func + sizeof(Q_FUNC_INFO) - suffix;
        // This is an upper bound of the size since the normalized signature should always be smaller
        // (Unless there is a QList -> QVector change, but that should not happen)
        constexpr int len = sizeof(Q_FUNC_INFO) - suffix - prefix;
#endif
        std::array<char, len + 1> result {};
        qNormalizeType(begin, end, result.data());
        return result;
    }
}

template<typename T, typename = void>
struct BuiltinMetaType : std::integral_constant<int, 0>
{
};
template<typename T>
struct BuiltinMetaType<T, std::enable_if_t<QMetaTypeId2<T>::IsBuiltIn>>
    : std::integral_constant<int, QMetaTypeId2<T>::MetaType>
{
};

template<typename T, bool = (QTypeTraits::has_operator_equal_v<T> && !std::is_pointer_v<T>)>
struct QEqualityOperatorForType
{
    static bool equals(const QMetaTypeInterface *, const void *a, const void *b)
    { return *reinterpret_cast<const T *>(a) == *reinterpret_cast<const T *>(b); }
};

template<typename T>
struct QEqualityOperatorForType <T, false>
{
    static constexpr QMetaTypeInterface::EqualsFn equals = nullptr;
};

template<typename T, bool = (QTypeTraits::has_operator_less_than_v<T> && !std::is_pointer_v<T>)>
struct QLessThanOperatorForType
{
    static bool lessThan(const QMetaTypeInterface *, const void *a, const void *b)
    { return *reinterpret_cast<const T *>(a) < *reinterpret_cast<const T *>(b); }
};

template<typename T>
struct QLessThanOperatorForType <T, false>
{
    static constexpr QMetaTypeInterface::LessThanFn lessThan = nullptr;
};

template<typename T, bool = (QTypeTraits::has_ostream_operator_v<QDebug, T> && !std::is_pointer_v<T>)>
struct QDebugStreamOperatorForType
{
    static void debugStream(const QMetaTypeInterface *, QDebug &dbg, const void *a)
    { dbg << *reinterpret_cast<const T *>(a); }
};

template<typename T>
struct QDebugStreamOperatorForType <T, false>
{
    static constexpr QMetaTypeInterface::DebugStreamFn debugStream = nullptr;
};

template<typename T, bool = QTypeTraits::has_stream_operator_v<QDataStream, T>>
struct QDataStreamOperatorForType
{
    static void dataStreamOut(const QMetaTypeInterface *, QDataStream &ds, const void *a)
    { ds << *reinterpret_cast<const T *>(a); }
    static void dataStreamIn(const QMetaTypeInterface *, QDataStream &ds, void *a)
    { ds >> *reinterpret_cast<T *>(a); }
};

template<typename T>
struct QDataStreamOperatorForType <T, false>
{
    static constexpr QMetaTypeInterface::DataStreamOutFn dataStreamOut = nullptr;
    static constexpr QMetaTypeInterface::DataStreamInFn dataStreamIn = nullptr;
};

template<typename S>
class QMetaTypeForType
{
    static const decltype(typenameHelper<S>()) name;

    template<typename T>
    static constexpr QMetaTypeInterface::DefaultCtrFn getDefaultCtr()
    {
      if constexpr (std::is_default_constructible_v<T>) {
        return [](const QMetaTypeInterface *, void *addr) { new (addr) T(); };
      } else {
        return nullptr;
      }
    }

    template<typename T>
    static constexpr QMetaTypeInterface::CopyCtrFn getCopyCtr()
    {
      if constexpr (std::is_copy_constructible_v<T>) {
        return [](const QMetaTypeInterface *, void *addr, const void *other) {
          new (addr) T(*reinterpret_cast<const T *>(other));
        };
      } else {
        return nullptr;
      }
    }

    template<typename T>
    static constexpr QMetaTypeInterface::MoveCtrFn getMoveCtr()
    {
      if constexpr (std::is_move_constructible_v<T>) {
        return [](const QMetaTypeInterface *, void *addr, void *other) {
          new (addr) T(std::move(*reinterpret_cast<T *>(other)));
        };
      } else {
        return nullptr;
      }
    }

    template<typename T>
    static constexpr QMetaTypeInterface::DtorFn getDtor()
    {
      if constexpr (std::is_destructible_v<T>)
        return [](const QMetaTypeInterface *, void *addr) { reinterpret_cast<T *>(addr)->~T(); };
      else
        return nullptr;
    }

    template<typename T>
    static constexpr QMetaTypeInterface::LegacyRegisterOp getLegacyRegister()
    {
      if constexpr (QMetaTypeId2<T>::Defined && !QMetaTypeId2<T>::IsBuiltIn) {
        return []() { QMetaTypeId2<T>::qt_metatype_id(); };
      } else {
        return nullptr;
      }
    }

    static constexpr const char *getName()
    {
        if constexpr (bool(QMetaTypeId2<S>::IsBuiltIn)) {
            return QMetaTypeId2<S>::nameAsArray.data();
        } else {
            return name.data();
        }
    }

public:
    static QMetaTypeInterface metaType;
};

template<typename T>
QMetaTypeInterface QMetaTypeForType<T>::metaType = {
    /*.revision=*/ 0,
    /*.alignment=*/ alignof(T),
    /*.size=*/ sizeof(T),
    /*.flags=*/ QMetaTypeTypeFlags<T>::Flags,
    /*.typeId=*/ BuiltinMetaType<T>::value,
    /*.metaObject=*/ MetaObjectForType<T>::value(),
    /*.name=*/ getName(),
    /*.defaultCtr=*/ getDefaultCtr<T>(),
    /*.copyCtr=*/ getCopyCtr<T>(),
    /*.moveCtr=*/ getMoveCtr<T>(),
    /*.dtor=*/ getDtor<T>(),
    /*.equals=*/ QEqualityOperatorForType<T>::equals,
    /*.lessThan=*/ QLessThanOperatorForType<T>::lessThan,
    /*.debugStream=*/ QDebugStreamOperatorForType<T>::debugStream,
    /*.dataStreamOut=*/ QDataStreamOperatorForType<T>::dataStreamOut,
    /*.dataStreamIn=*/ QDataStreamOperatorForType<T>::dataStreamIn,
    /*.legacyRegisterOp=*/ getLegacyRegister<T>()
};

template<typename T>
constexpr const decltype(typenameHelper<T>()) QMetaTypeForType<T>::name = typenameHelper<T>();

template<>
class QMetaTypeForType<void>
{
    static const decltype(typenameHelper<void>()) name;

public:
    static inline QMetaTypeInterface metaType =
    {
        /*.revision=*/ 0,
        /*.alignment=*/ 0,
        /*.size=*/ 0,
        /*.flags=*/ 0,
        /*.typeId=*/ BuiltinMetaType<void>::value,
        /*.metaObject=*/ nullptr,
        /*.name=*/ "void",
        /*.defaultCtr=*/ nullptr,
        /*.copyCtr=*/ nullptr,
        /*.moveCtr=*/ nullptr,
        /*.dtor=*/ nullptr,
        /*.equals=*/ nullptr,
        /*.lessThan=*/ nullptr,
        /*.debugStream=*/ nullptr,
        /*.dataStreamOut=*/ nullptr,
        /*.dataStreamIn=*/ nullptr,
        /*.legacyRegisterOp=*/ nullptr
    };
};
#undef QT_METATYPE_CONSTEXPRLAMDA

#ifndef QT_BOOTSTRAPPED
#define QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER(TypeName, Id, Name)                               \
    extern template class Q_CORE_EXPORT QMetaTypeForType<Name>;
QT_WARNING_PUSH
QT_WARNING_DISABLE_MSVC(4910) // '__declspec(dllexport)' and 'extern' are incompatible on an explicit instantiation
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes") // false positive because of QMetaTypeForType<void>
QT_FOR_EACH_STATIC_PRIMITIVE_TYPE(QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER)
QT_WARNING_POP
QT_FOR_EACH_STATIC_PRIMITIVE_POINTER(QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER)
QT_FOR_EACH_STATIC_CORE_CLASS(QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER)
QT_FOR_EACH_STATIC_CORE_POINTER(QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER)
QT_FOR_EACH_STATIC_CORE_TEMPLATE(QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER)
QT_WARNING_POP
#undef QT_METATYPE_DECLARE_EXTERN_TEMPLATE_ITER
#endif

template<typename T>
constexpr QMetaTypeInterface *qMetaTypeInterfaceForType()
{
    using Ty = std::remove_cv_t<std::remove_reference_t<T>>;
    return &QMetaTypeForType<Ty>::metaType;
}

namespace detail {

template <typename T, typename ODR_VIOLATION_PREVENTER>
struct is_complete_helper {
    template <typename U>
    static auto check(U*)  -> std::integral_constant<bool, sizeof(U) != 0>;
    static auto check(...) -> std::false_type;
    using type = decltype(check(static_cast<T*>(nullptr)));
};

}

template <typename T, typename ODR_VIOLATION_PREVENTER>
struct is_complete : detail::is_complete_helper<T, ODR_VIOLATION_PREVENTER>::type {};

template<typename T>
struct qRemovePointerLike
{
    using type = std::remove_pointer_t<T>;
};

#define Q_REMOVE_POINTER_LIKE_IMPL(Pointer) \
template <typename T> \
struct qRemovePointerLike<Pointer<T>> \
{ \
    using type = T; \
};

QT_FOR_EACH_AUTOMATIC_TEMPLATE_SMART_POINTER(Q_REMOVE_POINTER_LIKE_IMPL)
template<typename T>
using qRemovePointerLike_t = typename qRemovePointerLike<T>::type;
#undef Q_REMOVE_POINTER_LIKE_IMPL

template<typename T, typename ForceComplete_>
struct TypeAndForceComplete
{
    using type = T;
    using ForceComplete = ForceComplete_;
};

template<typename Unique, typename TypeCompletePair>
constexpr QMetaTypeInterface *qTryMetaTypeInterfaceForType()
{
    using T = typename TypeCompletePair::type;
    using ForceComplete = typename TypeCompletePair::ForceComplete;
    using Ty = std::remove_cv_t<std::remove_reference_t<T>>;
    using Tz = qRemovePointerLike_t<Ty>;
    if constexpr (!is_complete<Tz, Unique>::value && !ForceComplete::value) {
        return nullptr;
    } else {
        return &QMetaTypeForType<Ty>::metaType;
    }
}

} // namespace QtPrivate

template<typename T>
QMetaType QMetaType::fromType()
{
    return QMetaType(QtPrivate::qMetaTypeInterfaceForType<T>());
}

template<typename... T>
QtPrivate::QMetaTypeInterface *const qt_metaTypeArray[] = {
    QtPrivate::qMetaTypeInterfaceForType<T>()...
};

template<typename Unique,typename... T>
QtPrivate::QMetaTypeInterface *const qt_incomplete_metaTypeArray[] = {
    QtPrivate::qTryMetaTypeInterfaceForType<Unique, T>()...
};

QT_END_NAMESPACE

#endif // QMETATYPE_H
