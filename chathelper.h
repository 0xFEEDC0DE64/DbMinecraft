#pragma once

#include <QString>
#include <QJsonObject>

QJsonObject normalTextObj(const QString &text);
QJsonObject boldTextObj(const QString &text);

QString normalText(const QString &text);
QString boldText(const QString &text);
