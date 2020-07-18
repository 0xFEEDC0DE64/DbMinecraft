#pragma once

#include <optional>
#include <vector>

#include <QString>
#include <QJsonObject>

struct ChatPart {
    std::optional<QString> text;
    std::optional<bool> bold;
    std::optional<bool> italic;
    std::optional<bool> underlined;
    std::optional<bool> strikethrough;
    std::optional<bool> obfuscated;
    std::optional<QString> color;
    std::vector<ChatPart> extra;

    QJsonObject toObject() const;
    QString toString() const;
};
