#ifndef WEBREMOTECONTROLLER_H
#define WEBREMOTECONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QSet>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>

class SoundboardProjectController;
class SoundTileController;

class WebRemoteController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString bindAddress READ bindAddress WRITE setBindAddress NOTIFY bindAddressChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY credentialsChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY credentialsChanged)
    Q_PROPERTY(int connectedClients READ connectedClients NOTIFY clientsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString localUrl READ localUrl NOTIFY statusMessageChanged)
    Q_PROPERTY(QStringList availableUrls READ availableUrls NOTIFY statusMessageChanged)

public:
    explicit WebRemoteController(QObject *parent = nullptr);
    ~WebRemoteController() override;

    void setProjectController(SoundboardProjectController *project);
    SoundboardProjectController* projectController() const { return m_projectController; }

    bool isRunning() const { return m_server.isListening(); }

    bool autoStart() const { return m_autoStart; }
    void setAutoStart(bool autoStart);

    int port() const { return m_port; }
    void setPort(int port);

    QString bindAddress() const { return m_bindAddress; }
    void setBindAddress(const QString &addr);

    QString username() const { return m_username; }
    void setUsername(const QString &user);

    QString password() const { return m_password; }
    void setPassword(const QString &pass);

    int connectedClients() const { return m_streams.size(); }
    QString statusMessage() const { return m_statusMessage; }

    QString localUrl() const;
    QStringList availableUrls() const;

    Q_INVOKABLE QStringList availableIpAddresses() const;

public slots:
    bool startServer();
    void stopServer();
    bool toggleServer();
    void broadcastState();

signals:
    void runningChanged(bool running);
    void autoStartChanged(bool autoStart);
    void portChanged(int port);
    void bindAddressChanged(const QString &addr);
    void credentialsChanged();
    void clientsChanged(int count);
    void statusMessageChanged();

private slots:
    void onNewConnection();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onHeartbeat();

private:
    void loadSettings();
    void saveSettings();
    void updateCredentialsHash();
    void handleRequest(QTcpSocket *socket);
    void sendResponse(QTcpSocket *socket, const QString &status, const QByteArray &body = QByteArray(),
                      const QString &contentType = QString("text/plain; charset=utf-8"),
                      const QString &extraHeaders = QString());
    QByteArray generateStateJson() const;
    void connectTileSignals();

    SoundboardProjectController *m_projectController{nullptr};
    QTcpServer m_server;
    QSet<QTcpSocket*> m_clients;
    QSet<QTcpSocket*> m_streams;
    QSet<SoundTileController*> m_monitoredTiles;

    bool m_autoStart{false};
    int m_port{5052};
    QString m_bindAddress{"0.0.0.0"};
    QString m_username{"caster"};
    QString m_password{"caster"};
    QByteArray m_credentialsHash;
    QString m_statusMessage;
    QByteArray m_previousState;

    QTimer *m_heartbeatTimer{nullptr};
    QByteArray m_cachedIndexHtml;
};

#endif // WEBREMOTECONTROLLER_H
