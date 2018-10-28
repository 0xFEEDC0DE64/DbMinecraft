QT = core network

CONFIG += c++1z

DEFINES += QT_DEPRECATED_WARNINGS QT_DISABLE_DEPRECATED_BEFORE=0x060000

SOURCES += main.cpp \
    client.cpp \
    mcdatastream.cpp \
    packets.cpp

HEADERS += \
    client.h \
    mcdatastream.h \
    packets.h
