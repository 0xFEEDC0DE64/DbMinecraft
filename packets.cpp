#include "packets.h"

#include <QtGlobal>

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
    tempStream.writeVar<qint32>(qint32(PacketType::Response));
    tempStream.writeVar<QString>(jsonResponse);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::status::clientbound::Pong::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::Pong));
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
    tempStream.writeVar<qint32>(qint32(PacketType::LoginSuccess));
    //tempStream.writeUuid(uuid);
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

packets::play::serverbound::InteractEntity::InteractEntity(McDataStream &stream)
{
    entityId = stream.readVar<qint32>();
    type = Type(stream.readVar<qint32>());
    switch (type)
    {
    case InteractAt:
        targetX = stream.readFloat();
        targetY = stream.readFloat();
        targetZ = stream.readFloat();
        [[fallthrough]];
    case Interact:
        hand = Hand(stream.readVar<qint32>());
    }
}

packets::play::serverbound::PluginMessage::PluginMessage(McDataStream &stream)
{
    channel = stream.readVar<QString>();
    //TODO read to end of buffer
}

void packets::play::clientbound::SpawnMob::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::SpawnMob));
    tempStream.writeVar<qint32>(entityId);
    tempStream.writeUuid(uuid);
    tempStream.writeVar<qint32>(type);
    tempStream.writeDouble(x);
    tempStream.writeDouble(y);
    tempStream.writeDouble(z);
    tempStream << yaw << pitch << headPitch << velocityX << velocityY << velocityZ;
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::ServerDifficulty::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::ServerDifficulty));
    tempStream << difficulty;
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::ChatMessage::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::ChatMessage));
    tempStream.writeVar<QString>(jsonData);
    tempStream << qint8(position);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::PluginMessage::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::PluginMessage));
    tempStream.writeVar<QString>(channel);
    buffer.append(data);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::Disconnect::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::Disconnect));
    tempStream.writeVar<QString>(reason);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::JoinGame::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::JoinGame));
    tempStream << entityid
               << quint8(gamemode)
               << qint32(dimension)
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
    tempStream.writeVar<qint32>(qint32(PacketType::PlayerAbilities));
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
    tempStream.writeVar<qint32>(qint32(PacketType::PlayerPositionAndLook));
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

void packets::play::clientbound::SetExperience::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::SetExperience));
    tempStream.writeFloat(experienceBar);
    tempStream.writeVar<qint32>(level);
    tempStream.writeVar<qint32>(totalExperience);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::UpdateHealth::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::UpdateHealth));
    tempStream.writeFloat(health);
    tempStream.writeVar<qint32>(food);
    tempStream.writeFloat(foodSaturation);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::SpawnPosition::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::SpawnPosition));
    tempStream.writePosition(location);
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::KeepAlive::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::KeepAlive));
    tempStream << keepAliveId;
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}

void packets::play::clientbound::ChunkData::serialize(McDataStream &stream)
{
    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream.writeVar<qint32>(qint32(PacketType::ChunkData));
    tempStream << chunkX << chunkY << fullChunk;
    tempStream.writeVar<qint32>(primaryBitMask);
    tempStream.writeVar<qint32>(data.size());
    tempStream.writeRawData(data.constData(), data.size());
    tempStream.writeVar<qint32>(blockEntities.size());
    for (const auto &blockEntitiy : blockEntities)
        tempStream.writeRawData(blockEntitiy.constData(), blockEntitiy.size());
    stream.writeVar<qint32>(buffer.length());
    stream.writeRawData(buffer.constData(), buffer.length());
}
