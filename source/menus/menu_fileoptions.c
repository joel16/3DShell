#include <stdlib.h>

#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dirbrowse.h"
#include "fs.h"
#include "keyboard.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

/*
*	Copy Flags
*/
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH   0
#define NOTHING_TO_COPY      -1

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
static int copymode = NOTHING_TO_COPY;
/*
*	Copy Move Origin
*/
static char copysource[1024];

static int delete_dialog_selection = 0, row = 0, column = 0;
static bool copy_status = false, cut_status = false, options_more = false;

static float delete_confirm_width = 0, delete_confirm_height = 0;
static float delete_cancel_width = 0, delete_cancel_height = 0;

static float properties_ok_width = 0, properties_ok_height = 0;
static float options_cancel_width = 0, options_cancel_height = 0;

static bool copying_from_sd = false, copying_from_nand = false;

void FileOptions_ResetClipboard(void) {
	multi_select_index = 0;
	memset(multi_select, 0, sizeof(multi_select));
	memset(multi_select_indices, 0, sizeof(multi_select_indices));
	memset(multi_select_dir, 0, sizeof(multi_select_dir));
	memset(multi_select_paths, 0, sizeof(multi_select_paths));
}

static Result FileOptions_CreateFolder(void) {
	Result ret = 0;
	char *buf = malloc(256);
	strcpy(buf, Keyboard_GetText("New folder", "Enter name"));

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[512];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	if (R_FAILED(ret = FS_RecursiveMakeDir(archive, path))) {
		Menu_DisplayError("FS_RecursiveMakeDir failed:", ret);
		return ret;
	}

	Dirbrowse_PopulateFiles(true);
	return 0;
}

static Result FileOptions_CreateFile(void) {
	Result ret = 0;
	char *buf = malloc(256);
	strcpy(buf, Keyboard_GetText("New file.txt", "Enter name"));

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[512];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	if (R_FAILED(ret = FS_CreateFile(archive, path))) {
		Menu_DisplayError("FS_CreateFile failed:", ret);
		return ret;
	}
	
	Dirbrowse_PopulateFiles(true);
	return 0;
}

static Result FileOptions_Rename(void) {
	Result ret = 0;
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return -1;

	if (strncmp(file->name, "..", 2) == 0)
		return -2;

	char oldPath[512], newPath[512];

	char *buf = malloc(256);

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);

	strcpy(buf, Keyboard_GetText(file->name, "Enter name"));
	strcat(newPath, buf);
	free(buf);

	if (file->isDir) {
		if (R_FAILED(ret = FS_RenameDir(archive, oldPath, newPath))) {
			Menu_DisplayError("FS_RenameDir failed:", ret);
			return ret;
		}
	}
	else {
		if (R_FAILED(ret = FS_RenameFile(archive, oldPath, newPath))) {
			Menu_DisplayError("FS_RenameFile failed:", ret);
			return ret;
		}
	}

	Dirbrowse_PopulateFiles(true);
	return 0;
}

static int FileOptions_DeleteFile(void) {
	File *file = Dirbrowse_GetFileIndex(position);

	if (file == NULL)
		return -1;
	
	if (strncmp(file->name, "..", 2) == 0)
			return -2;

	char path[512];
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	Result ret = 0;

	if (file->isDir) { // Delete folder
		if (R_FAILED(ret = FS_RemoveDirRecursive(archive, path))) {
			Menu_DisplayError("FS_RemoveDirRecursive failed:", ret);
			return ret;
		}
	}
	else { // Delete file
		if (R_FAILED(ret = FS_RemoveFile(archive, path))) {
			Menu_DisplayError("FS_RemoveFile failed:", ret);
			return ret;
		}
	}
	
	return 0;
}

