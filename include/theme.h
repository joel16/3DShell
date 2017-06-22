#ifndef THEME_H
#define THEME_H

char temp_arr[250], theme_dir[250], font_dir[250], colour_dir[250];

char background_path[250], selector_path[250], folder_path[250], file_path[250], audio_path[250], app_path[250],
	txt_path[250], system_path[250], zip_path[250], img_path[250], options_path[250], properties_path[250], 
	deletion_path[250], check_path[250], uncheck_path[250];

char storage_path[250], topScreen_path[250], topScreen_min_path[250], topScreen_bar_path[250], bottomScreen_path[250], 
	bottomScreen_bar_path[250], bottomScreen_text_path[250], options_select_path[250], options_text_path[250], options_title_text_path[250],
	settings_colour_path[250], settings_title_text_path[250], settings_text_path[250], settings_text_min_path[250];
	
char font_path[250];

struct colour 
{
   int	r;
   int	g;
   int	b;
};

char * setFileDefaultsChar(char path[], char data[], char var[]);
void replaceAsset(char arr[], char path[], char img_path[], char redirect_path[]);
void loadTheme();
void reloadTheme();

#endif