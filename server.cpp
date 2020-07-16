#include "server.h"

#include <QDateTime>

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
    m_playClients.insert(&playClient);
}

void Server::remove(PlayClient &playClient)
{
    m_playClients.erase(&playClient);
}

void Server::timeout()
{
    for (auto client : m_playClients)
    {
        client->keepAlive();
        client->sendChatMessage();
        client->trialDisconnect();
    }
}

void Server::newConnection()
{
    auto * const connection = m_server.nextPendingConnection();
    if (connection)
        new HandshakingClient{*connection, *this};
    //clients.push_back();
}
