#include <stdlib.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "menu_archive.h"
#include "menu_error.h"
#include "menu_gallery.h"
#include "menu_music.h"
#include "menu_textviewer.h"
#include "textures.h"
#include "utils.h"

#define FILES_PER_PAGE 5

int position = 0; // menu position
int file_count = 0; // file count
File *files = NULL; // file list

void Dirbrowse_RecursiveFree(File *node) {
	if (node == NULL) // End of list
		return;
	
	Dirbrowse_RecursiveFree(node->next); // Nest further
	free(node); // Free memory
}

// Sort directories alphabetically. Folder first, then files.
static int cmpstringp(const void *p1, const void *p2) {
	FS_DirectoryEntry *entryA = (FS_DirectoryEntry *)p1;
	FS_DirectoryEntry *entryB = (FS_DirectoryEntry *)p2;
	
	if ((entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && !(entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
		return -1;
	else if (!(entryA->attributes & FS_ATTRIBUTE_DIRECTORY) && (entryB->attributes & FS_ATTRIBUTE_DIRECTORY))
		return 1;
	else  {
		if (config.sort == 0) { // Sort alphabetically (ascending - A to Z)
			char entryNameA[256] = {'\0'}, entryNameB[256] = {'\0'};
			Utils_U16_To_U8((u8 *)entryNameA, entryA->name, sizeof(entryNameA) - 1);
			Utils_U16_To_U8((u8 *)entryNameB, entryB->name, sizeof(entryNameB) - 1);
			return strcasecmp(entryNameA, entryNameB);
		}
		else if (config.sort == 1) { // Sort alphabetically (descending - Z to A)
			char entryNameA[256] = {'\0'}, entryNameB[256] = {'\0'};
			Utils_U16_To_U8((u8 *)entryNameA, entryA->name, sizeof(entryNameA) - 1);
			Utils_U16_To_U8((u8 *)entryNameB, entryB->name, sizeof(entryNameB) - 1);
			return strcasecmp(entryNameB, entryNameA);
		}
		else if (config.sort == 2) { // Sort by file size (largest first)
			u64 sizeA = entryA->fileSize;
			u64 sizeB = entryB->fileSize;
			return sizeA > sizeB ? -1 : sizeA < sizeB ? 1 : 0;
		}
		else if (config.sort == 3) { // Sort by file size (smallest first)
			u64 sizeA = entryA->fileSize;
			u64 sizeB = entryB->fileSize;
			return sizeB > sizeA ? -1 : sizeB < sizeA ? 1 : 0;
		}
	}

	return 0;
}

Result Dirbrowse_PopulateFiles(bool clear) {
	Dirbrowse_RecursiveFree(files);
	files = NULL;
	file_count = 0;
	
	Handle dir;
	Result ret = 0;

	if (R_SUCCEEDED(ret = FS_OpenDir(&dir, archive, cwd))) {
		/* Add fake ".." entry except on root */
		if (strcmp(cwd, ROOT_PATH)) {
			files = (File *)malloc(sizeof(File)); // New list
			memset(files, 0, sizeof(File)); // Clear memory
			strcpy((char *)files->name, ".."); // Copy file Name
			files->isDir = 1; // Set folder flag
			file_count++;
		}
		
		u32 entryCount = 0;
		FS_DirectoryEntry *entries = (FS_DirectoryEntry *)calloc(MAX_FILES, sizeof(FS_DirectoryEntry));

		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries))) {
			qsort(entries, entryCount, sizeof(FS_DirectoryEntry), cmpstringp);
			char name[256] = {'\0'};
			
			for (u32 i = 0; i < entryCount; i++) {
				Utils_U16_To_U8((u8 *)&name[0], entries[i].name, 255);

				if (name[0] == '\0') // Ignore "." in all Directories
					continue;

				if ((!config.hidden_files) && (!strncmp(name, ".", 1))) // Ignore "." in all Directories
					continue;
				
				if ((!strcmp(cwd, ROOT_PATH)) && (!strncmp(name, "..", 2))) // Ignore ".." in Root Directory
					continue;
					
				File *item = (File *)malloc(sizeof(File));
				memset(item, 0, sizeof(File));
				strcpy((char *)item->name, name); // Copy file name
				strcpy(item->ext, entries[i].shortExt); // Copy file extension
				item->size = entries[i].fileSize; // Copy file size

				item->isDir = entries[i].attributes & FS_ATTRIBUTE_DIRECTORY; // Set folder flag
				item->isReadOnly = entries[i].attributes & FS_ATTRIBUTE_READ_ONLY; // Set read-Only flag
				item->isHidden = entries[i].attributes & FS_ATTRIBUTE_HIDDEN; // Set hidden file flag

				if ((!config.hidden_files) && (item->isHidden))
					continue;
				
				if (files == NULL) // New list
					files = item;
				
				// Existing list
				else {
					File *list = files;
					
					while(list->next != NULL)  // Append to list
						list = list->next;
					
					list->next = item; // Link item
				}
				
				file_count++; // Increment file count
			}
		}	
		else {
			free(entries);
			Menu_DisplayError("FSDIR_Read failed:", ret);
			return ret;
		}
		
		free(entries);
		
		if (R_FAILED(ret = FSDIR_Close(dir))) { // Close directory
			Menu_DisplayError("FSDIR_Close failed:", ret);
			return ret;
		}
	}
	else {
		Menu_DisplayError("FSUSER_OpenDirectory failed:", ret);
		return ret;
	}
		
	// Attempt to keep index
	if (!clear) {
		if (position >= file_count)
			position = file_count - 1; // Fix position
	}
	else
		position = 0; // Reset position
	
	return 0;
}

