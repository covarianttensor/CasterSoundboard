#include "McpServerController.h"
#include "SoundboardProjectController.h"
#include "AudioRoutingController.h"
#include "SoundboardTabModel.h"
#include "SoundboardGridModel.h"
#include "SoundTileController.h"
#include "ActionsController.h"

#include <QSettings>
#include <QDebug>
#include <iostream>

McpServerController::McpServerController(QObject *parent)
    : QObject(parent)
{
    loadSettings();

    connect(&m_server, &QTcpServer::newConnection,
            this, &McpServerController::onNewConnection);
}

McpServerController::~McpServerController()
{
    stopServer();
}

void McpServerController::setProjectController(SoundboardProjectController *project)
{
    m_projectController = project;
}

void McpServerController::setAudioRoutingController(AudioRoutingController *routing)
{
    m_audioRoutingController = routing;
}

void McpServerController::setAutoStart(bool autoStart)
{
    if (m_autoStart != autoStart) {
        m_autoStart = autoStart;
        saveSettings();
        emit autoStartChanged(m_autoStart);
    }
}

void McpServerController::setPort(int port)
{
    if (port <= 0 || port > 65535) port = 5055;
    if (m_port != port) {
        m_port = port;
        saveSettings();
        emit portChanged(m_port);
    }
}

void McpServerController::setBindAddress(const QString &addr)
{
    if (m_bindAddress != addr) {
        m_bindAddress = addr;
        saveSettings();
        emit bindAddressChanged(m_bindAddress);
    }
}

QString McpServerController::localUrl() const
{
    return QString("http://127.0.0.1:%1").arg(m_port);
}

QString McpServerController::sseUrl() const
{
    return QString("http://127.0.0.1:%1/sse").arg(m_port);
}

bool McpServerController::startServer()
{
    if (m_server.isListening()) {
        return true;
    }

    QHostAddress addr(m_bindAddress);
    if (addr.isNull()) {
        addr = QHostAddress::Any;
    }

    if (!m_server.listen(addr, static_cast<quint16>(m_port))) {
        m_statusMessage = QString("Failed to bind port %1: %2").arg(m_port).arg(m_server.errorString());
        emit statusMessageChanged();
        emit runningChanged(false);
        qWarning() << "[McpServer]" << m_statusMessage;
        return false;
    }

    m_statusMessage = QString("MCP Server listening on %1 (Port %2)").arg(m_bindAddress).arg(m_port);
    emit statusMessageChanged();
    emit runningChanged(true);
    qInfo().noquote() << "[McpServer]" << m_statusMessage << "SSE endpoint:" << sseUrl();
    return true;
}

void McpServerController::stopServer()
{
    if (m_server.isListening()) {
        for (auto *socket : m_clients) {
            if (socket) {
                socket->disconnectFromHost();
            }
        }
        m_clients.clear();
        m_sseClients.clear();
        m_clientSessions.clear();
        m_server.close();

        m_statusMessage = "MCP Server stopped";
        emit statusMessageChanged();
        emit runningChanged(false);
        emit clientsChanged(0);
        qInfo() << "[McpServer] Server stopped.";
    }
}

bool McpServerController::toggleServer()
{
    if (isRunning()) {
        stopServer();
        return false;
    } else {
        return startServer();
    }
}

void McpServerController::loadSettings()
{
    QSettings settings;
    settings.beginGroup("McpServer");
    m_autoStart = settings.value("autoStart", false).toBool();
    m_port = settings.value("port", 5055).toInt();
    m_bindAddress = settings.value("bindAddress", "127.0.0.1").toString();
    settings.endGroup();
}

void McpServerController::saveSettings()
{
    QSettings settings;
    settings.beginGroup("McpServer");
    settings.setValue("autoStart", m_autoStart);
    settings.setValue("port", m_port);
    settings.setValue("bindAddress", m_bindAddress);
    settings.endGroup();
}

void McpServerController::onNewConnection()
{
    while (m_server.hasPendingConnections()) {
        QTcpSocket *socket = m_server.nextPendingConnection();
        m_clients.insert(socket);

        connect(socket, &QTcpSocket::readyRead,
                this, &McpServerController::onSocketReadyRead);
        connect(socket, &QTcpSocket::disconnected,
                this, &McpServerController::onSocketDisconnected);
    }
}

