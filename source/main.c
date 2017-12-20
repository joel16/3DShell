#include "common.h"
#include "file/dirlist.h"
#include "file/fs.h"
#include "graphics/screen.h"
#include "language.h"
#include "main.h"
#include "menus/menu_gallery.h"
#include "menus/menu_main.h"
#include "menus/menu_music.h"
#include "menus/status_bar.h"
#include "net/net.h"
#include "theme.h"
#include "utils.h"

void initServices(void)
{
	openArchive(&fsArchive, ARCHIVE_SDMC);
	mcuHwcInit();
	ptmuInit();
	cfguInit();
	acInit();
	httpcInit(0);

	gfxInitDefault();
	romfsInit();
	screen_init();

	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	amInit();
	AM_QueryAvailableExternalTitleDatabase(NULL);

	makeDirectories();
	loadConfig();
	getLastDirectory();

	loadTheme();

	screen_load_texture_png(TEXTURE_BACKGROUND, background_path, true);
	screen_load_texture_png(TEXTURE_SELECTOR, selector_path, true);
	screen_load_texture_png(TEXTURE_OPTIONS, options_path, true);
	screen_load_texture_png(TEXTURE_PROPERTIES, properties_path, true);
	screen_load_texture_png(TEXTURE_DELETE, deletion_path, true);

	screen_load_texture_png(TEXTURE_FOLDER_ICON, folder_path, true);
	screen_load_texture_png(TEXTURE_FILE_ICON, file_path, true);
	screen_load_texture_png(TEXTURE_APP_ICON, app_path, true);
	screen_load_texture_png(TEXTURE_AUDIO_ICON, audio_path, true);
	screen_load_texture_png(TEXTURE_IMG_ICON, img_path, true);
	screen_load_texture_png(TEXTURE_SYSTEM_ICON, system_path, true);
	screen_load_texture_png(TEXTURE_TXT_ICON, txt_path, true);
	screen_load_texture_png(TEXTURE_ZIP_ICON, zip_path, true);

	screen_load_texture_png(TEXTURE_HOME_ICON, "romfs:/res/home.png", true);
	screen_load_texture_png(TEXTURE_OPTIONS_ICON, "romfs:/res/options_icon.png", true);
	screen_load_texture_png(TEXTURE_SETTINGS_ICON, "romfs:/res/settings.png", true);
	screen_load_texture_png(TEXTURE_UPDATE_ICON, "romfs:/res/update.png", true);
	screen_load_texture_png(TEXTURE_FTP_ICON, "romfs:/res/ftp.png", true);

	screen_load_texture_png(TEXTURE_HOME_ICON_SELECTED, "romfs:/res/s_home.png", true);
	screen_load_texture_png(TEXTURE_OPTIONS_ICON_SELECTED, "romfs:/res/s_options_icon.png", true);
	screen_load_texture_png(TEXTURE_SETTINGS_ICON_SELECTED, "romfs:/res/s_settings.png", true);
	screen_load_texture_png(TEXTURE_UPDATE_ICON_SELECTED, "romfs:/res/s_update.png", true);
	screen_load_texture_png(TEXTURE_FTP_ICON_SELECTED, "romfs:/res/s_ftp.png", true);

	screen_load_texture_png(TEXTURE_SD_ICON, "romfs:/res/sd.png", true);
	screen_load_texture_png(TEXTURE_NAND_ICON, "romfs:/res/nand.png", true);

	screen_load_texture_png(TEXTURE_SD_ICON_SELECTED, "romfs:/res/s_sd.png", true);
	screen_load_texture_png(TEXTURE_NAND_ICON_SELECTED, "romfs:/res/s_nand.png", true);

	screen_load_texture_png(TEXTURE_TOGGLE_ON, "romfs:/res/toggleOn.png", true);
	screen_load_texture_png(TEXTURE_TOGGLE_OFF, "romfs:/res/toggleOff.png", true);

	screen_load_texture_png(TEXTURE_CHECK_ICON, check_path, true);
	screen_load_texture_png(TEXTURE_UNCHECK_ICON, uncheck_path, true);

	screen_load_texture_png(TEXTURE_SEARCH_ICON, "romfs:/res/search.png", true);

	screen_load_texture_png(TEXTURE_THEME_ICON, "romfs:/res/theme.png", true);

	screen_load_texture_png(TEXTURE_BATTERY_0, "romfs:/res/battery/0.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_15, "romfs:/res/battery/15.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_28, "romfs:/res/battery/28.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_43, "romfs:/res/battery/43.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_57, "romfs:/res/battery/57.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_71, "romfs:/res/battery/71.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_85, "romfs:/res/battery/85.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_100, "romfs:/res/battery/100.png", true);
	screen_load_texture_png(TEXTURE_BATTERY_CHARGE, "romfs:/res/battery/charge.png", true);

	screen_load_texture_png(TEXTURE_WIFI_NULL, "romfs:/res/wifi/stat_sys_wifi_signal_null.png", true);
	screen_load_texture_png(TEXTURE_WIFI_0, "romfs:/res/wifi/stat_sys_wifi_signal_0.png", true);
	screen_load_texture_png(TEXTURE_WIFI_1, "romfs:/res/wifi/stat_sys_wifi_signal_1.png", true);
	screen_load_texture_png(TEXTURE_WIFI_2, "romfs:/res/wifi/stat_sys_wifi_signal_2.png", true);
	screen_load_texture_png(TEXTURE_WIFI_3, "romfs:/res/wifi/stat_sys_wifi_signal_3.png", true);

	screen_load_texture_png(TEXTURE_GALLERY_BAR, "romfs:/res/gallery/bar.png", true);

	screen_load_texture_png(TEXTURE_MUSIC_BOTTOM_BG, "romfs:/res/music/background_bottom.png", true);
	screen_load_texture_png(TEXTURE_MUSIC_TOP_BG, "romfs:/res/music/background_top.png", true);
	screen_load_texture_png(TEXTURE_MUSIC_PLAY, "romfs:/res/music/play.png", true);
	screen_load_texture_png(TEXTURE_MUSIC_PAUSE, "romfs:/res/music/pause.png", true);

	if (isN3DS())
		osSetSpeedupEnable(true);

	language = 1; //getLanguage();

	sprintf(welcomeMsg, "%s %s! %s", lang_welcome[language][0], getUsername(), lang_welcome[language][1]);

	sprintf(currDate, "%s %s %s.", lang_welcome[language][2], getDayOfWeek(0), getMonthOfYear(0));

	DEFAULT_STATE = STATE_HOME;
	BROWSE_STATE = STATE_SD;
}

void termServices(void)
{
	osSetSpeedupEnable(0);

	screen_unload_texture(TEXTURE_MUSIC_PAUSE);
	screen_unload_texture(TEXTURE_MUSIC_PLAY);
	screen_unload_texture(TEXTURE_MUSIC_TOP_BG);
	screen_unload_texture(TEXTURE_MUSIC_BOTTOM_BG);

	screen_unload_texture(TEXTURE_GALLERY_BAR);

	screen_unload_texture(TEXTURE_WIFI_NULL);
	screen_unload_texture(TEXTURE_WIFI_3);
	screen_unload_texture(TEXTURE_WIFI_2);
	screen_unload_texture(TEXTURE_WIFI_1);
	screen_unload_texture(TEXTURE_WIFI_0);

	screen_unload_texture(TEXTURE_BATTERY_CHARGE);
	screen_unload_texture(TEXTURE_BATTERY_100);
	screen_unload_texture(TEXTURE_BATTERY_85);
	screen_unload_texture(TEXTURE_BATTERY_71);
	screen_unload_texture(TEXTURE_BATTERY_57);
	screen_unload_texture(TEXTURE_BATTERY_43);
	screen_unload_texture(TEXTURE_BATTERY_28);
	screen_unload_texture(TEXTURE_BATTERY_15);
	screen_unload_texture(TEXTURE_BATTERY_0);

	screen_unload_texture(TEXTURE_THEME_ICON);

	screen_unload_texture(TEXTURE_SEARCH_ICON);

	screen_unload_texture(TEXTURE_TOGGLE_OFF);
	screen_unload_texture(TEXTURE_TOGGLE_ON);

	screen_unload_texture(TEXTURE_CHECK_ICON);
	screen_unload_texture(TEXTURE_UNCHECK_ICON);

	screen_unload_texture(TEXTURE_HOME_ICON);
	screen_unload_texture(TEXTURE_OPTIONS_ICON);
	screen_unload_texture(TEXTURE_SETTINGS_ICON);
	screen_unload_texture(TEXTURE_UPDATE_ICON);
	screen_unload_texture(TEXTURE_FTP_ICON);

	screen_unload_texture(TEXTURE_HOME_ICON_SELECTED);
	screen_unload_texture(TEXTURE_OPTIONS_ICON_SELECTED);
	screen_unload_texture(TEXTURE_SETTINGS_ICON_SELECTED);
	screen_unload_texture(TEXTURE_UPDATE_ICON_SELECTED);
	screen_unload_texture(TEXTURE_FTP_ICON_SELECTED);

	screen_unload_texture(TEXTURE_SD_ICON);
	screen_unload_texture(TEXTURE_NAND_ICON);

	screen_unload_texture(TEXTURE_SD_ICON_SELECTED);
	screen_unload_texture(TEXTURE_NAND_ICON_SELECTED);

	screen_unload_texture(TEXTURE_FOLDER_ICON);
	screen_unload_texture(TEXTURE_FILE_ICON);
	screen_unload_texture(TEXTURE_APP_ICON);
	screen_unload_texture(TEXTURE_AUDIO_ICON);
	screen_unload_texture(TEXTURE_IMG_ICON);
	screen_unload_texture(TEXTURE_SYSTEM_ICON);
	screen_unload_texture(TEXTURE_TXT_ICON);
	screen_unload_texture(TEXTURE_ZIP_ICON);

	screen_unload_texture(TEXTURE_BACKGROUND);
	screen_unload_texture(TEXTURE_SELECTOR);
	screen_unload_texture(TEXTURE_OPTIONS);
	screen_unload_texture(TEXTURE_PROPERTIES);
	screen_unload_texture(TEXTURE_DELETE);

	amExit();
	ndspExit();
	screen_exit();
	romfsExit();
	gfxExit();
	httpcExit();
	acExit();
	cfguExit();
	ptmuExit();
	mcuHwcExit();
	closeArchive(fsArchive);
}

int main(int argc, char *argv[])
{
	initServices();

	if (setjmp(exitJmp))
	{
		termServices();
		return 0;
	}

	menu_main(CLEAR);

	termServices();

	return 0;
}
