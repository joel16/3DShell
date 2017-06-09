#include "act.h"
#include "clock.h"
#include "fs.h"
#include "ftp.h"
#include "gallery.h"
#include "keyboard.h"
#include "main.h"
#include "mcu.h"
#include "net.h"
#include "power.h"
#include "screenshot.h"
#include "sound.h"
#include "text.h"
#include "theme.h"
#include "updater.h"
#include "utils.h"
#include "wifi.h"

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

/*
*	Menu position
*/
int position = 0;

/*
*	Number of files
*/
int fileCount = 0;

/*
*	File list
*/
File * files = NULL;

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
int copymode = NOTHING_TO_COPY;

void saveLastDirectory()
{
	char buf[250];
	strcpy(buf, cwd);
			
	FILE * write = fopen("/3ds/3DShell/lastdir.txt", "w");
	fprintf(write, "%s", buf);
	fclose(write);
}

void installDirectories()
{
	if (!(dirExists("/3ds/")))
		makeDir("/3ds");
	if (!(dirExists("/3ds/3DShell/")))
		makeDir("/3ds/3DShell");
	if (!(dirExists("/3ds/3DShell/themes/")))
		makeDir("/3ds/3DShell/themes");
	if (!(dirExists("/3ds/3DShell/themes/default/")))
		makeDir("/3ds/3DShell/themes/default");
	if (!(dirExists("/3ds/3DShell/colours/")))
		makeDir("/3ds/3DShell/colours");
	
	if (fileExists("/3ds/3DShell/lastdir.txt"))
	{
		char buf[250];
		
		FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
		fscanf(read, "%s", buf);
		fclose(read);
		
		if (dirExists(buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
			strcpy(cwd, buf);
		else 
			strcpy(cwd, START_PATH);
	}
	else
	{
		char buf[250];
		strcpy(buf, START_PATH);
			
		FILE * write = fopen("/3ds/3DShell/lastdir.txt", "w");
		fprintf(write, "%s", buf);
		fclose(write);
		
		strcpy(cwd, buf); // Set Start Path to "sdmc:/" if lastDir.txt hasn't been created.
	}
	
	/*if (!fileExists("/3ds/3DShell/bgm.txt"))
	{
		setBgm(true);
	}
	else
	{
		int initBgm = 0;
		
		FILE * read = fopen("/3ds/3DShell/bgm.txt", "r");
		fscanf(read, "%d", &initBgm);
		fclose(read);
		
		if (initBgm == 0)
			bgmEnable = false;
		else 
			bgmEnable = true;
	}*/
	
	if (!fileExists("/3ds/3DShell/sysProtection.txt")) // Initially set it to true
	{
		setConfig("/3ds/3DShell/sysProtection.txt", true);
		sysProtection = true;
	}
	else
	{
		int info = 0;
		
		FILE * read = fopen("/3ds/3DShell/sysProtection.txt", "r");
		fscanf(read, "%d", &info);
		fclose(read);
		
		if (info == 0)
			sysProtection = false;
		else 
			sysProtection = true;
	}
}

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
	actInit(SDK(11, 2, 0, 200), 0x20000);
	httpcInit(0);
	amInit();
	AM_InitializeExternalTitleDatabase(false);
	romfsInit();
	sf2d_init();
	sftd_init();
	
	audio_init();
	
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
	
	strcpy(userName, (char*)getNNID());
	sprintf(welcomeMsg, "Hello there %s! How are you today?", userName);
	
	sprintf(currDate, "Today is %s %s.", getDayOfWeek(0), getMonthOfYear(0));
	
	DEFAULT_STATE = STATE_HOME;
	BROWSE_STATE = STATE_SD;
	
	/*if (fileExists("/3ds/3DShell/bgm.ogg")) // Initally create this to avoid crashes
		bgm = sound_create(BGM);*/
}

void termServices()
{
	osSetSpeedupEnable(0);
	
	sftd_free_font(font);
	sftd_free_font(font2);
	
	sf2d_free_texture(_0);
	sf2d_free_texture(_15);
	sf2d_free_texture(_28);
	sf2d_free_texture(_43);
	sf2d_free_texture(_57);
	sf2d_free_texture(_71);
	sf2d_free_texture(_85);
	sf2d_free_texture(_100);
	sf2d_free_texture(_charge);
	
	sf2d_free_texture(toggleOn);
	sf2d_free_texture(toggleOff);
	
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
	sf2d_free_texture(uncheck);
	
	audio_stop();
	sftd_fini();
	sf2d_fini();
	romfsExit();
	amExit();
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
			sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Failed to initialize FTP.")) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "Failed to initialize FTP.");
			sprintf(buf, "WiFi not enabled.");
		}
		else
		{
			sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "FTP initialized")) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "FTP initialized");
			u32 ip = gethostid();
			sprintf(buf, "IP: %lu.%lu.%lu.%lu:5000", ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF);
		}
		
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, buf)) / 2), 60, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, buf);
		
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "File browser cannot be accessed at this time.")) / 2), 80, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "File browser cannot be accessed at this time.");
		
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Tap the FTP icon to disable FTP connection.")) / 2), 100, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "Tap the FTP icon to disable FTP connection.");
		
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
		
	touchPosition touch;
	
