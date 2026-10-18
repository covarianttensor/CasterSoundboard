#include "LegacyProjectImporter.h"
#include "PttProfile.h"

#include <QFile>
#include <QDataStream>
#include <QJsonArray>
#include <QColor>
#include <QRegularExpression>
#include <QSize>
#include <QUuid>
#include <algorithm>

bool LegacyProjectImporter::isLegacyBinary(const QByteArray &data)
{
    if (data.size() < 4)
        return false;
    return (reinterpret_cast<const quint8*>(data.constData())[0] == 0);
}

bool LegacyProjectImporter::isLegacyFile(const QString &filePath)
{
    if (filePath.isEmpty())
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray header = file.read(4);
    file.close();

    return isLegacyBinary(header);
}

LegacyProjectImporter::ImportResult LegacyProjectImporter::importLegacyProject(const QByteArray &data)
{
    ImportResult result;
    if (data.isEmpty()) {
        result.errorMessage = "Project data is empty.";
        return result;
    }

    QDataStream in(data);
    in.setVersion(QDataStream::Qt_5_0);

    QString name;
    in >> name;
    if (in.status() != QDataStream::Ok) {
        result.errorMessage = "Failed to read legacy project header.";
        return result;
    }

    QMap<QChar, QJsonObject> tiles;
    const QString layout = "12345678QWERTYUIASDFGHJKZXCVBNM,";
    QString keys = layout;
    std::sort(keys.begin(), keys.end());

    for (QChar key : keys) {
        QString path, css;
        qint32 volume = 0, begin = 0, end = 0, h = 0, s = 0, l = 0;
        bool loop = false, cue = false, colored = false;
        in >> path >> volume >> begin >> end >> loop >> cue >> h >> s >> l >> css >> colored;
        if (in.status() != QDataStream::Ok || volume < 0 || volume > 100) {
            result.errorMessage = "Malformed sound tile data in legacy project.";
            return result;
        }

        QJsonObject tile{
            {"filePath", path},
            {"hotkey", QString(key)},
            {"volume", volume / 100.0},
            {"isLooped", loop},
            {"cue", QJsonObject{{"enabled", cue}, {"beginMs", begin}, {"endMs", end}}}
        };

        // Flatten the legacy stylesheet gradient to its first solid colour.
        auto rgb = QRegularExpression("rgba\\(([^,]+),([^,]+),([^,]+),").match(css);
        if (colored && rgb.hasMatch()) {
            tile["color"] = QColor(qBound(0, qRound(rgb.captured(1).toDouble()), 255),
                                   qBound(0, qRound(rgb.captured(2).toDouble()), 255),
                                   qBound(0, qRound(rgb.captured(3).toDouble()), 255)).name();
        }
        tiles.insert(key, tile);
    }

    QJsonArray actionsArray;
    bool actionsRead = false;
    bool windowRead = false;

    while (!in.atEnd()) {
        quint32 magic = 0, version = 0;
        in >> magic >> version;
        if (magic == 0x4357535a && version == 1 && !windowRead) {
            // CWSZ window size extension block: advance past the bytes in the stream,
            // but completely discard window dimensions so the user's window geometry is untouched.
            QSize size;
            in >> size;
            if (size.isEmpty()) {
                result.errorMessage = "Invalid window size block in legacy project.";
                return result;
            }
            windowRead = true;
        } else if (magic == 0x43414354 && (version == 1 || version == 2) && !actionsRead) {
            // CACT Push-To-Talk and Mute actions extension block
            quint32 count = 0;
            qint32 active = -1;
            in >> count >> active;
            if (count > 100 || active < -1 || (active >= 0 && quint32(active) >= count)) {
                result.errorMessage = "Invalid action table in legacy project.";
                return result;
            }
            for (quint32 i = 0; i < count; ++i) {
                QString actionName, key, muteKey;
                qint32 mode = 0, buffer = 0;
                bool attached = false;
                in >> actionName >> mode >> key >> buffer;
                if (version == 2) {
                    in >> muteKey >> attached;
                }
                if (in.status() != QDataStream::Ok) {
                    result.errorMessage = "Truncated action entries in legacy project.";
                    return result;
                }

                Ptt::Profile prof;
                prof.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
                prof.name = actionName.isEmpty() ? QString("Imported PTT %1").arg(i + 1) : actionName;
                prof.mode = static_cast<Ptt::TalkMode>(qBound(0, mode, 2));
                prof.pttKey = key.isEmpty() ? QString("KEY_V") : key;
                prof.muteKey = muteKey.isEmpty() ? QString("KEY_M") : muteKey;
                prof.bufferMs = buffer > 0 ? buffer : 150;
                if (attached || (active >= 0 && quint32(active) == i)) {
                    prof.pinnedTabName = name.isEmpty() ? QString("Soundboard") : name;
                }
                actionsArray.append(prof.toJson());
            }
            result.hasPttProfiles = (count > 0);
            result.importedPttProfilesCount = count;
            actionsRead = true;
        } else {
            result.errorMessage = "Unknown or unsupported extension block in legacy project.";
            return result;
        }

        if (in.status() != QDataStream::Ok) {
            result.errorMessage = "Failed reading extension blocks in legacy project.";
            return result;
        }
    }

    QJsonArray sounds;
    for (QChar key : layout) {
        sounds.append(tiles.value(key));
    }

    // Notice: "window" object is deliberately omitted.
    QJsonObject root{
        {"formatVersion", 2},
        {"activeTabIndex", 0},
        {"tabs", QJsonArray{QJsonObject{{"name", name.isEmpty() ? QString("Soundboard") : name}, {"sounds", sounds}}}}
    };

    if (!actionsArray.isEmpty()) {
        root["pttProfiles"] = actionsArray;
    }

    result.projectJson = root;
    result.success = true;
    return result;
}
