#ifndef _3D_SHELL_STATUS_BAR_H
#define _3D_SHELL_STATUS_BAR_H

#include <ctime>

#include <config.h>
#include <textures.h>

#include <control.h>
#include <c2d_helper.h>
#include <colours.h>

namespace GUI {

/* Top screen chrome: background bar, clock (left), battery percentage,
 * battery icon and wifi icon (right). */
class StatusBar : public UI::Control {
public:
    StatusBar() {
        bounds_ = UI::Rect{0.0f, 0.0f, 400.0f, 15.0f};
    }

    void Draw() override {
        C2D::Rect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, cfg.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);

        const std::time_t time = std::time(nullptr);
        const std::tm calendar_time = *std::localtime(std::addressof(time));

        static char time_string[30];
        std::snprintf(time_string, 30, "%2i:%02i %s",
            (calendar_time.tm_hour % 12) == 0? 12 : (calendar_time.tm_hour % 12),
            calendar_time.tm_min,
            (calendar_time.tm_hour / 12)? "PM" : "AM");

        float text_height = 0.0f;
        C2D::GetTextSize(0.45f, nullptr, &text_height, time_string);
        C2D::Text(5, bounds_.y + ((15.0f - text_height) / 2.0f), 0.45f, WHITE, time_string);

        u8 level = 0;
        PTMU_GetBatteryLevel(&level);

        u8 percent = 0;
        MCUHWC_GetBatteryLevel(&percent);

        std::string percent_string = std::to_string(percent) + "%";

        float percent_width = 0.0f;
        C2D::GetTextSize(0.45f, &percent_width, nullptr, percent_string.c_str());
        C2D::Text(395 - percent_width, bounds_.y + ((15.0f - text_height) / 2.0f), 0.45f, WHITE, percent_string.c_str());

        C2D::Image(battery_icons[level], 395 - percent_width - battery_icons[level].subtex->width - 5, bounds_.y);
        C2D::Image(wifi_icons[osGetWifiStrength()], 395 - percent_width - wifi_icons[osGetWifiStrength()].subtex->width - 25, bounds_.y + 1);
    }

private:
    static constexpr float BAR_HEIGHT = 15.0f;
};

}

#endif
