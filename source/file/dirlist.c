#include "archive.h"
#include "cia.h"
#include "clock.h"
#include "common.h"
#include "compile_date.h"
#include "dirlist.h"
#include "file_operations.h"
#include "fs.h"
#include "ftp.h"
#include "gallery.h"
#include "language.h"
#include "main.h"
#include "mcu.h"
#include "net.h"
#include "power.h"
#include "screen.h"
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

void updateList(int clearindex)
{
	recursiveFree(files);
	files = NULL;
	fileCount = 0;
	
	Handle dirHandle;
	Result directory = FSUSER_OpenDirectory(&dirHandle, fsArchive, fsMakePath(PATH_ASCII, cwd));
	
	u32 entriesRead;
	static char dname[1024];

	if(!(directory))
	{
		/* Add fake ".." entry except on root */
		if (strcmp(cwd, ROOT_PATH)) 
		{
			// New list
			files = (File *)malloc(sizeof(File));

			// Clear memory
			memset(files, 0, sizeof(File));

			// Copy file Name
			strcpy(files->name, "..");

			// Set folder flag
			files->isDir = 1;

			fileCount++;
		}

		do
		{
			static FS_DirectoryEntry info;
			memset(&info, 0, sizeof(FS_DirectoryEntry));
			
			entriesRead = 0;
			FSDIR_Read(dirHandle, &entriesRead, 1, &info);
			
			if(entriesRead)
			{
				utfn2ascii(&dname[0], info.name, 255);
				
				// Ingore null filename
				if(dname[0] == '\0') 
					continue;

				// Ignore "." in all Directories
				if(strncmp(dname, ".", 1) == 0) 
					continue;

				// Ignore ".." in Root Directory
				if(strcmp(cwd, ROOT_PATH) == 0 && strncmp(dname, "..", 2) == 0) 
					continue;

				// Allocate memory
				File * item = (File *)malloc(sizeof(File));

				// Clear memory
				memset(item, 0, sizeof(File));

				// Copy file name
				strcpy(item->name, dname);

				// Set folder flag
				item->isDir = info.attributes & FS_ATTRIBUTE_DIRECTORY;
				
				// Set read-Only flag
				item->isReadOnly = info.attributes & FS_ATTRIBUTE_READ_ONLY; 
				
				// Set read-Only flag
				item->isHidden = info.attributes & FS_ATTRIBUTE_HIDDEN; 
				
				// Copy file extension
				strcpy(item->ext, info.shortExt);
				
				// Copy file size
				item->size = info.fileSize;

				// New list
				if(files == NULL) 
					files = item;

				// Existing list
				else
				{
					// Iterator variable
					File * list = files;

					// Append to list
					while(list->next != NULL) list = list->next;

					// Link item
					list->next = item;
				}

				// Increase file count
				fileCount++;
			}
		} 
		while(entriesRead);

		// Close directory
		FSDIR_Close(dirHandle);
	}

	// Attempt to keep index
	if(!clearindex)
	{
		if(position >= fileCount) 
			position = fileCount - 1; // Fix position
	}

	// Reset position
	else 
		position = 0;
}

void recursiveFree(File * node)
{
	// End of list
	if(node == NULL)
		return;

	// Nest further
	recursiveFree(node->next);

	// Free memory
	free(node);
}

