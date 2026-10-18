#ifndef MCPSERVERCONTROLLER_H
#define MCPSERVERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QSet>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPointer>
#include <QUuid>
#include <QMap>

class SoundboardProjectController;
class AudioRoutingController;

class McpServerController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString bindAddress READ bindAddress WRITE setBindAddress NOTIFY bindAddressChanged)
    Q_PROPERTY(int connectedClients READ connectedClients NOTIFY clientsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString localUrl READ localUrl NOTIFY statusMessageChanged)
    Q_PROPERTY(QString sseUrl READ sseUrl NOTIFY statusMessageChanged)
    Q_PROPERTY(int toolInvocationsCount READ toolInvocationsCount NOTIFY toolInvocationsChanged)
    Q_PROPERTY(QString lastInvokedTool READ lastInvokedTool NOTIFY toolInvocationsChanged)

public:
    explicit McpServerController(QObject *parent = nullptr);
    ~McpServerController() override;

    void setProjectController(SoundboardProjectController *project);
    SoundboardProjectController* projectController() const { return m_projectController; }

    void setAudioRoutingController(AudioRoutingController *routing);
    AudioRoutingController* audioRoutingController() const { return m_audioRoutingController; }

    bool isRunning() const { return m_server.isListening(); }

    bool autoStart() const { return m_autoStart; }
    void setAutoStart(bool autoStart);

    int port() const { return m_port; }
    void setPort(int port);

    QString bindAddress() const { return m_bindAddress; }
    void setBindAddress(const QString &addr);

    int connectedClients() const { return m_sseClients.size(); }
    QString statusMessage() const { return m_statusMessage; }
    QString localUrl() const;
    QString sseUrl() const;

    int toolInvocationsCount() const { return m_toolInvocationsCount; }
    QString lastInvokedTool() const { return m_lastInvokedTool; }

public slots:
    bool startServer();
    void stopServer();
    bool toggleServer();

    // Core JSON-RPC 2.0 processor (used by both HTTP/SSE and Stdio)
    QJsonObject handleJsonRpc(const QJsonObject &request);

    // Stdio line processor for CLI --mcp-stdio mode
    void processStdioLine(const QByteArray &line);

signals:
    void runningChanged(bool running);
    void autoStartChanged(bool autoStart);
    void portChanged(int port);
    void bindAddressChanged(const QString &addr);
    void clientsChanged(int count);
    void statusMessageChanged();
    void toolInvocationsChanged();
    void toolExecuted(const QString &toolName, const QString &resultText, bool isError);

private slots:
    void onNewConnection();
    void onSocketDisconnected();
    void onSocketReadyRead();

private:
    void loadSettings();
    void saveSettings();

    void handleHttpRequest(QTcpSocket *socket, const QByteArray &requestData);
    void sendHttpResponse(QTcpSocket *socket, const QString &status, const QByteArray &body = QByteArray(),
                          const QString &contentType = QString("application/json; charset=utf-8"),
                          const QString &extraHeaders = QString());
    void sendSseEvent(QTcpSocket *socket, const QString &eventName, const QByteArray &data);

    // MCP Protocol Dispatchers
    QJsonObject handleInitialize(const QJsonObject &id, const QJsonObject &params);
    QJsonObject handleToolsList(const QJsonObject &id);
    QJsonObject handleToolsCall(const QJsonObject &id, const QJsonObject &params);

    // Tool Implementations
    QJsonObject toolGetState(const QJsonObject &args);
    QJsonObject toolTriggerSound(const QJsonObject &args);
    QJsonObject toolStopAll(const QJsonObject &args);
    QJsonObject toolSetDucking(const QJsonObject &args);
    QJsonObject toolSwitchTab(const QJsonObject &args);
    QJsonObject toolSetVolume(const QJsonObject &args);
    QJsonObject toolGetAudioRouting(const QJsonObject &args);

    // Helper builders
    static QJsonObject makeToolDefinition(const QString &name, const QString &description, const QJsonObject &schema);
    static QJsonObject makeSuccessResponse(const QJsonObject &id, const QJsonObject &result);
    static QJsonObject makeErrorResponse(const QJsonObject &id, int code, const QString &message);
    static QJsonObject makeToolResult(const QString &text, bool isError = false);

    SoundboardProjectController *m_projectController{nullptr};
    AudioRoutingController *m_audioRoutingController{nullptr};

    QTcpServer m_server;
    QSet<QTcpSocket*> m_clients;
    QMap<QString, QPointer<QTcpSocket>> m_sseClients; // sessionId -> socket
    QMap<QPointer<QTcpSocket>, QString> m_clientSessions; // socket -> sessionId

    bool m_autoStart{false};
    int m_port{5055};
    QString m_bindAddress{"127.0.0.1"};
    QString m_statusMessage{"Server stopped"};
    int m_toolInvocationsCount{0};
    QString m_lastInvokedTool;
};

#endif // MCPSERVERCONTROLLER_H
