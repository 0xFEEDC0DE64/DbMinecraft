#include "closedclient.h"

#include <QTcpSocket>
#include <QTimer>

#include "server.h"

ClosedClient::ClosedClient(std::unique_ptr<QTcpSocket> &&socket, Server &server) :
    QObject{&server}, m_socket{std::move(socket)}, m_server{server}, m_dataStream{m_socket.get()}
{
    connect(m_socket.get(), &QIODevice::readyRead, this, &ClosedClient::readyRead);
    connect(m_socket.get(), &QAbstractSocket::disconnected, this, &QObject::deleteLater);

    QTimer::singleShot(1000, this, &QObject::deleteLater);
}

ClosedClient::~ClosedClient() = default;

void ClosedClient::readyRead()
{
    while(m_socket && m_socket->bytesAvailable())
    {
        if(!m_packetSize)
        {
            m_packetSize = m_dataStream.readVar<qint32>();
            qDebug() << "packet size" << m_packetSize;
        }

        if(m_socket->bytesAvailable() < m_packetSize)
        {
            qWarning() << "packet not fully available" << m_socket->bytesAvailable();
            return;
        }

        qint32 bytesRead;
        const auto type = m_dataStream.readVar<qint32>(bytesRead);
        m_packetSize -= bytesRead;
        const auto buffer = m_socket->read(m_packetSize);
        Q_ASSERT(buffer.size() == m_packetSize);
        m_packetSize = 0;

        readPacket(packets::closed::serverbound::PacketType(type), buffer);
    }
}

void ClosedClient::readPacket(packets::closed::serverbound::PacketType type, const QByteArray &buffer)
{
    qWarning() << "does not support receiving any packets" << type << buffer;
}
