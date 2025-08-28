#ifndef WAYLANDMANAGER_H
#define WAYLANDMANAGER_H

#include "global.h"
#include <wayland-client.h>

class WaylandManager
{
public:
    WaylandManager();

    struct Output {
        std::string name;
        int32_t width = 0;
        int32_t height = 0;
        int32_t refresh = 0;  // Hz
        int32_t scale = 1;
        int32_t x = 0;
        int32_t y = 0;
        bool internal = false;
        int32_t physical_width = 0;
        int32_t physical_height = 0;
    };

    struct RegistryState {
        std::vector<Output*> outputs;
    };

    static void output_geometry(void* data, wl_output* output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char* make, const char* model, int32_t transform) {
        Logger::verbose("Calling output_geometry");
        auto* out = static_cast<Output*>(data);
        out->name = std::string(make) + "-" + std::string(model);
        out->physical_width = physical_width;
        out->physical_height = physical_height;
        out->x = x;
        out->y = y;
        out->internal = (out->name.find("eDP") != std::string::npos || out->name.find("LVDS") != std::string::npos || out->name.find("DSI") != std::string::npos); // This is not guaranteed
    }

    static void output_mode(void* data, wl_output* output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
        Logger::verbose("Calling output_mode");
        auto* out = static_cast<Output*>(data);

        if (flags & WL_OUTPUT_MODE_CURRENT) {
            out->width = width;
            out->height = height;
            out->refresh = refresh / 1000; // Wayland reports mHz
        }
    }

    static void output_done(void* data, wl_output* output) {
        Logger::verbose("Calling output_done");
    }

    static void output_scale(void* data, wl_output* output, int32_t factor) {
        Logger::verbose("Calling output_scale");
        auto* out = static_cast<Output*>(data);
        out->scale = factor;
    }

    constexpr static const wl_output_listener output_listener = {
        output_geometry,
        output_mode,
        output_done,
        output_scale
    };

    static void registry_global(
        void* data, wl_registry* registry,
        uint32_t name, const char* interface,
        uint32_t version)
    {
        Logger::verbose(QString("Calling registry_global on item %1").arg(name));
        auto* state = static_cast<RegistryState*>(data);

        if (std::string(interface) == "wl_output") {
            auto* out = new Output();
            wl_output* wlOut = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, 2));
            wl_output_add_listener(wlOut, &output_listener, out);
            state->outputs.push_back(out);
        }
    }

    static void registry_remove(void* data, wl_registry* registry, uint32_t name) {
        Logger::verbose(QString("Calling registry_remove on item %1").arg(name));
    }

    constexpr static const wl_registry_listener registry_listener = {
        registry_global,
        registry_remove
    };

    json getAllWaylandDisplays() {
        Logger::print("Getting wayland displays...");
        json results;

        results["status"] = false;
        results["displays"] = json::array();

        wl_display* display = wl_display_connect(nullptr);
        Logger::print("Found main Wayland display");

        if (!display) {
            Logger::warn("Unable to connect to Wayland searver!");
            return results;
        }

        wl_registry* registry = wl_display_get_registry(display);
        RegistryState state;

        wl_registry_add_listener(registry, &registry_listener, &state);
        wl_display_roundtrip(display);
        wl_display_roundtrip(display);

        std::sort(state.outputs.begin(), state.outputs.end(), [](Output* a, Output* b) {
            if (a->x != b->x) return a->x < b->x;
            return a->y < b->y;
        });

        for (auto* out : state.outputs) {
            Logger::verbose(QString("Found display: name=%1 width=%2 height=%3 refresh=%4 scale=%5 internal=%6").arg(QString::fromStdString(out->name)).arg(out->width).arg(out->height).arg(out->refresh).arg(out->scale).arg(out->internal ? "true" : "false"));
            json result;

            result["name"] = out->name;
            result["width"] = out->width;
            result["height"] = out->height;
            result["refresh"] = out->refresh;
            result["scale"] = out->scale;
            result["internal"] = out->internal;
            result["crtc"] = true;

            if (out->width > 0 && out->height > 0 && out->physical_width > 0 && out->physical_height > 0) {
                int mm = std::sqrt(out->physical_width * out->physical_width + out->physical_height * out->physical_height);
                result["length"] = mm;
            }

            Logger::verbose(QString("Pushing display %1...").arg(result["name"].get<std::string>()));
            results["displays"].push_back(result);
            delete out;
        }

        Logger::print("Disconnecting main Wayland display...");
        wl_display_disconnect(display);

        results["status"] = true;
        return results;
    }
};

#endif // WAYLANDMANAGER_H