// Copy file from src to dst
static int FileOptions_CopyFile(char *src, char *dst, bool display_animation) {
	Handle src_handle, dst_handle;
	Result ret = 0;

	if (R_FAILED(ret = FS_OpenFile(&src_handle, copying_from_sd? sdmc_archive : nand_archive, src, FS_OPEN_READ, 0))) {
		Menu_DisplayError("FSUSER_OpenFile failed:", ret);
		return ret;
	}

	if (R_FAILED(ret = FS_OpenFile(&dst_handle, archive, dst, FS_OPEN_CREATE | FS_OPEN_WRITE, 0))) {
		FSFILE_Close(src_handle);
		Menu_DisplayError("FSUSER_OpenFile failed:", ret);
		return ret;
	}

	u32 bytes_read = 0;
	u64 offset = 0, size = 0;
	size_t buf_size = 0x10000;
	u8 *buf = malloc(buf_size); // Chunk size

	FS_GetFileSize(copying_from_sd? sdmc_archive : nand_archive, src, &size);

	do {
		memset(buf, 0, buf_size);

		if (R_FAILED(ret = FSFILE_Read(src_handle, &bytes_read, offset, buf, buf_size))) {
			free(buf);
			FSFILE_Close(src_handle);
			FSFILE_Close(dst_handle);
			Menu_DisplayError("FSFILE_Read failed:", ret);
			return ret;
		}
		if (R_FAILED(ret = FSFILE_Write(dst_handle, NULL, offset, buf, bytes_read, FS_WRITE_FLUSH))) {
			free(buf);
			FSFILE_Close(src_handle);
			FSFILE_Close(dst_handle);
			Menu_DisplayError("FSFILE_Write failed:", ret);
			return ret;
		}

		offset += bytes_read;

		if (display_animation)
			ProgressBar_DisplayProgress(copymode == 1? "Moving" : "Copying", Utils_Basename(src), offset, size);
	}
	while(offset < size);

	free(buf);
	FSFILE_Close(src_handle);
	FSFILE_Close(dst_handle);
	return 0;
}

