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

#ifndef QVARIANT_H
#define QVARIANT_H

#include <QtCore/qatomic.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qmap.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qobject.h>
#include <QtCore/qtaggedpointer.h>
#ifndef QT_BOOTSTRAPPED
#include <QtCore/qbytearraylist.h>
#endif
#include <memory>

#if __has_include(<variant>) && __cplusplus >= 201703L
#include <variant>
#elif defined(Q_CLANG_QDOC)
namespace std { template<typename...> struct variant; }
#endif

QT_BEGIN_NAMESPACE


class QBitArray;
class QDataStream;
class QDate;
class QDateTime;
#if QT_CONFIG(easingcurve)
class QEasingCurve;
#endif
class QLine;
class QLineF;
class QLocale;
class QTransform;
class QStringList;
class QTime;
class QPoint;
class QPointF;
class QSize;
class QSizeF;
class QRect;
class QRectF;
#if QT_CONFIG(regularexpression)
class QRegularExpression;
#endif // QT_CONFIG(regularexpression)
class QTextFormat;
class QTextLength;
class QUrl;
class QVariant;
class QVariantComparisonHelper;

template<typename T>
inline T qvariant_cast(const QVariant &);

class Q_CORE_EXPORT QVariant
{
 public:
    enum Type {
        Invalid = QMetaType::UnknownType,
        Bool = QMetaType::Bool,
        Int = QMetaType::Int,
        UInt = QMetaType::UInt,
        LongLong = QMetaType::LongLong,
        ULongLong = QMetaType::ULongLong,
        Double = QMetaType::Double,
        Char = QMetaType::QChar,
        Map = QMetaType::QVariantMap,
        List = QMetaType::QVariantList,
        String = QMetaType::QString,
        StringList = QMetaType::QStringList,
        ByteArray = QMetaType::QByteArray,
        BitArray = QMetaType::QBitArray,
        Date = QMetaType::QDate,
        Time = QMetaType::QTime,
        DateTime = QMetaType::QDateTime,
        Url = QMetaType::QUrl,
        Locale = QMetaType::QLocale,
        Rect = QMetaType::QRect,
        RectF = QMetaType::QRectF,
        Size = QMetaType::QSize,
        SizeF = QMetaType::QSizeF,
        Line = QMetaType::QLine,
        LineF = QMetaType::QLineF,
        Point = QMetaType::QPoint,
        PointF = QMetaType::QPointF,
#if QT_CONFIG(regularexpression)
        RegularExpression = QMetaType::QRegularExpression,
#endif
        Hash = QMetaType::QVariantHash,
#if QT_CONFIG(easingcurve)
        EasingCurve = QMetaType::QEasingCurve,
#endif
        Uuid = QMetaType::QUuid,
#if QT_CONFIG(itemmodel)
        ModelIndex = QMetaType::QModelIndex,
        PersistentModelIndex = QMetaType::QPersistentModelIndex,
#endif
        LastCoreType = QMetaType::LastCoreType,

        Font = QMetaType::QFont,
        Pixmap = QMetaType::QPixmap,
        Brush = QMetaType::QBrush,
        Color = QMetaType::QColor,
        Palette = QMetaType::QPalette,
        Image = QMetaType::QImage,
        Polygon = QMetaType::QPolygon,
        Region = QMetaType::QRegion,
        Bitmap = QMetaType::QBitmap,
        Cursor = QMetaType::QCursor,
#if QT_CONFIG(shortcut)
        KeySequence = QMetaType::QKeySequence,
#endif
        Pen = QMetaType::QPen,
        TextLength = QMetaType::QTextLength,
        TextFormat = QMetaType::QTextFormat,
        Transform = QMetaType::QTransform,
        Matrix4x4 = QMetaType::QMatrix4x4,
        Vector2D = QMetaType::QVector2D,
        Vector3D = QMetaType::QVector3D,
        Vector4D = QMetaType::QVector4D,
        Quaternion = QMetaType::QQuaternion,
        PolygonF = QMetaType::QPolygonF,
        Icon = QMetaType::QIcon,
        LastGuiType = QMetaType::LastGuiType,

        SizePolicy = QMetaType::QSizePolicy,

        UserType = QMetaType::User,
        LastType = 0xffffffff // need this so that gcc >= 3.4 allocates 32 bits for Type
    };

    QVariant() noexcept : d() {}
    ~QVariant();
    explicit QVariant(Type type);
    explicit QVariant(QMetaType type, const void *copy = nullptr);
    QVariant(const QVariant &other);

