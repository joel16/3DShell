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
#include "screenshot.h"
#include "sound.h"
#include "theme.h"
#include "updater.h"
#include "utils.h"
#include "wifi.h"

struct colour BottomScreen_colour;
struct colour BottomScreen_bar_colour;
struct colour BottomScreen_text_colour;

//static struct sound *bgm;

void initServices()
{
	srvInit();
	fsInit();
	sdmcInit();
	openSdArchive();
	sdmcWriteSafe(false);
	aptInit();
	mcuInit();
	ptmuInit();
	hidInit();
	cfguInit();
	acInit();
	httpcInit(0);
	romfsInit();
	sf2d_init();
	sftd_init();
	
	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);
	
	amInit();
	AM_QueryAvailableExternalTitleDatabase(NULL);
	
	sf2d_set_clear_color(RGBA8(0, 0, 0, 255));
	sf2d_set_vblank_wait(0);
	
	installDirectories();
	
	loadTheme();
	
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
	
	homeIcon = sfil_load_PNG_file("romfs:/res/home.png", SF2D_PLACE_RAM); setBilinearFilter(homeIcon);
	optionsIcon = sfil_load_PNG_file("romfs:/res/options_icon.png", SF2D_PLACE_RAM); setBilinearFilter(optionsIcon);
	sdIcon = sfil_load_PNG_file("romfs:/res/sd.png", SF2D_PLACE_RAM); setBilinearFilter(sdIcon);
	nandIcon = sfil_load_PNG_file("romfs:/res/nand.png", SF2D_PLACE_RAM); setBilinearFilter(nandIcon);
	settingsIcon = sfil_load_PNG_file("romfs:/res/settings.png", SF2D_PLACE_RAM); setBilinearFilter(settingsIcon);
	updateIcon = sfil_load_PNG_file("romfs:/res/update.png", SF2D_PLACE_RAM); setBilinearFilter(updateIcon);
	ftpIcon = sfil_load_PNG_file("romfs:/res/ftp.png", SF2D_PLACE_RAM); setBilinearFilter(ftpIcon);
	dlIcon = sfil_load_PNG_file("romfs:/res/url.png", SF2D_PLACE_RAM); setBilinearFilter(dlIcon);
	themeIcon = sfil_load_PNG_file("romfs:/res/theme.png", SF2D_PLACE_RAM); setBilinearFilter(themeIcon);
	s_HomeIcon = sfil_load_PNG_file("romfs:/res/s_home.png", SF2D_PLACE_RAM); setBilinearFilter(s_HomeIcon);
	s_OptionsIcon = sfil_load_PNG_file("romfs:/res/s_options_icon.png", SF2D_PLACE_RAM); setBilinearFilter(s_OptionsIcon);
	s_SdIcon = sfil_load_PNG_file("romfs:/res/s_sd.png", SF2D_PLACE_RAM); setBilinearFilter(s_SdIcon);
	s_SettingsIcon = sfil_load_PNG_file("romfs:/res/s_settings.png", SF2D_PLACE_RAM); setBilinearFilter(s_SettingsIcon);
	s_NandIcon = sfil_load_PNG_file("romfs:/res/s_nand.png", SF2D_PLACE_RAM); setBilinearFilter(s_NandIcon);
	s_UpdateIcon = sfil_load_PNG_file("romfs:/res/s_update.png", SF2D_PLACE_RAM); setBilinearFilter(s_UpdateIcon);
	s_ftpIcon = sfil_load_PNG_file("romfs:/res/s_ftp.png", SF2D_PLACE_RAM); setBilinearFilter(s_ftpIcon);
	s_dlIcon = sfil_load_PNG_file("romfs:/res/s_url.png", SF2D_PLACE_RAM); setBilinearFilter(s_dlIcon);
	searchIcon = sfil_load_PNG_file("romfs:/res/search.png", SF2D_PLACE_RAM); setBilinearFilter(searchIcon);
	
	toggleOn = sfil_load_PNG_file("romfs:/res/toggleOn.png", SF2D_PLACE_RAM); setBilinearFilter(toggleOn);
	toggleOff = sfil_load_PNG_file("romfs:/res/toggleOff.png", SF2D_PLACE_RAM); setBilinearFilter(toggleOff);
	
	_0 = sfil_load_PNG_file("romfs:/res/battery/0.png", SF2D_PLACE_RAM); setBilinearFilter(_0);
	_15 = sfil_load_PNG_file("romfs:/res/battery/15.png", SF2D_PLACE_RAM); setBilinearFilter(_15);
	_28 = sfil_load_PNG_file("romfs:/res/battery/28.png", SF2D_PLACE_RAM); setBilinearFilter(_28);
	_43 = sfil_load_PNG_file("romfs:/res/battery/43.png", SF2D_PLACE_RAM); setBilinearFilter(_43);
	_57 = sfil_load_PNG_file("romfs:/res/battery/57.png", SF2D_PLACE_RAM); setBilinearFilter(_57);
	_71 = sfil_load_PNG_file("romfs:/res/battery/71.png", SF2D_PLACE_RAM); setBilinearFilter(_71);
	_85 = sfil_load_PNG_file("romfs:/res/battery/85.png", SF2D_PLACE_RAM); setBilinearFilter(_85);
	_100 = sfil_load_PNG_file("romfs:/res/battery/100.png", SF2D_PLACE_RAM); setBilinearFilter(_100);
	_charge = sfil_load_PNG_file("romfs:/res/battery/charge.png", SF2D_PLACE_RAM); setBilinearFilter(_charge);
	
	wifiIcon0 = sfil_load_PNG_file("romfs:/res/wifi/stat_sys_wifi_signal_0.png", SF2D_PLACE_RAM); setBilinearFilter(wifiIcon0);
	wifiIcon1 = sfil_load_PNG_file("romfs:/res/wifi/stat_sys_wifi_signal_1.png", SF2D_PLACE_RAM); setBilinearFilter(wifiIcon1);
	wifiIcon2 = sfil_load_PNG_file("romfs:/res/wifi/stat_sys_wifi_signal_2.png", SF2D_PLACE_RAM); setBilinearFilter(wifiIcon2);
	wifiIcon3 = sfil_load_PNG_file("romfs:/res/wifi/stat_sys_wifi_signal_3.png", SF2D_PLACE_RAM); setBilinearFilter(wifiIcon3);
	wifiIconNull = sfil_load_PNG_file("romfs:/res/wifi/stat_sys_wifi_signal_null.png", SF2D_PLACE_RAM); setBilinearFilter(wifiIconNull);
	
	galleryBar = sfil_load_PNG_file("romfs:/res/gallery/bar.png", SF2D_PLACE_RAM); setBilinearFilter(galleryBar);
	
	font = sftd_load_font_file(font_path);
	font2 = sftd_load_font_file(font_path);
	
	if (isN3DS())
		osSetSpeedupEnable(true);
	
	//APT_SetAppCpuTimeLimit(80);
	
	language = 1; //getLanguage();
	
	sprintf(welcomeMsg, "%s %s! %s", lang_welcome[language][0], getUsername(), lang_welcome[language][1]);
	
	sprintf(currDate, "%s %s %s.", lang_welcome[language][2], getDayOfWeek(0), getMonthOfYear(0));
	
	DEFAULT_STATE = STATE_HOME;
	BROWSE_STATE = STATE_SD;
	
	/*if (fileExists(sdmcArchive, "/3ds/3DShell/bgm.ogg")) // Initally create this to avoid crashes
		bgm = sound_create(BGM);*/
}

