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
        bool internal = false;
    };

    // ---- wl_output listener callbacks ----
    static void output_geometry(
        void* data, wl_output* output,
        int32_t x, int32_t y,
        int32_t physical_width, int32_t physical_height,
        int32_t subpixel, const char* make,
        const char* model, int32_t transform)
    {
        auto* out = static_cast<Output*>(data);
        out->name = std::string(make) + "-" + std::string(model);

        // crude detection for laptop panels
        out->internal = (out->name.find("eDP") != std::string::npos ||
                         out->name.find("LVDS") != std::string::npos ||
                         out->name.find("DSI") != std::string::npos);
    }

    static void output_mode(
        void* data, wl_output* output,
        uint32_t flags, int32_t width,
        int32_t height, int32_t refresh)
    {
        auto* out = static_cast<Output*>(data);

        if (flags & WL_OUTPUT_MODE_CURRENT) {
            out->width = width;
            out->height = height;
            out->refresh = refresh / 1000;  // Wayland reports mHz
        }
    }

    static void output_done(void* data, wl_output* output) {
        // no-op
    }

    static void output_scale(void* data, wl_output* output, int32_t factor) {
        auto* out = static_cast<Output*>(data);
        out->scale = factor;
    }

    constexpr static const wl_output_listener output_listener = {
        output_geometry,
        output_mode,
        output_done,
        output_scale
    };

    // ---- registry listener ----
    struct RegistryState {
        std::vector<Output*> outputs;
    };

    static void registry_global(
        void* data, wl_registry* registry,
        uint32_t name, const char* interface,
        uint32_t version)
    {
        auto* state = static_cast<RegistryState*>(data);

        if (std::string(interface) == "wl_output") {
            auto* out = new Output();
            wl_output* wlOut = static_cast<wl_output*>(
                wl_registry_bind(registry, name, &wl_output_interface, 2));
            wl_output_add_listener(wlOut, &output_listener, out);

            state->outputs.push_back(out);
        }
    }

    static void registry_remove(void* data, wl_registry* registry, uint32_t name) {
        // no-op
    }

    constexpr static const wl_registry_listener registry_listener = {
        registry_global,
        registry_remove
    };

    // ---- Main function ----
    json getAllWaylandDisplays() {
        json results;
        results["status"] = false;
        results["displays"] = json::array();

        wl_display* display = wl_display_connect(nullptr);
        if (!display) {
            Logger::warn("Unable to connect to Wayland server!");
            return results;
        }

        wl_registry* registry = wl_display_get_registry(display);
        RegistryState state;

        wl_registry_add_listener(registry, &registry_listener, &state);
        wl_display_roundtrip(display);
        wl_display_roundtrip(display);

        for (auto* out : state.outputs) {
            json result;
            result["name"] = out->name;
            result["width"] = out->width;
            result["height"] = out->height;
            result["refresh"] = out->refresh;
            result["scale"] = out->scale;
            result["internal"] = out->internal;
            results["displays"].push_back(result);
            delete out;
        }

        wl_display_disconnect(display);
        results["status"] = true;
        return results;
    }
};

#endif // WAYLANDMANAGER_H