void McpServerController::onSocketDisconnected()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        m_clients.remove(socket);
        if (m_clientSessions.contains(socket)) {
            const QString session = m_clientSessions.take(socket);
            m_sseClients.remove(session);
            emit clientsChanged(m_sseClients.size());
        }
        socket->deleteLater();
    }
}

void McpServerController::onSocketReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    handleHttpRequest(socket, data);
}

void McpServerController::handleHttpRequest(QTcpSocket *socket, const QByteArray &requestData)
{
    const QString requestStr = QString::fromUtf8(requestData);
    const QStringList lines = requestStr.split("\r\n");
    if (lines.isEmpty()) return;

    const QStringList requestLineParts = lines.first().split(' ');
    if (requestLineParts.size() < 2) return;

    const QString method = requestLineParts.at(0).toUpper();
    const QString rawPath = requestLineParts.at(1);
    const QString path = rawPath.split('?').first();

    // CORS preflight
    if (method == "OPTIONS") {
        sendHttpResponse(socket, "200 OK", QByteArray(), "text/plain",
                         "Access-Control-Allow-Origin: *\r\n"
                         "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                         "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n");
        return;
    }

    // 1. Root / Status Info
    if (method == "GET" && (path == "/" || path == "/status")) {
        QJsonObject info;
        info["server"] = "Caster Soundboard MCP Server";
        info["version"] = "2.0.0";
        info["protocolVersion"] = "2024-11-05";
        info["sseEndpoint"] = sseUrl();
        info["messageEndpoint"] = localUrl() + "/message";
        info["connectedClients"] = connectedClients();
        info["invocationsCount"] = m_toolInvocationsCount;
        info["lastInvokedTool"] = m_lastInvokedTool;

        QJsonArray toolsArr;
        const QJsonObject toolsListRes = handleToolsList(QJsonObject());
        if (toolsListRes.contains("result")) {
            toolsArr = toolsListRes["result"].toObject()["tools"].toArray();
        }
        info["availableTools"] = toolsArr;

        sendHttpResponse(socket, "200 OK", QJsonDocument(info).toJson(QJsonDocument::Indented), "application/json");
        return;
    }

    // 2. Server-Sent Events (SSE) Stream
    if (method == "GET" && path == "/sse") {
        const QString sessionId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        m_sseClients.insert(sessionId, socket);
        m_clientSessions.insert(socket, sessionId);
        emit clientsChanged(m_sseClients.size());

        const QString headers = QString(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/event-stream\r\n"
            "Cache-Control: no-cache, no-transform\r\n"
            "Connection: keep-alive\r\n"
            "Access-Control-Allow-Origin: *\r\n\r\n"
        );
        socket->write(headers.toUtf8());

        // Send endpoint event with session ID per MCP specification
        const QString endpointUri = QString("/message?sessionId=%1").arg(sessionId);
        sendSseEvent(socket, "endpoint", endpointUri.toUtf8());
        return;
    }

    // 3. JSON-RPC Message Endpoint (POST /message or POST /)
    if (method == "POST" && (path == "/message" || path == "/")) {
        const int bodyIdx = requestData.indexOf("\r\n\r\n");
        QByteArray body;
        if (bodyIdx != -1) {
            body = requestData.mid(bodyIdx + 4);
        }

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(body, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            QJsonObject errObj = makeErrorResponse(QJsonObject(), -32700, "Parse error: Invalid JSON");
            sendHttpResponse(socket, "400 Bad Request", QJsonDocument(errObj).toJson(), "application/json");
            return;
        }

        QJsonObject response = handleJsonRpc(doc.object());

        // Also broadcast to SSE client if a session ID is given
        QString sessionId;
        if (rawPath.contains("sessionId=")) {
            sessionId = rawPath.section("sessionId=", 1, 1).section('&', 0, 0);
        }
        if (!sessionId.isEmpty() && m_sseClients.contains(sessionId)) {
            QTcpSocket *sseSock = m_sseClients.value(sessionId).data();
            if (sseSock) {
                sendSseEvent(sseSock, "message", QJsonDocument(response).toJson(QJsonDocument::Compact));
            }
        }

        sendHttpResponse(socket, "200 OK", QJsonDocument(response).toJson(QJsonDocument::Compact), "application/json");
        return;
    }

    // Fallback 404
    sendHttpResponse(socket, "404 Not Found", "{\"error\": \"Not Found\"}", "application/json");
}

void McpServerController::sendHttpResponse(QTcpSocket *socket, const QString &status, const QByteArray &body,
                                          const QString &contentType, const QString &extraHeaders)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) return;

    QString headers = QString("HTTP/1.1 %1\r\n").arg(status);
    headers += QString("Content-Type: %1\r\n").arg(contentType);
    headers += QString("Content-Length: %1\r\n").arg(body.size());
    headers += "Access-Control-Allow-Origin: *\r\n";
    headers += "Connection: close\r\n";
    if (!extraHeaders.isEmpty()) {
        headers += extraHeaders;
    }
    headers += "\r\n";

    socket->write(headers.toUtf8());
    if (!body.isEmpty()) {
        socket->write(body);
    }
    socket->flush();
    socket->disconnectFromHost();
}

