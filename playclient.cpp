#include "playclient.h"

#include <QTcpSocket>
#include <QDateTime>

#include "server.h"
#include "closedclient.h"
#include "chathelper.h"
#include "chunkhelper.h"

PlayClient::PlayClient(const QString &name, std::unique_ptr<QTcpSocket> &&socket, Server &server) :
    QObject{&server}, m_name{name}, m_socket{std::move(socket)}, m_server{server}, m_dataStream{m_socket.get()}
{
    m_server.add(*this);

    emit distributeChatMessage(ChatPart{.extra={ChatPart{.text=m_name,.color="red"},ChatPart{.text=tr(" joined the game!")}}}.toString());

    connect(m_socket.get(), &QIODevice::readyRead, this, &PlayClient::readyRead);
    connect(m_socket.get(), &QAbstractSocket::disconnected, this, &QObject::deleteLater);

    {
        packets::play::clientbound::JoinGame packet;
        packet.entityid = 1;
        packet.gamemode = packets::play::clientbound::JoinGame::Survival;
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
//    {
//        packets::play::clientbound::ChunkData packet;
//        packet.fullChunk = true;
//        packet.primaryBitMask = 0b11000; // send only 2 chunks
//        packet.data += createChunkSection();
//        packet.data += createChunkSection();
//        packet.data += createBiomes();  // we are in Overworld

//        for (int y = -3; y <= 3; y++)
//            for (int x = -3; x <= 3; x++)
//            {
//                packet.chunkX = x;
//                packet.chunkY = y;
//                packet.serialize(m_dataStream);
//            }
//    }
//    {
//        packets::play::clientbound::SpawnMob packet;
//        packet.entityId = 2;
//        packet.uuid = QUuid::createUuid();
//        packet.type = 19;
//        packet.x = 50.;
//        packet.y = 50.;
//        packet.z = 50.;
//        packet.yaw = 0;
//        packet.pitch = 0;
//        packet.headPitch = 0;
//        packet.velocityX = 0;
//        packet.velocityY = 0;
//        packet.velocityZ = 0;
//        packet.serialize(m_dataStream);
//    }
}

PlayClient::~PlayClient()
{
    qDebug() << "called" << m_socket->readAll();

    emit distributeChatMessage(ChatPart{.extra={ChatPart{.text=m_name,.color="red"},ChatPart{.text=tr(" left the game!")}}}.toString());

    m_server.remove(*this);
}

void PlayClient::tick()
{
    const auto now = QDateTime::currentDateTime();
    if (!m_lastKeepAliveSent.isValid() || m_lastKeepAliveSent.secsTo(now) >= 1)
    {
        packets::play::clientbound::KeepAlive packet;
        packet.keepAliveId = 0;
        packet.serialize(m_dataStream);
        m_lastKeepAliveSent = now;
    }

    if (!m_lastStats.isValid() || m_lastStats.msecsTo(now) >= 500)
    {
        {
            packets::play::clientbound::SetExperience packet;
            packet.experienceBar = QRandomGenerator::system()->generateDouble();
            packet.level = QRandomGenerator::system()->bounded(50);
            packet.totalExperience = QRandomGenerator::system()->bounded(500);
            packet.serialize(m_dataStream);
        }
        {
            packets::play::clientbound::UpdateHealth packet;
            packet.health = QRandomGenerator::system()->generateDouble() * 20;
            packet.food = QRandomGenerator::system()->bounded(20);
            packet.foodSaturation = QRandomGenerator::system()->generateDouble() * 5;
            packet.serialize(m_dataStream);
        }

        m_lastStats = now;
    }
}

void PlayClient::sendChatMessage(const QString &message)
{
    packets::play::clientbound::ChatMessage packet;
    packet.jsonData = message;
    packet.position = packets::play::clientbound::ChatMessage::Chat;
    packet.serialize(m_dataStream);
}

void PlayClient::disconnectClient(const QString &reason)
{
    packets::play::clientbound::Disconnect packet;
    packet.reason = reason;
    packet.serialize(m_dataStream);
    m_socket->flush();
    m_dataStream.setDevice({});
    new ClosedClient{std::move(m_socket), m_server};
    deleteLater();
}

void PlayClient::readyRead()
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

        readPacket(packets::play::serverbound::PacketType(type), buffer);
    }
}

void PlayClient::readPacket(packets::play::serverbound::PacketType type, const QByteArray &buffer)
{
    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::play;
    using namespace serverbound;
    case PacketType::ChatMessage:
    {
        qDebug() << type;
        {
            serverbound::ChatMessage packet{dataStream};
            qDebug() << "message" << packet.message;
            if (packet.message.startsWith('/'))
            {
                auto args = packet.message.split(' ', Qt::SkipEmptyParts);
                if (args.first() == "/help")
                {
                    sendChatMessage(ChatPart{.text=tr("Help:")}.toString());
                    sendChatMessage(ChatPart{.text=tr(" * /help")}.toString());
                    sendChatMessage(ChatPart{.text=tr(" * /disconnect")}.toString());
                    sendChatMessage(ChatPart{.text=tr(" * /menu")}.toString());
                }
                else if (args.first() == "/disconnect")
                {
                    disconnectClient(ChatPart{.text="Your trial has ended.",.bold=true}.toString());
                }
                else if (args.first() == "/menu")
                {
                }
                else
                {
                    sendChatMessage(ChatPart{.text=tr("Unknown command %0").arg(args.first()),.color="red"}.toString());
                }
            }
            else
            {
                const auto formatted = ChatPart{.extra={ChatPart{.text=m_name,.color="red"},ChatPart{.text=": " + packet.message}}}.toString();
                sendChatMessage(formatted);
                emit distributeChatMessage(formatted);
            }
        }
        break;
    }
    case PacketType::ClientSettings:
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
        qWarning() << "unknown packet type" << type << buffer;
    }
}
