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
    explicit PlayClient(const QString &name, std::unique_ptr<QTcpSocket> &&socket, Server &server);
    ~PlayClient() override;

    void tick();

signals:
    void distributeChatMessage(const QString &message);

public slots:
    void sendChatMessage(const QString &message);
    void disconnectClient(const QString &reason);

private slots:
    void readyRead();

private:
    void readPacket(packets::play::serverbound::PacketType type, const QByteArray &buffer);

    const QString m_name;
    std::unique_ptr<QTcpSocket> m_socket;
    Server &m_server;

    McDataStream m_dataStream;

    qint32 m_packetSize{};

    const QDateTime m_connectedSince{QDateTime::currentDateTime()};
    QDateTime m_lastKeepAliveSent;
    QDateTime m_lastKeepAliveReceived;
};
