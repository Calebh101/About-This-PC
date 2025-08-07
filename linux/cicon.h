#ifndef CICON_H
#define CICON_H

#include <QIcon>
#include <QObject>

class CIcon : public QObject {
public:
    CIcon(std::string id);
    QIcon* build();

    static CIcon* eye();
    static CIcon* eyeClosed();
    static CIcon* lock();
    static CIcon* lockOpen();
private:
    QIcon* icon;
    std::string file;
    std::string identifier;
    std::string extension;
};

#endif // CICON_H
