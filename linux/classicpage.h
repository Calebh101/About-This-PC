#ifndef CLASSICPAGE_H
#define CLASSICPAGE_H

#include <QWidget>
#include "mainwindow.h"

class ClassicPage
{
public:
    ClassicPage();
    static QWidget* page(MainWindow* parent);
    static QWidget* hspacer();
};

#endif // CLASSICPAGE_H
