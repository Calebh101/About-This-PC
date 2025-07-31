#ifndef TABPAGE_H
#define TABPAGE_H
#include <qwidget.h>

class LocalTabPage
{
private:
    LocalTabPage();
public:
    static QWidget* overview(QWidget* parent);
    static QWidget* displays(QWidget* parent);
    static QWidget* storage(QWidget* parent);
};

#endif // TABPAGE_H
