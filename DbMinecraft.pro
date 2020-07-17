QT = core network

CONFIG += c++1z

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += main.cpp \
    chathelper.cpp \
    chunkhelper.cpp \
    closedclient.cpp \
    handshakingclient.cpp \
    loginclient.cpp \
    mcdatastream.cpp \
    packets.cpp \
    playclient.cpp \
    server.cpp \
    statusclient.cpp

HEADERS += \
    chathelper.h \
    chunkhelper.h \
    closedclient.h \
    handshakingclient.h \
    loginclient.h \
    mcdatastream.h \
    packets.h \
    playclient.h \
    server.h \
    statusclient.h
