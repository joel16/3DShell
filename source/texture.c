#include "pp2d.h"
#include "textures.h"
#include "theme.h"

void Load_Textures(void)
{
	Theme_Load();
	
	pp2d_load_texture_png(TEXTURE_BACKGROUND, background_path);
	pp2d_load_texture_png(TEXTURE_DIALOG, dialog_path);
	pp2d_load_texture_png(TEXTURE_OPTIONS, options_path);
	pp2d_load_texture_png(TEXTURE_PROPERTIES, properties_path);
	pp2d_load_texture_png(TEXTURE_SELECTOR, selector_path);

	pp2d_load_texture_png(TEXTURE_ICON_APP, app_path);
	pp2d_load_texture_png(TEXTURE_ICON_ARCHIVE, archive_path);
	pp2d_load_texture_png(TEXTURE_ICON_AUDIO, audio_path);
	pp2d_load_texture_png(TEXTURE_ICON_FILE, file_path);
	pp2d_load_texture_png(TEXTURE_ICON_FOLDER, folder_path);
	pp2d_load_texture_png(TEXTURE_ICON_IMG, img_path);
	pp2d_load_texture_png(TEXTURE_ICON_SYSTEM, system_path);
	pp2d_load_texture_png(TEXTURE_ICON_TEXT, text_path);

	pp2d_load_texture_png(TEXTURE_ICON_CHECK, check_path);
	pp2d_load_texture_png(TEXTURE_ICON_UNCHECK, uncheck_path);

	pp2d_load_texture_png(TEXTURE_HOME_ICON, "romfs:/res/drawable/ic_material_light_home.png");
	pp2d_load_texture_png(TEXTURE_OPTIONS_ICON, "romfs:/res/drawable/ic_material_light_options.png");
	pp2d_load_texture_png(TEXTURE_SETTINGS_ICON, "romfs:/res/drawable/ic_material_light_settings.png");
	pp2d_load_texture_png(TEXTURE_FTP_ICON, "romfs:/res/drawable/ic_material_light_ftp.png");

	pp2d_load_texture_png(TEXTURE_HOME_ICON_SELECTED, "romfs:/res/drawable/ic_overlay_home.png");
	pp2d_load_texture_png(TEXTURE_OPTIONS_ICON_SELECTED, "romfs:/res/drawable/ic_overlay_options.png");
	pp2d_load_texture_png(TEXTURE_SETTINGS_ICON_SELECTED, "romfs:/res/drawable/ic_overlay_settings.png");
	pp2d_load_texture_png(TEXTURE_FTP_ICON_SELECTED, "romfs:/res/drawable/ic_overlay_ftp.png");

	pp2d_load_texture_png(TEXTURE_SD_ICON, "romfs:/res/drawable/ic_material_light_sd.png");
	pp2d_load_texture_png(TEXTURE_NAND_ICON, "romfs:/res/drawable/ic_material_light_nand.png");
	pp2d_load_texture_png(TEXTURE_SD_ICON_SELECTED, "romfs:/res/drawable/ic_overlay_sd.png");
	pp2d_load_texture_png(TEXTURE_NAND_ICON_SELECTED, "romfs:/res/drawable/ic_overlay_nand.png");

	pp2d_load_texture_png(TEXTURE_TOGGLE_ON, "romfs:/res/drawable/btn_material_toggle_on.png");
	pp2d_load_texture_png(TEXTURE_TOGGLE_OFF, "romfs:/res/drawable/btn_material_toggle_off.png");
	pp2d_load_texture_png(TEXTURE_RADIO_ON, "romfs:/res/drawable/btn_material_radio_on.png");
	pp2d_load_texture_png(TEXTURE_RADIO_OFF, "romfs:/res/drawable/btn_material_radio_off.png");

	pp2d_load_texture_png(TEXTURE_THEMES_LAUNCHER, "romfs:/res/drawable/ic_launcher_themes.png");

	pp2d_load_texture_png(TEXTURE_SEARCH_ICON, "romfs:/res/drawable/ic_material_light_search.png");

	pp2d_load_texture_png(TEXTURE_BATTERY_0, "romfs:/res/drawable/battery/0.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_15, "romfs:/res/drawable/battery/15.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_28, "romfs:/res/drawable/battery/28.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_43, "romfs:/res/drawable/battery/43.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_57, "romfs:/res/drawable/battery/57.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_71, "romfs:/res/drawable/battery/71.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_85, "romfs:/res/drawable/battery/85.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_100, "romfs:/res/drawable/battery/100.png");
	pp2d_load_texture_png(TEXTURE_BATTERY_CHARGE, "romfs:/res/drawable/battery/charge.png");

	pp2d_load_texture_png(TEXTURE_WIFI_NULL, "romfs:/res/drawable/wifi/stat_sys_wifi_signal_null.png");
	pp2d_load_texture_png(TEXTURE_WIFI_0, "romfs:/res/drawable/wifi/stat_sys_wifi_signal_0.png");
	pp2d_load_texture_png(TEXTURE_WIFI_1, "romfs:/res/drawable/wifi/stat_sys_wifi_signal_1.png");
	pp2d_load_texture_png(TEXTURE_WIFI_2, "romfs:/res/drawable/wifi/stat_sys_wifi_signal_2.png");
	pp2d_load_texture_png(TEXTURE_WIFI_3, "romfs:/res/drawable/wifi/stat_sys_wifi_signal_3.png");

	pp2d_load_texture_png(TEXTURE_GALLERY_BAR, "romfs:/res/drawable/gallery/ic_gallery_bar.png");

	pp2d_load_texture_png(TEXTURE_MUSIC_TOP_BG, "romfs:/res/drawable/music/ic_music_bg_top.png");
	pp2d_load_texture_png(TEXTURE_MUSIC_BOTTOM_BG, "romfs:/res/drawable/music/ic_music_bg_bottom.png");
	pp2d_load_texture_png(TEXTURE_MUSIC_PLAY, "romfs:/res/drawable/music/ic_music_play.png");
	pp2d_load_texture_png(TEXTURE_MUSIC_PAUSE, "romfs:/res/drawable/music/ic_music_pause.png");
	pp2d_load_texture_png(TEXTURE_MUSIC_STATUS, "romfs:/res/drawable/music/ic_music_status.png");

	pp2d_load_texture_png(TEXTURE_TEXTVIEWER_BG, "romfs:/res/drawable/textviewer/textview.png");
}