void displayFiles(void)
{
	screen_begin_frame();
	screen_select(GFX_BOTTOM);
		
	screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
	screen_draw_rect(0, 0, 320, 20, RGBA8(BottomScreen_bar_colour.r, BottomScreen_bar_colour.g, BottomScreen_bar_colour.b, 255));
	
	if (DEFAULT_STATE == STATE_HOME)
	{
		screen_draw_texture(TEXTURE_HOME_ICON_SELECTED, -2, -2);
		screen_draw_string(((320 - screen_get_string_width(welcomeMsg, 0.41f, 0.41f)) / 2), 40, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), welcomeMsg);
		screen_draw_string(((320 - screen_get_string_width(currDate, 0.41f, 0.41f)) / 2), 60, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), currDate);
		screen_draw_stringf(2, 225, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), "3DShell v%i.%i Beta - %d%02d%02d", VERSION_MAJOR, VERSION_MINOR, YEAR, MONTH, DAY);
	}
	else
		screen_draw_texture(TEXTURE_HOME_ICON, -2, -2);
	
	if (DEFAULT_STATE == STATE_SETTINGS)
	{
		screen_draw_texture(TEXTURE_SETTINGS_ICON_SELECTED, 50, 1);
		screen_draw_rect(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
		
		screen_draw_string(10, 30, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_settings[language][0]);
		
		screen_draw_string(10, 50, 0.41f, 0.41f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), lang_settings[language][5]); // Grey'd out - cannot be accessed yet.
		screen_draw_stringf(10, 62, 0.41f, 0.41f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "%s (/3ds/3DShell/bgm.ogg)", lang_settings[language][6]);
		
		screen_draw_string(10, 90, 0.41f, 0.41f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][1]);
		screen_draw_string(10, 102, 0.41f, 0.41f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), lang_settings[language][2]);
		
		screen_draw_string(10, 130, 0.41f, 0.41f, RGBA8(Settings_text_colour.r, Settings_text_colour.g, Settings_text_colour.b, 255), lang_settings[language][3]);
		screen_draw_stringf(10, 142, 0.41f, 0.41f, RGBA8(Settings_text_min_colour.r, Settings_text_min_colour.g, Settings_text_min_colour.b, 255), "%s %s", lang_settings[language][4], theme_dir);
		
		if (bgmEnable)
			screen_draw_texture(TEXTURE_TOGGLE_ON, 280, 50);
		else 
			screen_draw_texture(TEXTURE_TOGGLE_OFF, 280, 50);
		
		if (sysProtection)
			screen_draw_texture(TEXTURE_TOGGLE_ON, 280, 90);
		else 
			screen_draw_texture(TEXTURE_TOGGLE_OFF, 280, 90);
		
		screen_draw_texture(TEXTURE_THEME_ICON, 283, 125);
	}
	else
		screen_draw_texture(TEXTURE_SETTINGS_ICON, 50, 1);
	
	/*if (DEFAULT_STATE == STATE_UPDATE)
	{
		screen_draw_texture(TEXTURE_UPDATE_ICON_SELECTED, 75, 0);
		screen_draw_string(((320 - (screen_get_string_width(lang_update[language][0], 0.41f, 0.41f))) / 2), 40, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_update[language][0]);
		
		Result ret = installCIA("/3ds/3DShell/3DShell.cia", MEDIATYPE_SD, true);
		if (ret != 0)
			screen_draw_string(((320 - (screen_get_string_width(lang_update[language][1], 0.41f, 0.41f))) / 2), 60, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_update[language][1]);
		else
			screen_draw_string(((320 - (screen_get_string_width(lang_update[language][2], 0.41f, 0.41f))) / 2), 60, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_update[language][2]);
		
		screen_draw_string(((320 - (screen_get_string_width(lang_update[language][3], 0.41f, 0.41f))) / 2), 80, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_update[language][3]);
		
		wait(300000000);
		longjmp(exitJmp, 1);
	}
	else*/
		screen_draw_texture(TEXTURE_UPDATE_ICON, 75, 0);
	
	screen_draw_texture(TEXTURE_FTP_ICON, 100, 0);
	
	if (DEFAULT_STATE == STATE_THEME)
		screen_draw_string(((320 - screen_get_string_width(lang_themes[language][0], 0.41f, 0.41f)) / 2), 40, 0.41f, 0.41f, RGBA8(BottomScreen_text_colour.r, BottomScreen_text_colour.g , BottomScreen_text_colour.b, 255), lang_themes[language][0]);
	
	if (DEFAULT_STATE == STATE_DOWNLOAD)
	{
		screen_draw_texture(TEXTURE_DOWNLOAD_ICON_SELECTED, 125, 0);
		
		screen_draw_rect(0, 20, 320, 220, RGBA8(Settings_colour.r, Settings_colour.g, Settings_colour.b, 255));
		
		screen_draw_stringf(10, 40, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "Enter URL: %s", dl_url);	
	
		screen_draw_stringf(10, 60, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), "%lu", dl_size);	
	}
	else
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
	
	if (DEFAULT_STATE == STATE_OPTIONS)
	{
		screen_draw_texture(TEXTURE_OPTIONS_ICON_SELECTED, 25, 0);
		
		screen_draw_texture(TEXTURE_OPTIONS, 37, 20);
	
		screen_draw_rect(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(Options_select_colour.r, Options_select_colour.g, Options_select_colour.b, 255));
			
		screen_draw_string(42, 36, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][0]);
		screen_draw_string(232, 196, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][8]);
		
		screen_draw_string(47, 72, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][1]);
		screen_draw_string(47, 109, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][3]);
		screen_draw_string(47, 146, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][5]);
		
		screen_draw_string(170, 72, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][2]);
	
		if(copyF == false)
			screen_draw_string(170, 109, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
		else
			screen_draw_string(170, 109, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
	
		if(cutF == false)
			screen_draw_string(170, 146, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
		else
			screen_draw_string(170, 146, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
	}
	else
		screen_draw_texture(TEXTURE_OPTIONS_ICON, 25, 0);
		
	screen_select(GFX_TOP);
        		
	screen_draw_texture(TEXTURE_BACKGROUND, 0, 0);
	
	screen_draw_stringf(84, 28, 0.44f, 0.44f, RGBA8(TopScreen_bar_colour.r, TopScreen_bar_colour.g, TopScreen_bar_colour.b, 255), "%.35s", cwd); // Display current path
	
	drawWifiStatus(270, 2);
	drawBatteryStatus(295, 2);
	digitalTime();
	
	FS_ArchiveResource	resource = {0};
	
	if (BROWSE_STATE == STATE_SD)
		FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_SD);
	else 
		FSUSER_GetArchiveResource(&resource, SYSTEM_MEDIATYPE_CTR_NAND);
	
	double totalStorage = (((u64) resource.totalClusters * (u64) resource.clusterSize) / 1024.0 / 1024.0);
	double usedStorage = (totalStorage - (((u64) resource.freeClusters * (u64) resource.clusterSize) / 1024.0 / 1024.0));
	double fill = ((usedStorage / totalStorage) * 208.0);
	
	screen_draw_rect(83, 47, fill, 2, RGBA8(Storage_colour.r, Storage_colour.g, Storage_colour.b, 255)); // Draw storage bar

	// File iterator variable
	int i = 0;

	// Print counter
	int printed = 0;

	// Draw file list
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
				screen_draw_texture(TEXTURE_SELECTOR, 0, 53 + (38 * printed));
			
			char path[500];
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);

			screen_draw_texture(TEXTURE_UNCHECK_ICON, 8, 66 + (38 * printed));

			if (file->isDir)
				screen_draw_texture(TEXTURE_FOLDER_ICON, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "3ds", 3) == 0) || (strncmp(file->ext, "3DS", 3) == 0) || 
					(strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "CIA", 3) == 0))
				screen_draw_texture(TEXTURE_APP_ICON, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "mp3", 3) == 0) || (strncmp(file->ext, "MP3", 3) == 0))
				screen_draw_texture(TEXTURE_AUDIO_ICON, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "jpg", 3) == 0) || (strncmp(file->ext, "JPG", 3) == 0) || 
					(strncmp(file->ext, "png", 3) == 0) || (strncmp(file->ext, "PNG", 3) == 0) || 
					(strncmp(file->ext, "gif", 3) == 0) || (strncmp(file->ext, "GIF", 3) == 0) ||
					(strncmp(file->ext, "bmp", 3) == 0) || (strncmp(file->ext, "BMP", 3) == 0))
				screen_draw_texture(TEXTURE_IMG_ICON, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "bin", 3) == 0) || (strncmp(file->ext, "BIN", 3) == 0) || 
					(strncmp(file->ext, "fir", 3) == 0) || (strncmp(file->ext, "FIR", 3) == 0))
				screen_draw_texture(TEXTURE_SYSTEM_ICON, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0))
				screen_draw_texture(TEXTURE_TXT_ICON, 30, 58 + (38 * printed));
			else if ((strncmp(file->ext, "rar", 3) == 0) || (strncmp(file->ext, "RAR", 3) == 0) || 
					(strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
				screen_draw_texture(TEXTURE_ZIP_ICON, 30, 58 + (38 * printed));
			else
				screen_draw_texture(TEXTURE_FILE_ICON, 30, 58 + (38 * printed));
			
			char buf[64], size[16];

			strncpy(buf, file->name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';
			int len = strlen(buf);
			len = 40 - len;
			
			while(len -- > 0)
				strcat(buf, " ");
			
			screen_draw_stringf(70, 60 + (38 * printed), 0.44f, 0.44f, RGBA8(TopScreen_colour.r ,TopScreen_colour.g, TopScreen_colour.b, 255), "%.52s", buf); // Display file name
			
			if ((file->isDir) && (strncmp(file->name, "..", 2) != 0))
			{
				if (file->isReadOnly)
					screen_draw_stringf(70, 75 + (38 * printed), 0.41f, 0.41f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s dr-xr-x---", getFileModifiedTime(path));
				else
					screen_draw_stringf(70, 75 + (38 * printed), 0.41f, 0.41f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s drwxr-x---", getFileModifiedTime(path));
				
			}
			else if (strncmp(file->name, "..", 2) == 0)
				screen_draw_string(70, 75 + (38 * printed), 0.44f, 0.44f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), lang_files[language][0]);
			else
			{
				getSizeString(size, file->size);
				
				if (file->isReadOnly)
					screen_draw_stringf(70, 75 + (38 * printed), 0.41f, 0.41f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s -r--r-----", getFileModifiedTime(path));
				else
					screen_draw_stringf(70, 75 + (38 * printed), 0.41f, 0.41f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s -rw-rw----", getFileModifiedTime(path));
				
				screen_draw_stringf(395 - screen_get_string_width(size, 0.41f, 0.41f), 75 + (38 * printed), 0.41f, 0.41f, RGBA8(TopScreen_colour.r, TopScreen_colour.g, TopScreen_colour.b, 255), "%s", size);
			}
			
			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}
	
	// length is 187
	//screen_draw_stringf(5, 1, 0.44f, 0.44f, RGBA8(181, 181, 181, 255), "%d/%d", (position + 1), fileCount); // debug stuff
	
	screen_end_frame();
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
	
	if (file->isDir)
	{
		// Attempt to navigate to target
		if(navigate(0) == 0)
		{	
			if (BROWSE_STATE != STATE_NAND)
				saveLastDirectory();
			
			updateList(CLEAR);
			displayFiles();
		}
	}
	
	else if ((strncmp(file->ext, "png", 3) == 0) || (strncmp(file->ext, "PNG", 3) == 0) || (strncmp(file->ext, "jpg", 3) == 0) || (strncmp(file->ext, "JPG", 3) == 0) ||
	(strncmp(file->ext, "gif", 3) == 0) || (strncmp(file->ext, "GIF", 3) == 0) || (strncmp(file->ext, "bmp", 3) == 0) || (strncmp(file->ext, "BMP", 3) == 0))
		displayImage(path);
	else if ((strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "CIA", 3) == 0))
		displayCIA(path);
	else if ((strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
	{
		extractZip(path, cwd);
		updateList(CLEAR);
		displayFiles();
	}
	else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0))
		displayText(path);
}

// Navigate to Folder
int navigate(int _case)
{
	// Find file
	File * file = findindex(position);

	// Not a folder
	if(file == NULL || !file->isDir) 
		return -1;

	// Special case ".."
	if((_case == -1) || (strncmp(file->name, "..", 2) == 0))
	{
		// Slash pointer
		char * slash = NULL;

		// Find last '/' in working directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--)
		{
			// Slash discovered
			if(cwd[i] == '/')
			{
				// Save pointer
				slash = cwd + i + 1;

				// Stop search
				break;
			}
		}

		// Terminate working directory
		slash[0] = 0;
		
		if (BROWSE_STATE != STATE_NAND)
			saveLastDirectory();
	}

	// Normal folder
	else
	{
		// Append folder to working directory
		strcpy(cwd + strlen(cwd), file->name);
		cwd[strlen(cwd) + 1] = 0;
		cwd[strlen(cwd)] = '/';
		
		if (BROWSE_STATE != STATE_NAND)
			saveLastDirectory();
	}

	// Return success
	return 0;
}

// Find file information by index
File * findindex(int index)
{
	// File iterator variable
	int i = 0;

	// Find file Item
	File * file = files; 
	
	for(; file != NULL && i != index; file = file->next) 
		i++;

	// Return file
	return file;
}

int drawDeletionDialog(void)
{	
	while(deleteDialog == true)
	{	
		hidScanInput();
		hidTouchRead(&touch);
		
		screen_begin_frame();
		screen_select(GFX_BOTTOM);
		
		screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

		screen_draw_texture(TEXTURE_DELETE, 20, 55);
		
		screen_draw_string(27, 72, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][0]);
		
		screen_draw_string(206, 159, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][3]);
		screen_draw_string(255, 159, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][4]);
	
		screen_draw_string(((320 - screen_get_string_width(lang_deletion[language][1], 0.41f, 0.41f)) / 2), 100, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_deletion[language][1]);
		screen_draw_string(((320 - screen_get_string_width(lang_deletion[language][2], 0.41f, 0.41f)) / 2), 115, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_deletion[language][2]);
			
		screen_end_frame();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
			
		if ((kPressed & KEY_A) || ((touchInRect(240, 320, 142, 185))  && (kPressed & KEY_TOUCH)))
		{
			if(delete() == 0)
			{
				updateList(CLEAR);
				displayFiles();
			}
			
			break;
		}
		else if ((kPressed & KEY_B) || ((touchInRect(136, 239, 142, 185))  && (kPressed & KEY_TOUCH)))
			break;
	}
	
	deleteDialog = false;
	selectionX = 0;
	selectionY = 0;
	copyF = false;
	cutF = false;
	return 0;
}

