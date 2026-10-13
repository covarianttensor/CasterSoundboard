#include "ActionProfile.h"
#include <QIODevice>
#include <QRegularExpression>

bool Actions::valid(const Profile &p) {
    return !p.name.trimmed().isEmpty() && p.name.size() <= 100 &&
        p.mode >= None && p.mode <= Mute && p.bufferMs >= 0 && p.bufferMs <= 5000 &&
        QRegularExpression("^KEY_[A-Z0-9_]{1,40}$").match(p.key).hasMatch() &&
        QRegularExpression("^KEY_[A-Z0-9_]{1,40}$").match(p.muteKey).hasMatch();
}
void Actions::write(QDataStream &out, const QVector<Profile> &profiles, int selected) {
    out << quint32(0x43414354) << quint32(2) << quint32(profiles.size()) << qint32(selected);
    for (const auto &p : profiles) out << p.name << qint32(p.mode) << p.key << qint32(p.bufferMs) << p.muteKey << p.attached;
}
void Actions::read(QDataStream &in, QVector<Profile> &profiles, int &selected) {
    profiles.clear(); selected = -1;
    if (in.status() != QDataStream::Ok || in.device()->atEnd()) return; // legacy .caster
    quint32 magic = 0, version = 0, count = 0; qint32 active = -1;
    in >> magic >> version >> count >> active;
    if (in.status() != QDataStream::Ok) return;
    if (magic != 0x43414354 || (version != 1 && version != 2) || count > 100 ||
        (count == 0 ? active != -1 : active < -1 || (active >= 0 && quint32(active) >= count))) {
        in.setStatus(QDataStream::ReadCorruptData); return;
    }
    QVector<Profile> loaded;
    for (quint32 i = 0; i < count && in.status() == QDataStream::Ok; ++i) {
        Profile p; qint32 mode = 0, buffer = 0;
        in >> p.name >> mode >> p.key >> buffer;
        if (in.status() != QDataStream::Ok) return;
        if (version >= 2) in >> p.muteKey >> p.attached;
        else p.muteKey = p.key; // v1 shared one key; preserve its behaviour.
        if (in.status() != QDataStream::Ok) return;
        p.mode = Mode(mode); p.bufferMs = buffer;
        if (!valid(p)) { in.setStatus(QDataStream::ReadCorruptData); return; }
        loaded.append(p);
    }
    if (in.status() == QDataStream::Ok) { profiles = loaded; selected = active; }
}
