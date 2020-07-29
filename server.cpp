#include "server.h"

#include <memory>

#include <QDateTime>
#include <QTcpSocket>

#include "playclient.h"
#include "handshakingclient.h"

Server::Server(QObject *parent) :
    QObject{parent}
{
    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &Server::timeout);
    m_timer.start();

    connect(&m_server, &QTcpServer::newConnection, this, &Server::newConnection);

    if(!m_server.listen(QHostAddress::Any, 25565))
        qFatal("could not start listening %s", m_server.errorString().toUtf8().constData());
}

void Server::add(PlayClient &playClient)
{
    for (auto otherClient : m_playClients)
    {
        connect(&playClient, &PlayClient::distributeChatMessage, otherClient, &PlayClient::sendChatMessage);
        connect(otherClient, &PlayClient::distributeChatMessage, &playClient, &PlayClient::sendChatMessage);
    }

    m_playClients.insert(&playClient);
}

void Server::remove(PlayClient &playClient)
{
    m_playClients.erase(&playClient);
}

void Server::timeout()
{
    for (auto client : m_playClients)
        client->tick();
}

void Server::newConnection()
{
    auto connection = std::unique_ptr<QTcpSocket>(m_server.nextPendingConnection());
    if (connection)
        new HandshakingClient{std::move(connection), *this};
}
