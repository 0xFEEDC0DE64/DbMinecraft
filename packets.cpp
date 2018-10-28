#include "packets.h"

#include "mcdatastream.h"

packets::handshaking::serverbound::Handshake::Handshake(McDataStream &stream)
{
    protocolVersion = stream.readVar<qint32>();
    serverAddress = stream.readVar<QString>();
    stream >> serverPort;
    const auto socketState = stream.readVar<qint32>();
    Q_ASSERT(socketState == 1 || socketState == 2);
    nextState = SocketState(socketState);
}

packets::status::serverbound::Request::Request(McDataStream &stream)
{
    Q_UNUSED(stream);
}

packets::status::serverbound::Ping::Ping(McDataStream &stream)
{
    stream >> payload;
}

void packets::status::clientbound::Response::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketResponse);
    tempStream.writeVar<QString>(jsonResponse);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::status::clientbound::Pong::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketPong);
    tempStream << payload;
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

packets::login::serverbound::Login::Login(McDataStream &stream)
{
    name = stream.readVar<QString>();
}

void packets::login::clientbound::LoginSuccess::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketLoginSuccess);
    tempStream.writeVar<QString>(uuid);
    tempStream.writeVar<QString>(username);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

packets::play::serverbound::ClientSettings::ClientSettings(McDataStream &stream)
{
    locale = stream.readVar<QString>();
    stream >> viewDistance;
    chatMode = stream.readVar<qint32>();
    stream >> chatColors
           >> displayedSkinParts;
    mainHand = stream.readVar<qint32>();
}

packets::play::serverbound::PluginMessage::PluginMessage(McDataStream &stream)
{
    channel = stream.readVar<QString>();
    //TODO read to end of buffer
}

void packets::play::clientbound::ServerDifficulty::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketServerDifficulty);
    tempStream << difficulty;
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::PluginMessage::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketPluginMessage);
    tempStream.writeVar<QString>(channel);
    buffer.append(data);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::JoinGame::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketJoinGame);
    tempStream << entityid
               << gamemode
               << dimension
               << difficulty
               << maxPlayers;
    tempStream.writeVar<QString>(levelType);
    tempStream << reducedDebugInfo;
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::PlayerAbilities::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketPlayerAbilities);
    tempStream << flags;
    tempStream.writeFloat(flyingSpeed);
    tempStream.writeFloat(fieldOfViewModifier);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::PlayerPositionAndLook::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketPlayerPositionAndLook);
    tempStream.writeDouble(x);
    tempStream.writeDouble(y);
    tempStream.writeDouble(z);
    tempStream.writeFloat(yaw);
    tempStream.writeFloat(pitch);
    tempStream << flags;
    tempStream.writeVar<qint32>(teleportId);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::SpawnPosition::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(PacketSpawnPosition);
    tempStream.writePosition(location);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}
