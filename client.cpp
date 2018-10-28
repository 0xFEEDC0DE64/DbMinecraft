#include "client.h"

#include <QDebug>
#include <QTcpSocket>
#include <QUuid>
#include <QTimer>

Client::Client(QTcpSocket *socket, QObject *parent) :
    QObject(parent), m_socket(socket), m_dataStream(m_socket),
    m_packetSize(0), m_state(HandshakingState)
{
    m_socket->setParent(this);

    connect(m_socket, &QIODevice::readyRead, this, &Client::readyRead);
    connect(m_socket, &QAbstractSocket::disconnected, this, &Client::disconnected);

    qDebug() << m_socket->peerPort();
}

void Client::readyRead()
{
    while(m_socket->bytesAvailable())
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

        switch(m_state)
        {
        case HandshakingState:
            readPacketHandshaking(packets::handshaking::serverbound::PacketType(type), buffer);
            break;
        case StatusState:
            readPacketStatus(packets::status::serverbound::PacketType(type), buffer);
            break;
        case LoginState:
            readPacketLogin(packets::login::serverbound::PacketType(type), buffer);
            break;
        case PlayState:
            readPacketPlay(packets::play::serverbound::PacketType(type), buffer);
            break;
        default:
            qWarning() << "unhandled state" << m_state << type << buffer;
        }
    }
}

void Client::disconnected()
{
    qDebug() << m_socket->peerPort();
    deleteLater();
}

void Client::readPacketHandshaking(packets::handshaking::serverbound::PacketType type, const QByteArray &buffer)
{
    qDebug() << type;

    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::handshaking;
    case serverbound::PacketHandshake:
    {
        serverbound::Handshake packet(dataStream);
        m_state = packet.nextState;
        break;
    }
    default:
        qWarning() << "unknown type!";
    }
}

void Client::readPacketStatus(const packets::status::serverbound::PacketType type, const QByteArray &buffer)
{
    qDebug() << type;

    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::status;
    case serverbound::PacketRequest:
    {
        {
            serverbound::Request packet(dataStream);
        }
        {
            clientbound::Response packet;
            packet.jsonResponse =
                    "{"
                    "    \"version\": {"
                    "        \"name\": \"1.13.1\","
                    "        \"protocol\": 401"
                    "    },"
                    "    \"players\": {"
                    "        \"max\": 1000,"
                    "        \"online\": 2000,"
                    "        \"sample\": ["
                    "            {"
                    "                \"name\": \"0xFEEDC0DE64\","
                    "                \"id\": \"6ebf7396-b6da-40b1-b7d9-9b7961450d5a\""
                    "            }"
                    "        ]"
                    "    },	"
                    "    \"description\": {"
                    "        \"text\": \"Mein monster server in C++\""
                    "    },"
                    "    \"favicon\": \"\""
                    "}";
            packet.serialize(m_dataStream);
        }
        break;
    }
    case serverbound::PacketPing:
    {
        qint64 payload;
        {
            serverbound::Ping packet(dataStream);
            payload = packet.payload;
        }
        {
            clientbound::Pong packet;
            packet.payload = payload;
            packet.serialize(m_dataStream);
        }
        break;
    }
    default:
        qWarning() << "unknown type!";
    }
}

void Client::readPacketLogin(const packets::login::serverbound::PacketType type, const QByteArray &buffer)
{
    qDebug() << type;

    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::login;
    case serverbound::PacketLogin:
    {
        QString name;
        {
            serverbound::Login packet(dataStream);
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
        m_state = PlayState;
        {
            packets::play::clientbound::JoinGame packet;
            packet.entityid = 1;
            packet.gamemode = 0;
            packet.dimension = 0;
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
        break;
    }
    default:
        qWarning() << "unknown type!";
    }
}

void Client::readPacketPlay(const packets::play::serverbound::PacketType type, const QByteArray &buffer)
{
    qDebug() << type;

    McDataStream dataStream(const_cast<QByteArray *>(&buffer), QIODevice::ReadOnly);

    switch(type)
    {
    using namespace packets::play;
    case serverbound::PacketClientSettings:
    {
        {
            serverbound::ClientSettings packet(dataStream);
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
    case serverbound::PacketPluginMessage:
    {
        serverbound::PluginMessage packet(dataStream);
        qDebug() << "channel" << packet.channel;
        break;
    }
    default:
        qWarning() << "unknown type!";
    }
}