// Recursively copy file from src to dst
static Result FileOptions_CopyDir(char *src, char *dst) {
	Handle dir;
	Result ret = 0; // Open working Directory

	// Opened directory
	if (R_SUCCEEDED(ret = FS_OpenDir(&dir, copying_from_sd? sdmc_archive : nand_archive, src))) {
		FS_MakeDir(archive, dst); // Create output directory (is allowed to fail, we can merge folders after all)

		u32 entryCount = 0;
		FS_DirectoryEntry *entries = (FS_DirectoryEntry *)calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		
		if (R_SUCCEEDED(ret = FSDIR_Read(dir, &entryCount, MAX_FILES, entries))) {
			char name[255] = {'\0'};
			for (u32 i = 0; i < entryCount; i++) {
				Utils_U16_To_U8((u8 *)&name[0], entries[i].name, 254);

				if (strlen(name) > 0) {
					// Calculate buffer size
					int insize = strlen(src) + strlen(name) + 2;
					int outsize = strlen(dst) + strlen(name) + 2;

					// Allocate buffer
					char *inbuffer = malloc(insize);
					char *outbuffer = malloc(outsize);

					// Puzzle input path
					strcpy(inbuffer, src);
					inbuffer[strlen(inbuffer) + 1] = 0;
					inbuffer[strlen(inbuffer)] = '/';
					strcpy(inbuffer + strlen(inbuffer), name);

					// Puzzle output path
					strcpy(outbuffer, dst);
					outbuffer[strlen(outbuffer) + 1] = 0;
					outbuffer[strlen(outbuffer)] = '/';
					strcpy(outbuffer + strlen(outbuffer), name);

					if (entries[i].attributes & FS_ATTRIBUTE_DIRECTORY) // Another folder
						FileOptions_CopyDir(inbuffer, outbuffer); // Copy folder (via recursion)
					else {
						if (R_FAILED(ret = FileOptions_CopyFile(inbuffer, outbuffer, false))) // Copy file
							return ret;
					}

					// Free buffer
					free(inbuffer);
					free(outbuffer);
				}

				ProgressBar_DisplayProgress(copymode == 1? "Moving" : "Copying", Utils_Basename(name), i, entryCount);
			}
		}
		else {
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

	return 0;
}

static void FileOptions_Copy(int flag) {
	File *file = Dirbrowse_GetFileIndex(position);
	
	if (file == NULL)
		return;

	// Copy file source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);

	if ((file->isDir) && (strncmp(file->name, "..", 2) != 0)) // If directory, add recursive folder flag
		flag |= COPY_FOLDER_RECURSIVE;

	copymode = flag; // Set copy flags
}

// Paste file or folder
static Result FileOptions_Paste(void) {
	if (copymode == NOTHING_TO_COPY) // No copy source
		return -1;

	// Source and target folder are identical
	char *last_slash = NULL;
	size_t i = 0;

	for(; i < strlen(copysource); i++)
		if (copysource[i] == '/')
			last_slash = copysource + i;

	char backup = last_slash[1];
	last_slash[1] = 0;
	int identical = strcmp(copysource, cwd) == 0;
	last_slash[1] = backup;

	if (identical)
		return -2;

	char *filename = last_slash + 1; // Source filename

	int required_length = strlen(cwd) + strlen(filename) + 1; // Required target path buffer size
	char *copytarget = malloc(required_length); // Allocate target path buffer

	// Puzzle target path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);

	Result ret = -3; // Return result

	// Recursive folder copy
	if ((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE) {
		// Check files in current folder
		File *node = files; for(; node != NULL; node = node->next) {
			if ((strcmp(filename, node->name) == 0) && (!node->isDir)) // Found a file matching the name (folder = ok, file = not)
				return -4; // Error out
		}

		ret = FileOptions_CopyDir(copysource, copytarget); // Copy folder recursively

		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH) {
			// Needs to add a forward "/"
			if (!(strcmp(&(copysource[(strlen(copysource)-1)]), "/") == 0))
				strcat(copysource, "/");

			FS_RemoveDirRecursive(archive, copysource); // Delete dir
		}
	}
	else { // Simple file copy
		ret = FileOptions_CopyFile(copysource, copytarget, true); // Copy file
		
		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			FS_RemoveFile(archive, copysource); // Delete file
	}

	// Paste success
	if (R_SUCCEEDED(ret)) {
		memset(copysource, 0, sizeof(copysource)); // Erase cache data
		copymode = NOTHING_TO_COPY;
	}

	free(copytarget); // Free target path buffer
	return ret; // Return result
}

static void HandleDelete(void) {
	if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
		for (int i = 0; i < multi_select_index; i++) {
			if (strlen(multi_select_paths[i]) != 0) {
				if (strncmp(multi_select_paths[i], "..", 2) != 0) {
					if (FS_DirExists(archive, multi_select_paths[i]))
						FS_RemoveDirRecursive(archive, multi_select_paths[i]);
					else if (FS_FileExists(archive, multi_select_paths[i]))
						FS_RemoveFile(archive, multi_select_paths[i]);
				}
			}
		}

		FileOptions_ResetClipboard();
	}
	else if (FileOptions_DeleteFile() != 0)
		return;

	Dirbrowse_PopulateFiles(true);
	MENU_STATE = MENU_STATE_HOME;
}

void Menu_ControlDeleteDialog(u32 input) {
	if (input & KEY_RIGHT)
		delete_dialog_selection++;
	else if (input & KEY_LEFT)
		delete_dialog_selection--;

	Utils_SetMax(&delete_dialog_selection, 0, 1);
	Utils_SetMin(&delete_dialog_selection, 1, 0);

	if (input & KEY_B) {
		delete_dialog_selection = 0;
		MENU_STATE = MENU_STATE_FILEOPTIONS;
	}

	if (input & KEY_A) {
		if (delete_dialog_selection == 1)
			HandleDelete();
		else
			MENU_STATE = MENU_STATE_FILEOPTIONS;

		delete_dialog_selection = 0;
	}

	if (TouchInRect((288 - delete_cancel_width) - 5, (159 - delete_cancel_height) - 5, ((288 - delete_cancel_width) - 5) + delete_cancel_width + 10, ((159 - delete_cancel_height) - 5) + delete_cancel_height + 10)) {
		delete_dialog_selection = 0;

		if (input & KEY_TOUCH) {
			MENU_STATE = MENU_STATE_FILEOPTIONS;
			delete_dialog_selection = 0;
		}
	}
	else if (TouchInRect((248 - (delete_confirm_width)) - 5, (159 - delete_confirm_height) - 5, ((248 - (delete_confirm_width)) - 5) + delete_confirm_width + 10, ((159 - delete_confirm_height) - 5) + delete_confirm_height + 10)) {
		delete_dialog_selection = 1;

		if (input & KEY_TOUCH) {
			HandleDelete();
			delete_dialog_selection = 0;
		}
	}
}

void Menu_DisplayDeleteDialog(void) {
	float text_width = 0;
	Draw_GetTextSize(0.45f, &text_width, NULL, "Do you want to continue?");

	Draw_GetTextSize(0.45f, &delete_confirm_width, &delete_confirm_height, "YES");
	Draw_GetTextSize(0.45f, &delete_cancel_width, &delete_cancel_height, "NO");

	Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

	Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Confirm deletion");

	Draw_Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Do you wish to continue?");

	if (delete_dialog_selection == 0)
		Draw_Rect((288 - delete_cancel_width) - 5, (159 - delete_cancel_height) - 5, delete_cancel_width + 10, delete_cancel_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (delete_dialog_selection == 1)
		Draw_Rect((248 - (delete_confirm_width)) - 5, (159 - delete_confirm_height) - 5, delete_confirm_width + 10, delete_confirm_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	Draw_Text(248 - (delete_confirm_width), (159 - delete_confirm_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
	Draw_Text(288 - delete_cancel_width, (159 - delete_cancel_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");
}

void Menu_ControlProperties(u32 input) {
	if ((input & KEY_A) || (input & KEY_B))
		MENU_STATE = MENU_STATE_FILEOPTIONS;

	if (TouchInRect((253 - properties_ok_width) - 5, (218 - properties_ok_height) - 5, ((253 - properties_ok_width) - 5) + properties_ok_width + 10, ((218 - properties_ok_height) - 5) + properties_ok_height + 10))
		if (input & KEY_TOUCH)
			MENU_STATE = MENU_STATE_FILEOPTIONS;
}

void Menu_DisplayProperties(void) {
	// Find File
	File *file = Dirbrowse_GetFileIndex(position);

	char path[1024];
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	Draw_Image(config.dark_theme? properties_dialog_dark : properties_dialog, 54, 30);
	Draw_Text(61, 37, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Properties");

	char utils_size[16];
	u64 size = 0;

	if (!file->isDir) {
		FS_GetFileSize(archive, path, &size);
		Utils_GetSizeString(utils_size, size);
	}

	Draw_Textf(66, 60, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Name: %.20s", file->name);
	Draw_Textf(66, 76, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Parent: %.20s", cwd);

	if (!file->isDir) {
		Draw_Textf(66, 92, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Size: %s", utils_size);
		Draw_Textf(66, 108, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Modified time: %s", FS_GetFileTimestamp(path));
	}
	else {
		if (strncmp(file->name, "..", 2))
			Draw_Textf(66, 92, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Modified time: %s", FS_GetFileTimestamp(path));
	}

	Draw_GetTextSize(0.45f, &properties_ok_width, &properties_ok_height, "OK");
	Draw_Rect((253 - properties_ok_width) - 5, (218 - properties_ok_height) - 5, properties_ok_width + 10, properties_ok_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	Draw_Text(253 - properties_ok_width, 218 - properties_ok_height, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");
}

static void FileOptions_ClearCopyStatus(void) {
	copying_from_sd = false;
	copying_from_nand = false;
	copy_status = false;
}

static void HandleCopy(void) {
	if ((!copy_status) && (!cut_status)) {
		copy_status = true;
		FileOptions_Copy(COPY_KEEP_ON_FINISH);
		MENU_STATE = MENU_STATE_HOME;

		if (BROWSE_STATE == BROWSE_STATE_SD)
			copying_from_sd = true;
		else
			copying_from_nand = true;
	}
	else if (copy_status) {
		if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
			char dest[512];
			
			for (int i = 0; i < multi_select_index; i++) {
				if (strlen(multi_select_paths[i]) != 0) {
					if (strncmp(multi_select_paths[i], "..", 2) != 0) {
						snprintf(dest, 512, "%s%s", cwd, Utils_Basename(multi_select_paths[i]));
				
						if (FS_DirExists(copying_from_sd? sdmc_archive : nand_archive, multi_select_paths[i]))
							FileOptions_CopyDir(multi_select_paths[i], dest);
						else if (FS_FileExists(copying_from_sd? sdmc_archive : nand_archive, multi_select_paths[i]))
							FileOptions_CopyFile(multi_select_paths[i], dest, true);
					}
				}
			}
			
			FileOptions_ResetClipboard();
			copymode = NOTHING_TO_COPY;
		}
		else if (FileOptions_Paste() != 0) {
			FileOptions_ClearCopyStatus();
			return;
		}

		FileOptions_ClearCopyStatus();
		Dirbrowse_PopulateFiles(true);
		MENU_STATE = MENU_STATE_HOME;
	}
}

static void HandleCut(void) {
	if ((!cut_status) && (!copy_status)) {
		cut_status = true;
		FileOptions_Copy(COPY_DELETE_ON_FINISH);
		MENU_STATE = MENU_STATE_HOME;
	}
	else if (cut_status) {
		char dest[512];

		if ((multi_select_index > 0) && (strlen(multi_select_dir) != 0)) {
			for (int i = 0; i < multi_select_index; i++) {
				if (strlen(multi_select_paths[i]) != 0) {
					snprintf(dest, 512, "%s%s", cwd, Utils_Basename(multi_select_paths[i]));
					
					if (FS_DirExists(archive, multi_select_paths[i]))
						FS_RenameDir(archive, multi_select_paths[i], dest);
					else if (FS_FileExists(archive, multi_select_paths[i]))
						FS_RenameFile(archive, multi_select_paths[i], dest);
				}
			}

			FileOptions_ResetClipboard();
		}
		else {
			snprintf(dest, 512, "%s%s", cwd, Utils_Basename(copysource));

			if (FS_DirExists(archive, copysource))
				FS_RenameDir(archive, copysource, dest);
			else if (FS_FileExists(archive, copysource))
				FS_RenameFile(archive, copysource, dest);
		}

		cut_status = false;
		copymode = NOTHING_TO_COPY;
		Dirbrowse_PopulateFiles(true);
		MENU_STATE = MENU_STATE_HOME;
	}
}

void Menu_ControlFileOptions(u32 input) {
	if (input & KEY_RIGHT)
		row++;
	else if (input & KEY_LEFT)
		row--;

	if (input & KEY_DDOWN)
		column++;
	else if (input & KEY_DUP)
		column--;

	if (!options_more) {
		Utils_SetMax(&row, 0, 1);
		Utils_SetMin(&row, 1, 0);

		Utils_SetMax(&column, 0, 3);
		Utils_SetMin(&column, 3, 0);
	}
	else {
		Utils_SetMax(&column, 0, 2);
		Utils_SetMin(&column, 2, 0);

		if (column == 0) {
			Utils_SetMax(&row, 0, 1);
			Utils_SetMin(&row, 1, 0);
		}
		else if (column == 1) {
			Utils_SetMax(&row, 0, 0);
			Utils_SetMin(&row, 0, 0);
		}
	}
	
	if (input & KEY_A) {
		if (row == 0 && column == 0) {
			if (options_more)
				FileOptions_CreateFolder();
			else
				MENU_STATE = MENU_STATE_PROPERTIES;
		}
		else if (row == 1 && column == 0) {
			if (options_more)
				FileOptions_CreateFile();
			else {
				options_more = false;
				row = 0;
				column = 0;
				Dirbrowse_PopulateFiles(true);
				MENU_STATE = MENU_STATE_HOME;
			}
		}
		else if (row == 0 && column == 1) {
			if (options_more)
				FileOptions_Rename();
			else
				HandleCopy();
		}
		else if (row == 1 && column == 1)
			HandleCut();
		else if (row == 0 && column == 2 && !options_more)
			MENU_STATE = MENU_STATE_DELETE;
		else if (row == 1 && column == 2 && !options_more) {
			row = 0;
			column = 0;
			options_more = true;
		}
		else if (column == 3 && !options_more) {
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
		else if (column == 2 && options_more) {
			options_more = false;
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
	}

	if (input & KEY_B) {
		if (!options_more) {
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
		else {
			row = 0;
			column = 0;
			options_more = false;
		}
	}

	if (input & KEY_X)
		MENU_STATE = MENU_STATE_HOME;

	if (TouchInRect(56, 69, 159, 104)) {
		row = 0;
		column = 0;
		
		if (input & KEY_TOUCH) {
			if (options_more)
				FileOptions_CreateFolder();
			else
				MENU_STATE = MENU_STATE_PROPERTIES;
		}
	}
	else if (TouchInRect(160, 69, 263, 104)) {
		row = 1;
		column = 0;
		
		if (input & KEY_TOUCH) {
			if (options_more)
				FileOptions_CreateFile();
			else {
				options_more = false;
				row = 0;
				column = 0;
				Dirbrowse_PopulateFiles(true);
				MENU_STATE = MENU_STATE_HOME;
			}
		}
	}
	else if (TouchInRect(56, 105, 159, 141)) {
		row = 0;
		column = 1;
		
		if (input & KEY_TOUCH) {
			if (options_more)
				FileOptions_Rename();
			else
				HandleCopy();
		}
	}
	else if (TouchInRect(160, 105, 263, 141) && !options_more) {
		row = 1;
		column = 1;
		
		if (input & KEY_TOUCH)
			HandleCut();
	}
	else if (TouchInRect(56, 142, 159, 178) && !options_more) {
		row = 0;
		column = 2;
		
		if (input & KEY_TOUCH)
			MENU_STATE = MENU_STATE_DELETE;
	}
	else if (TouchInRect(160, 142, 263, 178) && !options_more) {
		row = 1;
		column = 2;
		
		if (input & KEY_TOUCH) {
			row = 0;
			column = 0;
			options_more = true;
		}
	}
	else if (TouchInRect((256 - options_cancel_width) - 5, (221 - options_cancel_height) - 5, ((256 - options_cancel_width) - 5) + options_cancel_width + 10, 
		((221 - options_cancel_height) - 5) + options_cancel_height + 10)) {
		if (options_more)
			column = 2;
		else
			column = 3;

		if (input & KEY_TOUCH) {
			options_more = false;
			copy_status = false;
			cut_status = false;
			row = 0;
			column = 0;
			MENU_STATE = MENU_STATE_HOME;
		}
	}
}

void Menu_DisplayFileOptions(void) {
	Draw_Image(config.dark_theme? options_dialog_dark : options_dialog, 54, 30);
	Draw_Text(61, 37, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Actions");

	Draw_GetTextSize(0.45f, &options_cancel_width, &options_cancel_height, "CANCEL");
	
	if (row == 0 && column == 0)
		Draw_Rect(56, 69, 103, 36, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 0)
		Draw_Rect(160, 69, 103, 36, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 0 && column == 1)
		Draw_Rect(56, 105, 103, 36, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 1)
		Draw_Rect(160, 105, 103, 36, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 0 && column == 2 && !options_more)
		Draw_Rect(56, 142, 103, 36, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (row == 1 && column == 2 && !options_more)
		Draw_Rect(160, 142, 103, 36, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (column == 3 && !options_more)
		Draw_Rect((256 - options_cancel_width) - 5, (221 - options_cancel_height) - 5, options_cancel_width+ 10, options_cancel_height + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	else if (column == 2 && options_more)
		Draw_Rect((256 - options_cancel_width) - 5, (221 - options_cancel_height) - 5, options_cancel_width + 10, options_cancel_height + 10, 
			config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

	Draw_Text(256 - options_cancel_width, 221 - options_cancel_height, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "CANCEL");

	if (!options_more) {
		Draw_Text(66, 80, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Properties");
		Draw_Text(66, 116, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, copy_status? "Paste" : "Copy");
		Draw_Text(66, 152, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Delete");
		
		Draw_Text(170, 80, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Refresh");
		Draw_Text(170, 116, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, cut_status? "Paste" : "Move");
		Draw_Text(170, 152, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "More...");
	}
	else {
		Draw_Text(66, 80, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "New folder");
		Draw_Text(66, 116, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Rename");

		Draw_Text(170, 80, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "New file");
	}
}