    QVariant(int i);
    QVariant(uint ui);
    QVariant(qlonglong ll);
    QVariant(qulonglong ull);
    QVariant(bool b);
    QVariant(double d);
    QVariant(float f);
#ifdef __CHERI__
    QVariant(__intcap_t c);
    QVariant(__uintcap_t c);
#endif
#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN QVariant(const char *str)
        : QVariant(QString::fromUtf8(str))
    {}
#endif

    QVariant(const QByteArray &bytearray);
    QVariant(const QBitArray &bitarray);
    QVariant(const QString &string);
    QVariant(QLatin1String string);
    QVariant(const QStringList &stringlist);
    QVariant(QChar qchar);
    QVariant(QDate date);
    QVariant(QTime time);
    QVariant(const QDateTime &datetime);
    QVariant(const QList<QVariant> &list);
    QVariant(const QMap<QString,QVariant> &map);
    QVariant(const QHash<QString,QVariant> &hash);
#ifndef QT_NO_GEOM_VARIANT
    QVariant(const QSize &size);
    QVariant(const QSizeF &size);
    QVariant(const QPoint &pt);
    QVariant(const QPointF &pt);
    QVariant(const QLine &line);
    QVariant(const QLineF &line);
    QVariant(const QRect &rect);
    QVariant(const QRectF &rect);
#endif
    QVariant(const QLocale &locale);
#if QT_CONFIG(regularexpression)
    QVariant(const QRegularExpression &re);
#endif // QT_CONFIG(regularexpression)
#if QT_CONFIG(easingcurve)
    QVariant(const QEasingCurve &easing);
#endif
    QVariant(const QUuid &uuid);
#ifndef QT_BOOTSTRAPPED
    QVariant(const QUrl &url);
    QVariant(const QJsonValue &jsonValue);
    QVariant(const QJsonObject &jsonObject);
    QVariant(const QJsonArray &jsonArray);
    QVariant(const QJsonDocument &jsonDocument);
#endif // QT_BOOTSTRAPPED
#if QT_CONFIG(itemmodel)
    QVariant(const QModelIndex &modelIndex);
    QVariant(const QPersistentModelIndex &modelIndex);
#endif

    QVariant& operator=(const QVariant &other);
    inline QVariant(QVariant &&other) noexcept : d(other.d)
    { other.d = Private(); }
    inline QVariant &operator=(QVariant &&other) noexcept
    { QVariant moved(std::move(other)); swap(moved); return *this; }

    inline void swap(QVariant &other) noexcept { qSwap(d, other.d); }

    Type type() const;
    int userType() const;
    const char *typeName() const;
    QMetaType metaType() const;

    bool canConvert(QMetaType targetType) const
    { return QMetaType::canConvert(d.type(), targetType); }
    bool convert(QMetaType type);

#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_6_0
    bool canConvert(int targetTypeId) const
    { return QMetaType::canConvert(d.type(), QMetaType(targetTypeId)); }
    QT_DEPRECATED_VERSION_6_0
    bool convert(int targetTypeId)
    { return convert(QMetaType(targetTypeId)); }
#endif

    inline bool isValid() const;
    bool isNull() const;

    void clear();

    void detach();
    inline bool isDetached() const;

    int toInt(bool *ok = nullptr) const;
    uint toUInt(bool *ok = nullptr) const;
    qlonglong toLongLong(bool *ok = nullptr) const;
    qulonglong toULongLong(bool *ok = nullptr) const;
    bool toBool() const;
    double toDouble(bool *ok = nullptr) const;
    float toFloat(bool *ok = nullptr) const;
    qreal toReal(bool *ok = nullptr) const;
    QByteArray toByteArray() const;
    QBitArray toBitArray() const;
    QString toString() const;
    QStringList toStringList() const;
    QChar toChar() const;
    QDate toDate() const;
    QTime toTime() const;
    QDateTime toDateTime() const;
    QList<QVariant> toList() const;
    QMap<QString, QVariant> toMap() const;
    QHash<QString, QVariant> toHash() const;

#ifndef QT_NO_GEOM_VARIANT
    QPoint toPoint() const;
    QPointF toPointF() const;
    QRect toRect() const;
    QSize toSize() const;
    QSizeF toSizeF() const;
    QLine toLine() const;
    QLineF toLineF() const;
    QRectF toRectF() const;
#endif
    QLocale toLocale() const;
#if QT_CONFIG(regularexpression)
    QRegularExpression toRegularExpression() const;
#endif // QT_CONFIG(regularexpression)
#if QT_CONFIG(easingcurve)
    QEasingCurve toEasingCurve() const;
#endif
    QUuid toUuid() const;
#ifndef QT_BOOTSTRAPPED
    QUrl toUrl() const;
    QJsonValue toJsonValue() const;
    QJsonObject toJsonObject() const;
    QJsonArray toJsonArray() const;
    QJsonDocument toJsonDocument() const;
#endif // QT_BOOTSTRAPPED
#if QT_CONFIG(itemmodel)
    QModelIndex toModelIndex() const;
    QPersistentModelIndex toPersistentModelIndex() const;
#endif

#ifndef QT_NO_DATASTREAM
    void load(QDataStream &ds);
    void save(QDataStream &ds) const;
#endif
#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_6_0
    static const char *typeToName(int typeId)
    { return QMetaType(typeId).name(); }
    QT_DEPRECATED_VERSION_6_0
    static Type nameToType(const char *name)
    {
        int metaType = QMetaType::fromName(name).id();
        return metaType <= int(UserType) ? QVariant::Type(metaType) : UserType;
    }
#endif

