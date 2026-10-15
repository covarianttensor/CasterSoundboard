#include "OscServerController.h"
#include "SoundboardProjectController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"

#include <QtEndian>
#include <QSettings>
#include <QDebug>
#include <cstring>

OscServerController::OscServerController(QObject *parent)
    : QObject(parent)
{
    m_socket = new QUdpSocket(this);
    loadSettings();
}

OscServerController::~OscServerController()
{
    stopServer();
}

void OscServerController::loadSettings()
{
    QSettings s("CasterSoundboard", "OscSettings");
    m_inboundPort = s.value("inboundPort", 5051).toInt();
    m_outboundHost = s.value("outboundHost", "127.0.0.1").toString();
    m_outboundPort = s.value("outboundPort", 9000).toInt();
    m_autoStart = s.value("autoStart", false).toBool();
    m_feedbackEnabled = s.value("feedbackEnabled", true).toBool();
}

void OscServerController::saveSettings()
{
    QSettings s("CasterSoundboard", "OscSettings");
    s.setValue("inboundPort", m_inboundPort);
    s.setValue("outboundHost", m_outboundHost);
    s.setValue("outboundPort", m_outboundPort);
    s.setValue("autoStart", m_autoStart);
    s.setValue("feedbackEnabled", m_feedbackEnabled);
}

void OscServerController::setProjectController(SoundboardProjectController *project)
{
    m_project = project;
    connectProjectSignals();
}

bool OscServerController::isRunning() const
{
    return m_socket && (m_socket->state() == QAbstractSocket::BoundState);
}

void OscServerController::setAutoStart(bool autoStart)
{
    if (m_autoStart == autoStart)
        return;
    m_autoStart = autoStart;
    saveSettings();
    emit autoStartChanged();
}

void OscServerController::setFeedbackEnabled(bool enabled)
{
    if (m_feedbackEnabled == enabled)
        return;
    m_feedbackEnabled = enabled;
    saveSettings();
    emit feedbackEnabledChanged();
}

void OscServerController::setInboundPort(int port)
{
    if (m_inboundPort == port || port < 1 || port > 65535)
        return;
    m_inboundPort = port;
    saveSettings();
    emit inboundPortChanged();

    if (isRunning()) {
        stopServer();
        startServer();
    }
}

void OscServerController::setOutboundHost(const QString &host)
{
    if (m_outboundHost == host)
        return;
    m_outboundHost = host;
    saveSettings();
    emit outboundHostChanged();
}

void OscServerController::setOutboundPort(int port)
{
    if (m_outboundPort == port || port < 1 || port > 65535)
        return;
    m_outboundPort = port;
    saveSettings();
    emit outboundPortChanged();
}

bool OscServerController::startServer()
{
    if (isRunning())
        return true;

    if (m_socket->bind(QHostAddress::Any, static_cast<quint16>(m_inboundPort))) {
        connect(m_socket, &QUdpSocket::readyRead, this, &OscServerController::onReadyRead, Qt::UniqueConnection);
        m_statusMessage = tr("Listening on UDP port %1").arg(m_inboundPort);
        emit runningChanged();
        emit statusMessageChanged();
        if (m_feedbackEnabled) {
            syncClients();
        }
        return true;
    } else {
        m_statusMessage = tr("Failed to bind UDP port %1: %2").arg(m_inboundPort).arg(m_socket->errorString());
        emit runningChanged();
        emit statusMessageChanged();
        return false;
    }
}

void OscServerController::stopServer()
{
    if (!isRunning())
        return;

    disconnect(m_socket, &QUdpSocket::readyRead, this, &OscServerController::onReadyRead);
    m_socket->close();
    m_statusMessage = tr("Server stopped");
    emit runningChanged();
    emit statusMessageChanged();
}

void OscServerController::toggleServer()
{
    if (isRunning()) {
        stopServer();
    } else {
        startServer();
    }
}

