#include "clock.h"
#include "common.h"
#include "dirlist.h"
#include "file_operations.h"
#include "fs.h"
#include "ftp.h"
#include "gallery.h"
#include "keyboard.h"
#include "language.h"
#include "main.h"
#include "mcu.h"
#include "net.h"
#include "power.h"
#include "screen.h"
#include "screenshot.h"
#include "sound.h"
#include "task.h"
#include "theme.h"
#include "updater.h"
#include "utils.h"
#include "wifi.h"

struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;

//static struct sound *bgm;

void initServices(void)
{
	srvInit();
	fsInit();
	sdmcInit();
	openArchive(ARCHIVE_SDMC);
	sdmcWriteSafe(false);
	aptInit();
	mcuInit();
	ptmuInit();
	hidInit();
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
	
	installDirectories();
	
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
	screen_load_texture_png(TEXTURE_DOWNLOAD_ICON, "romfs:/res/url.png", true);
	
	screen_load_texture_png(TEXTURE_HOME_ICON_SELECTED, "romfs:/res/s_home.png", true);
	screen_load_texture_png(TEXTURE_OPTIONS_ICON_SELECTED, "romfs:/res/s_options_icon.png", true);
	screen_load_texture_png(TEXTURE_SETTINGS_ICON_SELECTED, "romfs:/res/s_settings.png", true);
	screen_load_texture_png(TEXTURE_UPDATE_ICON_SELECTED, "romfs:/res/s_update.png", true);
	screen_load_texture_png(TEXTURE_FTP_ICON_SELECTED, "romfs:/res/s_ftp.png", true);
	screen_load_texture_png(TEXTURE_DOWNLOAD_ICON_SELECTED, "romfs:/res/s_url.png", true);
	
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

	if (isN3DS())
		osSetSpeedupEnable(true);
	
	//APT_SetAppCpuTimeLimit(80);
	
	language = 1; //getLanguage();
	
	sprintf(welcomeMsg, "%s %s! %s", lang_welcome[language][0], getUsername(), lang_welcome[language][1]);
	
	sprintf(currDate, "%s %s %s.", lang_welcome[language][2], getDayOfWeek(0), getMonthOfYear(0));
	
	DEFAULT_STATE = STATE_HOME;
	BROWSE_STATE = STATE_SD;
	
	/*if (fileExists(fsArchive, "/3ds/3DShell/bgm.ogg")) // Initally create this to avoid crashes
		bgm = sound_create(BGM);*/
}

void termServices(void)
{	
	osSetSpeedupEnable(0);
	
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
	screen_unload_texture(TEXTURE_DOWNLOAD_ICON);
	
	screen_unload_texture(TEXTURE_HOME_ICON_SELECTED);
	screen_unload_texture(TEXTURE_OPTIONS_ICON_SELECTED);
	screen_unload_texture(TEXTURE_SETTINGS_ICON_SELECTED);
	screen_unload_texture(TEXTURE_UPDATE_ICON_SELECTED);
	screen_unload_texture(TEXTURE_FTP_ICON_SELECTED);
	screen_unload_texture(TEXTURE_DOWNLOAD_ICON_SELECTED);
	
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
	hidExit();
	ptmuExit();
	mcuExit();
	aptExit();
	closeArchive();
	sdmcExit();
	fsExit();
	srvExit();
}

