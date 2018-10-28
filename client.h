#pragma once

#include <QObject>

#include "mcdatastream.h"
#include "packets.h"

class QTcpSocket;

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QTcpSocket *socket, QObject *parent = nullptr);

private Q_SLOTS:
    void readyRead();
    void disconnected();

private:
    void readPacketHandshaking(const packets::handshaking::serverbound::PacketType type, const QByteArray &buffer);
    void readPacketStatus(const packets::status::serverbound::PacketType type, const QByteArray &buffer);
    void readPacketLogin(const packets::login::serverbound::PacketType type, const QByteArray &buffer);
    void readPacketPlay(const packets::play::serverbound::PacketType type, const QByteArray &buffer);

    QTcpSocket *m_socket;
    McDataStream m_dataStream;

    qint32 m_packetSize;

    SocketState m_state;
};
