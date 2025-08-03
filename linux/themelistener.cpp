#include "themelistener.h"
#include <QApplication>
#include "logger.h"

ThemeListener::ThemeListener() {
    qApp->installEventFilter(this);
}

ThemeListener* ThemeListener::instance() {
    static ThemeListener listener;
    return &listener;
}

bool ThemeListener::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::PaletteChange) {
        emit themeChanged();
        Logger::print("Signal: theme changed");
    }

    return QObject::eventFilter(obj, event);
}
