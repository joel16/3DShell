#include "fs.h"
#include "common.h"
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

char * setFileDefaultsChar(char path[], char data[], char var[])
{
	if (!(fileExists(fsArchive, path)))
		writeFile(path, data);
		
	FILE * file;
	file = fopen(path, "r");
	fscanf(file, "%s", var);
	fclose(file);
	
	return var;
}

void replaceAsset(char arr[], char path[], char img_path[], char redirect_path[])
{
	strcpy(arr, path);
	strcat(arr, img_path); 
	strcpy(redirect_path, arr);
}

void createFontColours(char * path, int r, int g, int b)
{	 
	if (!(fileExists(fsArchive, path)))
	{
		char buf[11];
		snprintf(buf, 12, "%d\n%d\n%d", r, g, b);
		writeFile(path, buf);
	}
}

void loadTheme(void)
{	
	strcpy(theme_dir, setFileDefaultsChar("/3ds/3DShell/theme.bin", "romfs:/res", theme_dir));
	strcpy(font_dir, setFileDefaultsChar("/3ds/3DShell/font.bin", "romfs:/font", font_dir));
	strcpy(colour_dir, setFileDefaultsChar("/3ds/3DShell/colours.bin", "/3ds/3DShell", colour_dir));

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
	
	char storage_res[100] = "/colours/storage.txt";
	char topScreen_res[100] = "/colours/topScreen.txt";
	char topScreen_min_res[100] = "/colours/topScreenMin.txt";
	char topScreen_bar_res[100] = "/colours/topScreenBar.txt";
	char bottomScreen_res[100] = "/colours/bottomScreen.txt";
	char bottomScreen_bar_res[100] = "/colours/bottomScreenBar.txt";
	char bottomScreen_text_res[100] = "/colours/bottomScreenText.txt";
	char options_select_res[100] = "/colours/optionsSelect.txt";
	char options_text_res[100] = "/colours/optionsText.txt";
	char options_title_text_res[100] = "/colours/optionsTitleText.txt";
	char settings_colour_res[100] = "/colours/settingsColour.txt";
	char settings_title_text_res[100] = "/colours/settingsTitleText.txt";
	char settings_text_res[100] = "/colours/settingsText.txt";
	char settings_text_min_res[100] = "/colours/settingsTextMin.txt";
	
	char font_res[100] = "/font.ttf";
	
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
	
	replaceAsset(temp_arr, colour_dir, storage_res, storage_path);
	replaceAsset(temp_arr, colour_dir, topScreen_res, topScreen_path);
	replaceAsset(temp_arr, colour_dir, topScreen_min_res, topScreen_min_path);
	replaceAsset(temp_arr, colour_dir, topScreen_bar_res, topScreen_bar_path);
	replaceAsset(temp_arr, colour_dir, bottomScreen_res, bottomScreen_path);
	replaceAsset(temp_arr, colour_dir, bottomScreen_bar_res, bottomScreen_bar_path);
	replaceAsset(temp_arr, colour_dir, bottomScreen_text_res, bottomScreen_text_path);
	replaceAsset(temp_arr, colour_dir, options_select_res, options_select_path);
	replaceAsset(temp_arr, colour_dir, options_text_res, options_text_path);
	replaceAsset(temp_arr, colour_dir, options_title_text_res, options_title_text_path);
	replaceAsset(temp_arr, colour_dir, settings_colour_res, settings_colour_path);
	replaceAsset(temp_arr, colour_dir, settings_title_text_res, settings_title_text_path);
	replaceAsset(temp_arr, colour_dir, settings_text_res, settings_text_path);
	replaceAsset(temp_arr, colour_dir, settings_text_min_res, settings_text_min_path);
	
	replaceAsset(temp_arr, font_dir, font_res, font_path);
	
	FILE * file;
	
	createFontColours(storage_path, 48, 174, 222);
	createFontColours(topScreen_path, 0, 0, 0);
	createFontColours(topScreen_min_path, 95, 95, 95);
	createFontColours(topScreen_bar_path, 255, 255, 255);
	createFontColours(bottomScreen_path, 30, 136, 229);
	createFontColours(bottomScreen_bar_path, 25, 118, 210);
	createFontColours(bottomScreen_text_path, 251, 251, 251);
	createFontColours(options_select_path, 237, 237, 237);
	createFontColours(options_text_path, 120, 120, 120);
	createFontColours(options_title_text_path, 32, 32, 32);
	createFontColours(settings_colour_path, 251, 251, 251);
	createFontColours(settings_title_text_path, 30, 136, 229);
	createFontColours(settings_text_path, 32, 32, 32);
	createFontColours(settings_text_min_path, 120, 120, 120);
	
	file = fopen(storage_path, "r");
	fscanf(file, "%d %d %d", &Storage_colour.r, &Storage_colour.g, &Storage_colour.b);
	fclose(file);
	
	file = fopen(topScreen_path, "r");
	fscanf(file, "%d %d %d", &TopScreen_colour.r, &TopScreen_colour.g, &TopScreen_colour.b);
	fclose(file);
	
	file = fopen(topScreen_min_path, "r");
	fscanf(file, "%d %d %d", &TopScreen_min_colour.r, &TopScreen_min_colour.g, &TopScreen_min_colour.b);
	fclose(file);
	
	file = fopen(topScreen_bar_path, "r");
	fscanf(file, "%d %d %d", &TopScreen_bar_colour.r, &TopScreen_bar_colour.g, &TopScreen_bar_colour.b);
	fclose(file);
	
	file = fopen(bottomScreen_path, "r");
	fscanf(file, "%d %d %d", &BottomScreen_colour.r, &BottomScreen_colour.g, &BottomScreen_colour.b);
	fclose(file);
	
	file = fopen(bottomScreen_bar_path, "r");
	fscanf(file, "%d %d %d", &BottomScreen_bar_colour.r, &BottomScreen_bar_colour.g, &BottomScreen_bar_colour.b);
	fclose(file);
	
	file = fopen(bottomScreen_text_path, "r");
	fscanf(file, "%d %d %d", &BottomScreen_text_colour.r, &BottomScreen_text_colour.g, &BottomScreen_text_colour.b);
	fclose(file);
	
	file = fopen(options_select_path, "r");
	fscanf(file, "%d %d %d", &Options_select_colour.r, &Options_select_colour.g, &Options_select_colour.b);
	fclose(file);
	
	file = fopen(options_text_path, "r");
	fscanf(file, "%d %d %d", &Options_text_colour.r, &Options_text_colour.g, &Options_text_colour.b);
	fclose(file);
	
	file = fopen(options_title_text_path, "r");
	fscanf(file, "%d %d %d", &Options_title_text_colour.r, &Options_title_text_colour.g, &Options_title_text_colour.b);
	fclose(file);
	
	file = fopen(settings_colour_path, "r");
	fscanf(file, "%d %d %d", &Settings_colour.r, &Settings_colour.g, &Settings_colour.b);
	fclose(file);
	
	file = fopen(settings_title_text_path, "r");
	fscanf(file, "%d %d %d", &Settings_title_text_colour.r, &Settings_title_text_colour.g, &Settings_title_text_colour.b);
	fclose(file);
	
	file = fopen(settings_text_path, "r");
	fscanf(file, "%d %d %d", &Settings_text_colour.r, &Settings_text_colour.g, &Settings_text_colour.b);
	fclose(file);
	
	file = fopen(settings_text_min_path, "r");
	fscanf(file, "%d %d %d", &Settings_text_min_colour.r, &Settings_text_min_colour.g, &Settings_text_min_colour.b);
	fclose(file);
}