/*turnOnBGM:
	if(fileExists("/3ds/dspfirm.cdc"))
	{
		if (fileExists("/3ds/3DShell/bgm.ogg"))
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
		u32 kPress = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
		
		if ((kPress & KEY_TOUCH) && (touchInRect(0, 22, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_HOME;
		}
		else if ((kPress & KEY_TOUCH) && (touchInRect(23, 47, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_OPTIONS;
		}
		else if ((kPress & KEY_TOUCH) && (touchInRect(48, 73, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_SETTINGS;
		}
		/*else if ((kPress & KEY_TOUCH) && (touchInRect(74, 97, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_UPDATE;
		}*/
		
		/*else if ((kPress & KEY_TOUCH) && (touchInRect(0, 320, 50, 72)) && (IF_SETTINGS))
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
		
		if ((kPress & KEY_TOUCH) && (touchInRect(0, 320, 90, 112)) && (IF_SETTINGS))
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
		
		else if ((kPress & KEY_TOUCH) && (touchInRect(283, 303, 125, 145)) && (IF_SETTINGS))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_THEME;
			strcpy(cwd, "/3ds/3DShell/themes/");
			updateList(CLEAR);
			displayFiles(CLEAR);
		}
		
		if ((kPress & KEY_TOUCH) && (touchInRect(98, 123, 0, 20)))
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
		
		if ((kPress & KEY_TOUCH) && (touchInRect(124, 147, 0, 20)))
		{
			wait(100000000);
			DEFAULT_STATE = STATE_DOWNLOAD;
		}
		
		if (DEFAULT_STATE == STATE_DOWNLOAD)
		{
			bool downloadReady = false;
			
			if ((kPress & KEY_TOUCH) && (touchInRect(0, 320, 40, 54)))
			{
				strcpy(dl_url, keyboard_3ds_get(255, "Enter URL"));
				if (strcmp(dl_url, "") != 0)
					downloadReady = true;
			}
			
			if (downloadReady == true)
				downloadFile(dl_url, "/");
		}
	
		if ((kPress & KEY_TOUCH) && (touchInRect(148, 173, 0, 20)))
		{
			wait(100000000);
			char buf[250];
		
			FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
			fscanf(read, "%s", buf);
			fclose(read);
		
			if (dirExists(buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
				strcpy(cwd, buf);
			else 
				strcpy(cwd, START_PATH);
			
			BROWSE_STATE = STATE_SD;
			updateList(CLEAR);
			displayFiles(CLEAR);
		}
		/*else if ((kPress & KEY_TOUCH) && (touchInRect(174, 199, 0, 20))) //Mount stuff goes here
		{
			wait(100000000);
			strcpy(cwd, "nand:/");
			BROWSE_STATE = STATE_NAND;
			updateList(CLEAR);
			displayFiles(CLEAR);
		}*/
		
		if ((kPress & KEY_TOUCH) && (touchInRect(290, 320, 0, 20)))
		{
			strcpy(cwd, keyboard_3ds_get(250, "Enter path"));
				
			if (dirExists(cwd))
			{
				updateList(CLEAR);
				displayFiles(CLEAR);
			}
				
			else 
				displayFiles(KEEP);
		}
		
		if (kPress & KEY_START) // exit
			break;			
		
		if(fileCount > 0)
		{
			// Position Decrement
			if (kPress & KEY_DUP)
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
			else if (kPress & KEY_DDOWN)
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
			
			else if (kPress & KEY_A)
			{
				wait(66666666);
				
				if (IF_THEME)
				{
					File * file = findindex(position);
					
					strcpy(fileName, file->name);
					
					if ((strcmp(fileName, "default") == 0))
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
					else if ((strcmp(fileName, "..") != 0))
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
			
			else if ((strcmp(cwd, ROOT_PATH) != 0) && (kPress & KEY_B))
			{
				wait(66666666);
				
				if (IF_THEME)
				{
					char buf[250];
		
					FILE * read = fopen("/3ds/3DShell/lastdir.txt", "r");
					fscanf(read, "%s", buf);
					fclose(read);
			
					if (dirExists(buf)) // Incase a directory previously visited had been deleted, set start path to sdmc:/ to avoid errors.
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
			
			/*else if ((strcmp(cwd, ROOT_PATH) != 0) && (kPress & KEY_X))
			{
				wait(100000000);
				
				decodeQr();
			}*/
			
			if ((kPress & KEY_TOUCH) && (touchInRect(37, 282, 179, 217)) && (IF_OPTIONS)) // Cancel
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
			
			else if ((kPress & KEY_TOUCH) && (touchInRect(37, 160, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 1;
				
				wait(100000000);
				
				newFolder();
			}
			
			else if ((kPress & KEY_TOUCH) && (touchInRect(37, 160, 131, 167)) && (IF_OPTIONS))
			{
				selectionX = 0;
				selectionY = 2;
				
				wait(100000000);
				
				deleteDialog = true;
				drawDeletionDialog();
			}
			
			else if ((kPress & KEY_TOUCH) && (touchInRect(161, 284, 56, 93)) && (IF_OPTIONS))
			{
				selectionX = 1;
				selectionY = 0;
				
				wait(100000000);
				
				renameF();
			}

			if ((CAN_COPY) && (kPress & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
			{
				selectionX = 1;
				selectionY = 1; 
				wait(100000000);
				copy(COPY_KEEP_ON_FINISH);
				copyF = true;
				displayFiles(KEEP);
			}
			else if (((copyF == true) && (deleteDialog == false)) && (kPress & KEY_TOUCH) && (touchInRect(161, 284, 94, 130)) && (IF_OPTIONS))
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
			
			if ((CAN_CUT) && (kPress & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
			{
				selectionX = 1;
				selectionY = 2;
				wait(100000000);
				copy(COPY_DELETE_ON_FINISH);
				cutF = true;
				displayFiles(KEEP);
			}
			else if (((cutF == true) && (deleteDialog == false)) && (kPress & KEY_TOUCH) && (touchInRect(161, 284, 131, 167)) && (IF_OPTIONS))
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
	
	mainMenu(CLEAR);
	
	termServices();
	
	return 0;
}

void updateList(int clearindex)
{
	// Clear List
	recursiveFree(files);
	files = NULL;
	fileCount = 0;

	// Open Working Directory
	DIR * directory = opendir(cwd);

	if (directory)
	{
		/* Add fake ".." entry except on root */
		if (strcmp(cwd, ROOT_PATH)) 
		{
			// New List
			files = (File *)malloc(sizeof(File));

			// Clear Memory
			memset(files, 0, sizeof(File));
			
			// Copy File Name
			strcpy(files->name, "..");

			// Set Folder Flag
			files->isFolder = 1;

			fileCount++;
		}
		
		struct dirent * info;

		// Iterate Files
		while ((info = readdir(directory)) != NULL)
		{	
			// Ingore null filename
			if(info->d_name[0] == '\0') 
				continue;

			// Ignore "." in all Directories
			if(strcmp(info->d_name, ".") == 0) 
				continue;

			// Ignore ".." in Root Directory
			if(strcmp(cwd, ROOT_PATH) == 0 && strcmp(info->d_name, "..") == 0) 
				continue;

			// Allocate Memory
			File * item = (File *)malloc(sizeof(File));

			// Clear Memory
			memset(item, 0, sizeof(File));
			
			// Copy File Name
			strcpy(item->name, info->d_name);

			// Set Folder Flag
			item->isFolder = info->d_type == DT_DIR;

			// New List
			if(files == NULL) 
				files = item;

			// Existing List
			else
			{
				// Iterator Variable
				File * list = files;

				// Append to List
				while(list->next != NULL) 
					list = list->next;
	
				// Link Item
				list->next = item;
			}

			// Increase File Count
			fileCount++;
		}

		// Close Directory
		closedir(directory);
	}
	

	// Attempt to keep Index
	if(!clearindex)
	{
		// Fix Position
		if(position >= fileCount) 
			position = fileCount - 1;
	}

	// Reset Position
	else 
		position = 0;
}

void recursiveFree(File * node)
{
	// End of List
	if(node == NULL)
		return;

	// Nest Further
	recursiveFree(node->next);

	// Free Memory
	free(node);
}

void displayFiles()
{
	// Bottom screen options
	sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
	sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
	sf2d_draw_rectangle(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
	
	if (DEFAULT_STATE == STATE_HOME)
	{
		sf2d_draw_texture(s_HomeIcon, -2, -2);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, welcomeMsg)) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, welcomeMsg);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, currDate)) / 2), 60, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, currDate);
		sftd_draw_textf(font, 2, 225, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "3DShell v%i.%i BETA", VERSION_MAJOR, VERSION_MINOR);
	}
	else
		sf2d_draw_texture(homeIcon, -2, -2);
	
	if (DEFAULT_STATE == STATE_SETTINGS)
	{
		sf2d_draw_texture(s_SettingsIcon, 50, 1);
		sf2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
		
		sftd_draw_text(font, 10, 30, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "General");
		
		sftd_draw_text(font, 10, 50, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 11, "BGM"); // Grey'd out - cannot be accessed yet.
		sftd_draw_text(font2, 10, 62, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 10, "Enable BGM upon start up. (/3ds/3DShell/bgm.ogg)");
		
		sftd_draw_text(font, 10, 90, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), 11, "System file protection");
		sftd_draw_text(font2, 10, 102, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 10, "Prevents deletion of system files.");
		
		sftd_draw_text(font, 10, 130, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), 11, "Custom themes");
		sftd_draw_textf(font2, 10, 142, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), 10, "Current: %s", theme_dir);
		
		if (bgmEnable)
			sf2d_draw_texture(toggleOn, 280, 50);
		else 
			sf2d_draw_texture(toggleOff, 280, 50);
		
		if (sysProtection)
			sf2d_draw_texture(toggleOn, 280, 90);
		else 
			sf2d_draw_texture(toggleOff, 280, 90);
		
		sf2d_draw_texture(themeIcon, 283, 125);
	}
	else
		sf2d_draw_texture(settingsIcon, 50, 1);
	
	if (DEFAULT_STATE == STATE_UPDATE)
	{
		sf2d_draw_texture(s_UpdateIcon, 75, 0);
		sftd_draw_text(font, ((320 - (sftd_get_text_width(font, 11, "Checking for updates..."))) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "Checking for updates...");
		wait(100000000);
		
		if (strcmp(checkForUpdate(), "Update found") == 0)
		{
			wait(100000000);
			downloadUpdate();
		}
	}
	else
		sf2d_draw_texture(updateIcon, 75, 0);
	
	sf2d_draw_texture(ftpIcon, 100, 0);
	
	if (DEFAULT_STATE == STATE_THEME)
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Select a theme")) / 2), 40, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), 11, "Select a theme");
	
	if (DEFAULT_STATE == STATE_DOWNLOAD)
	{
		sf2d_draw_texture(s_dlIcon, 125, 0);
		
		sf2d_draw_rectangle(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
		
		sftd_draw_textf(font, 10, 40, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "Enter URL: %s", dl_url);	
	
		sftd_draw_textf(font, 10, 60, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "%lu", dl_size);	
	}
	else
		sf2d_draw_texture(dlIcon, 125, 0);
	
	if (BROWSE_STATE == STATE_SD)
		sf2d_draw_texture(s_SdIcon, 150, 0);
	else
		sf2d_draw_texture(sdIcon, 150, 0);
	
	if (BROWSE_STATE == STATE_NAND)
		sf2d_draw_texture(s_NandIcon, 175, 0);
	else
		sf2d_draw_texture(nandIcon, 175, 0);
	
	sf2d_draw_texture(searchIcon, (320 - searchIcon->width), -2);
	
	if (DEFAULT_STATE == STATE_OPTIONS)
	{
		sf2d_draw_texture(s_OptionsIcon, 25, 0);
		
		sf2d_draw_texture(options, 37, 20);
	
		sf2d_draw_rectangle(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(Options_select_colour.r, Options_select_colour.g, Options_select_colour.b, 255));
			
		sftd_draw_text(font, 42, 36, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "Actions");
		sftd_draw_text(font2, 232, 196, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, "CANCEL");
		
		sftd_draw_text(font, 47, 72, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Properties");
		sftd_draw_text(font, 47, 109,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "New folder");
		sftd_draw_text(font, 47, 146,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Delete");
		
		sftd_draw_text(font, 170, 72,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Rename");
	
		if(copyF == false)
			sftd_draw_text(font, 170, 109,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Copy");
		else
			sftd_draw_text(font, 170, 109,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Paste");
	
		if(cutF == false)
			sftd_draw_text(font, 170, 146,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Cut");
		else
			sftd_draw_text(font, 170, 146,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 11, "Paste");
	}
	else
		sf2d_draw_texture(optionsIcon, 25, 0);
		
	sf2d_end_frame();
	// Ends here
	
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
        		
	sf2d_draw_texture(background, 0, 0);
	
	sftd_draw_textf(font, 84, 28, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), 11, "%.35s", cwd); // Display current path
	
	drawWifiStatus(270, 2);
	drawBatteryStatus(295, 2);
	digitalTime(346, 1);
	
	FS_ArchiveResource	resource = {0};
	
	if (BROWSE_STATE == STATE_SD)
		FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_SD);
	else 
		FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_CTR_NAND);
	
	double totalStorage = (((u64) resource.totalClusters * (u64) resource.clusterSize) / 1024.0 / 1024.0);
	double usedStorage = (totalStorage - (((u64) resource.freeClusters * (u64) resource.clusterSize) / 1024.0 / 1024.0));
	double fill = ((usedStorage / totalStorage) * 208.0);
	
	sf2d_draw_rectangle(83, 47, fill, 2, RGBA8(Storage_colour.r, Storage_colour.g, Storage_colour.b, 255)); // Draw storage bar

	// File Iterator Variable
	int i = 0;

	// Print Counter
	int printed = 0;

	// Draw File List
	File * file = files;
	
	for(; file != NULL; file = file->next)
	{
		// Limit the files per page
		if(printed == FILES_PER_PAGE) 
			break;

		if(position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE))
		{
			// Draw selector
			if(i == position)
				sf2d_draw_texture(selector, 0, 53 + (38 * printed));
			
			sf2d_draw_texture(uncheck, 6, 64 + (38 * printed));
			
			//char * ext = strrchr(file->name, '.');

			if (file->isFolder)
				sf2d_draw_texture(folderIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "mp3") == 0) || (strcmp(get_filename_ext(file->name), "MP3") == 0))
				sf2d_draw_texture(audioIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "txt") == 0) || (strcmp(get_filename_ext(file->name), "TXT") == 0))
				sf2d_draw_texture(txtIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "bin") == 0) || (strcmp(get_filename_ext(file->name), "BIN") == 0) || 
					(strcmp(get_filename_ext(file->name), "firm") == 0) || (strcmp(get_filename_ext(file->name), "FIRM") == 0))
				sf2d_draw_texture(systemIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "rar") == 0) || (strcmp(get_filename_ext(file->name), "RAR") == 0) || 
					(strcmp(get_filename_ext(file->name), "zip") == 0) || (strcmp(get_filename_ext(file->name), "ZIP") == 0))
				sf2d_draw_texture(zipIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "jpg") == 0) || (strcmp(get_filename_ext(file->name), "JPG") == 0) || 
					(strcmp(get_filename_ext(file->name), "png") == 0) || (strcmp(get_filename_ext(file->name), "PNG") == 0) || 
					(strcmp(get_filename_ext(file->name), "gif") == 0) || (strcmp(get_filename_ext(file->name), "GIF") == 0))
				sf2d_draw_texture(imgIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "3dsx") == 0) || (strcmp(get_filename_ext(file->name), "3DSX") == 0) || 
					(strcmp(get_filename_ext(file->name), "cia") == 0) || (strcmp(get_filename_ext(file->name), "CIA") == 0))
				sf2d_draw_texture(appIcon, 30, 58 + (38 * printed));
			else
				sf2d_draw_texture(fileIcon, 30, 58 + (38 * printed));
			
			char buf[64], path[500], size[16];

			strncpy(buf, file->name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			int len = strlen(buf);
			len = 40 - len;
			
			while(len -- > 0)
				strcat(buf, " ");
			
			sftd_draw_textf(font, 70, 60 + (38 * printed), RGBA8(TopScreen_colour.r ,TopScreen_colour.g, TopScreen_colour.b, 255), 11, "%.52s", buf); // Display file name
			
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);
			
			if ((file->isFolder) && (strcmp(file->name, "..") != 0))
				sftd_draw_textf(font2, 70, 75 + (38 * printed), RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), 10, "%s drwxr-x---", getFileModifiedTime(path));
			else if (strcmp(file->name, "..") == 0)
				sftd_draw_text(font2, 70, 75 + (38 * printed), RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), 10, "Parent folder");
			else
			{
				getSizeString(size, getFileSize(path));
				sftd_draw_textf(font2, 70, 75 + (38 * printed), RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), 10, "%s -rw-rw----", getFileModifiedTime(path));
				sftd_draw_textf(font2, 395 - sftd_get_text_width(font2, 10, size), 75 + (38 * printed), RGBA8(TopScreen_colour.r, TopScreen_colour.g, TopScreen_colour.b, 255), 10, "%s", size);
			}
			
			printed++; // Increase printed counter
		}

		i++; // Increase Counter
	}
	
	endDrawing();
}

