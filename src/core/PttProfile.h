#ifndef PTTPROFILE_H
#define PTTPROFILE_H

#include <QString>
#include <QJsonObject>
#include <QMetaType>

namespace Ptt {

enum TalkMode {
    ModeNone = 0,
    ModePushToTalk = 1,
    ModeMuteToggle = 2
};

struct Profile {
    QString id;
    QString name;
    TalkMode mode = ModeNone;
    QString pinnedTabName;
    QString pttKey = "KEY_V";
    QString muteKey = "KEY_M";
    int bufferMs = 150;

    QString activeKey() const {
        return mode == ModeMuteToggle ? muteKey : pttKey;
    }

    bool isValid() const {
        return !name.trimmed().isEmpty() && bufferMs >= 0 && bufferMs <= 5000;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["name"] = name;
        obj["mode"] = static_cast<int>(mode);
        obj["pinnedTabName"] = pinnedTabName;
        obj["pttKey"] = pttKey;
        obj["muteKey"] = muteKey;
        obj["bufferMs"] = bufferMs;
        return obj;
    }

    static Profile fromJson(const QJsonObject &obj) {
        Profile p;
        p.id = obj.value("id").toString();
        p.name = obj.value("name").toString();
        p.mode = static_cast<TalkMode>(obj.value("mode").toInt(ModeNone));
        p.pinnedTabName = obj.value("pinnedTabName").toString();
        p.pttKey = obj.value("pttKey").toString("KEY_V");
        p.muteKey = obj.value("muteKey").toString("KEY_M");
        p.bufferMs = obj.value("bufferMs").toInt(150);
        return p;
    }

    bool operator==(const Profile &other) const {
        return id == other.id &&
               name == other.name &&
               mode == other.mode &&
               pinnedTabName == other.pinnedTabName &&
               pttKey == other.pttKey &&
               muteKey == other.muteKey &&
               bufferMs == other.bufferMs;
    }

    bool operator!=(const Profile &other) const {
        return !(*this == other);
    }
};

} // namespace Ptt

Q_DECLARE_METATYPE(Ptt::Profile)

#endif // PTTPROFILE_H
