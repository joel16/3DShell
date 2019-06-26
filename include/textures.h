#ifndef _3D_SHELL_TEXTURES_H
#define _3D_SHELL_TEXTURES_H

#include <citro2d.h>

C2D_Image icon_app, icon_archive, icon_audio, icon_dir, icon_file, icon_image, icon_text, icon_dir_dark, \
	icon_check, icon_uncheck, icon_check_dark, icon_uncheck_dark, icon_radio_off, icon_radio_on, \
	icon_radio_dark_off, icon_radio_dark_on, icon_toggle_on, icon_toggle_dark_on, icon_toggle_off, \
	dialog, options_dialog, properties_dialog, dialog_dark, options_dialog_dark, properties_dialog_dark, \
	icon_home, icon_home_dark, icon_home_overlay, icon_options, icon_options_dark, icon_options_overlay, \
	icon_settings, icon_settings_dark, icon_settings_overlay, icon_ftp, icon_ftp_dark, icon_ftp_overlay, \
	icon_sd, icon_sd_dark, icon_sd_overlay, icon_secure, icon_secure_dark, icon_secure_overlay, icon_search, \
	icon_updates, icon_updates_dark, icon_updates_overlay, \
	default_artwork, default_artwork_blur, ic_music_bg_bottom, btn_play, btn_pause, btn_rewind, btn_forward, \
	btn_repeat, btn_shuffle, btn_repeat_overlay, btn_shuffle_overlay, icon_lock, \
	icon_nav_drawer, icon_actions, icon_back, \
	icon_wifi_0, icon_wifi_1, icon_wifi_2, icon_wifi_3, \
	battery_20, battery_20_charging, battery_30, battery_30_charging, battery_50, battery_50_charging, \
	battery_60, battery_60_charging, battery_80, battery_80_charging, battery_90, battery_90_charging, \
	battery_full, battery_full_charging, battery_low, battery_unknown;

void Textures_Load(void);
void Textures_Free(void);

#endif
