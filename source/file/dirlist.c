#include "archive.h"
#include "common.h"
#include "compile_date.h"
#include "file/dirlist.h"
#include "file/file_operations.h"
#include "file/fs.h"
#include "language.h"
#include "menus/menu_gallery.h"
#include "menus/menu_main.h"
#include "menus/menu_music.h"
#include "menus/menu_text.h"
#include "net/ftp.h"
#include "net/net.h"
#include "graphics/screen.h"
#include "screenshot.h"
#include "theme.h"
#include "utils.h"

#define MAX_FILES 1024

struct colour TopScreen_colour;
struct colour TopScreen_min_colour;
struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Options_title_text_colour;
struct colour Settings_title_text_colour;
struct colour BottomScreen_colour;

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

static int cmpstringp(const void *p1, const void *p2) 
{
   	FS_DirectoryEntry* entryA = (FS_DirectoryEntry*) p1;
   	FS_DirectoryEntry* entryB = (FS_DirectoryEntry*) p2;

   	if((entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && !(entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
   		return -1;
	else if(!(entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && (entryB->attributes & FS_ATTRIBUTE_DIRECTORY)) 
		return 1;
   	else 
   	{
   	    char entryNameA[0x200] = {'\0'}, entryNameB[0x200] = {'\0'};

   	    u16_to_u8((u8 *) entryNameA, entryA->name, sizeof(entryNameA) - 1);
       	u16_to_u8((u8 *) entryNameB, entryB->name, sizeof(entryNameB) - 1);

       	return strcasecmp(entryNameA, entryNameB);
   	}
}

Result updateList(int clearindex)
{
	recursiveFree(files);
	files = NULL;
	fileCount = 0;
	
	Handle dirHandle;
	Result ret = 0;

	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dirHandle, fsArchive, fsMakePath(PATH_ASCII, cwd))))
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

		u32 entryCount = 0;
        FS_DirectoryEntry* entries = (FS_DirectoryEntry*) calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		
		if (R_SUCCEEDED(ret = FSDIR_Read(dirHandle, &entryCount, MAX_FILES, entries)))
		{
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), cmpstringp);
			char name[255] = {'\0'};

			for (u32 i = 0; i < entryCount; i++) 
			{
				u16_to_u8(&name[0], entries[i].name, 254);

				if (name[0] == '\0') // Ingore null filenames
					continue;
						
				if (strncmp(name, ".", 1) == 0) // Ignore "." in all Directories
					continue;

				if (strcmp(cwd, ROOT_PATH) == 0 && strncmp(name, "..", 2) == 0) // Ignore ".." in Root Directory
					continue;

				File * item = (File *)malloc(sizeof(File)); // Allocate memory
				memset(item, 0, sizeof(File)); // Clear memory

				strcpy(item->name, name); // Copy file name
				strcpy(item->ext, entries[i].shortExt); // Copy file extension
				item->size = entries[i].fileSize; // Copy file size

				item->isDir = entries[i].attributes & FS_ATTRIBUTE_DIRECTORY; // Set folder flag
				item->isReadOnly = entries[i].attributes & FS_ATTRIBUTE_READ_ONLY; // Set read-Only flag
				item->isHidden = entries[i].attributes & FS_ATTRIBUTE_HIDDEN; // Set hidden file flag

				if ((!isHiddenEnabled) && (item->isHidden))
					continue;

				if (files == NULL) // New list
					files = item;

				// Existing list
				else
				{
					File * list = files;
						
					while(list->next != NULL)  // Append to list
						list = list->next;
					
					list->next = item; // Link item
				}
					
				fileCount++; // Increment file count
			}
		}

		if (R_FAILED(ret = FSDIR_Close(dirHandle))) // Close directory
			return ret;
	}
	else
		return ret;

	// Attempt to keep index
	if (!clearindex)
	{
		if (position >= fileCount)
			position = fileCount - 1; // Fix position
	}
	else
		position = 0; // Reset position
	
	return 0;
}

void recursiveFree(File * node)
{
	if (node == NULL) // End of list
		return;
	
	recursiveFree(node->next); // Nest further
	free(node); // Free memory
}

