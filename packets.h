#pragma once

#include <QtGlobal>
#include <QString>

#include <tuple>

class McDataStream;

enum SocketState { HandshakingState, StatusState, LoginState, PlayState, ClosedState };

namespace packets {
    namespace handshaking {
        namespace serverbound {
            enum PacketType { PacketHandshake };

            struct Handshake {
                Handshake(McDataStream &stream);

                qint32 protocolVersion;
                QString serverAddress;
                quint16 serverPort;
                SocketState nextState;
            };
        }

        namespace clientbound {
        }
    }

    namespace status {
        namespace serverbound {
            enum PacketType { PacketRequest, PacketPing };

            struct Request {
                Request(McDataStream &stream);
            };

            struct Ping {
                Ping(McDataStream &stream);

                qint64 payload;
            };
        }

        namespace clientbound {
            enum PacketType { PacketResponse, PacketPong };

            struct Response {
                QString jsonResponse;

                void serialize(McDataStream &stream);
            };

            struct Pong {
                qint64 payload;

                void serialize(McDataStream &stream);
            };
        }
    }

    namespace login {
        namespace serverbound {
            enum PacketType { PacketLogin };

            struct Login {
                Login(McDataStream &stream);

                QString name;
            };
        }

        namespace clientbound {
            enum PacketType { PacketLoginSuccess = 0x02 };

            struct LoginSuccess {
                QString uuid;
                QString username;

                void serialize(McDataStream &stream);
            };
        }
    }

    namespace play {
        namespace serverbound {
            enum PacketType { PacketClientSettings = 0x04, PacketPluginMessage = 0x0A };

            struct ClientSettings {
                ClientSettings(McDataStream &stream);

                QString locale;
                qint8 viewDistance;
                qint32 chatMode;
                bool chatColors;
                quint8 displayedSkinParts;
                qint32 mainHand;
            };

            struct PluginMessage {
                PluginMessage(McDataStream &stream);

                QString channel;
                QByteArray data;
            };
        }

        namespace clientbound {
            enum PacketType { PacketServerDifficulty = 0x0D, PacketPluginMessage = 0x19, PacketJoinGame = 0x25, PacketPlayerAbilities = 0x2E, PacketPlayerPositionAndLook = 0x32,
                              PacketSpawnPosition = 0x49 };

            struct ServerDifficulty {
                quint8 difficulty;

                void serialize(McDataStream &stream);
            };

            struct PluginMessage {
                QString channel;
                QByteArray data;

                void serialize(McDataStream &stream);
            };

            struct JoinGame {
                qint32 entityid;
                quint8 gamemode;
                qint32 dimension;
                quint8 difficulty;
                quint8 maxPlayers;
                QString levelType;
                bool reducedDebugInfo;

                void serialize(McDataStream &stream);
            };

            struct PlayerAbilities {
                qint8 flags;
                float flyingSpeed;
                float fieldOfViewModifier;

                void serialize(McDataStream &stream);
            };

            struct PlayerPositionAndLook {
                double x;
                double y;
                double z;
                float yaw;
                float pitch;
                qint8 flags;
                qint32 teleportId;

                void serialize(McDataStream &stream);
            };

            struct SpawnPosition {
                std::tuple<qint32, qint16, qint32> location;

                void serialize(McDataStream &stream);
            };
        }
    }
}