void McpServerController::sendSseEvent(QTcpSocket *socket, const QString &eventName, const QByteArray &data)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) return;

    QByteArray payload;
    if (!eventName.isEmpty()) {
        payload += "event: " + eventName.toUtf8() + "\n";
    }
    payload += "data: " + data + "\n\n";

    socket->write(payload);
    socket->flush();
}

void McpServerController::processStdioLine(const QByteArray &line)
{
    const QByteArray trimmed = line.trimmed();
    if (trimmed.isEmpty()) return;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(trimmed, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        QJsonObject errObj = makeErrorResponse(QJsonObject(), -32700, "Parse error");
        std::cout << QJsonDocument(errObj).toJson(QJsonDocument::Compact).toStdString() << std::endl;
        return;
    }

    QJsonObject response = handleJsonRpc(doc.object());
    std::cout << QJsonDocument(response).toJson(QJsonDocument::Compact).toStdString() << std::endl;
}

QJsonObject McpServerController::handleJsonRpc(const QJsonObject &request)
{
    const QString method = request.value("method").toString();
    const QJsonObject idObj = request.contains("id") ? request.value("id").toObject() : QJsonObject();
    const QJsonObject params = request.value("params").toObject();

    if (method == "initialize") {
        return handleInitialize(idObj, params);
    } else if (method == "notifications/initialized") {
        QJsonObject res;
        res["jsonrpc"] = "2.0";
        return res;
    } else if (method == "ping") {
        return makeSuccessResponse(idObj, QJsonObject());
    } else if (method == "tools/list") {
        return handleToolsList(idObj);
    } else if (method == "tools/call") {
        return handleToolsCall(idObj, params);
    }

    return makeErrorResponse(idObj, -32601, QString("Method not found: %1").arg(method));
}

QJsonObject McpServerController::handleInitialize(const QJsonObject &id, const QJsonObject &/*params*/)
{
    QJsonObject result;
    result["protocolVersion"] = "2024-11-05";

    QJsonObject capabilities;
    capabilities["tools"] = QJsonObject();
    result["capabilities"] = capabilities;

    QJsonObject serverInfo;
    serverInfo["name"] = "caster-soundboard";
    serverInfo["version"] = "2.0.0";
    result["serverInfo"] = serverInfo;

    return makeSuccessResponse(id, result);
}