void termServices()
{
	osSetSpeedupEnable(0);
	
	sftd_free_font(font);
	sftd_free_font(font2);
	
	sf2d_free_texture(galleryBar);
	
	sf2d_free_texture(wifiIconNull);
	sf2d_free_texture(wifiIcon3);
	sf2d_free_texture(wifiIcon2);
	sf2d_free_texture(wifiIcon1);
	sf2d_free_texture(wifiIcon0);
	
	sf2d_free_texture(_charge);
	sf2d_free_texture(_100);
	sf2d_free_texture(_85);
	sf2d_free_texture(_71);
	sf2d_free_texture(_57);
	sf2d_free_texture(_43);
	sf2d_free_texture(_28);
	sf2d_free_texture(_15);
	sf2d_free_texture(_0);
	
	sf2d_free_texture(toggleOff);
	sf2d_free_texture(toggleOn);
	
	sf2d_free_texture(homeIcon);
	sf2d_free_texture(optionsIcon);
	sf2d_free_texture(sdIcon);
	sf2d_free_texture(nandIcon);
	sf2d_free_texture(settingsIcon);
	sf2d_free_texture(updateIcon);
	sf2d_free_texture(ftpIcon);
	sf2d_free_texture(themeIcon);
	sf2d_free_texture(s_HomeIcon);
	sf2d_free_texture(s_OptionsIcon);
	sf2d_free_texture(s_SdIcon);
	sf2d_free_texture(s_SettingsIcon);
	sf2d_free_texture(s_NandIcon);
	sf2d_free_texture(s_UpdateIcon);
	sf2d_free_texture(s_ftpIcon);
	sf2d_free_texture(searchIcon);
	
	sf2d_free_texture(background);
	sf2d_free_texture(options);
	sf2d_free_texture(_properties);
	sf2d_free_texture(deletion);
	sf2d_free_texture(selector);
	sf2d_free_texture(folderIcon);
	sf2d_free_texture(fileIcon);
	sf2d_free_texture(audioIcon);
	sf2d_free_texture(appIcon);
	sf2d_free_texture(txtIcon);
	sf2d_free_texture(systemIcon);
	sf2d_free_texture(zipIcon);
	sf2d_free_texture(imgIcon);
	sf2d_free_texture(check);
	sf2d_free_texture(uncheck);
	
	amExit();
	ndspExit();
	sftd_fini();
	sf2d_fini();
	romfsExit();
	httpcExit();
	acExit();
	cfguExit();
	hidExit();
	ptmuExit();
	mcuExit();
	aptExit();
	closeSdArchive();
	sdmcExit();
	fsExit();
	srvExit();
}

