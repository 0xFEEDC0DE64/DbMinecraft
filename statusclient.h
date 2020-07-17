#pragma once

#include <QObject>

#include "mcdatastream.h"
#include "packets.h"

class QTcpSocket;
class QByteArray;

class Server;

class StatusClient : public QObject
{
    Q_OBJECT

public:
    explicit StatusClient(std::unique_ptr<QTcpSocket> &&socket, Server &server);
    ~StatusClient() override;

private slots:
    void readyRead();

private:
    void readPacket(packets::status::serverbound::PacketType type, const QByteArray &buffer);

    std::unique_ptr<QTcpSocket> m_socket;
    Server &m_server;

    McDataStream m_dataStream;

    qint32 m_packetSize{};
};