QJsonObject McpServerController::handleToolsList(const QJsonObject &id)
{
    QJsonArray tools;

    // 1. caster_get_state
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject();
        tools.append(makeToolDefinition(
            "caster_get_state",
            "Retrieve current Caster Soundboard runtime state, including sound banks, loaded sound tiles, playback states, audio volume, ducking, and active PTT key.",
            schema
        ));
    }

    // 2. caster_trigger_sound
    {
        QJsonObject schema;
        schema["type"] = "object";
        QJsonObject props;

        QJsonObject soundNameProp;
        soundNameProp["type"] = "string";
        soundNameProp["description"] = "Sound tile title or partial name to trigger (case-insensitive substring match).";
        props["soundName"] = soundNameProp;

        QJsonObject tabIndexProp;
        tabIndexProp["type"] = "integer";
        tabIndexProp["description"] = "Zero-based tab index (optional if soundName is provided).";
        props["tabIndex"] = tabIndexProp;

        QJsonObject tileIndexProp;
        tileIndexProp["type"] = "integer";
        tileIndexProp["description"] = "Zero-based tile index on the tab (optional if soundName is provided).";
        props["tileIndex"] = tileIndexProp;

        schema["properties"] = props;
        tools.append(makeToolDefinition(
            "caster_trigger_sound",
            "Trigger, toggle, or play a sound tile. If currently playing, it toggles (pauses or stops depending on cue/loop configuration). Also triggers virtual Push-to-Talk.",
            schema
        ));
    }

    // 3. caster_stop_all
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject();
        tools.append(makeToolDefinition(
            "caster_stop_all",
            "Emergency panic button: immediately halt all active sound playback and release any transmitting Push-to-Talk keys.",
            schema
        ));
    }

    // 4. caster_set_ducking
    {
        QJsonObject schema;
        schema["type"] = "object";
        QJsonObject props;

        QJsonObject enabledProp;
        enabledProp["type"] = "boolean";
        enabledProp["description"] = "Set ducking state explicitly (true for active, false for inactive). If omitted, toggles ducking.";
        props["enabled"] = enabledProp;

        QJsonObject ratioProp;
        ratioProp["type"] = "number";
        ratioProp["description"] = "Ducking attenuation ratio between 0.05 (heavy attenuation) and 0.75 (subtle attenuation). Default is 0.25.";
        props["duckingRatio"] = ratioProp;

        schema["properties"] = props;
        tools.append(makeToolDefinition(
            "caster_set_ducking",
            "Toggle or configure background audio ducking attenuation across all soundboard media.",
            schema
        ));
    }

    // 5. caster_switch_tab
    {
        QJsonObject schema;
        schema["type"] = "object";
        QJsonObject props;

        QJsonObject tabNameProp;
        tabNameProp["type"] = "string";
        tabNameProp["description"] = "Display name of the soundbank tab to switch to (e.g. 'Main Show', 'Commercials').";
        props["tabName"] = tabNameProp;

        QJsonObject tabIndexProp;
        tabIndexProp["type"] = "integer";
        tabIndexProp["description"] = "Zero-based index of the tab to switch to.";
        props["tabIndex"] = tabIndexProp;

        schema["properties"] = props;
        tools.append(makeToolDefinition(
            "caster_switch_tab",
            "Switch the active soundbank tab in the desktop UI and Web Remote.",
            schema
        ));
    }

    // 6. caster_set_volume
    {
        QJsonObject schema;
        schema["type"] = "object";
        QJsonObject props;

        QJsonObject masterVolProp;
        masterVolProp["type"] = "number";
        masterVolProp["description"] = "Master volume level from 0.0 (silent) to 1.0 (full volume).";
        props["masterVolume"] = masterVolProp;

        schema["properties"] = props;
        tools.append(makeToolDefinition(
            "caster_set_volume",
            "Adjust master soundboard output volume level.",
            schema
        ));
    }

    // 7. caster_get_audio_routing
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject();
        tools.append(makeToolDefinition(
            "caster_get_audio_routing",
            "Retrieve PipeWire / PulseAudio virtual routing status, active hardware microphones, monitor endpoints, and virtual mic source name.",
            schema
        ));
    }

    QJsonObject result;
    result["tools"] = tools;
    return makeSuccessResponse(id, result);
}

QJsonObject McpServerController::handleToolsCall(const QJsonObject &id, const QJsonObject &params)
{
    const QString toolName = params.value("name").toString();
    const QJsonObject args = params.value("arguments").toObject();

    m_toolInvocationsCount++;
    m_lastInvokedTool = toolName;
    emit toolInvocationsChanged();

    QJsonObject toolResult;

    if (toolName == "caster_get_state") {
        toolResult = toolGetState(args);
    } else if (toolName == "caster_trigger_sound") {
        toolResult = toolTriggerSound(args);
    } else if (toolName == "caster_stop_all") {
        toolResult = toolStopAll(args);
    } else if (toolName == "caster_set_ducking") {
        toolResult = toolSetDucking(args);
    } else if (toolName == "caster_switch_tab") {
        toolResult = toolSwitchTab(args);
    } else if (toolName == "caster_set_volume") {
        toolResult = toolSetVolume(args);
    } else if (toolName == "caster_get_audio_routing") {
        toolResult = toolGetAudioRouting(args);
    } else {
        return makeErrorResponse(id, -32602, QString("Unknown tool: %1").arg(toolName));
    }

    const bool isError = toolResult.value("isError").toBool();
    QString resultSnippet;
    if (toolResult.contains("content")) {
        const QJsonArray contentArr = toolResult["content"].toArray();
        if (!contentArr.isEmpty()) {
            resultSnippet = contentArr.first().toObject().value("text").toString();
        }
    }
    emit toolExecuted(toolName, resultSnippet, isError);

    return makeSuccessResponse(id, toolResult);
}