void displayFiles(void)
{
	screen_begin_frame();
	screen_select(GFX_BOTTOM);

	menu_displayMainMenu();
	
	int i = 0;
	int printed = 0; // Print counter

	File * file = files; // Draw file list

	//scroll_x = 395;

	for(; file != NULL; file = file->next)
	{
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if (position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE))
		{
			if (i == position)
				screen_draw_texture(TEXTURE_SELECTOR, 0, 53 + (38 * printed)); // Draw selector

			char path[500];
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);

			screen_draw_texture(TEXTURE_UNCHECK_ICON, 8, 66 + (38 * printed));

			if (file->isDir)
				screen_draw_texture(TEXTURE_FOLDER_ICON, 30, 58 + (38 * printed));
			else if ((strncasecmp(file->ext, "3ds", 3) == 0) || (strncasecmp(file->ext, "cia", 3) == 0))
				screen_draw_texture(TEXTURE_APP_ICON, 30, 58 + (38 * printed));
			else if ((strncasecmp(file->ext, "mp3", 3) == 0) || (strncasecmp(file->ext, "ogg", 3) == 0) || (strncasecmp(file->ext, "wav", 3) == 0) || (strncasecmp(file->ext, "fla", 3) == 0) || (strncasecmp(file->ext, "bcs", 3) == 0))
				screen_draw_texture(TEXTURE_AUDIO_ICON, 30, 58 + (38 * printed));
			else if ((strncasecmp(file->ext, "jpg", 3) == 0) || (strncasecmp(file->ext, "png", 3) == 0) || (strncasecmp(file->ext, "gif", 3) == 0) || (strncasecmp(file->ext, "bmp", 3) == 0))
				screen_draw_texture(TEXTURE_IMG_ICON, 30, 58 + (38 * printed));
			else if ((strncasecmp(file->ext, "bin", 3) == 0) || (strncasecmp(file->ext, "fir", 3) == 0))
				screen_draw_texture(TEXTURE_SYSTEM_ICON, 30, 58 + (38 * printed));
			else if (strncasecmp(file->ext, "txt", 3) == 0)
				screen_draw_texture(TEXTURE_TXT_ICON, 30, 58 + (38 * printed));
			else if (strncasecmp(file->ext, "zip", 3) == 0)
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

			screen_draw_stringf(70, 58 + (38 * printed), 0.45f, 0.45f, RGBA8(TopScreen_colour.r ,TopScreen_colour.g, TopScreen_colour.b, 255), "%.45s", buf); // Display file name

			if ((file->isDir) && (strncmp(file->name, "..", 2) != 0))
			{
				if (file->isReadOnly)
					screen_draw_stringf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s dr-xr-x---", getFileModifiedTime(path));
				else
					screen_draw_stringf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s drwxr-x---", getFileModifiedTime(path));

			}
			else if (strncmp(file->name, "..", 2) == 0)
				screen_draw_string(70, 76 + (38 * printed), 0.45f, 0.45f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), lang_files[language][0]);
			else
			{
				getSizeString(size, file->size);

				if (file->isReadOnly)
					screen_draw_stringf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s -r--r-----", getFileModifiedTime(path));
				else
					screen_draw_stringf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(TopScreen_min_colour.r, TopScreen_min_colour.g, TopScreen_min_colour.b, 255), "%s -rw-rw----", getFileModifiedTime(path));

				screen_draw_stringf(395 - screen_get_string_width(size, 0.42f, 0.42f), 76 + (38 * printed), 0.42f, 0.42f, RGBA8(TopScreen_colour.r, TopScreen_colour.g, TopScreen_colour.b, 255), "%s", size);
			}

			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}

	//length is 187
	/*screen_draw_stringf(5, 1, 0.45f, 0.45f, RGBA8(181, 181, 181, 255), "%d/%d", (position + 1), fileCount); // debug stuff

	if (fileCount > 5) // if file count is greater than the max amount on screen
	{
		if (osGetTime() - scroll_time >= (1500))
			scroll_x = 400;

		screen_draw_rect(scroll_x, 53, 5, 187, RGBA8(181, 181, 181, 100));
		double scroll_length = (187.0 / (double)fileCount);
		screen_draw_rect(scroll_x, 53 + (scroll_length * (position)), 5, scroll_length, RGBA8(181, 181, 181, 255));
	}*/

	screen_end_frame();
}

/**
 * Executes an operation on the file depending on the filetype.
 */
void openFile(void)
{
	char path[1024];
	File * file = getFileIndex(position);

	if (file == NULL)
		return;

	strcpy(fileName, file->name);

	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	if (file->isDir)
	{
		// Attempt to navigate to target
		if (R_SUCCEEDED(navigate(0)))
		{
			if (BROWSE_STATE != STATE_NAND)
				saveLastDirectory();

			updateList(CLEAR);
			displayFiles();
		}
	}
	else if (strncasecmp(file->ext, "png", 3) == 0 || strncasecmp(file->ext, "jpg", 3) == 0 || strncasecmp(file->ext, "gif", 3) == 0 || strncasecmp(file->ext, "bmp", 3) == 0)
		menu_displayImage(path);
	else if (strncasecmp(file->ext, "zip", 3) == 0)
	{
		extractZip(path, cwd);
		updateList(CLEAR);
		displayFiles();
	}
	else if (strncasecmp(file->ext, "txt", 3) == 0)
		menu_displayText(path);
	else if(getMusicFileType(path) != 0)
		menu_musicPlayer(path);
}

