#include <QCoreApplication>
#include <qlogging.h>
#include <QTcpServer>

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

    QTcpServer server;

    QObject::connect(&server, &QTcpServer::newConnection, [&server](){
        new Client(server.nextPendingConnection());
    });

    if(!server.listen(QHostAddress::Any, 25565))
        qFatal("could not start listening %s", server.errorString().toUtf8().constData());

    qDebug() << "started listening";

    return a.exec();
}
