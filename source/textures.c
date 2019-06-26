#include "C2D_helper.h"
#include "common.h"
#include "sprites.h"
#include "textures.h"

static C2D_SpriteSheet spritesheet;

void Textures_Load(void) {
	spritesheet = C2D_SpriteSheetLoad("romfs:/res/drawable/sprites.t3x");

	icon_app = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_executable_idx);
	icon_archive = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_compress_idx);
	icon_audio = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_audio_idx);
	icon_dir = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_folder_idx);
	icon_dir_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_folder_dark_idx);
	icon_file = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_default_idx);
	icon_image = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_image_idx);
	icon_text = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_fso_type_text_idx);
	icon_check = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_on_normal_idx);
	icon_check_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_on_normal_dark_idx);
	icon_uncheck = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_off_normal_idx);
	icon_uncheck_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_check_off_normal_dark_idx);
	dialog = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_dialog_idx);
	options_dialog = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_options_dialog_idx);
	properties_dialog = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_properties_dialog_idx);
	dialog_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_dialog_dark_idx);
	options_dialog_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_options_dialog_dark_idx);
	properties_dialog_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_properties_dialog_dark_idx);
	icon_radio_off = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_off_normal_idx);
	icon_radio_on = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_on_normal_idx);
	icon_radio_dark_off = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_off_normal_dark_idx);
	icon_radio_dark_on = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_radio_on_normal_dark_idx);
	icon_toggle_on = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_toggle_on_normal_idx);
	icon_toggle_dark_on = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_toggle_on_normal_dark_idx);
	icon_toggle_off = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_material_light_toggle_off_normal_idx);
	icon_home = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_home_idx);
	icon_home_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_home_dark_idx);
	icon_home_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_home_overlay_idx);
	icon_options = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_filesystem_idx);
	icon_options_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_filesystem_dark_idx);
	icon_options_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_filesystem_overlay_idx);
	icon_settings = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_settings_idx);
	icon_settings_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_settings_dark_idx);
	icon_settings_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_settings_overlay_idx);
	icon_ftp = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_remote_idx);
	icon_ftp_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_remote_dark_idx);
	icon_ftp_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_remote_overlay_idx);
	icon_sd = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_sdcard_idx);
	icon_sd_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_sdcard_dark_idx);
	icon_sd_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_sdcard_overlay_idx);
	icon_secure = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_secure_idx);
	icon_secure_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_secure_dark_idx);
	icon_secure_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_secure_overlay_idx);
	icon_search = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_search_idx);
	icon_updates = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_tab_idx);
	icon_updates_dark = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_tab_dark_idx);
	icon_updates_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_tab_overlay_idx);
	default_artwork_blur = C2D_SpriteSheetGetImage(spritesheet, sprites_default_artwork_blur_idx);
	default_artwork = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_album_lg_idx);
	ic_music_bg_bottom = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_music_bg_bottom_idx);
	btn_play = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_play_idx);
	btn_pause = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_pause_idx);
	btn_rewind = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_rewind_idx);
	btn_forward = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_forward_idx);
	btn_repeat = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_repeat_idx);
	btn_shuffle = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_shuffle_idx);
	btn_repeat_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_repeat_overlay_idx);
	btn_shuffle_overlay = C2D_SpriteSheetGetImage(spritesheet, sprites_btn_playback_shuffle_overlay_idx);
	icon_lock = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_dialog_fs_locked_idx);
	icon_nav_drawer = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_navigation_drawer_idx);
	icon_actions = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_material_light_contextual_action_idx);
	icon_back = C2D_SpriteSheetGetImage(spritesheet, sprites_ic_arrow_back_normal_idx);
	icon_wifi_0 = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_0_idx);
	icon_wifi_1 = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_1_idx);
	icon_wifi_2 = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_2_idx);
	icon_wifi_3 = C2D_SpriteSheetGetImage(spritesheet, sprites_stat_sys_wifi_signal_3_idx);
	battery_20 = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_20_idx);
	battery_20_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_20_charging_idx);
	battery_30 = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_30_idx);
	battery_30_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_30_charging_idx);
	battery_50 = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_50_idx);
	battery_50_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_50_charging_idx);
	battery_60 = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_60_idx);
	battery_60_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_60_charging_idx);
	battery_80 = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_80_idx);
	battery_80_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_80_charging_idx);
	battery_90 = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_90_idx);
	battery_90_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_90_charging_idx);
	battery_full = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_full_idx);
	battery_full_charging = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_full_charging_idx);
	battery_low = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_low_idx);
	battery_unknown = C2D_SpriteSheetGetImage(spritesheet, sprites_battery_unknown_idx);
}

void Textures_Free(void) {
	C2D_SpriteSheetFree(spritesheet);
}
