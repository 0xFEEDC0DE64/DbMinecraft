#include "playclient.h"

#include <QTcpSocket>
#include <QDateTime>

#include "server.h"
#include "closedclient.h"

PlayClient::PlayClient(QTcpSocket &socket, Server &server) :
    QObject{&server}, m_socket{socket}, m_server{server}, m_dataStream{&m_socket}
{
    m_server.add(*this);

    m_socket.setParent(this);

    connect(&m_socket, &QIODevice::readyRead, this, &PlayClient::readyRead);
    connect(&m_socket, &QAbstractSocket::disconnected, this, &QObject::deleteLater);

    {
        packets::play::clientbound::JoinGame packet;
        packet.entityid = 1;
        packet.gamemode = packets::play::clientbound::JoinGame::Creative;
        packet.dimension = packets::play::clientbound::JoinGame::Overworld;
        packet.difficulty = 2;
        packet.maxPlayers = 255;
        packet.levelType = QStringLiteral("default");
        packet.reducedDebugInfo = false;
        packet.serialize(m_dataStream);
    }
    {
        packets::play::clientbound::PluginMessage packet;
        packet.channel = QStringLiteral("minecraft:brand");
        packet.data = QByteArrayLiteral("bullshit");
        packet.serialize(m_dataStream);
    }
    {
        packets::play::clientbound::ServerDifficulty packet;
        packet.difficulty = 2;
        packet.serialize(m_dataStream);
    }
    {
        packets::play::clientbound::SpawnPosition packet;
        packet.location = std::make_tuple(100, 64, 100);
        packet.serialize(m_dataStream);
    }
    {
        packets::play::clientbound::PlayerAbilities packet;
        packet.flags = 0x0F;
        packet.flyingSpeed = 1.;
        packet.fieldOfViewModifier = 60.;
        packet.serialize(m_dataStream);
    }
}

PlayClient::~PlayClient()
{
    m_server.remove(*this);
}

void PlayClient::keepAlive()
{
    const auto now = QDateTime::currentDateTime();
    if (!m_lastKeepAliveSent.isValid() || m_lastKeepAliveSent.secsTo(now) >= 1)
    {
        packets::play::clientbound::KeepAlive packet;
        packet.keepAliveId = 0;
        packet.serialize(m_dataStream);
        m_lastKeepAliveSent = now;
    }
}

void PlayClient::sendChatMessage()
{
    const auto now = QDateTime::currentDateTime();
    if (!m_lastChatMessage.isValid() || m_lastChatMessage.secsTo(now) >= 2)
    {
        packets::play::clientbound::ChatMessage packet;
        packet.jsonData = "{"
                              "\"text\": \"Chat message\", "
                              "\"bold\": \"true\" "
                          "}";
        packet.position = packets::play::clientbound::ChatMessage::Chat;
        packet.serialize(m_dataStream);
        m_lastChatMessage = now;
    }
}

void PlayClient::trialDisconnect()
{
    const auto now = QDateTime::currentDateTime();
    if (m_connectedSince.secsTo(now) >= 20)
    {
        packets::play::clientbound::Disconnect packet;
        packet.reason = "{"
                            "\"text\": \"Your trial has ended.\", "
                            "\"bold\": \"true\" "
                        "}";
        packet.serialize(m_dataStream);
        m_socket.flush();
        new ClosedClient{m_socket, m_server};
        deleteLater();
    }
}

void PlayClient::readyRead()
{
    while(m_socket.bytesAvailable())
    {
        if(!m_packetSize)
        {
            m_packetSize = m_dataStream.readVar<qint32>();
            qDebug() << "packet size" << m_packetSize;
        }

        if(m_socket.bytesAvailable() < m_packetSize)
        {
            qWarning() << "packet not fully available" << m_socket.bytesAvailable();
            return;
        }

        qint32 bytesRead;
        const auto type = m_dataStream.readVar<qint32>(bytesRead);
        m_packetSize -= bytesRead;
        const auto buffer = m_socket.read(m_packetSize);
        Q_ASSERT(buffer.size() == m_packetSize);
        m_packetSize = 0;

        readPacket(packets::play::serverbound::PacketType(type), buffer);
    }
}

void PlayClient::readPacket(packets::play::serverbound::PacketType type, const QByteArray &buffer)
{
    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::play;
    case serverbound::PacketType::ClientSettings:
    {
        qDebug() << type;
        {
            serverbound::ClientSettings packet{dataStream};
            qDebug() << "locale" << packet.locale;
            qDebug() << "viewDistance" << packet.viewDistance;
            qDebug() << "chatMode" << packet.chatMode;
            qDebug() << "chatColors" << packet.chatColors;
            qDebug() << "displayedSkinParts" << packet.displayedSkinParts;
            qDebug() << "mainHand" << packet.mainHand;
        }
        {
            clientbound::PlayerPositionAndLook packet;
            packet.x = 50.;
            packet.y = 64.;
            packet.z = 50.;
            packet.yaw = 0.;
            packet.pitch = 0.;
            packet.flags = 0;
            packet.teleportId = 0;
            packet.serialize(m_dataStream);
        }
        break;
    }
    case serverbound::PacketType::InteractEntity:
    {
        qDebug() << type;
        serverbound::InteractEntity packet{dataStream};
        qDebug() << "entityId" << packet.entityId;
        qDebug() << "type" << packet.type;
//        qDebug() << "targetX" << packet.targetX;
//        qDebug() << "targetY" << packet.targetY;
//        qDebug() << "targetZ" << packet.targetZ;
//        qDebug() << "hand" << packet.hand;
        break;
    }
    case serverbound::PacketType::PluginMessage:
    {
        qDebug() << type;
        serverbound::PluginMessage packet{dataStream};
        qDebug() << "channel" << packet.channel;
        break;
    }
    default:
        qWarning() << "unknown packet type" << type;
    }
}