void Dirbrowse_DisplayFiles(void) {
	char file_size[16];
	float title_height = 0, text_height = 0, file_size_width = 0, file_size_height = 0;
	Draw_GetTextSize(0.45f, NULL, &title_height, cwd);
	Draw_Textf(70, 18 + ((34 - title_height) / 2), 0.45f, WHITE, strlen(cwd) > 45? "%.45s..." : "%s", cwd);

	int i = 0, printed = 0;
	File *file = files; // Draw file list

	for(; file != NULL; file = file->next) {
		if (printed == FILES_PER_PAGE) // Limit the files per page
			break;

		if (position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE)) {
			if (i == position)
				Draw_Rect(0, 52 + (38 * printed), 400, 38, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

			// Do not allow parent dir to be multi-selected
			if (strncmp(file->name, "..", 2)) {
				if (!strcmp(multi_select_dir, cwd)) {
					multi_select[i] == true? Draw_Image(config.dark_theme? icon_check_dark : icon_check, 5, 61 + (38 * printed)) : 
						Draw_Image(config.dark_theme? icon_uncheck_dark : icon_uncheck, 5, 61 + (38 * printed));
				}
				else
					Draw_Image(config.dark_theme? icon_uncheck_dark : icon_uncheck, 5, 61 + (38 * printed));
			}

			if (file->isDir)
				Draw_Image(config.dark_theme? icon_dir_dark : icon_dir, 30, 56 + (38 * printed));
			else if ((!strncasecmp(file->ext, "3ds", 3)) || (!strncasecmp(file->ext, "cia", 3)) || (!strncasecmp(file->ext, "bin", 3)))
				Draw_Image(icon_app, 30, 56 + (38 * printed));
			else if ((!strncasecmp(file->ext, "7z", 2)) || (!strncasecmp(file->ext, "ar", 2)) || (!strncasecmp(file->ext, "cpi", 3))
				|| (!strncasecmp(file->ext, "grz", 3)) || (!strncasecmp(file->ext, "iso", 3)) || (!strncasecmp(file->ext, "lrz", 3))
				|| (!strncasecmp(file->ext, "mtr", 3)) || (!strncasecmp(file->ext, "rar", 3)) || (!strncasecmp(file->ext, "sha", 3))
				|| (!strncasecmp(file->ext, "tar", 3)) || (!strncasecmp(file->ext, "taz", 3)) || (!strncasecmp(file->ext, "tbz", 3))
				|| (!strncasecmp(file->ext, "tgz", 3)) || (!strncasecmp(file->ext, "tlz", 3)) || (!strncasecmp(file->ext, "txz", 3))
				|| (!strncasecmp(file->ext, "tz", 2)) || (!strncasecmp(file->ext, "tz2", 3)) || (!strncasecmp(file->ext, "tzm", 3))
				|| (!strncasecmp(file->ext, "tzo", 3)) || (!strncasecmp(file->ext, "tzs", 3)) || (!strncasecmp(file->ext, "uu", 2))
				|| (!strncasecmp(file->ext, "war", 3)) || (!strncasecmp(file->ext, "xar", 3)) || (!strncasecmp(file->ext, "zip", 3))
				|| (!strncasecmp(file->ext, "zst", 3)))
				Draw_Image(icon_archive, 30, 56 + (38 * printed));
			else if (/*(!strncasecmp(file->ext, "fla", 3)) || */(!strncasecmp(file->ext, "it", 2)) || (!strncasecmp(file->ext, "mod", 3))
				|| (!strncasecmp(file->ext, "mp3", 3)) || (!strncasecmp(file->ext, "ogg", 3)) /*|| (!strncasecmp(file->ext, "opu", 3))*/
				|| (!strncasecmp(file->ext, "s3m", 3)) || (!strncasecmp(file->ext, "wav", 3)) || (!strncasecmp(file->ext, "xm", 2)))
				Draw_Image(icon_audio, 30, 56 + (38 * printed));
			else if ((!strncasecmp(file->ext, "bmp", 3)) || (!strncasecmp(file->ext, "gif", 3)) || (!strncasecmp(file->ext, "jpg", 3))
				|| (!strncasecmp(file->ext, "jpe", 3)) || (!strncasecmp(file->ext, "pcx", 3)) || (!strncasecmp(file->ext, "png", 3))
				|| (!strncasecmp(file->ext, "pgm", 3)) || (!strncasecmp(file->ext, "ppm", 3)) || (!strncasecmp(file->ext, "tga", 3)))
				Draw_Image(icon_image, 30, 56 + (38 * printed));
			else if ((!strncasecmp(file->ext, "txt", 3)) || (!strncasecmp(file->ext, "log", 3)) || (!strncasecmp(file->ext, "lua", 3))
				|| (!strncasecmp(file->ext, "cfg", 3)))
				Draw_Image(icon_text, 30, 56 + (38 * printed));
			else
				Draw_Image(icon_file, 30, 56 + (38 * printed));

			Draw_GetTextSize(0.45f, NULL, &text_height, file->name);

			if (!strncmp(file->name, "..", 2))
				Draw_Text(70, 52 + ((38 - text_height) / 2) + (38 * printed), 0.45f, config.dark_theme? WHITE : BLACK, "Parent folder");
			else {
				Draw_Textf(70, 52 + ((38 - text_height) / 2) + (38 * printed), 0.45f, config.dark_theme? WHITE : BLACK, strlen(file->name) > 42? "%.42s..." : "%s", file->name);

				if (!file->isDir) {
					Utils_GetSizeString(file_size, file->size);
					Draw_GetTextSize(0.40f, &file_size_width, &file_size_height, file_size);
					Draw_Text((395 - file_size_width), 52 + (38 - file_size_height - 5) + (38 * printed), 0.40f, config.dark_theme? WHITE : BLACK, file_size);
				}
			}

			printed++; // Increase printed counter
		}

		i++; // Increase counter
	}
}

