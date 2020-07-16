#include "client.h"

#include <QDebug>
#include <QTcpSocket>
#include <QUuid>
#include <QTimer>

Client::Client(QTcpSocket *socket, QObject *parent) :
    QObject(parent), m_socket(socket), m_dataStream(m_socket),
    m_packetSize(0), m_state(HandshakingState), m_connectedSince(QDateTime::currentDateTime())
{
    m_socket->setParent(this);

    connect(m_socket, &QIODevice::readyRead, this, &Client::readyRead);
    connect(m_socket, &QAbstractSocket::disconnected, this, &Client::disconnected);

    qDebug() << m_socket->peerPort();
}

void Client::keepAlive()
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

void Client::sendChatMessage()
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

void Client::trialDisconnect()
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
        m_socket->flush();
        deleteLater();
    }
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
    case serverbound::PacketType::Handshake:
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
    case serverbound::PacketType::Request:
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
                    "                \"name\": \"feedc0de\","
                    "                \"id\": \"6ebf7396-b6da-40b1-b7d9-9b7961450d5a\""
                    "            }"
                    "        ]"
                    "    },	"
                    "    \"description\": {"
                    "        \"text\": \"Minecraft server implemented in C++\""
                    "    },"
                    "    \"favicon\": \"data:image/png;base64,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7O8HmHhytkwrkh0AABARaVRYdFhNTDpjb20uYWRvYmUueG1wAAAAAAA8P3hwYWNrZXQgYmVnaW49Iu+7vyIgaWQ9Ilc1TTBNcENlaGlIenJlU3pOVGN6a2M5ZCI/Pgo8eDp4bXBtZXRhIHhtbG5zOng9ImFkb2JlOm5zOm1ldGEvIiB4OnhtcHRrPSJYTVAgQ29yZSA0LjQuMC1FeGl2MiI+CiA8cmRmOlJERiB4bWxuczpyZGY9Imh0dHA6Ly93d3cudzMub3JnLzE5OTkvMDIvMjItcmRmLXN5bnRheC1ucyMiPgogIDxyZGY6RGVzY3JpcHRpb24gcmRmOmFib3V0PSIiCiAgICB4bWxuczppcHRjRXh0PSJodHRwOi8vaXB0Yy5vcmcvc3RkL0lwdGM0eG1wRXh0LzIwMDgtMDItMjkvIgogICAgeG1sbnM6eG1wTU09Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9tbS8iCiAgICB4bWxuczpzdEV2dD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL3NUeXBlL1Jlc291cmNlRXZlbnQjIgogICAgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiCiAgICB4bWxuczpwbHVzPSJodHRwOi8vbnMudXNlcGx1cy5vcmcvbGRmL3htcC8xLjAvIgogICAgeG1sbnM6R0lNUD0iaHR0cDovL3d3dy5naW1wLm9yZy94bXAvIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIgogICB4bXBNTTpEb2N1bWVudElEPSJ4bXAuZGlkOjE2MDVCNzFGMTczNjExRTc4RUNBREEwQzRGMjBGNThEIgogICB4bXBNTTpJbnN0YW5jZUlEPSJ4bXAuaWlkOmZjMzcwZTE3LTIyM2YtNGJjMC05OGJiLWJhZTZlZGMwZmMzNSIKICAgeG1wTU06T3JpZ2luYWxEb2N1bWVudElEPSJ4bXAuZGlkOjZhYzI4NDk4LWY1NjktNDMxMS1hNjM1LTEzMmIwMTA4MWI2OSIKICAgR0lNUDpBUEk9IjIuMCIKICAgR0lNUDpQbGF0Zm9ybT0iTGludXgiCiAgIEdJTVA6VGltZVN0YW1wPSIxNTk0OTI4OTUxMjg1MDcwIgogICBHSU1QOlZlcnNpb249IjIuMTAuMjAiCiAgIGRjOkZvcm1hdD0iaW1hZ2UvcG5nIgogICB4bXA6Q3JlYXRvclRvb2w9IkdJTVAgMi4xMCI+CiAgIDxpcHRjRXh0OkxvY2F0aW9uQ3JlYXRlZD4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OkxvY2F0aW9uQ3JlYXRlZD4KICAgPGlwdGNFeHQ6TG9jYXRpb25TaG93bj4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OkxvY2F0aW9uU2hvd24+CiAgIDxpcHRjRXh0OkFydHdvcmtPck9iamVjdD4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OkFydHdvcmtPck9iamVjdD4KICAgPGlwdGNFeHQ6UmVnaXN0cnlJZD4KICAgIDxyZGY6QmFnLz4KICAgPC9pcHRjRXh0OlJlZ2lzdHJ5SWQ+CiAgIDx4bXBNTTpIaXN0b3J5PgogICAgPHJkZjpTZXE+CiAgICAgPHJkZjpsaQogICAgICBzdEV2dDphY3Rpb249InNhdmVkIgogICAgICBzdEV2dDpjaGFuZ2VkPSIvIgogICAgICBzdEV2dDppbnN0YW5jZUlEPSJ4bXAuaWlkOjgxZTViZmE4LTg3MGYtNDg3Ny04NjRjLTkyMGY1YzkyNTdlYiIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iR2ltcCAyLjEwIChMaW51eCkiCiAgICAgIHN0RXZ0OndoZW49IiswMjowMCIvPgogICAgPC9yZGY6U2VxPgogICA8L3htcE1NOkhpc3Rvcnk+CiAgIDx4bXBNTTpEZXJpdmVkRnJvbQogICAgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDoxNjA1QjcxRDE3MzYxMUU3OEVDQURBMEM0RjIwRjU4RCIKICAgIHN0UmVmOmluc3RhbmNlSUQ9InhtcC5paWQ6MTYwNUI3MUMxNzM2MTFFNzhFQ0FEQTBDNEYyMEY1OEQiLz4KICAgPHBsdXM6SW1hZ2VTdXBwbGllcj4KICAgIDxyZGY6U2VxLz4KICAgPC9wbHVzOkltYWdlU3VwcGxpZXI+CiAgIDxwbHVzOkltYWdlQ3JlYXRvcj4KICAgIDxyZGY6U2VxLz4KICAgPC9wbHVzOkltYWdlQ3JlYXRvcj4KICAgPHBsdXM6Q29weXJpZ2h0T3duZXI+CiAgICA8cmRmOlNlcS8+CiAgIDwvcGx1czpDb3B5cmlnaHRPd25lcj4KICAgPHBsdXM6TGljZW5zb3I+CiAgICA8cmRmOlNlcS8+CiAgIDwvcGx1czpMaWNlbnNvcj4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+CiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgCiAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAKICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgICAgIAogICAgICAgICAgICAgICAgICAgICAgICAgICAKPD94cGFja2V0IGVuZD0idyI/Pl4IosIAAAAGYktHRAAAAAAAAPlDu38AAAAJcEhZcwAACxMAAAsTAQCanBgAAAAHdElNRQfkBxATMQvs17uoAAAKgUlEQVR42t2beXRU5RXAf2+WTJLJHkIgLBIQNKJjWVyLGDRW3BeoVdsePZ7pFPRwsK21ttZYl1K1aqlLj2eY08V6FKJF64KAg6JIAjREGCCQQEKWIckkk2SWTGYmM++9/jFpanCAN5PJ5nfO+2fOm+999/e+e79777tXYISH0WzLB34G3AzM7P+5Hngf+JPFZHCM5HqEERR8AvAL4AEg/RS3eYFXgRcsJoPzWwHAaLZlAQ/2X5kK/+YG1gJrLSaDa1wCMJpt6cAq4OdAbpzTdAIvAi9bTAbvuABgNNv0wArgYWBigqZtB54DXrOYDL4xCcBotiUDRuDXQMEwbawW4A+AxWIyBMYEAKPZlgTcC/wGOGuEbGojsAb4u8Vk6BsVAEazTQvcDTwGzGJ0Rh3w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\""
                    "}";
            packet.serialize(m_dataStream);
        }
        break;
    }
    case serverbound::PacketType::Ping:
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
    case serverbound::PacketType::Login:
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
    case serverbound::PacketType::ClientSettings:
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
    case serverbound::PacketType::InteractEntity:
    {
        serverbound::InteractEntity packet(dataStream);
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
        serverbound::PluginMessage packet(dataStream);
        qDebug() << "channel" << packet.channel;
        break;
    }
    default:
        qWarning() << "unknown type!";
    }
}
