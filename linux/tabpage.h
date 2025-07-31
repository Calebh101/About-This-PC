#ifndef TABPAGE_H
#define TABPAGE_H
#include <qwidget.h>

class LocalTabPage
{
private:
    LocalTabPage();
public:
    static QWidget* overview(QWidget* parent);
};

#endif // TABPAGE_H