int displayProperties(void)
{
	File * file = findindex(position);

	if(file == NULL) 
		return -1;
	
	char path[255], fullPath[500];
	
	strcpy(fileName, file->name);
	
	strcpy(path, cwd);
	strcpy(fullPath, cwd);
	strcpy(fullPath + strlen(fullPath), fileName);
	
	char fileSize[16];
	getSizeString(fileSize, file->size);
	
	while(properties == true)
	{
		hidScanInput();
		hidTouchRead(&touch);
		
		screen_begin_frame();
		screen_select(GFX_BOTTOM);
		
		screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));
		
		screen_draw_texture(TEXTURE_PROPERTIES, 36, 20);
		
		screen_draw_string(41, 33, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_properties[language][0]);
		
		screen_draw_string(247, 201, 0.41f, 0.41f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_properties[language][6]);
	
		screen_draw_string(((320 - screen_get_string_width(lang_properties[language][1], 0.41f, 0.41f)) / 2), 50, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][1]);
		
		screen_draw_string(42, 74, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][2]);
			screen_draw_stringf(100, 74, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.36s", fileName);
		screen_draw_string(42, 94, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][3]);
			screen_draw_stringf(100, 94, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%s", path);
			
		screen_draw_string(42, 114, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][4]);	
		
		if (file->isDir)
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][1]);
		else if ((strncmp(file->ext, "CIA", 3) == 0) || (strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "3DS", 3) == 0) || (strncmp(file->ext, "3ds", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][2]);
		else if ((strncmp(file->ext, "bin", 3) == 0) || (strncmp(file->ext, "BIN", 3) == 0) || (strncmp(file->ext, "fir", 3) == 0) || (strncmp(file->ext, "FIR", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][3]);
		else if ((strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][4]);
		else if ((strncmp(file->ext, "rar", 3) == 0) || (strncmp(file->ext, "RAR", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][5]);
		else if ((strncmp(file->ext, "PNG", 3) == 0) || (strncmp(file->ext, "png", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][6]);
		else if ((strncmp(file->ext, "JPG", 3) == 0) || (strncmp(file->ext, "jpg", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][7]);
		else if ((strncmp(file->ext, "MP3", 3) == 0) || (strncmp(file->ext, "mp3", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][8]);
		else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0) || (strncmp(file->ext, "XML", 3) == 0) || (strncmp(file->ext, "xml", 3) == 0))
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][9]);
		else
			screen_draw_string(100, 114, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][10]);
		
		if (!(file->isDir))
		{
			screen_draw_string(42, 134, 0.41f, 0.41f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][5]);
				screen_draw_stringf(100, 134, 0.41f, 0.41f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%s", fileSize);
		}
		
		screen_end_frame();
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
		
		if ((kPressed & KEY_B) || (kPressed & KEY_A) || (touchInRect(36, 284, 192, 220)))
			properties = false;
	}
	
	return 0;
}