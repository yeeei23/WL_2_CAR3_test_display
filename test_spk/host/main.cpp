
#ifdef QT_OPENGL_ES_3
#undef QT_OPENGL_ES_3
#endif
#ifdef QT_OPENGL_ES_3_1
#undef QT_OPENGL_ES_3_1
#endif
#ifdef QT_OPENGL_ES_3_2
#undef QT_OPENGL_ES_3_2
#endif


#include <QGuiApplication>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QScreen>
#include <QWindow>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QtCore/Qt>
#include "TriggerServer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const quint16 triggerPort = 38474;  /* Qt UI 전용; 소리는 sound_trigger(38473) */
    TriggerServer triggerServer(triggerPort);
    if (!triggerServer.start()) {
        return 1;
    }

    QString qmlPath = argc > 1 ? QString::fromUtf8(argv[1]) : (QDir::currentPath() + QStringLiteral("/test_spk.qml"));
    if (!QFileInfo::exists(qmlPath)) {
        qWarning() << "QML not found:" << qmlPath;
        return 1;
    }

    QQuickView view;
    view.setFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);  /* 타이틀 없음, 최상단 유지 */
    view.engine()->rootContext()->setContextProperty("triggerHelper", &triggerServer);
    view.setSource(QUrl::fromLocalFile(qmlPath));
    view.setResizeMode(QQuickView::SizeViewToRootObject);
    /* Yocto/Weston: 상단바가 보이면 QT_QPA_PLATFORM=eglfs 로 실행하거나,
     * /etc/xdg/weston/weston.ini 에 [shell] panel-position=none 설정.
     * showFullScreen() 전에 화면 크기로 맞춰 두면 일부 환경에서 도움 됨. */
    QScreen *screen = app.primaryScreen();
    if (screen)
        view.setGeometry(screen->geometry());
    view.showFullScreen();

    return app.exec();
}
