#include "chunkhelper.h"

#include <algorithm>
#include <array>

#include "mcdatastream.h"

QByteArray createChunkSection()
{
    quint8 bitsPerBlock = 8;
    std::array<qint8, 2> palette { 0, 1 };
    std::array<qint64, 4096> blocks;

    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    tempStream << bitsPerBlock;
    tempStream.writeVar<qint32>(palette.size());
    for (const auto &entry : palette)
        tempStream.writeVar<qint32>(entry);
    tempStream.writeVar<qint32>(blocks.size());
    for (const auto &block : blocks)
        tempStream << block;
    for (int i = 0; i < blocks.size(); i++)
        tempStream << '\xFF';
    return buffer;
}

QByteArray createBiomes()
{
    qint32 biomes[256];
    std::fill(std::begin(biomes), std::end(biomes), 0);

    QByteArray buffer;
    McDataStream tempStream(&buffer, QIODevice::WriteOnly);
    for (const auto &biome : biomes)
        tempStream << biome;
    return buffer;
}
