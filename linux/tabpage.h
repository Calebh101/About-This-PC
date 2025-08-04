#ifndef TABPAGE_H
#define TABPAGE_H
#include <filesystem>
#include <qwidget.h>

namespace fs = std::filesystem;

class LocalTabPage
{
private:
    LocalTabPage();
public:
    static QWidget* overview(QWidget* parent);
    static QWidget* processImage(std::optional<fs::path> path, QWidget* parent, int size, int radius = 0);
    static std::optional<fs::path> getIconPath(std::string id);
    static QStringList bottomText();
};

#endif // TABPAGE_H
