#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>

class UpdateManager : public QObject
{
    Q_OBJECT
    QNetworkAccessManager* manager;
public:
    UpdateManager();
    void check(bool gui = true, bool implicit = true);
};

class Version : public QObject
{
public:
    Version(int major, int minor, int subminor, int patch, int release);
    static Version parse(QString input);

    bool operator ==(const Version& o) const {
        return (aa == o.aa && ab == o.ab && ac == o.ac && ba == o.ba && ca == o.ca);
    }

    bool operator<(const Version& o) const {
        if (aa != o.aa) return aa < o.aa;
        if (ab != o.ab) return ab < o.ab;
        if (ac != o.ac) return ac < o.ac;
        if (ba != o.ba) return ba < o.ba;
        return ca < o.ca;
    }

    bool operator<=(const Version& o) const {
        return *this < o || *this == o;
    }
private:
    int aa;
    int ab;
    int ac;
    int ba;
    int ca;
};

#endif // UPDATEMANAGER_H
