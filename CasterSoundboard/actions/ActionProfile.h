#pragma once
#include <QDataStream>
#include <QString>
#include <QVector>

namespace Actions {
enum Mode { None = 0, PTT = 1, Mute = 2 };
struct Profile {
    QString name;
    Mode mode = None;
    QString key = "KEY_V";
    QString muteKey = "KEY_V";
    bool attached = true;
    QString inputKey() const { return mode == Mute ? muteKey : key; }
    int bufferMs = 150;
    bool operator==(const Profile &p) const {
        return name == p.name && mode == p.mode && key == p.key && muteKey == p.muteKey && attached == p.attached && bufferMs == p.bufferMs;
    }
};
bool valid(const Profile &p);
void write(QDataStream &out, const QVector<Profile> &profiles, int selected);
void read(QDataStream &in, QVector<Profile> &profiles, int &selected);
}
