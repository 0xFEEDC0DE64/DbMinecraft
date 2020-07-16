#pragma once

#include <QObject>

#include "mcdatastream.h"
#include "packets.h"

class QTcpSocket;
class QByteArray;

class Server;

class HandshakingClient : public QObject
{
    Q_OBJECT

public:
    explicit HandshakingClient(QTcpSocket &socket, Server &parent);

private slots:
    void readyRead();

private:
    void readPacket(packets::handshaking::serverbound::PacketType type, const QByteArray &buffer);

    QTcpSocket &m_socket;
    Server &m_server;

    McDataStream m_dataStream;

    qint32 m_packetSize{};
};
