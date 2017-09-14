#include "file/fs.h"
#include "common.h"
#include "graphics/screen.h"
#include "theme.h"
#include "utils.h"

struct colour Storage_colour;
struct colour TopScreen_colour;
struct colour TopScreen_min_colour;
struct colour TopScreen_bar_colour;
struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;
struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Options_title_text_colour;
struct colour Settings_colour;
struct colour Settings_title_text_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;

const char * themeConfig =
	"theme = %s\n"
	"colours = %s";
	
const char * coloursConfig =
	"storage = RGBA(%d, %d, %d, 255)\n"
	"topscreen_bar = RGBA(%d, %d, %d, 255)\n"
	"topscreen_text = RGBA(%d, %d, %d, 255)\n"
	"topscreen_text_min = RGBA(%d, %d, %d, 255)\n"
	"bottomscreen_bg = RGBA(%d, %d, %d, 255)\n"
	"bottomscreen_bar = RGBA(%d, %d, %d, 255)\n"
	"bottomscreen_text = RGBA(%d, %d, %d, 255)\n"
	"options_selector = RGBA(%d, %d, %d, 255)\n"
	"options_title_text = RGBA(%d, %d, %d, 255)\n"
	"options_text = RGBA(%d, %d, %d, 255)\n"
	"settings_bg = RGBA(%d, %d, %d, 255)\n"
	"settings_title_text = RGBA(%d, %d, %d, 255)\n"
	"settins_text = RGBA(%d, %d, %d, 255)\n"
	"settings_text_min = RGBA(%d, %d, %d, 255)";

Result saveThemeConfig(char * themePath, char * coloursPath)
{
	Result ret = 0;
	
	char * buf = (char *)malloc(1024);
	snprintf(buf, 1024, themeConfig, themePath, coloursPath);
	
	if (R_FAILED(ret = fsWrite("/3ds/data/3DShell/theme.cfg", buf)))
		return ret;
	
	free(buf);
	return 0;
}
	
static Result loadThemeConfig(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!fileExists(fsArchive, "/3ds/data/3DShell/theme.cfg"))	
		saveThemeConfig("romfs:/res", "/3ds/data/3DShell/themes/default");
	
	if (R_FAILED(ret = fsOpen(&handle, "/3ds/data/3DShell/theme.cfg", FS_OPEN_READ)))
		return ret;
	
	u64 size64 = 0;
	u32 size = 0;
	
	if (R_FAILED(ret = FSFILE_GetSize(handle, &size64)))
		return ret;
		
	size = (u32)size64;
	
	char * buf = (char *)malloc(size + 1);
	u32 bytesread = 0;
	
	if (R_FAILED(ret = FSFILE_Read(handle, &bytesread, 0, (u32 *)buf, size)))
		return ret;
	
	buf[size] = '\0';
	
	sscanf(buf, themeConfig, theme_dir, colour_dir);
	
	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;
	
	free(buf);
	return 0;
}

static Result createFontColours(void)
{
	Result ret = 0;
	
	char * buf = (char *)malloc(1024);
	snprintf(buf, 1024, coloursConfig,  48, 174, 222,
										255, 255, 255,
										0, 0, 0,
										95, 95, 95,
										30, 136, 229,
										25, 118, 210,
										251, 251, 251,
										237, 237, 237,
										32, 32, 32,
										120, 120, 120,
										251, 251, 251,
										30, 136, 229,
										32, 32, 32,
										120, 120, 120);
	
	if (R_FAILED(ret = fsWrite("/3ds/data/3DShell/themes/default/colours.cfg", buf)))
		return ret;
	
	free(buf);
	return 0;
}

