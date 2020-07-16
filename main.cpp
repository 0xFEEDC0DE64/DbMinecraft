#include <QCoreApplication>
#include <qlogging.h>
#include <QTcpServer>
#include <QList>
#include <QPointer>
#include <QTimer>

#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qSetMessagePattern(QStringLiteral("%{time dd.MM.yyyy HH:mm:ss.zzz} "
                                      "["
                                      "%{if-debug}D%{endif}"
                                      "%{if-info}I%{endif}"
                                      "%{if-warning}W%{endif}"
                                      "%{if-critical}C%{endif}"
                                      "%{if-fatal}F%{endif}"
                                      "] "
                                      "%{function}(): "
                                      "%{message}"));

    QList<QPointer<Client>> clients;

    QTimer timer;
    timer.setInterval(100);
    QObject::connect(&timer, &QTimer::timeout, [&clients](){
        const auto now = QDateTime::currentDateTime();
        for (auto iter = std::begin(clients); iter != std::end(clients); )
        {
            if ((*iter).isNull())
            {
                iter = clients.erase(iter);
                continue;
            }

            auto &client = **iter;
            if (client.state() == PlayState)
            {
                client.keepAlive();
                client.sendChatMessage();
                client.trialDisconnect();
            }

            iter++;
        }
    });
    timer.start();

    QTcpServer server;

    QObject::connect(&server, &QTcpServer::newConnection, [&server,&clients](){
        clients.append(new Client(server.nextPendingConnection()));
    });

    if(!server.listen(QHostAddress::Any, 25565))
        qFatal("could not start listening %s", server.errorString().toUtf8().constData());

    qDebug() << "started listening";

    return a.exec();
}
