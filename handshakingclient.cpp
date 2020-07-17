#include "handshakingclient.h"

#include <QTcpSocket>

#include "server.h"
#include "statusclient.h"
#include "loginclient.h"

HandshakingClient::HandshakingClient(std::unique_ptr<QTcpSocket> &&socket, Server &server) :
    QObject{&server}, m_socket{std::move(socket)}, m_server{server}, m_dataStream{m_socket.get()}
{
    m_socket->setParent(this);

    connect(m_socket.get(), &QIODevice::readyRead, this, &HandshakingClient::readyRead);
    connect(m_socket.get(), &QAbstractSocket::disconnected, this, &QObject::deleteLater);
}

HandshakingClient::~HandshakingClient() = default;

void HandshakingClient::readyRead()
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

        readPacket(packets::handshaking::serverbound::PacketType(type), buffer);
    }
}

void HandshakingClient::readPacket(packets::handshaking::serverbound::PacketType type, const QByteArray &buffer)
{
    McDataStream dataStream{const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly};

    switch(type)
    {
    using namespace packets::handshaking;
    case serverbound::PacketType::Handshake:
    {
        qDebug() << type;
        serverbound::Handshake packet{dataStream};
        switch (packet.nextState)
        {
        using namespace serverbound;
        case Handshake::SocketState::StatusState:
            m_dataStream.setDevice({});
            new StatusClient{std::move(m_socket), m_server};
            break;
        case Handshake::SocketState::LoginState:
            m_dataStream.setDevice({});
            new LoginClient{std::move(m_socket), m_server};
            break;
        default:
            qCritical() << "client requested new state" << packet.nextState << "which is not allowed/invalid";
        }

        deleteLater();

        break;
    }
    default:
        qWarning() << "unknown packet type" << type;
    }
}
