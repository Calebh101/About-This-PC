#ifndef THEMELISTENER_H
#define THEMELISTENER_H

#include <QObject>

class ThemeListener : public QObject {
    Q_OBJECT

public:
    static ThemeListener* instance();

signals:
    void themeChanged();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    ThemeListener();
};

#endif // THEMELISTENER_H
