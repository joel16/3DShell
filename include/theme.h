#ifndef THEME_H
#define THEME_H

#include <3ds.h>

char temp_arr[250], theme_dir[250], colour_dir[250];

char background_path[250], dialog_path[250], options_path[250], properties_path[250], selector_path[250];
char app_path[250], archive_path[250], audio_path[250], img_path[250], file_path[250], folder_path[250], system_path[250], text_path[250];
char check_path[250], uncheck_path[250];

char bottomScreen_path[250], bottomScreen_bar_path[250], bottomScreen_text_path[250], options_select_path[250], options_text_path[250], 
	options_title_text_path[250], settings_colour_path[250], settings_text_path[250], settings_text_min_path[250], settings_title_text_path[250],
	storage_path[250], topScreen_path[250], topScreen_bar_path[250], topScreen_min_path[250];

struct colour 
{
   int	r;
   int	g;
   int	b;
};

Result Theme_SaveConfig(char * themePath, char * coloursPath);
void Theme_Load(void);
void Theme_Reload(void);

#endif