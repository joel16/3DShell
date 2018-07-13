#include <stdlib.h>

#include "archive.h"
#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_gallery.h"
#include "menu_music.h"
//#include "menu_book_reader.h"
#include "textures.h"
#include "utils.h"

int initialPosition = 0;
int position = 0; // menu position
int fileCount = 0; // file count
File *files = NULL; // file list

void Dirbrowse_RecursiveFree(File *node)
{
	if (node == NULL) // End of list
		return;
	
	Dirbrowse_RecursiveFree(node->next); // Nest further
	free(node); // Free memory
}

// Sort directories alphabetically. Folder first, then files.
static int cmpstringp(const void *p1, const void *p2)
{
	FS_DirectoryEntry* entryA = (FS_DirectoryEntry*) p1;
	FS_DirectoryEntry* entryB = (FS_DirectoryEntry*) p2;
	
	if ((entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && !(entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
		return -1;
	else if (!(entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && (entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
		return 1;
	else 
	{
		if (config_sort_by == 0) // Sort alphabetically (ascending - A to Z)
		{
			char entryNameA[256] = {'\0'}, entryNameB[256] = {'\0'};
			Utils_U16_To_U8((u8 *) entryNameA, entryA->name, sizeof(entryNameA) - 1);
			Utils_U16_To_U8((u8 *) entryNameB, entryB->name, sizeof(entryNameB) - 1);
			return strcasecmp(entryNameA, entryNameB);
		}
		else if (config_sort_by == 1) // Sort alphabetically (descending - Z to A)
		{
			char entryNameA[256] = {'\0'}, entryNameB[256] = {'\0'};
			Utils_U16_To_U8((u8 *) entryNameA, entryA->name, sizeof(entryNameA) - 1);
			Utils_U16_To_U8((u8 *) entryNameB, entryB->name, sizeof(entryNameB) - 1);
			return strcasecmp(entryNameB, entryNameA);
		}
		else if (config_sort_by == 2) // Sort by file size (largest first)
		{
			u64 sizeA = entryA->fileSize;
			u64 sizeB = entryB->fileSize;
			return sizeA > sizeB ? -1 : sizeA < sizeB ? 1 : 0;
		}
		else if (config_sort_by == 3) // Sort by file size (smallest first)
		{
			u64 sizeA = entryA->fileSize;
			u64 sizeB = entryB->fileSize;
			return sizeB > sizeA ? -1 : sizeB < sizeA ? 1 : 0;
		}
	}

	return 0;
}

Result Dirbrowse_PopulateFiles(bool clear)
{
	Dirbrowse_RecursiveFree(files);
	files = NULL;
	fileCount = 0;
	
	Handle dir;
	Result ret = 0;
	
	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dir, archive, fsMakePath(PATH_ASCII, cwd))))
	{
		/* Add fake ".." entry except on root */
		if (strcmp(cwd, ROOT_PATH))
		{
			files = (File *)malloc(sizeof(File)); // New list
			memset(files, 0, sizeof(File)); // Clear memory
			strcpy(files->name, ".."); // Copy file Name
			files->isDir = 1; // Set folder flag
			fileCount++;
		}
		
		u32 entryCount = 0;
		FS_DirectoryEntry* entries = (FS_DirectoryEntry*) calloc(MAX_FILES, sizeof(FS_DirectoryEntry));

		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries)))
		{
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), cmpstringp);
			u8 name[256] = {'\0'};
			
			for (u32 i = 0; i < entryCount; i++) 
			{
				Utils_U16_To_U8(&name[0], entries[i].name, 255);

				if (name[0] == '\0') // Ignore "." in all Directories
					continue;

				if ((!config_hidden_files) && (strncmp(name, ".", 1) == RL_SUCCESS)) // Ignore "." in all Directories
					continue;
				
				if (strcmp(cwd, ROOT_PATH) == 0 && strncmp(name, "..", 2) == 0) // Ignore ".." in Root Directory
					continue;
					
				File *item = (File *)malloc(sizeof(File));
				memset(item, 0, sizeof(File));

				strcpy(item->name, name); // Copy file name
				strcpy(item->ext, entries[i].shortExt); // Copy file extension
				item->size = entries[i].fileSize; // Copy file size

				item->isDir = entries[i].attributes & FS_ATTRIBUTE_DIRECTORY; // Set folder flag
				item->isReadOnly = entries[i].attributes & FS_ATTRIBUTE_READ_ONLY; // Set read-Only flag
				item->isHidden = entries[i].attributes & FS_ATTRIBUTE_HIDDEN; // Set hidden file flag

				if ((!config_hidden_files) && (item->isHidden))
					continue;
				
				if (files == NULL) // New list
					files = item;
				
				// Existing list
				else
				{
					File *list = files;
					
					while(list->next != NULL)  // Append to list
						list = list->next;
					
					list->next = item; // Link item
				}
				
				fileCount++; // Increment file count
			}
		}	
		else
		{
			free(entries);
			return ret;
		}
		
		free(entries);
		
		if (R_FAILED(ret = FSDIR_Close(dir))) // Close directory
			return ret;
	}
	else
		return ret;
		
	// Attempt to keep index
	if (!clear)
	{
		if (position >= fileCount)
			position = fileCount - 1; // Fix position
	}
	else
		position = 0; // Reset position
	
	return 0;
}

