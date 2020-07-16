#include "mcdatastream.h"

#include <QPoint>

#include <memory>

McDataStream::McDataStream() :
    QDataStream()
{
    setByteOrder(QDataStream::BigEndian);
}

McDataStream::McDataStream(QIODevice *device) :
    QDataStream(device)
{
    setByteOrder(QDataStream::BigEndian);
}

McDataStream::McDataStream(QByteArray *byteArray, QIODevice::OpenMode flags) :
    QDataStream(byteArray, flags)
{
    setByteOrder(QDataStream::BigEndian);
}

McDataStream::McDataStream(const QByteArray &byteArray) :
    QDataStream(byteArray)
{
    setByteOrder(QDataStream::BigEndian);
}

McDataStream::Position McDataStream::readPosition()
{
    quint64 val;
    *this >> val;
    const qint32 x = val >> 38;
    const qint16 y = (val >> 26) & 0xFFF;
    const qint32 z = val << 38 >> 38;
    return std::make_tuple(x, y, z);
}

void McDataStream::writePosition(const Position &position)
{
    const quint64 val = ((quint64(std::get<0>(position)) & 0x3FFFFFF) << 38) |
                        ((quint64(std::get<1>(position)) & 0xFFF) << 26) |
                        (quint64(std::get<2>(position)) & 0x3FFFFFF);
    *this << val;
}

float McDataStream::readFloat()
{
    const auto precision = floatingPointPrecision();
    setFloatingPointPrecision(SinglePrecision);
    float value;
    *this >> value;
    setFloatingPointPrecision(precision);
    return value;
}

void McDataStream::writeFloat(float value)
{
    const auto precision = floatingPointPrecision();
    setFloatingPointPrecision(SinglePrecision);
    *this << value;
    setFloatingPointPrecision(precision);
}

float McDataStream::readDouble()
{
    const auto precision = floatingPointPrecision();
    setFloatingPointPrecision(DoublePrecision);
    double value;
    *this >> value;
    setFloatingPointPrecision(precision);
    return value;
}

void McDataStream::writeDouble(double value)
{
    const auto precision = floatingPointPrecision();
    setFloatingPointPrecision(DoublePrecision);
    *this << value;
    setFloatingPointPrecision(precision);
}

QUuid McDataStream::readUuid()
{
    char buf[16];
    readRawData(buf, 16);
    return {};
}

void McDataStream::writeUuid(const QUuid &uuid)
{
    Q_UNUSED(uuid)

    writeRawData("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16);
}

template<>
qint32 McDataStream::readVar<qint32>(qint32 &bytesRead)
{
    bytesRead = 0;
    qint32 result = 0;
    qint8 read;
    do {
        *this >> read;
        qint32 value = read & 0b01111111;
        result |= value << (7 * bytesRead);

        bytesRead++;
        if (bytesRead > 5) {
            qFatal("VarInt is too big");
        }
    } while ((read & 0b10000000) != 0);

    return result;
}

template<>
qint64 McDataStream::readVar<qint64>(qint32 &bytesRead)
{
    bytesRead = 0;
    qint64 result = 0;
    qint8 read;
    do {
        *this >> read;
        qint32 value = read & 0b01111111;
        result |= value << (7 * bytesRead);

        bytesRead++;
        if (bytesRead > 10) {
            qFatal("VarInt is too big");
        }
    } while ((read & 0b10000000) != 0);

    return result;
}

template<>
QString McDataStream::readVar<QString>(qint32 &bytesRead)
{
    const auto length = readVar<qint32>(bytesRead);

    auto data = std::unique_ptr<char[]>(new char[length]);

    {
        const auto rawLength = readRawData(data.get(), length);
        bytesRead += rawLength;
        Q_ASSERT(length == rawLength);
    }

    return QString::fromUtf8(data.get(), length);
}

template<>
void McDataStream::writeVar(qint32 value)
{
    do {
        qint8 temp = value & 0b01111111;
        value >>= 7;
        if (value != 0) {
            temp |= 0b10000000;
        }
        *this << temp;
    } while (value != 0);
}

template<>
void McDataStream::writeVar(qint64 value)
{
    do {
        qint8 temp = value & 0b01111111;
        value >>= 7;
        if (value != 0) {
            temp |= 0b10000000;
        }
        *this << temp;
    } while (value != 0);
}

template<>
void McDataStream::writeVar(QString value)
{
    const auto bytes = value.toUtf8();
    writeVar<qint32>(bytes.length());
    writeRawData(bytes.constData(), bytes.length());
}