void openFile(void)
{
	File * file = findindex(position);

	if(file == NULL) 
		return;
	
	char path[1024];
	
	strcpy(fileName, file->name);
	
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);
	
	//char * ext = strrchr(path, '.');
	
	if (file->isFolder)
	{
		// Attempt to navigate to Target
		if(navigate(0) == 0)
		{	
			saveLastDirectory();
			updateList(CLEAR);
			displayFiles(CLEAR);
		}
	}
	
	else if ((strcmp(get_filename_ext(file->name), "png") == 0) || (strcmp(get_filename_ext(file->name), "PNG") == 0))
		displayImage(path, 0);
	else if ((strcmp(get_filename_ext(file->name), "jpg") == 0) || (strcmp(get_filename_ext(file->name), "JPG") == 0))
		displayImage(path, 1);
	/*else if ((strcmp(get_filename_ext(file->name), "gif") == 0) || (strcmp(get_filename_ext(file->name), "GIF") == 0))
		displayImage(path, 2);
	else if ((strcmp(get_filename_ext(file->name), "txt") == 0) || (strcmp(get_filename_ext(file->name), "TXT") == 0))
		displayText(path);
	else if ((strcmp(get_filename_ext(file->name), "cia") == 0) || (strcmp(get_filename_ext(file->name), "CIA") == 0))
		installCIA(path);*/
	else if ((strcmp(get_filename_ext(file->name), "zip") == 0) || (strcmp(get_filename_ext(file->name), "ZIP") == 0))
	{
		extractZip(path, cwd);
		updateList(CLEAR);
		displayFiles(CLEAR);
	}
}