void Dirbrowse_DisplayFiles(void)
{
	//Draw_Image(icon_nav_drawer, 20, 58);
	//Draw_Image(icon_actions, (380 - 64), 58);
	float title_height = 0;
	Draw_GetTextSize(0.48f, NULL, &title_height, cwd);
	Draw_Text(70, 18 + ((34 - title_height) / 2), 0.48f, WHITE, cwd);

	int i = 0, printed = 0;
	File *file = files; // Draw file list

	for(; file != NULL; file = file->next)
	{
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if (position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE))
		{
			if (i == position)
				Draw_Rect(0, 52 + (38 * printed), 400, 38, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

			if (strcmp(multi_select_dir, cwd) == 0)
			{
				multi_select[i] == true? Draw_Image(config_dark_theme? icon_check_dark : icon_check, 5, 61 + (38 * printed)) : 
					Draw_Image(config_dark_theme? icon_uncheck_dark : icon_uncheck, 5, 61 + (38 * printed));
			}
			else
				Draw_Image(config_dark_theme? icon_uncheck_dark : icon_uncheck, 5, 61 + (38 * printed));

			char path[512];
			strcpy(path, cwd);
			strcpy(path + strlen(path), file->name);

			if (file->isDir)
				Draw_Image(config_dark_theme? icon_dir_dark : icon_dir, 30, 56 + (38 * printed));
			else if ((strncasecmp(file->ext, "3ds", 3) == 0) || (strncasecmp(file->ext, "cia", 3) == 0) || (strncasecmp(file->ext, "bin", 3) == 0))
				Draw_Image(icon_app, 30, 56 + (38 * printed));
			else if ((strncasecmp(file->ext, "zip", 3) == 0) || (strncasecmp(file->ext, "tar", 3) == 0)
					|| (strncasecmp(file->ext, "lz4", 3) == 0))
				Draw_Image(icon_archive, 30, 56 + (38 * printed));
			else if ((strncasecmp(file->ext, "mp3", 3) == 0) || (strncasecmp(file->ext, "ogg", 3) == 0)
					|| (strncasecmp(file->ext, "wav", 3) == 0) || (strncasecmp(file->ext, "fla", 3) == 0))
				Draw_Image(icon_audio, 30, 56 + (38 * printed));
			else if ((strncasecmp(file->ext, "png", 3) == 0) || (strncasecmp(file->ext, "jpg", 3) == 0) 
					|| (strncasecmp(file->ext, "bmp", 3) == 0) || (strncasecmp(file->ext, "gif", 3) == 0))
				Draw_Image(icon_image, 30, 56 + (38 * printed));
			else if ((strncasecmp(file->ext, "txt", 3) == 0) || (strncasecmp(file->ext, "lua", 3) == 0) 
            		|| (strncasecmp(file->ext, "cfg", 3) == 0))
				Draw_Image(icon_text, 30, 56 + (38 * printed));
			else if ((strncasecmp(file->ext, "pdf", 3) == 0) || (strncasecmp(file->ext, "cbz", 3) == 0)
					|| (strncasecmp(file->ext, "fb2", 3) == 0) || (strncasecmp(file->ext, "epub", 4) == 0))
				Draw_Image(icon_doc, 30, 56 + (38 * printed));
			else
				Draw_Image(icon_file, 30, 56 + (38 * printed));

			char buf[64], size[16];
			strncpy(buf, file->name, sizeof(buf));
			buf[sizeof(buf) - 1] = '\0';

			/*if (!file->isDir)
			{
				Utils_GetSizeString(size, file->size);
				float width = 0;
				Draw_GetTextSize(0.48f, &width, NULL, size);
				Draw_Text(390 - width, 180 + (73 * printed), 0.48f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, size);
			}*/

			float height = 0;
			Draw_GetTextSize(0.48f, NULL, &height, buf);
			
			if (strncmp(file->name, "..", 2) == 0)
				Draw_Text(70, 52 + ((38 - height)/2) + (38 * printed), 0.48f, config_dark_theme? WHITE : BLACK, "Parent folder");
			else 
				Draw_Text(70, 52 + ((38 - height)/2) + (38 * printed), 0.48f, config_dark_theme? WHITE : BLACK, buf);

			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}
}

static Result Dirbrowse_SaveLastDirectory(void)
{
	Result ret = 0;

	if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/lastdir.txt", cwd)))
		return ret;

	return 0;
}

// Get file index
File *Dirbrowse_GetFileIndex(int index)
{
	int i = 0;
	File *file = files; // Find file Item
	
	for(; file != NULL && i != index; file = file->next)
		i++;

	return file; // Return file
}

/**
 * Executes an operation on the file depending on the filetype.
 */
void Dirbrowse_OpenFile(void)
{
	char path[512];
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return;

	strcpy(fileName, file->name);
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	if (file->isDir)
	{
		// Attempt to navigate to target
		if (R_SUCCEEDED(Dirbrowse_Navigate(0)))
		{
			Dirbrowse_SaveLastDirectory();
			Dirbrowse_PopulateFiles(true);
		}
	}
	else if ((strncasecmp(file->ext, "png", 3) == 0) || (strncasecmp(file->ext, "jpg", 3) == 0) || 
			(strncasecmp(file->ext, "bmp", 3) == 0) || (strncasecmp(file->ext, "gif", 3) == 0))
		Gallery_DisplayImage(path);
	else if (strncasecmp(file->ext, "zip", 3) == 0)
	{
		Archive_ExtractZip(path, cwd);
		Dirbrowse_PopulateFiles(true);
	}
	else if ((strncasecmp(file->ext, "mp3", 3) == 0) || (strncasecmp(file->ext, "ogg", 3) == 0)
			|| (strncasecmp(file->ext, "wav", 3) == 0) || (strncasecmp(file->ext, "fla", 3) == 0))
		Menu_PlayMusic(path);
	/*else if ((strncasecmp(file->ext, "pdf", 3) == 0) || (strncasecmp(file->ext, "cbz", 3) == 0)
			|| (strncasecmp(file->ext, "fb2", 3) == 0) || (strncasecmp(file->ext, "epub", 4) == 0))
		Menu_OpenBook(path);*/
}

// Navigate to Folder
int Dirbrowse_Navigate(int dir)
{
	File *file = Dirbrowse_GetFileIndex(position); // Get index
	
	if ((file == NULL) || (!file->isDir)) // Not a folder
		return -1;

	// Special case ".."
	if ((dir == -1) || (strncmp(file->name, "..", 2) == 0))
	{
		char *slash = NULL;

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
	}

	// Normal folder
	else
	{
		// Append folder to working directory
		strcpy(cwd + strlen(cwd), file->name);
		cwd[strlen(cwd) + 1] = 0;
		cwd[strlen(cwd)] = '/';
	}

	Dirbrowse_SaveLastDirectory();

	return 0; // Return success
}