QJsonObject McpServerController::toolGetState(const QJsonObject &/*args*/)
{
    if (!m_projectController) {
        return makeToolResult("Error: Soundboard project controller is not available.", true);
    }

    QJsonObject state;
    state["appName"] = "CasterSoundboard";
    state["version"] = "2.0.0";
    state["projectFile"] = m_projectController->filePath();
    state["masterVolume"] = m_projectController->masterVolume();
    state["isDuckingActive"] = m_projectController->isDuckingActive();
    state["duckingVolumeRatio"] = m_projectController->duckingVolumeRatio();

    // PTT info
    auto *actions = m_projectController->actionsController();
    state["isPttActive"] = actions ? actions->isTransmitting() : false;
    state["activeTransmittingKey"] = actions ? actions->activeTransmittingKey() : QString();

    // Tabs & Sound tiles
    auto *tabModel = m_projectController->tabModel();
    state["activeTabIndex"] = tabModel ? tabModel->currentIndex() : 0;

    QJsonArray tabsArr;
    if (tabModel) {
        for (int i = 0; i < tabModel->count(); ++i) {
            const QString tabName = tabModel->tabNameAt(i);
            const QString tabId = tabModel->tabIdAt(i);
            const QString tabHotkey = tabModel->tabHotkeyAt(i);

            QJsonObject tabObj;
            tabObj["index"] = i;
            tabObj["name"] = tabName;
            tabObj["id"] = tabId;
            tabObj["hotkey"] = tabHotkey;
            const bool tabTransmitting = actions ? actions->isTransmittingForTab(tabName) : false;
            tabObj["isTransmitting"] = tabTransmitting;
            tabObj["actionBadge"] = actions ? actions->activeBadgeForTab(tabName) : QString();

            auto *grid = tabModel->gridModelAt(i);
            QJsonArray tilesArr;
            if (grid) {
                for (int t = 0; t < grid->count(); ++t) {
                    auto *tile = grid->tileAt(t);
                    if (!tile) continue;

                    QJsonObject tileObj;
                    tileObj["index"] = t;
                    tileObj["name"] = tile->displayName();
                    tileObj["hasFile"] = !tile->filePath().isEmpty();
                    tileObj["hotkey"] = tile->hotkey();

                    QString stateStr = "Stopped";
                    if (tile->playbackState() == QMediaPlayer::PlayingState) {
                        stateStr = "Playing";
                    } else if (tile->playbackState() == QMediaPlayer::PausedState) {
                        stateStr = "Paused";
                    }
                    tileObj["state"] = stateStr;
                    tileObj["isLooped"] = tile->isLooped();
                    tileObj["isTransmitting"] = (tabTransmitting && tile->playbackState() == QMediaPlayer::PlayingState);
                    tileObj["durationMs"] = static_cast<double>(tile->durationMs());
                    tileObj["positionMs"] = static_cast<double>(tile->positionMs());
                    tileObj["volume"] = tile->volume();

                    tilesArr.append(tileObj);
                }
            }
            tabObj["tiles"] = tilesArr;
            tabsArr.append(tabObj);
        }
    }
    state["tabs"] = tabsArr;

    return makeToolResult(QString::fromUtf8(QJsonDocument(state).toJson(QJsonDocument::Indented)));
}