// Navigate to Folder
int navigate(int _case)
{
	// Find File
	File * file = findindex(position);

	// Not a Folder
	if(file == NULL || !file->isFolder) 
		return -1;

	// Special Case ".."
	if((_case == -1) || (strcmp(file->name, "..") == 0))
	{
		// Slash Pointer
		char * slash = NULL;

		// Find Last '/' in Working Directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--)
		{
			// Slash discovered
			if(cwd[i] == '/')
			{
				// Save Pointer
				slash = cwd + i + 1;

				// Stop Search
				break;
			}
		}

		// Terminate Working Directory
		slash[0] = 0;
		saveLastDirectory();
	}

	// Normal Folder
	else
	{
		// Append Folder to Working Directory
		strcpy(cwd + strlen(cwd), file->name);
		cwd[strlen(cwd) + 1] = 0;
		cwd[strlen(cwd)] = '/';
		
		saveLastDirectory();
	}

	// Return Success
	return 0;
}

// Find File Information by Index
File * findindex(int index)
{
	// File Iterator Variable
	int i = 0;

	// Find File Item
	File * file = files; 
	
	for(; file != NULL && i != index; file = file->next) 
		i++;

	// Return File
	return file;
}

int drawDeletionDialog()
{	
	touchPosition touch;
	
	while(deleteDialog == true)
	{
		hidScanInput();
		hidTouchRead(&touch);
		u32 kPress = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

		sf2d_draw_texture(deletion, 20, 55);
		
		
		sftd_draw_text(font, 27, 72, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "Confirm deletion");
		
		sftd_draw_text(font2, 206, 159, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, "NO");
		sftd_draw_text(font2, 255, 159, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, "YES");
	
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "This action cannot be undone.")) / 2), 100, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 11, "This action cannot be undone.");
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Do you want to continue?")) / 2), 115, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 11, "Do you want to continue?");
			
		endDrawing();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
			
		if ((kPress & KEY_A) || (touchInRect(240, 320, 142, 185)))
		{
			if(delete() == 0)
			{
				updateList(CLEAR);
				displayFiles(CLEAR);
			}
			
			deleteDialog = false;
		}
		else if ((kPress & KEY_B) || (touchInRect(136, 239, 142, 185)))
			deleteDialog = false;
	}
	
	return 0;
}

