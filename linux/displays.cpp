#include "displays.h"
#include "json.hpp"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "logger.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include "global.h"
#include "tabpage.h"
#include "waylandmanager.h"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

bool allowInternalDisplays = false;
Displays::Displays() {}

std::string getDisplayServer() {
    const char* session = std::getenv("XDG_SESSION_TYPE");
    return session ? session : "";
}

json getAllX11Displays() {
    json results;
    Display* display = XOpenDisplay(NULL);

    results["status"] = false;
    results["displays"] = std::vector<json>();

    if (!display) {
        Logger::warn("Unable to open display");
        return results;
    }

    Window root = DefaultRootWindow(display);
    XRRScreenResources* resources = XRRGetScreenResources(display, root);

    if (!resources) {
        Logger::warn("Unable to get resources");
        return results;
    }

    for (int i = 0; i < resources->noutput; ++i) {
        json result;
        RROutput output = resources->outputs[i];
        XRRModeInfo mode = resources->modes[i];
        XRROutputInfo* outputInfo = XRRGetOutputInfo(display, resources, output);

        if (!outputInfo || outputInfo->connection == RR_Disconnected) {
            XRRFreeOutputInfo(outputInfo);
            continue;
        }

        std::string name = outputInfo->name;
        result["name"] = name;
        result["crtc"] = false;

        if (outputInfo->crtc) {
            XRRCrtcInfo* crtcInfo = XRRGetCrtcInfo(display, resources, outputInfo->crtc);
            XRRModeInfo* activeMode = nullptr;

            for (int j = 0; j < resources->nmode; ++j) {
                if (resources->modes[j].id == crtcInfo->mode) {
                    activeMode = &resources->modes[j];
                    break;
                }
            }

            if (activeMode) {
                result["refresh"] = (activeMode->dotClock > 0 && activeMode->hTotal > 0 && activeMode->vTotal > 0) ? (double)activeMode->dotClock / ((double)activeMode->hTotal * activeMode->vTotal) : 0.0;
            }

            result["width"] = crtcInfo->width;
            result["height"] = crtcInfo->height;
            result["crtc"] = true;

            if (outputInfo->mm_width > 0 && outputInfo->mm_height > 0) {
                int mm = std::sqrt(outputInfo->mm_width * outputInfo->mm_width + outputInfo->mm_height * outputInfo->mm_height);
                result["length"] = mm;
            }

            XRRFreeCrtcInfo(crtcInfo);
        }

        results["displays"].push_back(result);
        XRRFreeOutputInfo(outputInfo);
    }

    XRRFreeScreenResources(resources);
    XCloseDisplay(display);
    results["status"] = true;
    return results;
}

json getDisplays(std::string server) {
    Logger::print(QString("Getting displays for server %1...").arg(QString::fromStdString(server)));

    if (server == "x11") {
        return getAllX11Displays();
    } else if (server == "wayland") {
        WaylandManager manager = WaylandManager();
        return manager.getAllWaylandDisplays();
    } else {
        return json();
    }
}

QWidget* Displays::page(QWidget* parent) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    std::string server = getDisplayServer();
    json data = getDisplays(server);

    if (!data.empty()) {
        if (data["status"] == true) {
            std::vector<json> displays = data["displays"];
            int length = displays.size();

            if (displays.empty()) {
                QLabel* titleLabel = new QLabel((parent));
                QFont titleFont;
                titleFont.setPointSize(16);
                titleFont.setBold(true);
                titleLabel->setFont(titleFont);
                titleLabel->setText("Whoops!");
                titleLabel->setAlignment(Qt::AlignCenter);

                QLabel* messageLabel = new QLabel(parent);
                messageLabel->setTextFormat(Qt::RichText);
                messageLabel->setText(QString("We couldn't find any displays."));
                messageLabel->setAlignment(Qt::AlignCenter);

                layout->addWidget(titleLabel);
                layout->addWidget(messageLabel);
            } else {
                QHBoxLayout *displayLayout = new QHBoxLayout();

                for (int i = 0; i < length; i++) {
                    QWidget* container = new QWidget;
                    container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                    QVBoxLayout* layout = new QVBoxLayout(container);
                    json display = displays[i];
                    std::string name = display["name"];
                    bool internal = allowInternalDisplays ? display["internal"].get<bool>() : false;
                    bool crtc = display["crtc"];

                    std::string iconPath = Global::getComputerIconPath(internal ? "laptop" : "monitor");
                    double imageSize = 192 / static_cast<double>(length);
                    QWidget* image = LocalTabPage::processImage(iconPath, parent, imageSize);
                    layout->addWidget(image);

                    QLabel* titleLabel = new QLabel(parent);
                    QFont titleFont;
                    titleLabel->setText(QString("%1").arg(QString::fromStdString(name)));
                    titleFont.setPointSize(12);
                    titleLabel->setFont(titleFont);
                    titleLabel->setAlignment(Qt::AlignCenter);
                    layout->addWidget(titleLabel);

                    if (crtc) {
                        QLabel* label = new QLabel(parent);
                        QFont font;
                        label->setText(QString("%1x%2").arg(QString::number(display["width"].get<int>())).arg(QString::number(display["height"].get<int>())));
                        font.setPointSize(8);
                        label->setFont(font);
                        label->setAlignment(Qt::AlignCenter);
                        layout->addWidget(label);
                    }

                    QStringList* message1Text = new QStringList();
                    if (display.contains("length")) message1Text->append(QString("%1").arg(Global::mmToString(display["length"].get<double>())));
                    if (display.contains("refresh")) message1Text->append(QString("%1Hz").arg(QString::number(display["refresh"].get<int>())));

                    if (!message1Text->empty()) {
                        QLabel* message1 = new QLabel(parent);
                        QFont font1;
                        message1->setText(message1Text->join(" "));
                        font1.setPointSize(8);
                        message1->setFont(font1);
                        message1->setAlignment(Qt::AlignCenter);
                        layout->addWidget(message1);
                    }

                    displayLayout->addWidget(container, 1);
                }

                layout->addLayout(displayLayout);
            }
        } else {
            QLabel* titleLabel = new QLabel(parent);
            QFont titleFont;
            titleFont.setPointSize(16);
            titleFont.setBold(true);
            titleLabel->setFont(titleFont);
            titleLabel->setText("Whoops!");
            titleLabel->setAlignment(Qt::AlignCenter);

            QLabel* messageLabel = new QLabel(parent);
            messageLabel->setTextFormat(Qt::RichText);
            messageLabel->setText(QString("We encountered an issue loading your displays."));
            messageLabel->setAlignment(Qt::AlignCenter);

            layout->addWidget(titleLabel);
            layout->addWidget(messageLabel);
        }
    } else {
        QLabel* titleLabel = new QLabel(parent);
        QFont titleFont;
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setText("Whoops!");
        titleLabel->setAlignment(Qt::AlignCenter);

        QLabel* messageLabel = new QLabel(parent);
        messageLabel->setTextFormat(Qt::RichText);
        messageLabel->setText(QString("The <span style='font-weight: bold;'>%1</span> display server is currently not supported.").arg(QString::fromStdString(server)));
        messageLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleLabel);
        layout->addWidget(messageLabel);
        layout->setAlignment(Qt::AlignCenter);
    }

    Logger::print(QString("Generated page for display server: %1").arg(QString::fromStdString(server)));
    page->setLayout(layout);
    return page;
}