static Result Dirbrowse_SaveLastDirectory(void) {
	Result ret = 0;

	if (!(BROWSE_STATE == BROWSE_STATE_NAND)) {
		if (R_FAILED(ret = FS_Write(archive, "/3ds/3DShell/lastdir.txt", cwd))) {
			Menu_DisplayError("Failed to save last directory:", ret);
			return ret;
		}
	}
	
	return 0;
}

// Get file index
File *Dirbrowse_GetFileIndex(int index) {
	int i = 0;
	File *file = files; // Find file Item
	
	for(; file != NULL && i != index; file = file->next)
		i++;

	return file; // Return file
}

/**
 * Executes an operation on the file depending on the filetype.
 */
void Dirbrowse_OpenFile(void) {
	char path[512];
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return;

	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	if (file->isDir) {
		// Attempt to navigate to target
		if (R_SUCCEEDED(Dirbrowse_Navigate(false))) {
			Dirbrowse_SaveLastDirectory();
			Dirbrowse_PopulateFiles(true);
		}
	}
	else if ((!strncasecmp(file->ext, "bmp", 3)) || (!strncasecmp(file->ext, "gif", 3)) || (!strncasecmp(file->ext, "jpg", 3))
		|| (!strncasecmp(file->ext, "jpe", 3)) || (!strncasecmp(file->ext, "pcx", 3)) || (!strncasecmp(file->ext, "png", 3))
		|| (!strncasecmp(file->ext, "pgm", 3)) || (!strncasecmp(file->ext, "ppm", 3)) || (!strncasecmp(file->ext, "tga", 3)))
		Gallery_DisplayImage(path);
	else if ((!strncasecmp(file->ext, "7z", 2)) || (!strncasecmp(file->ext, "ar", 2)) || (!strncasecmp(file->ext, "cpi", 3))
		|| (!strncasecmp(file->ext, "grz", 3)) || (!strncasecmp(file->ext, "iso", 3)) || (!strncasecmp(file->ext, "lrz", 3))
		|| (!strncasecmp(file->ext, "mtr", 3)) || (!strncasecmp(file->ext, "rar", 3)) || (!strncasecmp(file->ext, "sha", 3))
		|| (!strncasecmp(file->ext, "tar", 3)) || (!strncasecmp(file->ext, "taz", 3)) || (!strncasecmp(file->ext, "tbz", 3))
		|| (!strncasecmp(file->ext, "tgz", 3)) || (!strncasecmp(file->ext, "tlz", 3)) || (!strncasecmp(file->ext, "txz", 3))
		|| (!strncasecmp(file->ext, "tz", 2)) || (!strncasecmp(file->ext, "tz2", 3)) || (!strncasecmp(file->ext, "tzm", 3))
		|| (!strncasecmp(file->ext, "tzo", 3)) || (!strncasecmp(file->ext, "tzs", 3)) || (!strncasecmp(file->ext, "uu", 2))
		|| (!strncasecmp(file->ext, "war", 3)) || (!strncasecmp(file->ext, "xar", 3)) || (!strncasecmp(file->ext, "zip", 3))
		|| (!strncasecmp(file->ext, "zst", 3))) {
		if (R_SUCCEEDED(Archive_ExtractFile(path)))
			Dirbrowse_PopulateFiles(true);
	}
	else if (/*(!strncasecmp(file->ext, "fla", 3)) || */(!strncasecmp(file->ext, "it", 2)) || (!strncasecmp(file->ext, "mod", 3))
		|| (!strncasecmp(file->ext, "mp3", 3)) || (!strncasecmp(file->ext, "ogg", 3)) /*|| (!strncasecmp(file->ext, "opu", 3))*/
		|| (!strncasecmp(file->ext, "s3m", 3)) || (!strncasecmp(file->ext, "wav", 3)) || (!strncasecmp(file->ext, "xm", 2)))
		Menu_PlayMusic(path);
	else if ((!strncasecmp(file->ext, "txt", 3)) || (!strncasecmp(file->ext, "log", 3)) || (!strncasecmp(file->ext, "cfg", 3))
		|| (!strncasecmp(file->ext, "lua", 3)))
		Menu_DisplayText(path);
}

// Navigate to Folder
int Dirbrowse_Navigate(bool parent) {
	File *file = Dirbrowse_GetFileIndex(position); // Get index
	
	if (file == NULL)
		return -1;

	// Special case ".."
	if ((parent) || (!strncmp(file->name, "..", 2))) {
		char *slash = NULL;

		// Find last '/' in working directory
		int i = strlen(cwd) - 2; for(; i >= 0; i--) {
			// Slash discovered
			if (cwd[i] == '/') {
				slash = cwd + i + 1; // Save pointer
				break; // Stop search
			}
		}

		slash[0] = 0; // Terminate working directory
	}

	// Normal folder
	else {
		if (file->isDir) {
			// Append folder to working directory
			strcpy(cwd + strlen(cwd), file->name);
			cwd[strlen(cwd) + 1] = 0;
			cwd[strlen(cwd)] = '/';
		}
	}

	Dirbrowse_SaveLastDirectory();

	return 0; // Return success
}
