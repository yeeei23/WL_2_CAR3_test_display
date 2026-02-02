#ifndef TRIGGERSERVER_H
#define TRIGGERSERVER_H

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class TriggerServer : public QObject
{
    Q_OBJECT
public:
    explicit TriggerServer(quint16 port, QObject *parent = nullptr);
    bool start();

signals:
    /** direction: "L"/"R"/"F", distanceMeters: 거리(m), dangerLevel: 1~3 (1,2=노랑, 3=빨강) */
    void playAlertRequested(const QString &direction, int distanceMeters, int dangerLevel);

private slots:
    void onNewConnection();
    void onReadyRead();

private:
    static constexpr int BINARY_SIZE = 6;  /* 거리32 + 방향8 + 위험8 bit */

    QTcpServer m_server;
    quint16 m_port;
    QHash<QTcpSocket*, QByteArray> m_buffers;
};

#endif // TRIGGERSERVER_H