static loop_status_t loop(loop_status_t (*callback)(void))
{
	loop_status_t status = LOOP_CONTINUE;

	while(aptMainLoop())
	{
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
		sf2d_draw_rectangle(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
		
		sf2d_draw_texture(homeIcon, -2, -2);
		sf2d_draw_texture(optionsIcon, 25, 0);
		sf2d_draw_texture(settingsIcon, 50, 0);
		sf2d_draw_texture(updateIcon, 75, 0);
		sf2d_draw_texture(s_ftpIcon, 100, 0);
		
		if (BROWSE_STATE == STATE_SD)
			sf2d_draw_texture(s_SdIcon, 125, 0);
		else
			sf2d_draw_texture(sdIcon, 125, 0);
	
		if (BROWSE_STATE == STATE_NAND)
			sf2d_draw_texture(s_NandIcon, 150, 0);
		else
			sf2d_draw_texture(nandIcon, 150, 0);
		
		sf2d_draw_texture(searchIcon, (320 - searchIcon->width), -2);
		
		char buf[25];
		
		u32 wifiStatus = 0;
		ACU_GetWifiStatus(&wifiStatus);
		
		if (wifiStatus == 0)
		{
			sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_ftp[language][3])) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_ftp[language][3]);
			sprintf(buf, lang_ftp[language][4]);
		}
		else
		{
			sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_ftp[language][0])) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_ftp[language][0]);
			u32 ip = gethostid();
			sprintf(buf, "IP: %lu.%lu.%lu.%lu:5000", ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);
		}
		
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, buf)) / 2), 60, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, buf);
		
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_ftp[language][1])) / 2), 80, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_ftp[language][1]);
		
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, lang_ftp[language][2])) / 2), 100, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, lang_ftp[language][2]);
		
		endDrawing();
		
		status = callback();
		if(status != LOOP_CONTINUE)
			return status;
	}

	return LOOP_EXIT;
}

