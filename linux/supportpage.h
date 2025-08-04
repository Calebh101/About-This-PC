#ifndef SUPPORTPAGE_H
#define SUPPORTPAGE_H

#include <QWidget>
#include "global.h"

class SupportPage
{
public:
    SupportPage();
    static QWidget* page(QWidget* parent, ordered_json urls = Global::getSupportUrls());
};

#endif // SUPPORTPAGE_H