    void *data();
    const void *constData() const
    { return d.storage(); }
    inline const void *data() const { return constData(); }

    template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, QVariant>>>
    void setValue(T &&avalue)
    {
        using VT = std::decay_t<T>;
        QMetaType metaType = QMetaType::fromType<VT>();
        // If possible we reuse the current QVariant private.
        if (isDetached() && d.type() == metaType) {
            *reinterpret_cast<VT *>(const_cast<void *>(constData())) = std::forward<T>(avalue);
        } else {
            *this = QVariant::fromValue<VT>(std::forward<T>(avalue));
        }
    }

    void setValue(const QVariant &avalue)
    {
        *this = avalue;
    }

    void setValue(QVariant &&avalue)
    {
        *this = std::move(avalue);
    }

    template<typename T>
    inline T value() const
    { return qvariant_cast<T>(*this); }

    template<typename T>
    static inline QVariant fromValue(const T &value)
    {
        return QVariant(QMetaType::fromType<T>(), std::addressof(value));
    }

#if (__has_include(<variant>) && __cplusplus >= 201703L) || defined(Q_CLANG_QDOC)
    template<typename... Types>
    static inline QVariant fromStdVariant(const std::variant<Types...> &value)
    {
        if (value.valueless_by_exception())
            return QVariant();
        return std::visit([](const auto &arg) { return fromValue(arg); }, value);
    }
#endif

    template<typename T>
    bool canConvert() const
    { return canConvert(QMetaType::fromType<T>()); }

 public:
    struct PrivateShared
    {
    private:
        inline PrivateShared() : ref(1) { }
    public:
        static PrivateShared *create(QMetaType type)
        {
            size_t size = type.sizeOf();
            size_t align = type.alignOf();

            size += sizeof(PrivateShared);
            if (align > sizeof(PrivateShared)) {
                // The alignment is larger than the alignment we can guarantee for the pointer
                // directly following PrivateShared, so we need to allocate some additional
                // memory to be able to fit the object into the available memory with suitable
                // alignment.
                size += align - sizeof(PrivateShared);
            }
            void *data = operator new(size);
            auto *ps = new (data) QVariant::PrivateShared();
            ps->offset = int(((quintptr(ps) + sizeof(PrivateShared) + align - 1) & ~(align - 1)) - quintptr(ps));
            return ps;
        }
        static void free(PrivateShared *p)
        {
            p->~PrivateShared();
            operator delete(p);
        }

        alignas(8) QAtomicInt ref;
        int offset;

        const void *data() const
        { return reinterpret_cast<const unsigned char *>(this) + offset; }
        void *data()
        { return reinterpret_cast<unsigned char *>(this) + offset; }
    };
    struct Private
    {
        enum class Flags { IsNull = 1, IsShared = 2 };
        static constexpr size_t MaxInternalSize = 3*sizeof(void *);
        template<typename T>
        static constexpr bool CanUseInternalSpace = (sizeof(T) <= MaxInternalSize && alignof(T) <= alignof(double));
        static constexpr bool canUseInternalSpace(size_t s, size_t align)
        { return s <= MaxInternalSize && align <= alignof(double); }

        union
        {
            uchar data[MaxInternalSize] = {};
            PrivateShared *shared;
            double _forAlignment; // we want an 8byte alignment on 32bit systems as well
#ifdef __CHERI__
            __intcap_t _forAlignment2; // we need this to be capability-aligned for hybrid
#endif
        } data;
        QTaggedPointer<QtPrivate::QMetaTypeInterface> packedType;