int displayProperties()
{
	File * file = findindex(position);

	if(file == NULL) 
		return -1;
	
	char path[255], fullPath[500];
	
	strcpy(fileName, file->name);
	
	strcpy(path, cwd);
	strcpy(fullPath, cwd);
	strcpy(fullPath + strlen(fullPath), fileName);
	
	char size[16];
	getSizeString(size, getFileSize(fullPath));
	
	touchPosition touch;
	
	while(properties == true)
	{
		hidScanInput();
		hidTouchRead(&touch);
		u32 kPress = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT);
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
		
		sf2d_draw_texture(_properties, 36, 20);
		
		sftd_draw_text(font, 41, 33, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 11, "Properties");
		
		sftd_draw_text(font2, 247, 201, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), 10, "OK");
	
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "INFO")) / 2), 50,RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 11, "INFO");
		
		sftd_draw_text(font2, 42, 74, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, "Name:");
			sftd_draw_textf(font2, 100, 74,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "%.36s", fileName);
		sftd_draw_text(font2, 42, 94,RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, "Parent:");
			sftd_draw_textf(font2, 100, 94,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "%s", path);
			
		sftd_draw_text(font2, 42, 114,RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, "Type:");	
		if (file->isFolder)
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "Folder");
		else if ((strcmp(get_filename_ext(fileName), "CIA") == 0) || (strcmp(get_filename_ext(fileName), "cia") == 0) || (strcmp(get_filename_ext(fileName), "3DSX") == 0) || (strcmp(get_filename_ext(fileName), "3dsx") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "APP");
		else if ((strcmp(get_filename_ext(fileName), "bin") == 0) || (strcmp(get_filename_ext(fileName), "BIN") == 0) || (strcmp(get_filename_ext(fileName), "firm") == 0) || (strcmp(get_filename_ext(fileName), "FIRM") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "Payload");
		else if ((strcmp(get_filename_ext(fileName), "zip") == 0) || (strcmp(get_filename_ext(fileName), "ZIP") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "archive/zip");
		else if ((strcmp(get_filename_ext(fileName), "rar") == 0) || (strcmp(get_filename_ext(fileName), "RAR") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "archive/rar");
		else if ((strcmp(get_filename_ext(fileName), "PNG") == 0) || (strcmp(get_filename_ext(fileName), "png") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "image/png");
		else if ((strcmp(get_filename_ext(fileName), "JPG") == 0) || (strcmp(get_filename_ext(fileName), "jpg") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "image/jpeg");
		else if ((strcmp(get_filename_ext(fileName), "MP3") == 0) || (strcmp(get_filename_ext(fileName), "mp3") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "audio/mpeg");
		else if ((strcmp(get_filename_ext(fileName), "txt") == 0) || (strcmp(get_filename_ext(fileName), "TXT") == 0) || (strcmp(get_filename_ext(fileName), "XML") == 0) || (strcmp(get_filename_ext(fileName), "xml") == 0))
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "TEXT");
		else
			sftd_draw_textf(font2, 100, 114,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "FILE");
			
		if (!(file->isFolder))
		{
			sftd_draw_text(font2, 42, 134,RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), 10, "Size:");
				sftd_draw_textf(font2, 100, 134,  RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), 10, "%s", size);
		}
		
		endDrawing();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
		
		if ((kPress & KEY_B) || (kPress & KEY_A) || (touchInRect(36, 284, 192, 220)))
			properties = false;
	}
	
	return 0;
}

void newFolder()
{
	char tempFolder[256];
	strcpy(tempFolder, keyboard_3ds_get(256, "Enter name"));
	
	if (strcmp(tempFolder, "") == 0)
		mainMenu(KEEP);
	
	char path[500];
	strcpy(path, cwd);
	
	strcat(path, tempFolder);
	
	makeDir(path);
	
	mainMenu(CLEAR);	
}

int renameF()
{
	File * file = findindex(position);

	if(file == NULL) 
		return -1;

	if(strcmp(file->name, "..") == 0) 
		return -2;

	char oldPath[500], newPath[500], name[255], ext[6];

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);
	strcpy(ext, ""); // Set ext to NULL
	
	if (!(file->isFolder)) // If it's not a folder preserve its extension to avoid file corruption.
	{
		strcat(ext, ".");
		strcat(ext, get_filename_ext(file->name));
	}
	
	strcpy(name, keyboard_3ds_get(255, "Enter name"));
	strcat(newPath, name);
	
	if (strcmp(get_filename_ext(name), "") == 0) // Second check for ext, this is if the user does not specify a file extension.
		strcat(newPath, ext);
	
	rename(oldPath, newPath);
	
	mainMenu(CLEAR);
	
	return 0;
}

int delete_folder_recursive(char * path)
{
	// Internal File List
	File * filelist = NULL;

	// Open Working Directory
	DIR * directory = opendir(path);

	// Opened Directory
	if(directory)
	{
		struct dirent * info;

		while ((info = readdir(directory)) != NULL)
		{	
			// Valid Filename
			if(strlen(info->d_name) > 0)
			{
				if(strcmp(info->d_name, ".") == 0 || strcmp(info->d_name, "..") == 0)
					continue;

				// Allocate Memory
				File * item = (File *)malloc(sizeof(File));

				// Clear Memory
				memset(item, 0, sizeof(File));

				// Copy File Name
				strcpy(item->name, info->d_name);

				// Set Folder Flag
				item->isFolder = info->d_type == DT_DIR;

				// New List
				if(filelist == NULL) 
					filelist = item;

				// Existing List
				else
				{
					// Iterator Variable
					File * list = filelist;

					// Append to List
					while(list->next != NULL) 
						list = list->next;

					// Link Item
					list->next = item;
				}
			}
		}
	}

	// Close Directory
	closedir(directory);
	
	// List Node
	File * node = filelist;

	// Iterate Files
	for(; node != NULL; node = node->next)
	{
		// Directory
		if(node->isFolder)
		{
			// Required Buffer Size
			int size = strlen(path) + strlen(node->name) + 2;

			// Allocate Buffer
			char * buffer = (char *)malloc(size);

			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);
			buffer[strlen(buffer) + 1] = 0;
			buffer[strlen(buffer)] = '/';

			// Recursion Delete
			delete_folder_recursive(buffer);

			// Free Memory
			free(buffer);
		}

		// File
		else
		{
			// Required Buffer Size
			int size = strlen(path) + strlen(node->name) + 1;

			// Allocate Buffer
			char * buffer = (char *)malloc(size);

			// Combine Path
			strcpy(buffer, path);
			strcpy(buffer + strlen(buffer), node->name);

			// Delete File
			remove(buffer);

			// Free Memory
			free(buffer);
		}
	}

	// Free temporary List
	recursiveFree(filelist);

	// Delete now empty Folder
	return rmdir(path);
}

