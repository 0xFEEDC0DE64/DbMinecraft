#pragma once

#include <QObject>
#include <QTimer>
#include <QTcpServer>
#include <QPointer>

#include <set>

class PlayClient;

class Server : public QObject
{
    Q_OBJECT

public:
    Server(QObject *parent = nullptr);

    void add(PlayClient &playClient);
    void remove(PlayClient &playClient);

private slots:
    void timeout();
    void newConnection();

private:
    QTimer m_timer;
    QTcpServer m_server;

    std::set<PlayClient*> m_playClients;
};
