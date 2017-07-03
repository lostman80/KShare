#include "screenshotutil.hpp"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <platformbackend.hpp>

QPixmap screenshotutil::fullscreen(bool cursor) {
    QPixmap image;

    // Hack for https://bugreports.qt.io/browse/QTBUG-58110
    static QStringList qVer = QString(qVersion()).split('.');
    if (qVer.at(0).toInt() == 5 && qVer.at(1).toInt() < 9) {
        int height = 0, width = 0;
        for (QScreen *screen : QApplication::screens()) {
            QRect geo = screen->geometry();
            width = qMax(geo.left() + geo.width(), width);
            height = qMax(geo.top() + geo.height(), height);
        }
        image = QPixmap(width, height);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        width = 0;

        for (QScreen *screen : QApplication::screens()) {
            QPixmap currentScreen = window(0, screen);
            painter.drawPixmap(screen->geometry().topLeft(), currentScreen);
            width += screen->size().width();
        }
    } else
        image = window(0);
#ifdef PLATFORM_CAPABILITY_CURSOR
    if (cursor) {
        auto cursorData = PlatformBackend::inst().getCursor();
        painter.drawPixmap(QCursor::pos() - std::get<0>(cursorData), std::get<1>(cursorData));
    }
#endif
    painter.end();
    return image;
}

QPixmap screenshotutil::window(WId wid, QScreen *w) {
    return w->grabWindow(wid);
}

void screenshotutil::toClipboard(QString value) {
    QApplication::clipboard()->setText(value);
}

QPixmap screenshotutil::fullscreenArea(bool cursor, qreal x, qreal y, qreal w, qreal h) {
    return fullscreen(cursor).copy(x, y, w, h);
}
