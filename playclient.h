#pragma once

#include <QObject>

#include "mcdatastream.h"
#include "packets.h"

class QTcpSocket;
class QByteArray;

class Server;

class PlayClient : public QObject
{
    Q_OBJECT

public:
    explicit PlayClient(QTcpSocket &socket, Server &server);
    ~PlayClient() override;

    void keepAlive();
    void sendChatMessage();
    void trialDisconnect();

private slots:
    void readyRead();

private:
    void readPacket(packets::play::serverbound::PacketType type, const QByteArray &buffer);

    QTcpSocket &m_socket;
    Server &m_server;

    McDataStream m_dataStream;

    qint32 m_packetSize{};

    const QDateTime m_connectedSince{QDateTime::currentDateTime()};
    QDateTime m_lastKeepAliveSent;
    QDateTime m_lastKeepAliveReceived;
    QDateTime m_lastChatMessage;
};