void displayFTP()
{
	ftp_init();
	task_init();
	
	touchPosition touch;
	
	while(aptMainLoop())
	{
		screen_begin_frame();
		screen_select(GFX_BOTTOM);
		
		hidScanInput();
		hidTouchRead(&touch);

		if ((kPressed & KEY_TOUCH) && (touchInRect(98, 123, 0, 20))) 
			break;
		
		screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
		screen_draw_rect(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
		
		screen_draw_texture(TEXTURE_HOME_ICON, -2, -2);
		screen_draw_texture(TEXTURE_OPTIONS_ICON, 25, 0);
		screen_draw_texture(TEXTURE_SETTINGS_ICON, 50, 0);
		screen_draw_texture(TEXTURE_UPDATE_ICON, 75, 0);
		screen_draw_texture(TEXTURE_FTP_ICON_SELECTED, 100, 0);
		screen_draw_texture(TEXTURE_DOWNLOAD_ICON, 125, 0);
		
		if (BROWSE_STATE == STATE_SD)
			screen_draw_texture(TEXTURE_SD_ICON_SELECTED, 150, 0);
		else
			screen_draw_texture(TEXTURE_SD_ICON, 150, 0);
	
		if (BROWSE_STATE == STATE_NAND)
			screen_draw_texture(TEXTURE_NAND_ICON_SELECTED, 175, 0);
		else
			screen_draw_texture(TEXTURE_NAND_ICON, 175, 0);
		
		screen_draw_texture(TEXTURE_SEARCH_ICON, (320 - screen_get_texture_width(TEXTURE_SEARCH_ICON)), -2);

		ftp_loop();
		
		char buf[25];
		
		u32 wifiStatus = 0;
		ACU_GetWifiStatus(&wifiStatus);
		
		if (wifiStatus == 0)
		{
			screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][3], 0.41f, 0.41f)) / 2), 40, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][3]);
			sprintf(buf, lang_ftp[language][4]);
		}
		else
		{
			screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][0], 0.41f, 0.41f)) / 2), 40, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][0]);
			u32 ip = gethostid();
			sprintf(buf, "IP: %lu.%lu.%lu.%lu:5000", ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);
		}
		
		screen_draw_string(((320 - screen_get_string_width(buf, 0.41f, 0.41f)) / 2), 60, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), buf);
		
		screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][1], 0.41f, 0.41f)) / 2), 80, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][1]);
		
		screen_draw_string(((320 - screen_get_string_width(lang_ftp[language][2], 0.41f, 0.41f)) / 2), 100, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_ftp[language][2]);
		
		screen_end_frame();
	}
	
	task_exit();
	ftp_exit();
	DEFAULT_STATE = STATE_HOME;
}

