#include "cicon.h"
#include <QIcon>
#include <QPalette>
#include <QApplication>

bool isDark() {
    bool status = (qApp->palette().color(QPalette::Window).value() < 128);
    return status;
}

std::string getPath(std::string path) {
    QString file = QString(":/icons/%1-%2").arg(path).arg(isDark() ? "light" : "dark");
    return file.toStdString();
}

QIcon* path(std::string file) {
    QIcon* icon = new QIcon(QString::fromStdString(file));
    return icon;
}

CIcon::CIcon(std::string id) {
    this->file = getPath(id);
    this->identifier = id;
    icon = path(file);
}

QIcon* CIcon::build() {
    return icon;
}

CIcon* CIcon::eye() {return new CIcon("eye");}
CIcon* CIcon::eyeClosed() {return new CIcon("eye-slash");}
CIcon* CIcon::lock() {return new CIcon("lock");}
CIcon* CIcon::lockOpen() {return new CIcon("lock-open");}