static Result loadFontColours(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!fileExists(fsArchive, "/3ds/data/3DShell/themes/default/colours.cfg"))		
		createFontColours();
	
	char colours_cfg[100] = "/colours.cfg";
	strcat(colour_dir, colours_cfg);
	
	if (R_FAILED(ret = fsOpen(&handle, colour_dir, FS_OPEN_READ)))
		return ret;
	
	u64 size64 = 0;
	u32 size = 0;
	
	if (R_FAILED(ret = FSFILE_GetSize(handle, &size64)))
		return ret;
		
	size = (u32)size64;
	
	char * buf = (char *)malloc(size + 1);
	u32 bytesread = 0;
	
	if (R_FAILED(ret = FSFILE_Read(handle, &bytesread, 0, (u32 *)buf, size)))
		return ret;
	
	buf[size] = '\0';
	
	sscanf(buf, coloursConfig, &Storage_colour.r, &Storage_colour.g, &Storage_colour.b,
								&TopScreen_bar_colour.r, &TopScreen_bar_colour.g, &TopScreen_bar_colour.b,
								&TopScreen_colour.r, &TopScreen_colour.g, &TopScreen_colour.b,
								&TopScreen_min_colour.r, &TopScreen_min_colour.g, &TopScreen_min_colour.b,
								&BottomScreen_colour.r, &BottomScreen_colour.g, &BottomScreen_colour.b,
								&BottomScreen_bar_colour.r, &BottomScreen_bar_colour.g, &BottomScreen_bar_colour.b,
								&BottomScreen_text_colour.r, &BottomScreen_text_colour.g, &BottomScreen_text_colour.b,
								&Options_select_colour.r, &Options_select_colour.g, &Options_select_colour.b,
								&Options_title_text_colour.r, &Options_title_text_colour.g, &Options_title_text_colour.b,
								&Options_text_colour.r, &Options_text_colour.g, &Options_text_colour.b,
								&Settings_colour.r, &Settings_colour.g, &Settings_colour.b,
								&Settings_title_text_colour.r, &Settings_title_text_colour.g, &Settings_title_text_colour.b,
								&Settings_text_colour.r, &Settings_text_colour.g, &Settings_text_colour.b,
								&Settings_text_min_colour.r, &Settings_text_min_colour.g, &Settings_text_min_colour.b);
	
	if (R_FAILED(ret = FSFILE_Close(handle)))
		return ret;
	
	free(buf);
	return 0;
}

void replaceAsset(char arr[], char path[], char img_path[], char redirect_path[])
{
	strcpy(arr, path);
	strcat(arr, img_path);
	strcpy(redirect_path, arr);
}

void loadTheme(void)
{
	loadThemeConfig();
	
	char background_res[100] = "/background.png";
	char selector_res[100] = "/selector.png";
	char folder_res[100] = "/folder.png";
	char file_res[100] = "/file.png";
	char audio_res[100] = "/audio.png";
	char app_res[100] = "/app.png";
	char txt_res[100] = "/txt.png";
	char system_res[100] = "/system.png";
	char zip_res[100] = "/zip.png";
	char img_res[100] = "/img.png";
	char check_res[100] = "/check.png";
	char uncheck_res[100] = "/uncheck.png";
	char options_res[100] = "/options.png";
	char properties_res[100] = "/properties.png";
	char deletion_res[100] = "/delete.png";

	replaceAsset(temp_arr, theme_dir, background_res, background_path);
	replaceAsset(temp_arr, theme_dir, selector_res, selector_path);
	replaceAsset(temp_arr, theme_dir, folder_res, folder_path);
	replaceAsset(temp_arr, theme_dir, file_res, file_path);
	replaceAsset(temp_arr, theme_dir, audio_res, audio_path);
	replaceAsset(temp_arr, theme_dir, app_res, app_path);
	replaceAsset(temp_arr, theme_dir, txt_res, txt_path);
	replaceAsset(temp_arr, theme_dir, system_res, system_path);
	replaceAsset(temp_arr, theme_dir, zip_res, zip_path);
	replaceAsset(temp_arr, theme_dir, img_res, img_path);
	replaceAsset(temp_arr, theme_dir, check_res, check_path);
	replaceAsset(temp_arr, theme_dir, uncheck_res, uncheck_path);
	replaceAsset(temp_arr, theme_dir, options_res, options_path);
	replaceAsset(temp_arr, theme_dir, properties_res, properties_path);
	replaceAsset(temp_arr, theme_dir, deletion_res, deletion_path);

	loadFontColours();
}

void reloadTheme(void)
{
	screen_unload_texture(TEXTURE_BACKGROUND);
	screen_unload_texture(TEXTURE_SELECTOR);
	screen_unload_texture(TEXTURE_OPTIONS);
	screen_unload_texture(TEXTURE_PROPERTIES);
	screen_unload_texture(TEXTURE_DELETE);

	screen_unload_texture(TEXTURE_FOLDER_ICON);
	screen_unload_texture(TEXTURE_FILE_ICON);
	screen_unload_texture(TEXTURE_APP_ICON);
	screen_unload_texture(TEXTURE_AUDIO_ICON);
	screen_unload_texture(TEXTURE_IMG_ICON);
	screen_unload_texture(TEXTURE_SYSTEM_ICON);
	screen_unload_texture(TEXTURE_TXT_ICON);
	screen_unload_texture(TEXTURE_ZIP_ICON);

	screen_unload_texture(TEXTURE_CHECK_ICON);
	screen_unload_texture(TEXTURE_UNCHECK_ICON);

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

	screen_load_texture_png(TEXTURE_CHECK_ICON, check_path, true);
	screen_load_texture_png(TEXTURE_UNCHECK_ICON, uncheck_path, true);

	loadFontColours();
}