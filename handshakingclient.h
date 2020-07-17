#pragma once

#include <memory>

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
    explicit HandshakingClient(std::unique_ptr<QTcpSocket> &&socket, Server &parent);
    ~HandshakingClient() override;

private slots:
    void readyRead();

private:
    void readPacket(packets::handshaking::serverbound::PacketType type, const QByteArray &buffer);

    std::unique_ptr<QTcpSocket> m_socket;
    Server &m_server;

    McDataStream m_dataStream;

    qint32 m_packetSize{};
};
