#pragma once

#include <QtGlobal>
#include <QString>
#include <QDebug>
#include <QtCore>

#include <tuple>

class McDataStream;

namespace packets {
    namespace handshaking {
        namespace serverbound {
            Q_NAMESPACE

            enum class PacketType { Handshake };
            Q_ENUM_NS(PacketType)

            struct Handshake {
                Q_GADGET

            public:
                Handshake(McDataStream &stream);

                qint32 protocolVersion;
                QString serverAddress;
                quint16 serverPort;

                enum class SocketState { HandshakingState, StatusState, LoginState, PlayState, ClosedState };
                Q_ENUM(SocketState)

                SocketState nextState;
            };
        }

        namespace clientbound {
            Q_NAMESPACE

            enum class PacketType {};
            Q_ENUM_NS(PacketType)
        }
    }

    namespace status {
        namespace serverbound {
            Q_NAMESPACE

            enum class PacketType { Request, Ping };
            Q_ENUM_NS(PacketType)

            struct Request {
                Request(McDataStream &stream);
            };

            struct Ping {
                Ping(McDataStream &stream);

                qint64 payload;
            };
        }

        namespace clientbound {
            Q_NAMESPACE

            enum class PacketType { Response, Pong };
            Q_ENUM_NS(PacketType)

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
            Q_NAMESPACE

            enum class PacketType { Login };
            Q_ENUM_NS(PacketType)

            struct Login {
                Login(McDataStream &stream);

                QString name;
            };
        }

        namespace clientbound {
            Q_NAMESPACE

            enum class PacketType { LoginSuccess = 0x02 };
            Q_ENUM_NS(PacketType)

            struct LoginSuccess {
                QString uuid;
                QString username;

                void serialize(McDataStream &stream);
            };
        }
    }

    namespace play {
        namespace serverbound {
            Q_NAMESPACE

            enum class PacketType { ClientSettings = 0x04, InteractEntity = 0x0E, PluginMessage = 0x0A };
            Q_ENUM_NS(PacketType)

            struct ClientSettings {
                ClientSettings(McDataStream &stream);

                QString locale;
                qint8 viewDistance;
                qint32 chatMode;
                bool chatColors;
                quint8 displayedSkinParts;
                qint32 mainHand;
            };

            struct InteractEntity {
                InteractEntity(McDataStream &stream);

                qint32 entityId;
                enum Type : qint32 { Interact, Attack, InteractAt };
                Type type;
                std::optional<float> targetX;
                std::optional<float> targetY;
                std::optional<float> targetZ;
                enum Hand : qint32 { MainHand, OffHand };
                std::optional<Hand> hand;
            };

            struct PluginMessage {
                PluginMessage(McDataStream &stream);

                QString channel;
                QByteArray data;
            };
        }

        namespace clientbound {
            Q_NAMESPACE

            enum class PacketType { ServerDifficulty = 0x0D, ChatMessage = 0x0E, PluginMessage = 0x19, Disconnect = 0x1B, KeepAlive = 0x21,
                                    JoinGame = 0x25, PlayerAbilities = 0x2E, PlayerPositionAndLook = 0x32, SpawnPosition = 0x49 };
            Q_ENUM_NS(PacketType)

            struct ServerDifficulty {
                quint8 difficulty;

                void serialize(McDataStream &stream);
            };

            struct ChatMessage {
                QString jsonData;
                enum Position : qint8 { Chat, SystemMessage, GameInfo };
                Position position;

                void serialize(McDataStream &stream);
            };

            struct PluginMessage {
                QString channel;
                QByteArray data;

                void serialize(McDataStream &stream);
            };

            struct Disconnect {
                QString reason;

                void serialize(McDataStream &stream);
            };

            struct JoinGame {
                qint32 entityid;
                enum Gamemode : quint8 { Survival, Creative, Adventure, Spectator, Hardcore = 0x08 };
                Gamemode gamemode;
                enum Dimension : qint32 { Nether=-1, Overworld=0, End=1 };
                Dimension dimension;
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

            struct KeepAlive {
                qint64 keepAliveId;

                void serialize(McDataStream &stream);
            };
        }
    }

    namespace closed {
        namespace serverbound {
            Q_NAMESPACE

            enum class PacketType {};
            Q_ENUM_NS(PacketType)
        }

        namespace clientbound {
            Q_NAMESPACE

            enum class PacketType {};
            Q_ENUM_NS(PacketType)
        }
    }
}