void OscServerController::connectProjectSignals()
{
    if (!m_project)
        return;

    connect(m_project, &SoundboardProjectController::isDuckingActiveChanged, this, [this]() {
        if (m_feedbackEnabled && isRunning()) {
            sendOscMessage("/glo/s/audio_d_s", m_project->isDuckingActive() ? 1 : 0);
            sendOscMessage("/castersoundboard/ducking", m_project->isDuckingActive() ? 1.0f : 0.0f);
        }
    });

    if (m_project->tabModel()) {
        auto rebindCurrentTiles = [this]() {
            if (!m_project || !m_project->tabModel())
                return;

            if (m_feedbackEnabled && isRunning()) {
                sendOscMessage("/glo/s/current_tab", m_project->tabModel()->currentIndex() + 1);
            }

            SoundboardGridModel *grid = m_project->tabModel()->currentGridModel();
            if (!grid)
                return;

            for (int i = 0; i < grid->tiles().size(); ++i) {
                SoundTileController *tile = grid->tiles().at(i);
                int tileIdx = i + 1;
                QString boardName = m_project->tabModel()->tabNameAt(m_project->tabModel()->currentIndex());
                QString tileName = tile->displayName();
                connect(tile, &SoundTileController::playbackStateChanged, this, [this, tile, tileIdx, boardName, tileName]() {
                    if (m_feedbackEnabled && isRunning()) {
                        int state = (tile->playbackState() == QMediaPlayer::PlayingState) ? 1 :
                                    ((tile->playbackState() == QMediaPlayer::PausedState) ? 2 : 0);
                        sendOscMessage(QString("/cbp/%1/s/p_s").arg(tileIdx), state);
                        sendOscMessage(QString("/castersoundboard/board/%1/player/%2/status/play_state").arg(boardName, tileName), state);
                    }
                });

                connect(tile, &SoundTileController::volumeChanged, this, [this, tile, tileIdx]() {
                    if (m_feedbackEnabled && isRunning()) {
                        sendOscMessage(QString("/cbp/%1/s/vol").arg(tileIdx), static_cast<float>(tile->volume()));
                    }
                });
            }
        };

        connect(m_project->tabModel(), &SoundboardTabModel::currentGridModelChanged, this, rebindCurrentTiles);
        connect(m_project->tabModel(), &SoundboardTabModel::currentIndexChanged, this, rebindCurrentTiles);
        connect(m_project, &SoundboardProjectController::projectLoaded, this, rebindCurrentTiles);

        rebindCurrentTiles();
    }
}

void OscServerController::syncClients()
{
    if (!isRunning() || !m_project)
        return;

    sendOscMessage("/glo/s/audio_d_s", m_project->isDuckingActive() ? 1 : 0);
    sendOscMessage("/castersoundboard/ducking", m_project->isDuckingActive() ? 1.0f : 0.0f);

    if (m_project->tabModel()) {
        sendOscMessage("/glo/s/current_tab", m_project->tabModel()->currentIndex() + 1);
        SoundboardGridModel *grid = m_project->tabModel()->currentGridModel();
        if (grid) {
            QString boardName = m_project->tabModel()->tabNameAt(m_project->tabModel()->currentIndex());
            for (int i = 0; i < grid->tiles().size(); ++i) {
                SoundTileController *t = grid->tiles().at(i);
                int state = (t->playbackState() == QMediaPlayer::PlayingState) ? 1 :
                            ((t->playbackState() == QMediaPlayer::PausedState) ? 2 : 0);
                sendOscMessage(QString("/cbp/%1/s/p_s").arg(i + 1), state);
                sendOscMessage(QString("/cbp/%1/s/vol").arg(i + 1), static_cast<float>(t->volume()));
                sendOscMessage(QString("/castersoundboard/board/%1/player/%2/status/play_state").arg(boardName, t->displayName()), state);
            }
        }
    }
}

