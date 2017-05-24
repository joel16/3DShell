#include "act.h"
#include "clock.h"
#include "fs.h"
#include "gallery.h"
#include "keyboard.h"
#include "main.h"
#include "mcu.h"
#include "power.h"
#include "screenshot.h"
#include "sound.h"
#include "text.h"
#include "updater.h"
#include "utils.h"

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
}

//static struct sound *bgm;

void initServices()
{
	fsInit();
	sdmcInit();
	openSdArchive();
	mcuInit();
	hidInit();
	acInit();
	actInit(SDK(11,2,0,200), 0x20000);
	romfsInit();
	sf2d_init();
	sftd_init();
	httpcInit(0);
	amInit();
	AM_InitializeExternalTitleDatabase(false);
	
	audio_init();
	
	sf2d_set_clear_color(RGBA8(0, 0, 0, 255));
	sf2d_set_vblank_wait(0);
	
	background = sfil_load_PNG_file("romfs:/res/background.png", SF2D_PLACE_RAM); setBilinearFilter(background);
	options = sfil_load_PNG_file("romfs:/res/options.png", SF2D_PLACE_RAM); setBilinearFilter(options);
	_properties = sfil_load_PNG_file("romfs:/res/properties.png", SF2D_PLACE_RAM); setBilinearFilter(_properties);
	deletion = sfil_load_PNG_file("romfs:/res/delete.png", SF2D_PLACE_RAM); setBilinearFilter(deletion);
	selector = sfil_load_PNG_file("romfs:/res/selector.png", SF2D_PLACE_RAM); setBilinearFilter(selector);
	folderIcon = sfil_load_PNG_file("romfs:/res/folder.png", SF2D_PLACE_RAM); setBilinearFilter(folderIcon);
	fileIcon = sfil_load_PNG_file("romfs:/res/file.png", SF2D_PLACE_RAM); setBilinearFilter(fileIcon); 
	audioIcon = sfil_load_PNG_file("romfs:/res/audio.png", SF2D_PLACE_RAM); setBilinearFilter(audioIcon);
	appIcon = sfil_load_PNG_file("romfs:/res/app.png", SF2D_PLACE_RAM); setBilinearFilter(appIcon);
	txtIcon = sfil_load_PNG_file("romfs:/res/txt.png", SF2D_PLACE_RAM); setBilinearFilter(txtIcon);
	systemIcon = sfil_load_PNG_file("romfs:/res/system.png", SF2D_PLACE_RAM); setBilinearFilter(systemIcon);
	zipIcon = sfil_load_PNG_file("romfs:/res/zip.png", SF2D_PLACE_RAM); setBilinearFilter(zipIcon);
	imgIcon = sfil_load_PNG_file("romfs:/res/img.png", SF2D_PLACE_RAM); setBilinearFilter(imgIcon);
	uncheck = sfil_load_PNG_file("romfs:/res/uncheck.png", SF2D_PLACE_RAM); setBilinearFilter(uncheck);
	
	homeIcon = sfil_load_PNG_file("romfs:/res/home.png", SF2D_PLACE_RAM); setBilinearFilter(homeIcon);
	optionsIcon = sfil_load_PNG_file("romfs:/res/options_icon.png", SF2D_PLACE_RAM); setBilinearFilter(optionsIcon);
	sdIcon = sfil_load_PNG_file("romfs:/res/sd.png", SF2D_PLACE_RAM); setBilinearFilter(sdIcon);
	nandIcon = sfil_load_PNG_file("romfs:/res/nand.png", SF2D_PLACE_RAM); setBilinearFilter(nandIcon);
	settingsIcon = sfil_load_PNG_file("romfs:/res/settings.png", SF2D_PLACE_RAM); setBilinearFilter(settingsIcon);
	updateIcon = sfil_load_PNG_file("romfs:/res/update.png", SF2D_PLACE_RAM); setBilinearFilter(updateIcon);
	s_HomeIcon = sfil_load_PNG_file("romfs:/res/s_home.png", SF2D_PLACE_RAM); setBilinearFilter(s_HomeIcon);
	s_OptionsIcon = sfil_load_PNG_file("romfs:/res/s_options_icon.png", SF2D_PLACE_RAM); setBilinearFilter(s_OptionsIcon);
	s_SdIcon = sfil_load_PNG_file("romfs:/res/s_sd.png", SF2D_PLACE_RAM); setBilinearFilter(s_SdIcon);
	s_SettingsIcon = sfil_load_PNG_file("romfs:/res/s_settings.png", SF2D_PLACE_RAM); setBilinearFilter(s_SettingsIcon);
	s_NandIcon = sfil_load_PNG_file("romfs:/res/s_nand.png", SF2D_PLACE_RAM); setBilinearFilter(s_NandIcon);
	s_UpdateIcon = sfil_load_PNG_file("romfs:/res/s_update.png", SF2D_PLACE_RAM); setBilinearFilter(s_UpdateIcon);
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
	
	font = sftd_load_font_mem(Roboto_ttf, Roboto_ttf_size);
	font2 = sftd_load_font_mem(Roboto_ttf, Roboto_ttf_size);
	
	osSetSpeedupEnable(true);
	
	installDirectories();
	
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
	sf2d_free_texture(s_HomeIcon);
	sf2d_free_texture(s_OptionsIcon);
	sf2d_free_texture(s_SdIcon);
	sf2d_free_texture(s_SettingsIcon);
	sf2d_free_texture(s_NandIcon);
	sf2d_free_texture(s_UpdateIcon);
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
	amExit();
	httpcExit();
	sftd_fini();
	sf2d_fini();
	romfsExit();
	acExit();
	hidExit();
	mcuExit();
	closeSdArchive();
	sdmcExit();
	fsExit();
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
		
		if ((kPress & KEY_TOUCH) && (touchInRect(98, 123, 0, 20)))
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
		/*else if ((kPress & KEY_TOUCH) && (touchInRect(124, 157, 0, 20))) //Mount stuff goes here
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
				wait(1000000);
				
				openFile(); // Open file/dir
			}
			
			else if ((strcmp(cwd, ROOT_PATH) != 0) && (kPress & KEY_B))
			{
				wait(1000000);
				
				navigate(-1);
				updateList(CLEAR);
				displayFiles(CLEAR);
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

int main()
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
		
	sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(30, 136, 229, 255));
	sf2d_draw_rectangle(0, 0, 320, 20, RGBA8(25, 118, 210, 255));
	
	if (DEFAULT_STATE == STATE_HOME)
	{
		sf2d_draw_texture(s_HomeIcon, -2, -2);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, welcomeMsg)) / 2), 40, RGBA8(251, 251, 251, 255), 11, welcomeMsg);
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, currDate)) / 2), 60, RGBA8(251, 251, 251, 255), 11, currDate);
		sftd_draw_textf(font, 2, 225, RGBA8(251, 251, 251, 255), 11, "3DShell v%i.%i", VERSION_MAJOR, VERSION_MINOR);
	}
	else
		sf2d_draw_texture(homeIcon, -2, -2);
	
	if (DEFAULT_STATE == STATE_SETTINGS)
	{
		sf2d_draw_texture(s_SettingsIcon, 50, 0);
		sf2d_draw_rectangle(0, 20, 320, 220, RGBA8(251, 251, 251, 255));
		
		sftd_draw_text(font, 10, 30, RGBA8(30, 136, 229, 255), 11, "General");
		
		sftd_draw_text(font, 10, 50, RGBA8(32, 32, 32, 255), 11, "BGM");
		sftd_draw_text(font2, 10, 62, RGBA8(120, 120, 120, 255), 10, "Enable BGM upon start up. (/3ds/3DShell/bgm.ogg)");
		
		if (bgmEnable)
			sf2d_draw_texture(toggleOn, 280, 50);
		else 
			sf2d_draw_texture(toggleOff, 280, 50);
	}
	else
		sf2d_draw_texture(settingsIcon, 50, 0);
	
	if (DEFAULT_STATE == STATE_UPDATE)
	{
		sf2d_draw_texture(s_UpdateIcon, 75, 0);
		sftd_draw_text(font, ((320 - (sftd_get_text_width(font, 11, "Checking for updates..."))) / 2), 40, RGBA8(251, 251, 251, 255), 11, "Checking for updates...");
		wait(100000000);
		
		if (strcmp(checkForUpdate(), "Update found") == 0)
		{
			wait(100000000);
			downloadUpdate();
		}
	}
	else
		sf2d_draw_texture(updateIcon, 75, 0);
	
	if (BROWSE_STATE == STATE_SD)
		sf2d_draw_texture(s_SdIcon, 100, 0);
	else
		sf2d_draw_texture(sdIcon, 100, 0);
	
	if (BROWSE_STATE == STATE_NAND)
		sf2d_draw_texture(s_NandIcon, 125, 0);
	else
		sf2d_draw_texture(nandIcon, 125, 0);
	
	sf2d_draw_texture(searchIcon, (320 - searchIcon->width), -2);
	
	if (DEFAULT_STATE == STATE_OPTIONS)
	{
		sf2d_draw_texture(s_OptionsIcon, 25, 0);
		
		sf2d_draw_texture(options, 37, 20);
	
		if (selectionY == 3)
			selectionY = 0;
		if (selectionY == -1)
			selectionY = 2;
		
		if (selectionX == 2)
			selectionX = 0;
		if (selectionX == -1)
			selectionX = 1;
	
		sf2d_draw_rectangle(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(237, 237, 237, 255));
			
		sftd_draw_text(font, 47, 72, RGBA8(100, 100, 100, 255), 11, "Properties");
		sftd_draw_text(font, 47, 109, RGBA8(100, 100, 100, 255), 11, "New folder");
		sftd_draw_text(font, 47, 146, RGBA8(100, 100, 100, 255), 11, "Delete");
		
		sftd_draw_text(font, 170, 72, RGBA8(100, 100, 100, 255), 11, "Rename");
	
		if(copyF == false)
			sftd_draw_text(font, 170, 109, RGBA8(100, 100, 100, 255), 11, "Copy");
		else
			sftd_draw_text(font, 170, 109, RGBA8(100, 100, 100, 255), 11, "Paste");
	
		if(cutF == false)
			sftd_draw_text(font, 170, 146, RGBA8(100, 100, 100, 255), 11, "Cut");
		else
			sftd_draw_text(font, 170, 146, RGBA8(100, 100, 100, 255), 11, "Paste");
	}
	else
		sf2d_draw_texture(optionsIcon, 25, 0);
		
	sf2d_end_frame();
	// Ends here
	
	sf2d_start_frame(GFX_TOP, GFX_LEFT);
        		
	sf2d_draw_texture(background, 0, 0);
	
	sftd_draw_textf(font, 84, 28, RGBA8(255, 255, 255, 255), 11, "%.35s", cwd); // Display current path
	
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
	
	sf2d_draw_rectangle(83, 47, fill, 2, RGBA8(48, 174, 222, 255)); // Draw storage bar

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
			sf2d_draw_texture(uncheck, 6, 64 + (38 * printed));

			// Draw selector
			if(i == position)
				sf2d_draw_texture(selector, 0, 53 + (38 * printed));
			
			//char * ext = strrchr(file->name, '.');

			if (file->isFolder)
				sf2d_draw_texture(folderIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "mp3") == 0) || (strcmp(get_filename_ext(file->name), "MP3") == 0))
				sf2d_draw_texture(audioIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "txt") == 0) || (strcmp(get_filename_ext(file->name), "TXT") == 0))
				sf2d_draw_texture(txtIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "bin") == 0) || (strcmp(get_filename_ext(file->name), "BIN") == 0) || (strcmp(get_filename_ext(file->name), "firm") == 0) || (strcmp(get_filename_ext(file->name), "FIRM") == 0))
				sf2d_draw_texture(systemIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "rar") == 0) || (strcmp(get_filename_ext(file->name), "RAR") == 0) || (strcmp(get_filename_ext(file->name), "zip") == 0) || (strcmp(get_filename_ext(file->name), "ZIP") == 0))
				sf2d_draw_texture(zipIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "jpg") == 0) || (strcmp(get_filename_ext(file->name), "JPG") == 0) || (strcmp(get_filename_ext(file->name), "png") == 0) || (strcmp(get_filename_ext(file->name), "PNG") == 0))
				sf2d_draw_texture(imgIcon, 30, 58 + (38 * printed));
			else if ((strcmp(get_filename_ext(file->name), "3dsx") == 0) || (strcmp(get_filename_ext(file->name), "3DSX") == 0) || (strcmp(get_filename_ext(file->name), "cia") == 0) || (strcmp(get_filename_ext(file->name), "CIA") == 0))
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
			
			sftd_draw_textf(font, 70, 60 + (38 * printed), RGBA8(0, 0, 0, 255), 11, "%.52s", buf); // Display file name
			
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);
			
			if ((file->isFolder) && (strcmp(file->name, "..") != 0))
				sftd_draw_text(font2, 70, 75 + (38 * printed), RGBA8(95, 95, 95, 255), 10, "drwxr-x---");
			else if (strcmp(file->name, "..") == 0)
				sftd_draw_text(font2, 70, 75 + (38 * printed), RGBA8(95, 95, 95, 255), 10, "Parent folder");
			else
			{
				getSizeString(size, getFileSize(path));
				sftd_draw_text(font2, 70, 75 + (38 * printed), RGBA8(95, 95, 95, 255), 10, "-rw-rw----");
				sftd_draw_textf(font2, 395 - sftd_get_text_width(font2, 10, size), 75 + (38 * printed), RGBA8(0, 0, 0, 255), 10, "%s", size);
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
	/*else if ((strcmp(get_filename_ext(file->name), "txt") == 0) || (strcmp(get_filename_ext(file->name), "TXT") == 0))
		displayText(path);*/
	/*else if ((strcmp(get_filename_ext(file->name), "cia") == 0) || (strcmp(get_filename_ext(file->name), "CIA") == 0))
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
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(30, 136, 229, 255));

		sf2d_draw_texture(deletion, 20, 55);
	
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "This action cannot be undone.")) / 2), 100, RGBA8(0, 0, 0, 255), 11, "This action cannot be undone.");
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "Do you want to continue?")) / 2), 115, RGBA8(0, 0, 0, 255), 11, "Do you want to continue?");
			
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
	
	char path[500], fullPath[1024], type[7];
	
	strcpy(fileName, file->name);
	
	strcpy(path, cwd);
	strcpy(fullPath, cwd);
	strcpy(fullPath + strlen(fullPath), file->name);
	
	if (file->isFolder)
		strcpy(type, "Folder");
	else if ((strcmp(get_filename_ext(file->name), "CIA") == 0) || (strcmp(get_filename_ext(file->name), "cia") == 0) || (strcmp(get_filename_ext(file->name), "3DSX") == 0) || (strcmp(get_filename_ext(file->name), "3dsx") == 0))
		strcpy(type, "APP");
	else if ((strcmp(get_filename_ext(file->name), "bin") == 0) || (strcmp(get_filename_ext(file->name), "BIN") == 0) || (strcmp(get_filename_ext(file->name), "firm") == 0) || (strcmp(get_filename_ext(file->name), "FIRM") == 0))
		strcpy(type, "Payload");
	else if ((strcmp(get_filename_ext(file->name), "zip") == 0) || (strcmp(get_filename_ext(file->name), "ZIP") == 0))
		strcpy(type, "archive/zip");
	else if ((strcmp(get_filename_ext(file->name), "rar") == 0) || (strcmp(get_filename_ext(file->name), "RAR") == 0))
		strcpy(type, "archive/rar");
	else if ((strcmp(get_filename_ext(file->name), "PNG") == 0) || (strcmp(get_filename_ext(file->name), "png") == 0))
		strcpy(type, "image/png");
	else if ((strcmp(get_filename_ext(file->name), "JPG") == 0) || (strcmp(get_filename_ext(file->name), "jpg") == 0))
		strcpy(type, "image/jpeg");
	else if ((strcmp(get_filename_ext(file->name), "MP3") == 0) || (strcmp(get_filename_ext(file->name), "mp3") == 0))
		strcpy(type, "audio/mpeg");
	else if ((strcmp(get_filename_ext(file->name), "txt") == 0) || (strcmp(get_filename_ext(file->name), "TXT") == 0) || (strcmp(get_filename_ext(file->name), "XML") == 0) || (strcmp(get_filename_ext(file->name), "xml") == 0))
		strcpy(type, "TEXT");
	else
		strcpy(type, "File");
	
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
		
		sf2d_draw_rectangle(0, 0, 320, 240, RGBA8(30, 136, 229, 255));
		
		sf2d_draw_texture(_properties, 36, 20);
	
		sftd_draw_text(font, ((320 - sftd_get_text_width(font, 11, "INFO")) / 2), 50, RGBA8(0, 0, 0, 255), 11, "INFO");
		
		sftd_draw_text(font2, 42, 74, RGBA8(0, 0, 0, 255), 10, "Name:");
			sftd_draw_textf(font2, 100, 74, RGBA8(100, 100, 100, 255), 10, "%s", fileName);
		sftd_draw_text(font2, 42, 94, RGBA8(0, 0, 0, 255), 10, "Parent:");
			sftd_draw_textf(font2, 100, 94, RGBA8(100, 100, 100, 255), 10, "%s", path);
		sftd_draw_text(font2, 42, 114, RGBA8(0, 0, 0, 255), 10, "Type:");
			sftd_draw_textf(font2, 100, 114, RGBA8(100, 100, 100, 255), 10, "%s", type);
			
		if (!(file->isFolder))
		{
			sftd_draw_text(font2, 42, 134, RGBA8(0, 0, 0, 255), 10, "Size:");
				sftd_draw_textf(font2, 100, 134, RGBA8(100, 100, 100, 255), 10, "%s", size);
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

	if(strcmp(file->name, "..") == 0) 
		return -2;

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