void mainMenu(int clearindex)
{
	selectionX = 0, selectionY = 0;
	properties = false, deleteDialog = false;
	
	if (clearindex != 0)
		updateList(CLEAR);
	
/*turnOnBGM:
	if(fileExists(fsArchive, "/3ds/dspfirm.cdc"))
	{
		if (fileExists(fsArchive, "/3ds/3DShell/bgm.ogg"))
		{
			if((!(isPlaying)) && (bgmEnable == true))
				audio_load_ogg("/3ds/3DShell/bgm.ogg");
		}
	}

turnOffBGM:
	if ((isPlaying) && (bgmEnable == false))
		sound_stop(bgm);*/
	
	while (aptMainLoop())
	{
		// Display file list
		displayFiles();
		
		hidScanInput();
		hidTouchRead(&touch);
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(0, 22, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_HOME;
		}
		else if ((kPressed & KEY_TOUCH) && (touchInRect(23, 47, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_OPTIONS;
		}
		else if ((kPressed & KEY_TOUCH) && (touchInRect(48, 73, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_SETTINGS;
		}
		/*else if ((kPressed & KEY_TOUCH) && (touchInRect(74, 97, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_UPDATE;
		}
		
		else if ((kPressed & KEY_TOUCH) && (touchInRect(0, 320, 50, 72)) && (IF_SETTINGS))
		{
			if (bgmEnable == false)
			{
				setBgm(true);
				bgmEnable = true;
				goto turnOnBGM;
			}
			else if (bgmEnable == true)
			{
				setBgm(false);
				bgmEnable = false;
				goto turnOffBGM;
			}
		}*/
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(0, 320, 90, 112)) && (IF_SETTINGS))
		{
			wait(100000000);
			if (sysProtection == false)
			{
				setConfig("/3ds/3DShell/sysProtection.txt", true);
				sysProtection = true;
			}
			else 
			{
				setConfig("/3ds/3DShell/sysProtection.txt", false);
				sysProtection = false;
			}
		}
		
		else if ((kPressed & KEY_TOUCH) && (touchInRect(283, 303, 125, 145)) && (IF_SETTINGS))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_THEME;
			strcpy(cwd, "/3ds/3DShell/themes/");
			updateList(CLEAR);
			displayFiles();
		}
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(98, 123, 0, 20)))
		{	
			wait(100000000);
			DEFAULT_STATE = STATE_FTP;
		}
		
		if (DEFAULT_STATE == STATE_FTP)
		{
			displayFTP();
		}
		
		/*if ((kPressed & KEY_TOUCH) && (touchInRect(124, 147, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_DOWNLOAD;
		}
		
		if (DEFAULT_STATE == STATE_DOWNLOAD)
		{
			bool downloadReady = false;
			
			if ((kPressed & KEY_TOUCH) && (touchInRect(0, 320, 40, 54)))
			{
				strcpy(dl_url, keyboard_3ds_get(255, "", "Enter URL"));
				if (strcmp(dl_url, "") != 0)
					downloadReady = true;
			}
			
			if (downloadReady == true)
				downloadFile(dl_url, "/");
		}*/
	
		if ((kPressed & KEY_TOUCH) && (touchInRect(148, 173, 0, 20))) // SD
		{
			wait(100000000);
			
			writeFile("/3ds/3DShell/lastdir.txt", START_PATH);
			
			strcpy(cwd, START_PATH);
			
			BROWSE_STATE = STATE_SD;
			
			closeArchive();
			openArchive(ARCHIVE_SDMC);
			
			updateList(CLEAR);
			displayFiles();
		}
		else if ((kPressed & KEY_TOUCH) && (touchInRect(174, 199, 0, 20))) // CTR-NAND
		{
			wait(100000000);
			strcpy(cwd, START_PATH);
			
			BROWSE_STATE = STATE_NAND;
			
			closeArchive();
			openArchive(ARCHIVE_NAND_CTR_FS);
			
			updateList(CLEAR);
			displayFiles();
		}
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(290, 320, 0, 20)))
		{
			strcpy(cwd, keyboard_3ds_get(250, "", "Enter path"));
				
			if (dirExists(fsArchive, cwd))
			{
				updateList(CLEAR);
				displayFiles();
			}
				
			else 
				displayFiles();
		}
		
		if (kPressed & KEY_START) // exit
			break;
		
		if(fileCount > 0)
		{
			// Position Decrement
			if (kPressed & KEY_DUP)
			{
				wait(100000000);
				
				// Decrease Position
				if(position > 0) 
					position--;

				// Rewind Pointer
				else position = fileCount - 1;

				// Display file list
				displayFiles();
			}

			// Position Increment
			else if (kPressed & KEY_DDOWN)
			{
				wait(100000000);
				
				// Increase Position
				if(position < (fileCount - 1)) 
					position++;

				// Rewind Pointer
				else position = 0;

				// Display file list
				displayFiles();
			}
			
			if (kHeld & KEY_CPAD_UP)
			{
				wait(66666666);
				
				if(position > 0) 
					position--;

				else position = fileCount - 1;

				displayFiles();
			}

			else if (kHeld & KEY_CPAD_DOWN)
			{
				wait(66666666);
				
				if(position < (fileCount - 1)) 
					position++;
				
				else position = 0;
				
				displayFiles();
			}
			
			else if (kPressed & KEY_A)
			{
				wait(66666666);
				
				if (IF_THEME)
				{
					File * file = findindex(position);
					
					strcpy(fileName, file->name);
					
					if ((strncmp(fileName, "default", 7) == 0))
					{
						strcpy(theme_dir, "romfs:/res");
						strcpy(colour_dir, "/3ds/3DShell");
						
						writeFile("/3ds/3DShell/theme.bin", theme_dir);
						writeFile("/3ds/3DShell/colours.bin", colour_dir);
						
						wait(1000000);
						
						loadTheme();
						reloadTheme();
					}
					else if ((strncmp(fileName, "..", 2) != 0) && (file->isDir))
					{
						strcpy(theme_dir, cwd);
						strcpy(colour_dir, cwd);
						
						strcat(theme_dir, fileName);
						strcat(colour_dir, fileName);
						
						writeFile("/3ds/3DShell/theme.bin", theme_dir);
						writeFile("/3ds/3DShell/colours.bin", colour_dir);
						
						wait(1000000);
						
						loadTheme();
						reloadTheme();
					}
				}
				else
					openFile(); // Open file/dir
			}
			
			else if ((strcmp(cwd, ROOT_PATH) != 0) && (kPressed & KEY_B))
			{
				wait(66666666);
				
				if (IF_THEME)
				{
					char buf[250];
		
					FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
					fscanf(read, "%s", buf);
					fclose(read);
			
					if (dirExists(fsArchive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
						strcpy(cwd, buf);
					else 
						strcpy(cwd, START_PATH);
					
					wait(1000000);
					
					DEFAULT_STATE = STATE_SETTINGS;
					updateList(CLEAR);
					displayFiles();
				}
				
				else
				{
					navigate(-1);
					updateList(CLEAR);
					displayFiles();
				}
			}
			
			/*else if ((strcmp(cwd, ROOT_PATH) != 0) && (kPressed & KEY_X))
			{
				wait(100000000);
				
				decodeQr();
			}*/
			
			if ((kPressed & KEY_TOUCH) && (touchInRect(37, 282, 179, 217)) && (IF_OPTIONS)) // Cancel
			{	
				wait(100000000);
				copyF = false;
				cutF = false;
				DEFAULT_STATE = STATE_HOME;
			}
			
			else if ((kHeld & KEY_TOUCH) && (touchInRect(37, 160, 56, 93)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 0;
				
				wait(100000000);
				
				properties = true;
				displayProperties();
			}
			
			else if ((kPressed & KEY_TOUCH) && (touchInRect(37, 160, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 1;
				
				wait(100000000);
				
				createFolder();
			}
			
			else if ((kPressed & KEY_TOUCH) && (touchInRect(37, 160, 131, 167)) && (IF_OPTIONS) && (BROWSE_STATE != STATE_NAND))
			{
				selectionX = 0;
				selectionY = 2;
				
				wait(100000000);
				
				deleteDialog = true;
				drawDeletionDialog();
			}
			
			else if ((kPressed & KEY_TOUCH) && (touchInRect(161, 284, 56, 93)) && (IF_OPTIONS) && (BROWSE_STATE != STATE_NAND))
			{
				selectionX = 1;
				selectionY = 0;
				
				wait(100000000);
				
				renameFile();
			}

			if ((CAN_COPY) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 1;
				selectionY = 1; 
				wait(100000000);
				copy(COPY_KEEP_ON_FINISH);
				copyF = true;
				displayFiles();
			}
			else if (((copyF == true) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 0; 
				wait(100000000);
					
				if(paste() == 0)
				{
					copyF = false;
					updateList(CLEAR);
					displayFiles();
				}	
			}	
			
			if ((CAN_CUT) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS)  && (BROWSE_STATE != STATE_NAND))
			{
				selectionX = 1;
				selectionY = 2;
				wait(100000000);
				copy(COPY_DELETE_ON_FINISH);
				cutF = true;
				displayFiles();
			}
			else if (((cutF == true) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS)  && (BROWSE_STATE != STATE_NAND))
			{
				selectionX = 0;
				selectionY = 0;
				wait(100000000);
				
				if(paste() == 0)
				{
					cutF = false;
					updateList(CLEAR);
					displayFiles();
				}
			}
		}
	}
}

int main(int argc, char *argv[])
{
	initServices();
	
	if(setjmp(exitJmp)) 
	{
		termServices();
		return 0;
	}
	
	mainMenu(CLEAR);
	
	termServices();
	
	return 0;
}