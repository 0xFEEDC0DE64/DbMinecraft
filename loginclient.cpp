#include "loginclient.h"

#include <QTcpSocket>

#include "server.h"
#include "playclient.h"

LoginClient::LoginClient(std::unique_ptr<QTcpSocket> &&socket, Server &server) :
    QObject{&server}, m_socket{std::move(socket)}, m_server{server}, m_dataStream{m_socket.get()}
{
    m_socket->setParent(this);

    connect(m_socket.get(), &QIODevice::readyRead, this, &LoginClient::readyRead);
    connect(m_socket.get(), &QAbstractSocket::disconnected, this, &QObject::deleteLater);
}

LoginClient::~LoginClient() = default;

void LoginClient::readyRead()
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

        readPacket(packets::login::serverbound::PacketType(type), buffer);
    }
}

void LoginClient::readPacket(packets::login::serverbound::PacketType type, const QByteArray &buffer)
{
    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::login;
    case serverbound::PacketType::Login:
    {
        qDebug() << type;
        QString name;
        {
            serverbound::Login packet{dataStream};
            name = packet.name;
        }
        qDebug() << "Name" << name;
        {
            clientbound::LoginSuccess packet;
            const auto uuid = QUuid::createUuid().toString();
            packet.uuid = uuid.mid(1, uuid.length() - 2);
            packet.username = name;
            packet.serialize(m_dataStream);
        }
        m_dataStream.setDevice({});
        new PlayClient{std::move(m_socket), m_server};
        deleteLater();
        break;
    }
    default:
        qWarning() << "unknown packet type" << type;
    }
}