QJsonObject McpServerController::toolTriggerSound(const QJsonObject &args)
{
    if (!m_projectController) {
        return makeToolResult("Error: Soundboard project controller is not available.", true);
    }

    auto *tabModel = m_projectController->tabModel();
    if (!tabModel || tabModel->count() == 0) {
        return makeToolResult("Error: No soundboard tabs are loaded.", true);
    }

    const QString soundName = args.value("soundName").toString().trimmed();
    SoundTileController *targetTile = nullptr;
    int foundTabIdx = -1;
    int foundTileIdx = -1;
    QString foundTabName;

    // Search by name if provided
    if (!soundName.isEmpty()) {
        const QString lowerSearch = soundName.toLower();
        for (int i = 0; i < tabModel->count() && !targetTile; ++i) {
            auto *grid = tabModel->gridModelAt(i);
            if (!grid) continue;
            for (int t = 0; t < grid->count(); ++t) {
                auto *tile = grid->tileAt(t);
                if (tile && !tile->filePath().isEmpty() && tile->displayName().toLower().contains(lowerSearch)) {
                    targetTile = tile;
                    foundTabIdx = i;
                    foundTileIdx = t;
                    foundTabName = tabModel->tabNameAt(i);
                    break;
                }
            }
        }
    }

    // Fall back to tabIndex and tileIndex
    if (!targetTile && args.contains("tileIndex")) {
        const int tabIdx = args.value("tabIndex").toInt(tabModel->currentIndex());
        const int tileIdx = args.value("tileIndex").toInt();
        if (tabIdx >= 0 && tabIdx < tabModel->count()) {
            auto *grid = tabModel->gridModelAt(tabIdx);
            if (grid && tileIdx >= 0 && tileIdx < grid->count()) {
                targetTile = grid->tileAt(tileIdx);
                foundTabIdx = tabIdx;
                foundTileIdx = tileIdx;
                foundTabName = tabModel->tabNameAt(tabIdx);
            }
        }
    }

    if (!targetTile) {
        return makeToolResult(QString("Error: Could not locate sound tile '%1'.").arg(soundName.isEmpty() ? "by indices" : soundName), true);
    }

    if (targetTile->filePath().isEmpty()) {
        return makeToolResult(QString("Error: Sound slot %1 on tab %2 has no audio file assigned.").arg(foundTileIdx).arg(foundTabIdx), true);
    }

    // Trigger playback
    targetTile->trigger();

    QString newState = "Stopped";
    if (targetTile->playbackState() == QMediaPlayer::PlayingState) {
        newState = "Playing";
    } else if (targetTile->playbackState() == QMediaPlayer::PausedState) {
        newState = "Paused";
    }

    auto *actions = m_projectController->actionsController();
    const bool isTx = actions && actions->isTransmitting();
    const QString txKey = isTx ? actions->activeTransmittingKey() : QString();

    QString msg = QString("Triggered sound '%1' (Tab %2: '%3', Slot %4).\n• Current State: %5\n• Push-to-Talk: %6")
        .arg(targetTile->displayName())
        .arg(foundTabIdx)
        .arg(foundTabName)
        .arg(foundTileIdx)
        .arg(newState)
        .arg(isTx ? QString("Transmitting on %1").arg(txKey) : "Idle");

    return makeToolResult(msg);
}

QJsonObject McpServerController::toolStopAll(const QJsonObject &/*args*/)
{
    if (!m_projectController) {
        return makeToolResult("Error: Soundboard project controller is not available.", true);
    }

    m_projectController->stopAllSounds();
    return makeToolResult("Panic Stop All executed: all active sound playback halted and Push-to-Talk keys released.");
}

QJsonObject McpServerController::toolSetDucking(const QJsonObject &args)
{
    if (!m_projectController) {
        return makeToolResult("Error: Soundboard project controller is not available.", true);
    }

    if (args.contains("duckingRatio")) {
        const qreal ratio = args.value("duckingRatio").toDouble(0.25);
        m_projectController->setDuckingVolumeRatio(qBound(0.05, ratio, 0.75));
    }

    if (args.contains("enabled")) {
        m_projectController->setIsDuckingActive(args.value("enabled").toBool());
    } else {
        m_projectController->toggleAudioDucking();
    }

    const bool isActive = m_projectController->isDuckingActive();
    const qreal ratio = m_projectController->duckingVolumeRatio();
    return makeToolResult(QString("Audio ducking is now %1 (Volume attenuation ratio: %2).")
        .arg(isActive ? "ACTIVE" : "INACTIVE")
        .arg(ratio));
}