// Navigate to Folder
int navigate(int _case)
{
	File * file = getFileIndex(position); // Get index

	if ((file == NULL) || (!file->isDir)) // Not a folder
		return -1;

	// Special case ".."
	if ((_case == -1) || (strncmp(file->name, "..", 2) == 0))
	{
		char * slash = NULL;

		// Find last '/' in working directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--)
		{
			// Slash discovered
			if (cwd[i] == '/')
			{
				slash = cwd + i + 1; // Save pointer
				break; // Stop search
			}
		}

		slash[0] = 0; // Terminate working directory

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

	return 0; // Return success
}

// Get file index
File * getFileIndex(int index)
{
	int i = 0;

	File * file = files; // Find file Item

	for(; file != NULL && i != index; file = file->next)
		i++;
	
	return file; // Return file
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

		screen_draw_string(27, 72, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][0]);

		screen_draw_string(206, 159, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][3]);
		screen_draw_string(255, 159, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_deletion[language][4]);

		screen_draw_string(((320 - screen_get_string_width(lang_deletion[language][1], 0.45f, 0.45f)) / 2), 100, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_deletion[language][1]);
		screen_draw_string(((320 - screen_get_string_width(lang_deletion[language][2], 0.45f, 0.45f)) / 2), 115, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_deletion[language][2]);

		screen_end_frame();

		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();

		if ((kPressed & KEY_A) || ((touchInRect(240, 320, 142, 185))  && (kPressed & KEY_TOUCH)))
		{
			if (R_SUCCEEDED(delete()))
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
	File * file = getFileIndex(position);

	if (file == NULL)
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

		screen_draw_string(41, 33, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_properties[language][0]);

		screen_draw_string(247, 201, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_properties[language][6]);

		screen_draw_string(((320 - screen_get_string_width(lang_properties[language][1], 0.45f, 0.45f)) / 2), 50, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][1]);

		screen_draw_string(42, 74, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][2]);
			screen_draw_stringf(100, 74, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.28s", fileName);
		screen_draw_string(42, 94, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][3]);
			screen_draw_stringf(100, 94, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.28s", path);

		screen_draw_string(42, 114, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][4]);

		if (file->isDir)
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][1]);
		else if ((strncmp(file->ext, "CIA", 3) == 0) || (strncmp(file->ext, "cia", 3) == 0) || (strncmp(file->ext, "3DS", 3) == 0) || (strncmp(file->ext, "3ds", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][2]);
		else if ((strncmp(file->ext, "bin", 3) == 0) || (strncmp(file->ext, "BIN", 3) == 0) || (strncmp(file->ext, "fir", 3) == 0) || (strncmp(file->ext, "FIR", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][3]);
		else if ((strncmp(file->ext, "zip", 3) == 0) || (strncmp(file->ext, "ZIP", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][4]);
		else if ((strncmp(file->ext, "rar", 3) == 0) || (strncmp(file->ext, "RAR", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][5]);
		else if ((strncmp(file->ext, "PNG", 3) == 0) || (strncmp(file->ext, "png", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][6]);
		else if ((strncmp(file->ext, "JPG", 3) == 0) || (strncmp(file->ext, "jpg", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][7]);
		else if ((strncmp(file->ext, "MP3", 3) == 0) || (strncmp(file->ext, "mp3", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][8]);
		else if ((strncmp(file->ext, "txt", 3) == 0) || (strncmp(file->ext, "TXT", 3) == 0) || (strncmp(file->ext, "XML", 3) == 0) || (strncmp(file->ext, "xml", 3) == 0))
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][9]);
		else
			screen_draw_string(100, 114, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_files[language][10]);

		if (!(file->isDir))
		{
			screen_draw_string(42, 134, 0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), lang_properties[language][5]);
			screen_draw_stringf(100, 134, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), "%.28s", fileSize);
		}

		screen_end_frame();

		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();

		if ((kPressed & KEY_B) || (kPressed & KEY_A) || (touchInRect(36, 284, 192, 220)))
			properties = false;
	}

	return 0;
}