void mainMenu(int clearindex)
{
	selectionX = 0, selectionY = 0;
	properties = false, deleteDialog = false;
	
	if (clearindex != 0)
		updateList(CLEAR);
	
/*turnOnBGM:
	if(fileExists(sdmcArchive, "/3ds/dspfirm.cdc"))
	{
		if (fileExists(sdmcArchive, "/3ds/3DShell/bgm.ogg"))
		{
			if((!(isPlaying)) && (bgmEnable == true))
				audio_load_ogg("/3ds/3DShell/bgm.ogg");
		}
	}

turnOffBGM:
	if ((isPlaying) && (bgmEnable == false))
		sound_stop(bgm);*/
	
	loop_status_t status = LOOP_OFF;
	
	while (aptMainLoop())
	{
		// Display file list
		displayFiles(KEEP);
		
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
		else if ((kPressed & KEY_TOUCH) && (touchInRect(74, 97, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_UPDATE;
		}
		
		/*else if ((kPressed & KEY_TOUCH) && (touchInRect(0, 320, 50, 72)) && (IF_SETTINGS))
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
			displayFiles(CLEAR);
		}
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(98, 123, 0, 20)))
		{	
			wait(100000000);
			DEFAULT_STATE = STATE_FTP;
			wait(100000000);
			status = LOOP_RESTART;
		}
		
		if (DEFAULT_STATE == STATE_FTP)
		{
			while (status == LOOP_RESTART)
			{
				if(ftp_init() == 0)
				{
					// ftp loop
					status = loop(ftp_loop);

					// done with ftp
					ftp_exit();
				}
				else
				{
					wait(100000000);
					status = LOOP_EXIT;
					DEFAULT_STATE = STATE_HOME;
				}
			}
		}
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(124, 147, 0, 20)))
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
		}
	
		if ((kPressed & KEY_TOUCH) && (touchInRect(148, 173, 0, 20)))
		{
			wait(100000000);
			char buf[250];
		
			FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
			fscanf(read, "%s", buf);
			fclose(read);
		
			if (dirExists(sdmcArchive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
				strcpy(cwd, buf);
			else 
				strcpy(cwd, START_PATH);
			
			BROWSE_STATE = STATE_SD;
			updateList(CLEAR);
			displayFiles(CLEAR);
		}
		/*else if ((kPressed & KEY_TOUCH) && (touchInRect(174, 199, 0, 20))) //Mount stuff goes here
		{
			wait(100000000);
			strcpy(cwd, "nand:/");
			BROWSE_STATE = STATE_NAND;
			updateList(CLEAR);
			displayFiles(CLEAR);
		}*/
		
		if ((kPressed & KEY_TOUCH) && (touchInRect(290, 320, 0, 20)))
		{
			strcpy(cwd, keyboard_3ds_get(250, "", "Enter path"));
				
			if (dirExists(sdmcArchive, cwd))
			{
				updateList(CLEAR);
				displayFiles(CLEAR);
			}
				
			else 
				displayFiles(KEEP);
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
				displayFiles(KEEP);
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
				displayFiles(KEEP);
			}
			
			if (kHeld & KEY_CPAD_UP)
			{
				wait(66666666);
				
				if(position > 0) 
					position--;

				else position = fileCount - 1;

				displayFiles(KEEP);
			}

			else if (kHeld & KEY_CPAD_DOWN)
			{
				wait(66666666);
				
				if(position < (fileCount - 1)) 
					position++;
				
				else position = 0;
				
				displayFiles(KEEP);
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
						strcpy(font_dir, "romfs:/font");
						strcpy(colour_dir, "/3ds/3DShell");
						
						FILE * file = fopen("/3ds/3DShell/theme.bin", "w");
						fprintf(file, "%s", theme_dir);
						fclose(file);
						
						file = fopen("/3ds/3DShell/colours.bin", "w");
						fprintf(file, "%s", colour_dir);
						fclose(file);
						
						file = fopen("/3ds/3DShell/font.bin", "w");
						fprintf(file, "%s", font_dir);
						fclose(file);
						
						wait(1000000);
						
						loadTheme();
						reloadTheme();
					}
					else if ((strncmp(fileName, "..", 2) != 0))
					{
						strcpy(theme_dir, cwd);
						strcpy(colour_dir, cwd);
						strcpy(font_dir, cwd);
						
						strcat(theme_dir, fileName);
						strcat(colour_dir, fileName);
						strcat(font_dir, fileName);
						
						FILE * file = fopen("/3ds/3DShell/theme.bin", "w");
						fprintf(file, "%s", theme_dir);
						fclose(file);
						
						file = fopen("/3ds/3DShell/colours.bin", "w");
						fprintf(file, "%s", colour_dir);
						fclose(file);
						
						file = fopen("/3ds/3DShell/font.bin", "w");
						fprintf(file, "%s", font_dir);
						fclose(file);
						
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
			
					if (dirExists(sdmcArchive, buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
						strcpy(cwd, buf);
					else 
						strcpy(cwd, START_PATH);
					
					wait(1000000);
					
					DEFAULT_STATE = STATE_SETTINGS;
					updateList(CLEAR);
					displayFiles(CLEAR);
				}
				
				else
				{
					navigate(-1);
					updateList(CLEAR);
					displayFiles(CLEAR);
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
				
				newFolder();
			}
			
			else if ((kPressed & KEY_TOUCH) && (touchInRect(37, 160, 131, 167)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 2;
				
				wait(100000000);
				
				deleteDialog = true;
				drawDeletionDialog();
			}
			
			else if ((kPressed & KEY_TOUCH) && (touchInRect(161, 284, 56, 93)) && (IF_OPTIONS))
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
				displayFiles(KEEP);
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
					displayFiles(CLEAR);
				}	
			}	
			
			if ((CAN_CUT) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
			{
				selectionX = 1;
				selectionY = 2;
				wait(100000000);
				copy(COPY_DELETE_ON_FINISH);
				cutF = true;
				displayFiles(KEEP);
			}
			else if (((cutF == true) && (deleteDialog == false)) && (kPressed & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 0;
				wait(100000000);
				
				if(paste() == 0)
				{
					cutF = false;
					updateList(CLEAR);
					displayFiles(CLEAR);
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