#include "chathelper.h"

#include <QJsonDocument>

QJsonObject normalTextObj(const QString &text)
{
    return QJsonObject { { "text", text } };
}

QJsonObject boldTextObj(const QString &text)
{
    QJsonObject obj = normalTextObj(text);
    obj["bold"] = true;
    return obj;
}

QString normalText(const QString &text)
{
    return QJsonDocument{normalTextObj(text)}.toJson();
}

QString boldText(const QString &text)
{
    return QJsonDocument{boldTextObj(text)}.toJson();
}
