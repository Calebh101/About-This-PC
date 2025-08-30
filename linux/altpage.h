#ifndef ALTPAGE_H
#define ALTPAGE_H

#include <QWidget>
#include "mainwindow.h"

class AlternatePage
{
public:
    AlternatePage();
    static QWidget* page(MainWindow* parent);
    static QWidget* hspacer();
};

#endif // ALTPAGE_H