void reloadTheme(void)
{	
	sftd_free_font(font);
	sftd_free_font(font2);

	sf2d_free_texture(background);
	sf2d_free_texture(selector);
	sf2d_free_texture(folderIcon);
	sf2d_free_texture(options);
	sf2d_free_texture(_properties);
	sf2d_free_texture(deletion);
	sf2d_free_texture(fileIcon);
	sf2d_free_texture(audioIcon);
	sf2d_free_texture(appIcon);
	sf2d_free_texture(txtIcon);
	sf2d_free_texture(systemIcon);
	sf2d_free_texture(zipIcon);
	sf2d_free_texture(imgIcon);
	sf2d_free_texture(uncheck);
	
	background = sfil_load_PNG_file(background_path, SF2D_PLACE_RAM); setBilinearFilter(background);
	options = sfil_load_PNG_file(options_path, SF2D_PLACE_RAM); setBilinearFilter(options);
	_properties = sfil_load_PNG_file(properties_path, SF2D_PLACE_RAM); setBilinearFilter(_properties);
	deletion = sfil_load_PNG_file(deletion_path, SF2D_PLACE_RAM); setBilinearFilter(deletion);
	selector = sfil_load_PNG_file(selector_path, SF2D_PLACE_RAM); setBilinearFilter(selector);
	folderIcon = sfil_load_PNG_file(folder_path, SF2D_PLACE_RAM); setBilinearFilter(folderIcon);
	fileIcon = sfil_load_PNG_file(file_path, SF2D_PLACE_RAM); setBilinearFilter(fileIcon); 
	audioIcon = sfil_load_PNG_file(audio_path, SF2D_PLACE_RAM); setBilinearFilter(audioIcon);
	appIcon = sfil_load_PNG_file(app_path, SF2D_PLACE_RAM); setBilinearFilter(appIcon);
	txtIcon = sfil_load_PNG_file(txt_path, SF2D_PLACE_RAM); setBilinearFilter(txtIcon);
	systemIcon = sfil_load_PNG_file(system_path, SF2D_PLACE_RAM); setBilinearFilter(systemIcon);
	zipIcon = sfil_load_PNG_file(zip_path, SF2D_PLACE_RAM); setBilinearFilter(zipIcon);
	imgIcon = sfil_load_PNG_file(img_path, SF2D_PLACE_RAM); setBilinearFilter(imgIcon);
	check = sfil_load_PNG_file(check_path, SF2D_PLACE_RAM); setBilinearFilter(check);
	uncheck = sfil_load_PNG_file(uncheck_path, SF2D_PLACE_RAM); setBilinearFilter(uncheck);
	
	font = sftd_load_font_file(font_path);
	font2 = sftd_load_font_file(font_path);
	
	FILE * file;
	
	file = fopen(storage_path, "r");
	fscanf(file, "%d %d %d", &Storage_colour.r, &Storage_colour.g, &Storage_colour.b);
	fclose(file);
	
	file = fopen(topScreen_path, "r");
	fscanf(file, "%d %d %d", &TopScreen_colour.r, &TopScreen_colour.g, &TopScreen_colour.b);
	fclose(file);
	
	file = fopen(topScreen_min_path, "r");
	fscanf(file, "%d %d %d", &TopScreen_min_colour.r, &TopScreen_min_colour.g, &TopScreen_min_colour.b);
	fclose(file);
	
	file = fopen(topScreen_bar_path, "r");
	fscanf(file, "%d %d %d", &TopScreen_bar_colour.r, &TopScreen_bar_colour.g, &TopScreen_bar_colour.b);
	fclose(file);
	
	file = fopen(bottomScreen_path, "r");
	fscanf(file, "%d %d %d", &BottomScreen_colour.r, &BottomScreen_colour.g, &BottomScreen_colour.b);
	fclose(file);
	
	file = fopen(bottomScreen_bar_path, "r");
	fscanf(file, "%d %d %d", &BottomScreen_bar_colour.r, &BottomScreen_bar_colour.g, &BottomScreen_bar_colour.b);
	fclose(file);
	
	file = fopen(bottomScreen_text_path, "r");
	fscanf(file, "%d %d %d", &BottomScreen_text_colour.r, &BottomScreen_text_colour.g, &BottomScreen_text_colour.b);
	fclose(file);
	
	file = fopen(options_select_path, "r");
	fscanf(file, "%d %d %d", &Options_select_colour.r, &Options_select_colour.g, &Options_select_colour.b);
	fclose(file);
	
	file = fopen(options_text_path, "r");
	fscanf(file, "%d %d %d", &Options_text_colour.r, &Options_text_colour.g, &Options_text_colour.b);
	fclose(file);
	
	file = fopen(options_title_text_path, "r");
	fscanf(file, "%d %d %d", &Options_title_text_colour.r, &Options_title_text_colour.g, &Options_title_text_colour.b);
	fclose(file);
	
	file = fopen(settings_colour_path, "r");
	fscanf(file, "%d %d %d", &Settings_colour.r, &Settings_colour.g, &Settings_colour.b);
	fclose(file);
	
	file = fopen(settings_title_text_path, "r");
	fscanf(file, "%d %d %d", &Settings_title_text_colour.r, &Settings_title_text_colour.g, &Settings_title_text_colour.b);
	fclose(file);
	
	file = fopen(settings_text_path, "r");
	fscanf(file, "%d %d %d", &Settings_text_colour.r, &Settings_text_colour.g, &Settings_text_colour.b);
	fclose(file);
	
	file = fopen(settings_text_min_path, "r");
	fscanf(file, "%d %d %d", &Settings_text_min_colour.r, &Settings_text_min_colour.g, &Settings_text_min_colour.b);
	fclose(file);
}