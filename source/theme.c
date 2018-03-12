#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "common.h"
#include "pp2d.h"
#include "textures.h"
#include "theme.h"
#include "utils.h"

struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;
struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Options_title_text_colour;
struct colour Settings_colour;
struct colour Settings_text_colour;
struct colour Settings_text_min_colour;
struct colour Settings_title_text_colour;
struct colour Storage_colour;
struct colour TopScreen_colour;
struct colour TopScreen_bar_colour;
struct colour TopScreen_min_colour;

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

Result Theme_SaveConfig(char * themePath, char * coloursPath)
{
	Result ret = 0;
	
	char * buf = (char *)malloc(512);
	snprintf(buf, 512, themeConfig, themePath, coloursPath);
	
	if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/theme.cfg", buf)))
	{
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}
	
static Result Theme_LoadConfig(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/theme.cfg"))	
		Theme_SaveConfig("romfs:/res/drawable", "/3ds/3DShell/themes/default");
	
	u64 size64 = 0;
	u32 size = 0;
	
	size64 = FS_GetFileSize(archive, "/3ds/3DShell/theme.cfg");
	size = (u32)size64;
	char * buf = (char *)malloc(size + 1);

	if (R_FAILED(ret = FS_Read(archive, "/3ds/3DShell/theme.cfg", size, buf)))
	{
		free(buf);
		return ret;
	}
	
	buf[size] = '\0';
	
	sscanf(buf, themeConfig, theme_dir, colour_dir);

	free(buf);
	return 0;
}

static Result Theme_CreateFontColours(void)
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
	
	if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/themes/default/colours.cfg", buf)))
	{
		free(buf);
		return ret;
	}
	
	free(buf);
	return 0;
}

static Result Theme_LoadFontColours(void)
{
	Handle handle;
	Result ret = 0;
	
	if (!FS_FileExists(archive, "/3ds/3DShell/themes/default/colours.cfg"))		
		Theme_CreateFontColours();
	
	char colours_cfg[100] = "/colours.cfg";
	strcat(colour_dir, colours_cfg);
	
	u64 size64 = 0;
	u32 size = 0;
	
	size64 = FS_GetFileSize(archive, colour_dir);
	size = (u32)size64;
	char * buf = (char *)malloc(size + 1);

	if (R_FAILED(ret = FS_Read(archive, colour_dir, size, buf)))
	{
		free(buf);
		return ret;
	}
	
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
	
	free(buf);
	return 0;
}

static void Theme_ReplaceAsset(char arr[], char path[], char img_path[], char redirect_path[])
{
	strcpy(arr, path);
	strcat(arr, img_path);
	strcpy(redirect_path, arr);
}

void Theme_Load(void)
{
	Theme_LoadConfig();
	
	char background_res[100] = "/ic_background.png";
	char dialog_res[100] = "/ic_dialog.png";
	char options_res[100] = "/ic_options.png";
	char properties_res[100] = "/ic_properties.png";
	char selector_res[100] = "/ic_selector.png";

	char app_res[100] = "/ic_fso_type_app.png";
	char archive_res[100] = "/ic_fso_type_archive.png";
	char audio_res[100] = "/ic_fso_type_audio.png";
	char file_res[100] = "/ic_fso_type_file.png";
	char folder_res[100] = "/ic_fso_type_folder.png";
	char img_res[100] = "/ic_fso_type_img.png";
	char system_res[100] = "/ic_fso_type_system.png";
	char text_res[100] = "/ic_fso_type_text.png";

	char check_res[100] = "/btn_material_check_on.png";
	char uncheck_res[100] = "/btn_material_check_off.png";

	Theme_ReplaceAsset(temp_arr, theme_dir, background_res, background_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, dialog_res, dialog_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, options_res, options_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, properties_res, properties_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, selector_res, selector_path);
	
	Theme_ReplaceAsset(temp_arr, theme_dir, app_res, app_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, archive_res, archive_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, audio_res, audio_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, file_res, file_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, folder_res, folder_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, img_res, img_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, system_res, system_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, text_res, text_path);

	Theme_ReplaceAsset(temp_arr, theme_dir, check_res, check_path);
	Theme_ReplaceAsset(temp_arr, theme_dir, uncheck_res, uncheck_path);

	Theme_LoadFontColours();
}

void Theme_Reload(void)
{
	pp2d_free_texture(TEXTURE_BACKGROUND);
	pp2d_free_texture(TEXTURE_DIALOG);
	pp2d_free_texture(TEXTURE_OPTIONS);
	pp2d_free_texture(TEXTURE_PROPERTIES);
	pp2d_free_texture(TEXTURE_SELECTOR);

	pp2d_free_texture(TEXTURE_ICON_APP);
	pp2d_free_texture(TEXTURE_ICON_ARCHIVE);
	pp2d_free_texture(TEXTURE_ICON_AUDIO);
	pp2d_free_texture(TEXTURE_ICON_FILE);
	pp2d_free_texture(TEXTURE_ICON_FOLDER);
	pp2d_free_texture(TEXTURE_ICON_IMG);
	pp2d_free_texture(TEXTURE_ICON_SYSTEM);
	pp2d_free_texture(TEXTURE_ICON_TEXT);

	pp2d_free_texture(TEXTURE_ICON_CHECK);
	pp2d_free_texture(TEXTURE_ICON_UNCHECK);

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

	Theme_LoadFontColours();
}