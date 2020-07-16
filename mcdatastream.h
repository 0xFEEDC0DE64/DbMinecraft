#pragma once

#include <QDataStream>
#include <QUuid>

#include <tuple>

class McDataStream : public QDataStream
{
public:
    McDataStream();
    explicit McDataStream(QIODevice *device);
    McDataStream(QByteArray *byteArray, QIODevice::OpenMode flags);
    McDataStream(const QByteArray &byteArray);

    typedef std::tuple<qint32, qint16, qint32> Position;
    Position readPosition();
    void writePosition(const Position &position);

    float readFloat();
    void writeFloat(float value);

    float readDouble();
    void writeDouble(double value);

    template<typename T>
    T readVar();

    template<typename T>
    T readVar(qint32 &bytesRead);

    template<typename T>
    void writeVar(T value);

    QUuid readUuid();
    void writeUuid(const QUuid &uuid);
};

template<typename T>
T McDataStream::readVar()
{
    qint32 bytesRead;
    return readVar<T>(bytesRead);
}

template<>
qint32 McDataStream::readVar<qint32>(qint32 &bytesRead);

template<>
qint64 McDataStream::readVar<qint64>(qint32 &bytesRead);

template<>
QString McDataStream::readVar<QString>(qint32 &bytesRead);

template<>
void McDataStream::writeVar(qint32 value);

template<>
void McDataStream::writeVar(qint64 value);

template<>
void McDataStream::writeVar(QString value);
