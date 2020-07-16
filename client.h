#pragma once

#include <QObject>
#include <QDateTime>

#include "mcdatastream.h"
#include "packets.h"

class QTcpSocket;

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QTcpSocket *socket, QObject *parent = nullptr);

    SocketState state() const { return m_state; }

    void keepAlive();
    void sendChatMessage();
    void trialDisconnect();

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

    const QDateTime m_connectedSince;
    QDateTime m_lastKeepAliveSent;
    QDateTime m_lastKeepAliveReceived;
    QDateTime m_lastChatMessage;
};