        Private() noexcept : packedType(nullptr, uint8_t(Flags::IsNull)) {}
        explicit Private(QMetaType type) noexcept
        {
            qptraddr mt = qptraddr(type.d_ptr);
            Q_ASSERT((mt & 0x3) == 0);
            packedType = type.d_ptr;
        }
        explicit Private(int type) noexcept : Private(QMetaType(type)) {}

        const void *storage() const { return is_shared() ? data.shared->data() : &data.data; }

        const void *internalStorage() const
        {
            Q_ASSERT(is_shared());
            return &data.data;
        }

        // determine internal storage at compile time
        template<typename T>
        const T &get() const
        { return *static_cast<const T *>(storage()); }
        template<typename T>
        void set(const T &t)
        { *static_cast<T *>(CanUseInternalSpace<T> ? &data.data : data.shared->data()) = t; }
        bool is_shared() const { return packedType.tag() & uint8_t(Flags::IsShared); }
        bool is_null() const { return packedType.tag() & uint8_t(Flags::IsNull); }
        void set_shared(bool value)
        {
            packedType.setTag((packedType.tag() & ~1) | (value ? uint8_t(Flags::IsShared) : 0));
        }
        void set_null(bool value)
        {
            packedType.setTag((packedType.tag() & ~2) | (value ? uint8_t(Flags::IsNull) : 0));
        }
        inline QMetaType type() const { return QMetaType(packedType.data()); }
        inline int typeId() const
        {
            return type().id();
        }
    };
 public:
    inline bool operator==(const QVariant &v) const
    { return equals(v); }
    inline bool operator!=(const QVariant &v) const
    { return !equals(v); }

protected:
    friend inline bool operator==(const QVariant &, const QVariantComparisonHelper &);
#ifndef QT_NO_DEBUG_STREAM
    friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);
#endif
    template<typename T>
    friend inline T qvariant_cast(const QVariant &);
protected:
    Private d;
    void create(int type, const void *copy);
    bool equals(const QVariant &other) const;
    bool convert(const int t, void *ptr) const; // ### Qt6: drop const

private:
    // force compile error, prevent QVariant(bool) to be called
    inline QVariant(void *) = delete;
    // QVariant::Type is marked as \obsolete, but we don't want to
    // provide a constructor from its intended replacement,
    // QMetaType::Type, instead, because the idea behind these
    // constructors is flawed in the first place. But we also don't
    // want QVariant(QMetaType::String) to compile and falsely be an
    // int variant, so delete this constructor:
    QVariant(QMetaType::Type) = delete;

    // These constructors don't create QVariants of the type associcated
    // with the enum, as expected, but they would create a QVariant of
    // type int with the value of the enum value.
    // Use QVariant v = QColor(Qt::red) instead of QVariant v = Qt::red for
    // example.
    QVariant(Qt::GlobalColor) = delete;
    QVariant(Qt::BrushStyle) = delete;
    QVariant(Qt::PenStyle) = delete;
    QVariant(Qt::CursorShape) = delete;
#ifdef QT_NO_CAST_FROM_ASCII
    // force compile error when implicit conversion is not wanted
    inline QVariant(const char *) = delete;
#endif
public:
    typedef Private DataPtr;
    inline DataPtr &data_ptr() { return d; }
    inline const DataPtr &data_ptr() const { return d; }
};

template<>
inline QVariant QVariant::fromValue(const QVariant &value)
{
    return value;
}

#if __has_include(<variant>) && __cplusplus >= 201703L
template<>
inline QVariant QVariant::fromValue(const std::monostate &)
{
    return QVariant();
}
#endif

inline bool QVariant::isValid() const
{
    return d.type().isValid();
}

#ifndef QT_NO_DATASTREAM
Q_CORE_EXPORT QDataStream& operator>> (QDataStream& s, QVariant& p);
Q_CORE_EXPORT QDataStream& operator<< (QDataStream& s, const QVariant& p);
Q_CORE_EXPORT QDataStream& operator>> (QDataStream& s, QVariant::Type& p);
Q_CORE_EXPORT QDataStream& operator<< (QDataStream& s, const QVariant::Type p);
#endif

inline bool QVariant::isDetached() const
{
    return !d.is_shared() || d.data.shared->ref.loadRelaxed() == 1;
}

#ifdef Q_QDOC
    inline bool operator==(const QVariant &v1, const QVariant &v2);
    inline bool operator!=(const QVariant &v1, const QVariant &v2);
#else

