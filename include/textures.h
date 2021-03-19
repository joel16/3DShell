#ifndef _3D_SHELL_TEXTURES_H
#define _3D_SHELL_TEXTURES_H

#include <citro2d.h>
#include <string>
#include <vector>

constexpr int NUM_ICONS = 5;

extern C2D_Image file_icons[NUM_ICONS], icon_dir, icon_dir_dark, wifi_icons[4], \
	battery_icons[6], battery_icons_charging[6], icon_check, icon_uncheck, icon_check_dark, icon_uncheck_dark, \
	icon_radio_off, icon_radio_on, icon_radio_dark_off, icon_radio_dark_on, icon_toggle_on, icon_toggle_dark_on, \
	icon_toggle_off, dialog, options_dialog, properties_dialog, dialog_dark, options_dialog_dark, properties_dialog_dark, \
	icon_home, icon_home_dark, icon_home_overlay, icon_options, icon_options_dark, icon_options_overlay, \
	icon_settings, icon_settings_dark, icon_settings_overlay, icon_ftp, icon_ftp_dark, icon_ftp_overlay, \
	icon_sd, icon_sd_dark, icon_sd_overlay, icon_secure, icon_secure_dark, icon_secure_overlay, icon_search, \
	icon_nav_drawer, icon_actions, icon_back;

namespace Textures {
	void Init(void);
	void Exit(void);
	bool LoadImageFile(const std::string &path, C2D_Image *texture);
}

#endif
