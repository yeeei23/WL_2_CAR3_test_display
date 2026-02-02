#include "TriggerServer.h"
#include <QDebug>
#include <QStringList>
#include <QtEndian>
#include <cstring>

TriggerServer::TriggerServer(quint16 port, QObject *parent)
    : QObject(parent), m_port(port)
{
    connect(&m_server, &QTcpServer::newConnection, this, &TriggerServer::onNewConnection);
}

bool TriggerServer::start()
{
    /* LocalHost만 쓰려면 QHostAddress::LocalHost. 다른 기기에서 접속 시 QHostAddress::Any */
    if (!m_server.listen(QHostAddress::Any, m_port)) {
        qWarning() << "TriggerServer: listen failed" << m_server.errorString();
        return false;
    }
    return true;
}

void TriggerServer::onNewConnection()
{
    QTcpSocket *socket = m_server.nextPendingConnection();
    if (socket) {
        m_buffers[socket] = QByteArray();
        connect(socket, &QTcpSocket::readyRead, this, &TriggerServer::onReadyRead);
        connect(socket, &QAbstractSocket::disconnected, socket, &QObject::deleteLater);
        connect(socket, &QObject::destroyed, this, [this, socket]() { m_buffers.remove(socket); });
    }
}

void TriggerServer::onReadyRead()
{
    auto *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    m_buffers[socket].append(socket->readAll());

    QByteArray &buf = m_buffers[socket];
    QString direction = QStringLiteral("F");
    int distanceMeters = 0;
    int dangerLevel = 1;
    bool valid = false;
    int consumed = 0;

    /* 바이너리(6바이트): 거리32 + 방향8 + 위험8 bit */
    if (buf.size() >= BINARY_SIZE) {
        const uchar *p = reinterpret_cast<const uchar*>(buf.constData());
        quint32 dist_be;
        memcpy(&dist_be, p, 4);
        int dist = static_cast<int>(qFromBigEndian(dist_be));
        char dir = static_cast<char>(p[4]);
        int danger = static_cast<int>(p[5]);
        if ((dir == 'L' || dir == 'R' || dir == 'F' || dir == 'N') && danger >= 1 && danger <= 3) {
            direction = QString(QChar(dir));
            distanceMeters = dist;
            dangerLevel = danger;
            valid = true;
            consumed = BINARY_SIZE;
        }
    }

    if (!valid && buf.contains('\n')) {
        /* 텍스트 폴백 (기존 호환): 줄바꿈 있는 한 줄 파싱 */
        int idx = buf.indexOf('\n');
        QByteArray data = buf.left(idx).trimmed();
        buf.remove(0, idx + 1);
        consumed = idx + 1;

        if (data == "PLAY" || data == "play") {
            valid = true;
        } else if (data.startsWith("PLAY ") || data.startsWith("play ")) {
            QString rest = QString::fromUtf8(data.mid(5).trimmed());
            QStringList parts = rest.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString d = parts[0].toUpper();
                if (d == QLatin1String("L") || d == QLatin1String("R") || d == QLatin1String("F") || d == QLatin1String("N")) {
                    direction = d;
                    distanceMeters = parts[1].toInt();
                    dangerLevel = qBound(1, parts[2].toInt(), 3);
                } else {
                    distanceMeters = rest.toInt();
                }
            } else {
                distanceMeters = rest.toInt();
            }
            valid = true;
        } else {
            QString line = QString::fromUtf8(data);
            QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
            if (parts.size() >= 3) {
                QString d = parts[0].toUpper();
                if (d == QLatin1String("L") || d == QLatin1String("R") || d == QLatin1String("F") || d == QLatin1String("N")) {
                    direction = d;
                    distanceMeters = parts[1].toInt();
                    dangerLevel = qBound(1, parts[2].toInt(), 3);
                    valid = true;
                }
            }
        }
    }

    /* 줄바꿈 없이 "L 100 1" 형태로만 온 경우: 버퍼 전체를 한 줄로 파싱 시도 */
    if (!valid && buf.size() >= 7 && buf.size() <= 64 && !buf.contains('\n')) {
        QByteArray data = buf.trimmed();
        QString line = QString::fromUtf8(data);
        QStringList parts = line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() >= 3) {
            QString d = parts[0].toUpper();
            if (d == QLatin1String("L") || d == QLatin1String("R") || d == QLatin1String("F") || d == QLatin1String("N")) {
                direction = d;
                distanceMeters = parts[1].toInt();
                dangerLevel = qBound(1, parts[2].toInt(), 3);
                valid = true;
                consumed = buf.size();  /* 한 줄 전체 소비 */
                buf.clear();
            }
        }
    }

    if (valid) {
        if (consumed == BINARY_SIZE)
            buf.remove(0, consumed);
        emit playAlertRequested(direction, distanceMeters, dangerLevel);
        socket->disconnectFromHost();
    } else {
        if (buf.size() > 512)
            buf.clear();
    }
}