bool OscServerController::sendOscMessage(const QString &address, float value)
{
    if (!m_socket)
        return false;

    // Build OSC 1.0 message packet: address, tags, 32-bit float
    QByteArray packet;
    QByteArray addrBytes = address.toUtf8();
    addrBytes.append('\0');
    while ((addrBytes.size() % 4) != 0) {
        addrBytes.append('\0');
    }
    packet.append(addrBytes);

    // Type tag: ",f\0\0"
    packet.append(",f\0\0", 4);

    // Float argument big-endian
    quint32 raw;
    std::memcpy(&raw, &value, sizeof(float));
    quint32 be = qToBigEndian(raw);
    packet.append(reinterpret_cast<const char*>(&be), sizeof(be));

    qint64 sent = m_socket->writeDatagram(packet, QHostAddress(m_outboundHost), static_cast<quint16>(m_outboundPort));
    return sent == packet.size();
}

bool OscServerController::sendOscMessage(const QString &address, int value)
{
    if (!m_socket)
        return false;

    QByteArray packet;
    QByteArray addrBytes = address.toUtf8();
    addrBytes.append('\0');
    while ((addrBytes.size() % 4) != 0) {
        addrBytes.append('\0');
    }
    packet.append(addrBytes);

    // Type tag: ",i\0\0"
    packet.append(",i\0\0", 4);

    qint32 be = qToBigEndian(static_cast<qint32>(value));
    packet.append(reinterpret_cast<const char*>(&be), sizeof(be));

    qint64 sent = m_socket->writeDatagram(packet, QHostAddress(m_outboundHost), static_cast<quint16>(m_outboundPort));
    return sent == packet.size();
}

void OscServerController::onReadyRead()
{
    const qint64 maxPacketSize = 8192;
    while (m_socket->hasPendingDatagrams()) {
        qint64 size = m_socket->pendingDatagramSize();
        if (size < 8 || size > maxPacketSize || (size % 4) != 0) {
            char discard;
            if (m_socket->readDatagram(&discard, 1) < 0)
                break;
            continue;
        }

        QByteArray datagram(static_cast<int>(size), 0);
        QHostAddress sender;
        quint16 senderPort = 0;
        if (m_socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort) != size) {
            break;
        }

        processDatagram(datagram, sender, senderPort);
    }
}

void OscServerController::processDatagram(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort)
{
    Q_UNUSED(sender);
    Q_UNUSED(senderPort);

    // Check bundle prefix
    if (datagram.startsWith("#bundle")) {
        // Bundles ignored cleanly per regression suite
        return;
    }

    if (datagram.at(0) != '/')
        return;

    int addrEnd = datagram.indexOf('\0');
    if (addrEnd < 1)
        return;

    QString address = QString::fromUtf8(datagram.constData(), addrEnd);

    int tagOffset = (addrEnd + 4) & ~3;
    if (tagOffset >= datagram.size() || datagram.at(tagOffset) != ',')
        return;

    int tagEnd = datagram.indexOf('\0', tagOffset);
    if (tagEnd < 0)
        return;

    QByteArray tags = datagram.mid(tagOffset + 1, tagEnd - tagOffset - 1);
    int argOffset = (tagEnd + 4) & ~3;

    QVariantList args;
    for (int i = 0; i < tags.size(); ++i) {
        char tag = tags.at(i);
        if (tag == 'i') {
            if (argOffset + 4 > datagram.size())
                return;
            qint32 val = qFromBigEndian<qint32>(reinterpret_cast<const uchar*>(datagram.constData() + argOffset));
            argOffset += 4;
            args.append(val);
        } else if (tag == 'f') {
            if (argOffset + 4 > datagram.size())
                return;
            quint32 raw = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(datagram.constData() + argOffset));
            float val;
            std::memcpy(&val, &raw, sizeof(float));
            argOffset += 4;
            args.append(val);
        } else if (tag == 's') {
            int strEnd = datagram.indexOf('\0', argOffset);
            if (strEnd < 0)
                return;
            QString s = QString::fromUtf8(datagram.constData() + argOffset, strEnd - argOffset);
            argOffset = (strEnd + 4) & ~3;
            args.append(s);
        } else if (tag == 'h') {
            if (argOffset + 8 > datagram.size())
                return;
            qint64 val = qFromBigEndian<qint64>(reinterpret_cast<const uchar*>(datagram.constData() + argOffset));
            argOffset += 8;
            args.append(val);
        } else if (tag == 'T') {
            args.append(true);
        } else if (tag == 'F') {
            args.append(false);
        } else if (tag == 'b') {
            if (argOffset + 4 > datagram.size())
                return;
            qint32 blobLen = qFromBigEndian<qint32>(reinterpret_cast<const uchar*>(datagram.constData() + argOffset));
            argOffset += 4;
            if (blobLen < 0 || argOffset + blobLen > datagram.size())
                return;
            argOffset = (argOffset + blobLen + 3) & ~3;
        } else {
            // Unknown type tag - skip safely
            return;
        }
    }

    m_packetsReceived++;
    emit packetsReceivedChanged();

    m_lastMessageAddress = address;
    QString valSummary;
    for (int i = 0; i < args.size(); ++i) {
        if (i > 0) valSummary += ", ";
        valSummary += args.at(i).toString();
    }
    m_lastMessageValue = valSummary;
    emit lastMessageChanged();

    if (!args.isEmpty()) {
        emit messageReceived(address, args.first());
    } else {
        emit messageReceived(address, QVariant());
    }

    dispatchCommand(address, args);
}