/* Helper class to add one more level of indirection to prevent
   implicit casts.
*/
class QVariantComparisonHelper
{
public:
    inline QVariantComparisonHelper(const QVariant &var)
        : v(&var) {}
private:
    friend inline bool operator==(const QVariant &, const QVariantComparisonHelper &);
    const QVariant *v;
};

inline bool operator==(const QVariant &v1, const QVariantComparisonHelper &v2)
{
    return v1.equals(*v2.v);
}

inline bool operator!=(const QVariant &v1, const QVariantComparisonHelper &v2)
{
    return !operator==(v1, v2);
}
#endif
Q_DECLARE_SHARED(QVariant)

class Q_CORE_EXPORT QSequentialIterable
{
    QtMetaTypePrivate::QSequentialIterableImpl m_impl;
public:
    struct Q_CORE_EXPORT const_iterator
    {
    private:
        QtMetaTypePrivate::QSequentialIterableImpl m_impl;
        QAtomicInt *m_ref;
        friend class QSequentialIterable;
        explicit const_iterator(const QSequentialIterable &iter, QAtomicInt *ref);

        explicit const_iterator(const QtMetaTypePrivate::QSequentialIterableImpl &impl, QAtomicInt *ref);

    public:
        ~const_iterator();

        const_iterator(const const_iterator &other);

        const_iterator& operator=(const const_iterator &other);

        const QVariant operator*() const;
        bool operator==(const const_iterator &o) const;
        bool operator!=(const const_iterator &o) const;
        const_iterator &operator++();
        const_iterator operator++(int);
        const_iterator &operator--();
        const_iterator operator--(int);
        const_iterator &operator+=(int j);
        const_iterator &operator-=(int j);
        const_iterator operator+(int j) const;
        const_iterator operator-(int j) const;
        friend inline const_iterator operator+(int j, const const_iterator &k) { return k + j; }
    };

    friend struct const_iterator;

    explicit QSequentialIterable(const QtMetaTypePrivate::QSequentialIterableImpl &impl);
    QSequentialIterable() {}

    const_iterator begin() const;
    const_iterator end() const;

    QVariant at(int idx) const;
    int size() const;

    bool canReverseIterate() const;
};

class Q_CORE_EXPORT QAssociativeIterable
{
    QtMetaTypePrivate::QAssociativeIterableImpl m_impl;
public:
    struct Q_CORE_EXPORT const_iterator
    {
    private:
        QtMetaTypePrivate::QAssociativeIterableImpl m_impl;
        QAtomicInt *ref;
        friend class QAssociativeIterable;
        explicit const_iterator(const QAssociativeIterable &iter, QAtomicInt *ref_);

        explicit const_iterator(const QtMetaTypePrivate::QAssociativeIterableImpl &impl, QAtomicInt *ref_);

        void begin();
        void end();
        void find(const QVariant &key);
    public:
        ~const_iterator();
        const_iterator(const const_iterator &other);

        const_iterator& operator=(const const_iterator &other);

        const QVariant key() const;

        const QVariant value() const;

        const QVariant operator*() const;
        bool operator==(const const_iterator &o) const;
        bool operator!=(const const_iterator &o) const;
        const_iterator &operator++();
        const_iterator operator++(int);
        const_iterator &operator--();
        const_iterator operator--(int);
        const_iterator &operator+=(int j);
        const_iterator &operator-=(int j);
        const_iterator operator+(int j) const;
        const_iterator operator-(int j) const;
        friend inline const_iterator operator+(int j, const const_iterator &k) { return k + j; }
    };

    friend struct const_iterator;

    explicit QAssociativeIterable(const QtMetaTypePrivate::QAssociativeIterableImpl &impl);
    QAssociativeIterable() {}

    const_iterator begin() const;
    const_iterator end() const;
    const_iterator find(const QVariant &key) const;

    QVariant value(const QVariant &key) const;

    int size() const;
};

#ifndef QT_MOC

template<typename T> inline T qvariant_cast(const QVariant &v)
{
    QMetaType targetType = QMetaType::fromType<T>();
    if (v.d.type() == targetType)
        return v.d.get<T>();

    T t{};
    QMetaType::convert(v.metaType(), v.constData(), targetType, &t);
    return t;
}

template<> inline QVariant qvariant_cast<QVariant>(const QVariant &v)
{
    if (v.userType() == QMetaType::QVariant)
        return *reinterpret_cast<const QVariant *>(v.constData());
    return v;
}

#endif

#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant::Type);
#endif

QT_END_NAMESPACE

#endif // QVARIANT_H