int delete(void)
{
	// Find File
	File * file = findindex(position);

	// Not found
	if(file == NULL) 
		return -1;

	if (sysProtection)
	{
		if((strcmp(file->name, "..") == 0) || (SYS_FILES)) 
			return -2;
	}
	else
	{
		if(strcmp(file->name, "..") == 0) 
			return -2;
	}

	// File Path
	char path[1024];

	// Puzzle Path
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	// Delete Folder
	if (file->isFolder)
	{
		// Add Trailing Slash
		path[strlen(path) + 1] = 0;
		path[strlen(path)] = '/';

		// Delete Folder
		return delete_folder_recursive(path);
	}

	// Delete File
	else 
		return remove(path);
}

// Copy File or Folder
void copy(int flag)
{
	// Find File
	File * file = findindex(position);

	// Not found
	if(file == NULL) 
		return;

	// Copy File Source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);

	// Add Recursive Folder Flag
	if ((file->isFolder) && (strcmp(file->name, "..") != 0))
		flag |= COPY_FOLDER_RECURSIVE;

	// Set Copy Flags
	copymode = flag;
}

// Copy File from A to B
int copy_file(char * a, char * b)
{
	// Chunk Size
	int chunksize = 512 * 1024;

	// Reading Buffer
	char * buffer = (char *)malloc(chunksize);

	// Accumulated Writing
	int totalwrite = 0;

	// Accumulated Reading
	int totalread = 0;

	// Result
	int result = 0;

	// Open File for Reading
	int in = open(a, O_RDONLY, 0777);

	// Opened File for Reading
	if(in >= 0)
	{
		// Delete Output File (if existing)
		remove(b);

		// Open File for Writing
		int out = open(b, O_WRONLY | O_CREAT | O_TRUNC, 0777);

		// Opened File for Writing
		if(out >= 0)
		{
			// Read Byte Count
			int b_read = 0;

			// Copy Loop (512KB at a time)
			while((b_read = read(in, buffer, chunksize)) > 0)
			{
				// Accumulate Read Data
				totalread += b_read;

				// Write Data
				totalwrite += write(out, buffer, b_read);
			}

			// Close Output File
			close(out);

			// Insufficient Copy
			if(totalread != totalwrite) 
				result = -3;
		}

		// Output Open Error
		else 
			result = -2;

		// Close Input File
		close(in);
	}

	// Input Open Error
	else 
		result = -1;

	// Free Memory
	free(buffer);

	// Return Result
	return result;
}

