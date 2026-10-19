#ifndef _3D_SHELL_APPS_H
#define _3D_SHELL_APPS_H

#include <3ds.h>
#include <functional>
#include <memory>
#include <vector>

#include "panel_app.h"
#include "screen.h"
#include "textures.h"

#include "file_explorer_screen.h"
#include "net_loader_panel.h"
#include "settings_panel.h"

/* System application registry: the app drawer is built from this list.
 * Adding an application to the system = one entry here. */
namespace Apps {

    enum Id {
        AppExplorer,
        AppNetLoader,
        AppSettings
    };

    struct Entry {
        const char *name;
        C2D_Image icon;
        Id id;
        /* Exactly one of these: fullscreen screen app vs bottom-panel app. */
        std::function<std::unique_ptr<UI::Screen>()> create_screen;
        std::function<std::unique_ptr<GUI::PanelApp>()> create_panel;

        bool is_fullscreen() const { return (bool)create_screen; }
    };

    inline const std::vector<Entry> &List(void) {
        static const std::vector<Entry> list = {
            { "File Explorer", icon_dir, AppExplorer,
              []() -> std::unique_ptr<UI::Screen> {
                  auto screen = std::make_unique<GUI::FileExplorerScreen>();
                  if (R_FAILED(screen->Init()))
                      return nullptr;
                  return screen;
              },
              nullptr },
            { "NetLoader", icon_ftp, AppNetLoader, nullptr,
              []() -> std::unique_ptr<GUI::PanelApp> {
                  return std::make_unique<GUI::NetLoaderPanel>();
              } },
            { "Settings", icon_settings, AppSettings, nullptr,
              []() -> std::unique_ptr<GUI::PanelApp> {
                  return std::make_unique<GUI::SettingsPanel>();
              } }
        };
        return list;
    }
}

#endif
