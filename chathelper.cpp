#include "chathelper.h"

#include <QJsonDocument>
#include <QJsonArray>

QJsonObject ChatPart::toObject() const
{
    QJsonObject obj;
    if (text.has_value())
        obj["text"] = *text;
    if (bold.has_value())
        obj["bold"] = *bold;
    if (italic.has_value())
        obj["italic"] = *italic;
    if (underlined.has_value())
        obj["underlined"] = *underlined;
    if (strikethrough.has_value())
        obj["strikethrough"] = *strikethrough;
    if (obfuscated.has_value())
        obj["obfuscated"] = *obfuscated;
    if (color.has_value())
        obj["color"] = *color;
    if (!extra.empty())
    {
        QJsonArray arr;
        for (const auto &sub : extra)
            arr.append(sub.toObject());
        obj["extra"] = arr;
    }
    return obj;
}

QString ChatPart::toString() const
{
    return QJsonDocument{toObject()}.toJson(QJsonDocument::Compact);
}