// Copy Folder from A to B
int copy_folder_recursive(char * a, char * b)
{
	// Open Working Directory
	DIR * directory = opendir(a);

	// Opened Directory
	if(directory)
	{
		// Create Output Directory (is allowed to fail, we can merge folders after all)
		mkdir(b, 0777);
		
		struct dirent * info;

		// Iterate Files
		while ((info = readdir(directory)) != NULL)
		{
			// Valid Filename
			if(strlen(info->d_name) > 0)
			{
				// Calculate Buffer Size
				int insize = strlen(a) + strlen(info->d_name) + 2;
				int outsize = strlen(b) + strlen(info->d_name) + 2;

				// Allocate Buffer
				char * inbuffer = (char *)malloc(insize);
				char * outbuffer = (char *)malloc(outsize);

				// Puzzle Input Path
				strcpy(inbuffer, a);
				inbuffer[strlen(inbuffer) + 1] = 0;
				inbuffer[strlen(inbuffer)] = '/';
				strcpy(inbuffer + strlen(inbuffer), info->d_name);

				// Puzzle Output Path
				strcpy(outbuffer, b);
				outbuffer[strlen(outbuffer) + 1] = 0;
				outbuffer[strlen(outbuffer)] = '/';
				strcpy(outbuffer + strlen(outbuffer), info->d_name);

				// Another Folder
				if(info->d_type == DT_DIR)
					copy_folder_recursive(inbuffer, outbuffer); // Copy Folder (via recursion)

				// Simple File
				else
					copy_file(inbuffer, outbuffer); // Copy File

				// Free Buffer
				free(inbuffer);
				free(outbuffer);
			}
		}

		// Close Directory
		closedir(directory);

		// Return Success
		return 0;
	}
	
	// Open Error
	else 
		return -1;
}

