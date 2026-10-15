#ifndef OSCSERVERCONTROLLER_H
#define OSCSERVERCONTROLLER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariantList>
#include <QUdpSocket>

class SoundboardProjectController;

class OscServerController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart NOTIFY autoStartChanged)
    Q_PROPERTY(bool feedbackEnabled READ feedbackEnabled WRITE setFeedbackEnabled NOTIFY feedbackEnabledChanged)
    Q_PROPERTY(int inboundPort READ inboundPort WRITE setInboundPort NOTIFY inboundPortChanged)
    Q_PROPERTY(QString outboundHost READ outboundHost WRITE setOutboundHost NOTIFY outboundHostChanged)
    Q_PROPERTY(int outboundPort READ outboundPort WRITE setOutboundPort NOTIFY outboundPortChanged)
    Q_PROPERTY(int packetsReceived READ packetsReceived NOTIFY packetsReceivedChanged)
    Q_PROPERTY(QString lastMessageAddress READ lastMessageAddress NOTIFY lastMessageChanged)
    Q_PROPERTY(QString lastMessageValue READ lastMessageValue NOTIFY lastMessageChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit OscServerController(QObject *parent = nullptr);
    ~OscServerController() override;

    void setProjectController(SoundboardProjectController *project);

    bool isRunning() const;

    bool autoStart() const { return m_autoStart; }
    void setAutoStart(bool autoStart);

    bool feedbackEnabled() const { return m_feedbackEnabled; }
    void setFeedbackEnabled(bool enabled);

    int inboundPort() const { return m_inboundPort; }
    void setInboundPort(int port);

    QString outboundHost() const { return m_outboundHost; }
    void setOutboundHost(const QString &host);

    int outboundPort() const { return m_outboundPort; }
    void setOutboundPort(int port);

    int packetsReceived() const { return m_packetsReceived; }
    QString lastMessageAddress() const { return m_lastMessageAddress; }
    QString lastMessageValue() const { return m_lastMessageValue; }
    QString statusMessage() const { return m_statusMessage; }

public slots:
    bool startServer();
    void stopServer();
    void toggleServer();
    void syncClients();
    bool sendOscMessage(const QString &address, float value);
    bool sendOscMessage(const QString &address, int value);

signals:
    void runningChanged();
    void autoStartChanged();
    void feedbackEnabledChanged();
    void inboundPortChanged();
    void outboundHostChanged();
    void outboundPortChanged();
    void packetsReceivedChanged();
    void lastMessageChanged();
    void statusMessageChanged();
    void messageReceived(const QString &address, const QVariant &value);

private slots:
    void onReadyRead();

private:
    void processDatagram(const QByteArray &datagram, const QHostAddress &sender, quint16 senderPort);
    void dispatchCommand(const QString &address, const QVariantList &args);
    void connectProjectSignals();
    void loadSettings();
    void saveSettings();

    QUdpSocket *m_socket{nullptr};
    SoundboardProjectController *m_project{nullptr};
    bool m_autoStart{false};
    bool m_feedbackEnabled{true};
    int m_inboundPort{5051};
    QString m_outboundHost{"127.0.0.1"};
    int m_outboundPort{9000};
    int m_packetsReceived{0};
    QString m_lastMessageAddress;
    QString m_lastMessageValue;
    QString m_statusMessage{"Server stopped"};
};

#endif // OSCSERVERCONTROLLER_H