void OscServerController::dispatchCommand(const QString &address, const QVariantList &args)
{
    if (!m_project)
        return;

    QStringList parts = address.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return;

    // 0. Full client sync request: /sync, /glo/sync, /castersoundboard/sync
    if (address == "/sync" || address == "/glo/sync" || address == "/castersoundboard/sync" ||
        (parts.size() >= 2 && parts[0] == "glo" && parts[1] == "sync") ||
        (parts.size() >= 3 && parts[0] == "castersoundboard" && parts[1] == "global" && parts[2] == "sync")) {
        syncClients();
        return;
    }

    // 1. Stop all sounds: /castersoundboard/stop_all or /stop_all or /glo/m/all_p_s/stop
    if (address == "/stop_all" ||
        address == "/castersoundboard/stop_all" ||
        (parts.size() >= 4 && parts[0] == "castersoundboard" && parts[1] == "global" && parts[2] == "modify" && parts[3] == "all_play_states" && parts.value(4) == "stop") ||
        (parts.size() >= 4 && parts[0] == "glo" && parts[1] == "m" && parts[2] == "all_p_s" && parts.value(3) == "stop")) {
        if (args.isEmpty() || args.first().toInt() == 1) {
            m_project->stopAllSounds();
        }
        return;
    }

    // 2. Ducking: /castersoundboard/ducking or /glo/m/audio_d_s
    if (address == "/castersoundboard/ducking" ||
        (parts.size() >= 4 && parts[0] == "castersoundboard" && parts[1] == "global" && parts[2] == "modify" && parts[3] == "audio_ducking_state") ||
        (parts.size() >= 3 && parts[0] == "glo" && parts[1] == "m" && parts[2] == "audio_d_s")) {
        if (!args.isEmpty()) {
            m_project->setIsDuckingActive(args.first().toInt() > 0);
        } else {
            m_project->toggleAudioDucking();
        }
        return;
    }

    // 3. Tab navigation: /glo/m/current_tab/next or prev
    if (parts.size() >= 4 && parts[0] == "glo" && parts[1] == "m" && parts[2] == "current_tab") {
        if (args.isEmpty() || args.first().toInt() == 1) {
            if (parts[3] == "next") {
                m_project->tabModel()->nextTab();
            } else if (parts[3] == "prev") {
                m_project->tabModel()->previousTab();
            }
        }
        return;
    }

    // 4. Direct sound tile trigger: /play/<index> or /stop/<index>
    if (parts.size() >= 2 && (parts[0] == "play" || parts[0] == "stop")) {
        bool ok = false;
        int idx = parts[1].toInt(&ok);
        if (ok) {
            if (idx > 0) idx -= 1; // 1-based index support
            SoundboardGridModel *grid = m_project->tabModel()->currentGridModel();
            if (grid && idx >= 0 && idx < grid->tiles().size()) {
                if (parts[0] == "play") {
                    grid->tiles()[idx]->play();
                } else {
                    grid->tiles()[idx]->stop();
                }
            }
        }
        return;
    }

    // 5. Current Board Player: /cbp/<playerIndex>/m/...
    if (parts.size() >= 4 && parts[0] == "cbp" && parts[2] == "m") {
        bool ok = false;
        int idx = parts[1].toInt(&ok);
        if (ok) {
            if (idx > 0) idx -= 1;
            SoundboardGridModel *grid = m_project->tabModel()->currentGridModel();
            if (grid && idx >= 0 && idx < grid->tiles().size()) {
                SoundTileController *tile = grid->tiles()[idx];
                const QString prop = parts[3];
                const QString subProp = parts.value(4);

                if (prop == "vol" && !args.isEmpty()) {
                    tile->setVolume(args.first().toDouble());
                } else if (prop == "p_s" && !args.isEmpty() && args.first().toInt() == 1) {
                    if (subProp == "play_stop" || subProp == "play") {
                        tile->trigger();
                    } else if (subProp == "resume_pause") {
                        if (tile->playbackState() == QMediaPlayer::PlayingState) tile->pause();
                        else tile->play();
                    } else if (subProp == "stop") {
                        tile->stop();
                    }
                } else if (prop == "l_s" && !args.isEmpty()) {
                    tile->setIsLooped(args.first().toInt() > 0);
                }
            }
        }
        return;
    }

    // 6. Full legacy path: /castersoundboard/board/<boardName>/player/<playerKey>/modify/...
    if (parts.size() >= 6 && parts[0] == "castersoundboard" && parts[1] == "board" && parts[3] == "player") {
        const QString boardName = parts[2];
        const QString playerKey = parts[4];
        // Search in all tabs for matching board name or active board
        SoundboardGridModel *targetGrid = nullptr;
        for (int t = 0; t < m_project->tabModel()->count(); ++t) {
            if (m_project->tabModel()->tabNameAt(t).compare(boardName, Qt::CaseInsensitive) == 0) {
                targetGrid = m_project->tabModel()->gridModelAt(t);
                break;
            }
        }
        if (!targetGrid) {
            targetGrid = m_project->tabModel()->currentGridModel();
        }

        if (targetGrid) {
            bool ok = false;
            int idx = playerKey.toInt(&ok);
            SoundTileController *tile = nullptr;
            if (ok) {
                if (idx > 0) idx -= 1;
                if (idx >= 0 && idx < targetGrid->tiles().size()) {
                    tile = targetGrid->tiles()[idx];
                }
            } else {
                for (auto *t : targetGrid->tiles()) {
                    if (t->displayName().compare(playerKey, Qt::CaseInsensitive) == 0 || t->id() == playerKey) {
                        tile = t;
                        break;
                    }
                }
            }

            if (tile && parts.value(5) == "modify") {
                const QString prop = parts.value(6);
                const QString subProp = parts.value(7);

                if (prop == "volume" && !args.isEmpty()) {
                    tile->setVolume(args.first().toDouble());
                } else if (prop == "play_state" && !args.isEmpty() && args.first().toInt() == 1) {
                    if (subProp == "play") tile->play();
                    else if (subProp == "pause") tile->pause();
                    else if (subProp == "stop") tile->stop();
                } else if (prop == "loop_state" && !args.isEmpty()) {
                    tile->setIsLooped(args.first().toInt() > 0);
                }
            }
        }
    }
}
