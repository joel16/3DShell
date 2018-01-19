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
}

void Free_Textures(void)
{
	pp2d_free_texture(TEXTURE_MUSIC_STATUS);
	pp2d_free_texture(TEXTURE_MUSIC_PAUSE);
	pp2d_free_texture(TEXTURE_MUSIC_PLAY);
	pp2d_free_texture(TEXTURE_MUSIC_BOTTOM_BG);
	pp2d_free_texture(TEXTURE_MUSIC_TOP_BG);

	pp2d_free_texture(TEXTURE_GALLERY_BAR);

	pp2d_free_texture(TEXTURE_WIFI_3);
	pp2d_free_texture(TEXTURE_WIFI_2);
	pp2d_free_texture(TEXTURE_WIFI_1);
	pp2d_free_texture(TEXTURE_WIFI_0);
	pp2d_free_texture(TEXTURE_WIFI_NULL);

	pp2d_free_texture(TEXTURE_BATTERY_CHARGE);
	pp2d_free_texture(TEXTURE_BATTERY_100);
	pp2d_free_texture(TEXTURE_BATTERY_85);
	pp2d_free_texture(TEXTURE_BATTERY_71);
	pp2d_free_texture(TEXTURE_BATTERY_57);
	pp2d_free_texture(TEXTURE_BATTERY_43);
	pp2d_free_texture(TEXTURE_BATTERY_28);
	pp2d_free_texture(TEXTURE_BATTERY_15);
	pp2d_free_texture(TEXTURE_BATTERY_0);

	pp2d_free_texture(TEXTURE_ICON_UNCHECK);
	pp2d_free_texture(TEXTURE_ICON_CHECK);
	
	pp2d_free_texture(TEXTURE_ICON_TEXT);
	pp2d_free_texture(TEXTURE_ICON_SYSTEM);
	pp2d_free_texture(TEXTURE_ICON_IMG);
	pp2d_free_texture(TEXTURE_ICON_FOLDER);
	pp2d_free_texture(TEXTURE_ICON_FILE);
	pp2d_free_texture(TEXTURE_ICON_AUDIO);
	pp2d_free_texture(TEXTURE_ICON_ARCHIVE);
	pp2d_free_texture(TEXTURE_ICON_APP);

	pp2d_free_texture(TEXTURE_SELECTOR);
	pp2d_free_texture(TEXTURE_BACKGROUND);
}