// Paste File or Folder
int paste(void)
{
	// No Copy Source
	if(copymode == NOTHING_TO_COPY) 
		return -1;

	// Source and Target Folder are identical
	char * lastslash = NULL; 
	int i = 0; 
	
	for(; i < strlen(copysource); i++) 
		if(copysource[i] == '/') 
			lastslash = copysource + i;
		
	char backup = lastslash[1];
	lastslash[1] = 0;
	int identical = strcmp(copysource, cwd) == 0;
	lastslash[1] = backup;
	
	if(identical) 
		return -2;

	// Source Filename
	char * filename = lastslash + 1;

	// Required Target Path Buffer Size
	int requiredlength = strlen(cwd) + strlen(filename) + 1;

	// Allocate Target Path Buffer
	char * copytarget = (char *)malloc(requiredlength);

	// Puzzle Target Path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);

	// Return Result
	int result = -3;

	// Recursive Folder Copy
	if((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE)
	{
		// Check Files in current Folder
		File * node = files; for(; node != NULL; node = node->next)
		{
			// Found a file matching the name (folder = ok, file = not)
			if(strcmp(filename, node->name) == 0 && !node->isFolder)
				return -4; // Error out
		}

		// Copy Folder recursively
		result = copy_folder_recursive(copysource, copytarget);

		// Source Delete
		if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
		{
			// Append Trailing Slash (for recursion to work)
			copysource[strlen(copysource) + 1] = 0;
			copysource[strlen(copysource)] = '/';

			// Delete Source
			delete_folder_recursive(copysource);
		}
	}

	// Simple File Copy
	else
	{
		// Copy File
		result = copy_file(copysource, copytarget);

		// Source Delete
		if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			remove(copysource); // Delete File
	}

	// Paste Success
	if(result == 0)
	{
		// Erase Cache Data
		memset(copysource, 0, sizeof(copysource));
		copymode = NOTHING_TO_COPY;
	}

	// Free Target Path Buffer
	free(copytarget);

	// Return Result
	return result;
}