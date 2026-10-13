#ifndef WEBREMOTE_H
#define WEBREMOTE_H
#include <QObject>
#include <QSet>
#include <QTcpServer>
#include <QHostAddress>
class QMediaPlayer;
class QTabWidget;
class QTcpSocket;

// Deliberately small HTTP/SSE server; all access stays on the GUI event loop.
class WebRemote : public QObject
{
    Q_OBJECT
public:
    explicit WebRemote(QTabWidget *tabs, QObject *parent = nullptr);
    ~WebRemote() override;
    bool start(quint16 port, const QHostAddress &address, const QString &user, const QString &password);
    void stop();
    bool isRunning() const { return server.isListening(); }
    QString errorString() const { return startError; }
signals:
    void runningChanged(bool running);
private:
    QByteArray snapshot();
    void acceptConnections();
    void readRequest(QTcpSocket *socket);
    void broadcast();
    QTabWidget *tabs;
    QTcpServer server;
    QSet<QTcpSocket *> clients;
    QSet<QTcpSocket *> streams;
    QSet<QMediaPlayer *> watchedPlayers;
    QString startError;
    QByteArray credentialsHash;
    QByteArray previous;
    int ticks = 0;
};
#endif