QJsonObject McpServerController::toolSwitchTab(const QJsonObject &args)
{
    if (!m_projectController) {
        return makeToolResult("Error: Soundboard project controller is not available.", true);
    }

    auto *tabModel = m_projectController->tabModel();
    if (!tabModel || tabModel->count() == 0) {
        return makeToolResult("Error: No soundboard tabs exist.", true);
    }

    int targetIdx = -1;
    if (args.contains("tabName")) {
        const QString name = args.value("tabName").toString().toLower();
        for (int i = 0; i < tabModel->count(); ++i) {
            if (tabModel->tabNameAt(i).toLower().contains(name)) {
                targetIdx = i;
                break;
            }
        }
    } else if (args.contains("tabIndex")) {
        targetIdx = args.value("tabIndex").toInt();
    }

    if (targetIdx < 0 || targetIdx >= tabModel->count()) {
        return makeToolResult("Error: Target soundboard tab was not found.", true);
    }

    tabModel->setCurrentIndex(targetIdx);
    return makeToolResult(QString("Active soundboard tab switched to '%1' (Index: %2).")
        .arg(tabModel->tabNameAt(targetIdx))
        .arg(targetIdx));
}

QJsonObject McpServerController::toolSetVolume(const QJsonObject &args)
{
    if (!m_projectController) {
        return makeToolResult("Error: Soundboard project controller is not available.", true);
    }

    if (args.contains("masterVolume")) {
        const qreal vol = qBound(0.0, args.value("masterVolume").toDouble(1.0), 1.0);
        m_projectController->setMasterVolume(vol);
        return makeToolResult(QString("Master soundboard volume set to %1%").arg(qRound(vol * 100)));
    }

    return makeToolResult("Error: masterVolume parameter (0.0 to 1.0) is required.", true);
}

QJsonObject McpServerController::toolGetAudioRouting(const QJsonObject &/*args*/)
{
    if (!m_audioRoutingController) {
        return makeToolResult("Error: Audio routing controller is not available.", true);
    }

    QJsonObject routing;
    routing["isRoutingActive"] = m_audioRoutingController->isRouting();
    routing["selectedMicrophone"] = m_audioRoutingController->selectedMicrophone();
    routing["selectedSystemOutput"] = m_audioRoutingController->selectedSystemOutput();
    routing["selectedMonitorOutput"] = m_audioRoutingController->selectedMonitorOutput();
    routing["virtualMicName"] = m_audioRoutingController->virtualMicName();
    routing["mixMicrophone"] = m_audioRoutingController->mixMicrophone();
    routing["monitorSoundboard"] = m_audioRoutingController->monitorSoundboard();
    routing["statusMessage"] = m_audioRoutingController->statusMessage();
    routing["diagnosticsLog"] = m_audioRoutingController->diagnosticsLog();

    return makeToolResult(QString::fromUtf8(QJsonDocument(routing).toJson(QJsonDocument::Indented)));
}

// JSON-RPC Helpers
QJsonObject McpServerController::makeToolDefinition(const QString &name, const QString &description, const QJsonObject &schema)
{
    QJsonObject tool;
    tool["name"] = name;
    tool["description"] = description;
    tool["inputSchema"] = schema;
    return tool;
}

QJsonObject McpServerController::makeSuccessResponse(const QJsonObject &id, const QJsonObject &result)
{
    QJsonObject res;
    res["jsonrpc"] = "2.0";
    if (id.isEmpty()) {
        res["id"] = 1;
    } else {
        res["id"] = id.value("id");
    }
    res["result"] = result;
    return res;
}

QJsonObject McpServerController::makeErrorResponse(const QJsonObject &id, int code, const QString &message)
{
    QJsonObject res;
    res["jsonrpc"] = "2.0";
    if (id.isEmpty()) {
        res["id"] = QJsonValue::Null;
    } else {
        res["id"] = id.value("id");
    }
    QJsonObject err;
    err["code"] = code;
    err["message"] = message;
    res["error"] = err;
    return res;
}

QJsonObject McpServerController::makeToolResult(const QString &text, bool isError)
{
    QJsonObject res;
    QJsonArray contentArr;
    QJsonObject textObj;
    textObj["type"] = "text";
    textObj["text"] = text;
    contentArr.append(textObj);

    res["content"] = contentArr;
    res["isError"] = isError;
    return res;
}
