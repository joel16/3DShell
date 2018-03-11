#include <fcntl.h>
#include <unistd.h>

#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "keyboard.h"
#include "language.h"
#include "menu_properties.h"
#include "menu_settings.h"
#include "pp2d.h"
#include "progress_bar.h"
#include "textures.h"
#include "theme.h"
#include "touch.h"
#include "utils.h"

/*
*	Copy Flags
*/
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH   0
#define NOTHING_TO_COPY      -1

struct colour Options_select_colour;
struct colour Options_text_colour;
struct colour Settings_title_text_colour;

static int selectionX, selectionY;
static bool copy_enable, cut_enable;

#define CAN_COPY 	(((copy_enable == false) && (cut_enable != true)))
#define CAN_CUT 	(((cut_enable == false) && (copy_enable != true)))


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
static char selected_file[255][255]; // 255 slots, of length 255

static Result FileOptions_CreateFolder(void)
{
	char * buf = (char *)malloc(256);
	strcpy(buf, keyboard_3ds_get(256, "", "Enter name"));

	if (strncmp(buf, "", 1) == 0)
		return -1;

	char path[500];
	strcpy(path, cwd);
	strcat(path, buf);
	free(buf);

	FS_RecursiveMakeDir(archive, path);

	Dirlist_PopulateFiles(true);
	return 0;
}

static Result FileOptions_RenameFile(void)
{
	Result ret = 0;
	File * file = Dirlist_GetFileIndex(position);

	if (file == NULL)
		return -1;

	if (strncmp(file->name, "..", 2) == 0)
		return -2;

	char oldPath[500], newPath[500];

	char * buf = (char *)malloc(256);

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);

	strcpy(buf, keyboard_3ds_get(255, file->name, "Enter name"));
	strcat(newPath, buf);
	free(buf);

	if (file->isDir)
	{
		if (R_FAILED(ret = FS_RenameDir(archive, oldPath, newPath)))
			return ret;
	}
	else
	{
		if (R_FAILED(ret = FS_RenameFile(archive, oldPath, newPath)))
			return ret;
	}

	Dirlist_PopulateFiles(true);
	return 0;
}

// Copy file from src to dst
int FileOptions_CopyFile(char * src, char * dst, bool displayAnim)
{
	int chunksize = (512 * 1024); // Chunk size
	char * buffer = (char *)malloc(chunksize); // Reading buffer

	u64 totalwrite = 0; // Accumulated writing
	u64 totalread = 0; // Accumulated reading

	int result = 0; // Result

	int in = open(src, O_RDONLY, 0777); // Open file for reading
	u64 size = FS_GetFileSize(archive, src);

	// Opened file for reading
	if (in >= 0)
	{
		if (FS_FileExists(archive, dst))
			FS_Remove(archive, dst); // Delete output file (if existing)

		int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0777); // Open output file for writing

		if (out >= 0) // Opened file for writing
		{
			u64 b_read = 0; // Read byte count

			// Copy loop (512KB at a time)
			while((b_read = read(in, buffer, chunksize)) > 0)
			{
				totalread += b_read; // Accumulate read data
				totalwrite += write(out, buffer, b_read); // Write data

				if (displayAnim)
					ProgressBar_DisplayProgress(copymode == 1? "Moving" : "Copying", Utils_Basename(src), totalread, size);
			}

			close(out); // Close output file
			
			if (totalread != totalwrite) // Insufficient copy
				result = -3;
		}
		
		else // Output open error
			result = -2;
			
		close(in); // Close input file
	}

	// Input open error
	else
		result = -1;
	
	free(buffer); // Free memory
	return result; // Return result
}

// Recursively copy file from src to dst
static Result FileOptions_CopyDir(char * src, char * dst)
{
	Handle dirHandle;
	Result ret = 0; // Open working Directory

	// Opened directory
	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dirHandle, archive, fsMakePath(PATH_ASCII, src))))
	{
		FS_MakeDir(archive, dst); // Create output directory (is allowed to fail, we can merge folders after all)

		u32 entryCount = 0;
		FS_DirectoryEntry* entries = (FS_DirectoryEntry*) calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		
		if (R_SUCCEEDED(ret = FSDIR_Read(dirHandle, &entryCount, MAX_FILES, entries)))
		{
			u8 name[255] = {'\0'};
			for (u32 i = 0; i < entryCount; i++) 
			{
				u16_to_u8(&name[0], entries[i].name, 254);

				if (strlen(name) > 0)
				{
					// Calculate buffer size
					int insize = strlen(src) + strlen(name) + 2;
					int outsize = strlen(dst) + strlen(name) + 2;

					// Allocate buffer
					char * inbuffer = (char *)malloc(insize);
					char * outbuffer = (char *)malloc(outsize);

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
						
					else 
					{
						if (R_FAILED(ret = ret = FileOptions_CopyFile(inbuffer, outbuffer, false))) // Copy file
							return ret;
					}

					// Free buffer
					free(inbuffer);
					free(outbuffer);
				}

				ProgressBar_DisplayProgress(copymode == 1? "Moving" : "Copying", Utils_Basename(name), i, entryCount);
			}
		}
		else 
			return ret;

		free(entries);

		if (R_FAILED(ret = FSDIR_Close(dirHandle))) // Close directory
			return ret;
	}
	else
		return ret;

	return 0;
}

static void FileOptions_Copy(int flag)
{
	File * file = Dirlist_GetFileIndex(position);
	
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
static Result FileOptions_Paste(void)
{
	if (copymode == NOTHING_TO_COPY) // No copy source
		return -1;

	// Source and target folder are identical
	char * lastslash = NULL;
	int i = 0;

	for(; i < strlen(copysource); i++)
		if (copysource[i] == '/')
			lastslash = copysource + i;

	char backup = lastslash[1];
	lastslash[1] = 0;
	int identical = strcmp(copysource, cwd) == 0;
	lastslash[1] = backup;

	if (identical)
		return -2;

	char * filename = lastslash + 1; // Source filename

	int requiredlength = strlen(cwd) + strlen(filename) + 1; // Required target path buffer size
	char * copytarget = (char *)malloc(requiredlength); // Allocate target path buffer

	// Puzzle target path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);

	Result ret = -3; // Return result

	// Recursive folder copy
	if ((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE)
	{
		// Check files in current folder
		File * node = files; for(; node != NULL; node = node->next)
		{
			if ((strcmp(filename, node->name) == 0) && (!node->isDir)) // Found a file matching the name (folder = ok, file = not)
				return -4; // Error out
		}

		ret = FileOptions_CopyDir(copysource, copytarget); // Copy folder recursively

		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			FS_RmdirRecursive(archive, copysource); // Delete dir
	}

	// Simple file copy
	else
	{
		ret = FileOptions_CopyFile(copysource, copytarget, true); // Copy file
		
		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			FS_Remove(archive, copysource); // Delete file
	}

	// Paste success
	if (R_SUCCEEDED(ret))
	{
		memset(copysource, 0, sizeof(copysource)); // Erase cache data
		copymode = NOTHING_TO_COPY;
	}

	free(copytarget); // Free target path buffer
	return ret; // Return result
}

void Menu_DisplayFileOptions(void)
{
	pp2d_draw_texture(TEXTURE_OPTIONS, 37, 20);

	pp2d_draw_rectangle(37 + (selectionX * 123), 56 + (selectionY * 37), 123, 37, RGBA8(Options_select_colour.r, Options_select_colour.g, Options_select_colour.b, 255));

	pp2d_draw_text(42, 36, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][0]);
	pp2d_draw_text(232, 196, 0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), lang_options[language][8]);

	pp2d_draw_text(47, 72, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][1]);
	pp2d_draw_text(47, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][3]);
	pp2d_draw_text(47, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][5]);

	pp2d_draw_text(170, 72, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][2]);

	if (copy_enable)
	{
		pp2d_draw_text(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
		pp2d_draw_text(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
	}
	else if (cut_enable)
	{
		pp2d_draw_text(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][7]);
		pp2d_draw_text(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
	}
	else
	{
		pp2d_draw_text(170, 109, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][4]);
		pp2d_draw_text(170, 146, 0.45f, 0.45f, RGBA8(Options_text_colour.r, Options_text_colour.g, Options_text_colour.b, 255), lang_options[language][6]);
	}
}

void Menu_ControlFileOptions(u32 input)
{
	if ((input & KEY_TOUCH) && (touchInRect(37, 179, 282, 217))) // Cancel
	{
		wait(1);
		copy_enable = false;
		cut_enable = false;
		MENU_DEFAULT_STATE = MENU_STATE_HOME;
	}

	else if ((input & KEY_TOUCH) && (touchInRect(37, 56, 160, 93)))
	{
		selectionX = 0;
		selectionY = 0;
		wait(1);

		MENU_DEFAULT_STATE = MENU_STATE_PROPERTIES;
	}

	else if ((input & KEY_TOUCH) && (touchInRect(37, 94, 160, 130)))
	{
		selectionX = 0;
		selectionY = 1;
		wait(1);

		FileOptions_CreateFolder();
	}

	else if ((input & KEY_TOUCH) && (touchInRect(161, 56, 284, 93)))
	{
		selectionX = 1;
		selectionY = 0;
		wait(1);

		FileOptions_RenameFile();
	}
	
	else if ((input & KEY_TOUCH) && (touchInRect(37, 131, 160, 167)))
	{
		//selectionX = 0;
		//selectionY = 2;
		wait(1);

		MENU_DEFAULT_STATE = MENU_STATE_DIALOG;		
	}

	if ((CAN_COPY) && (input & KEY_TOUCH) && (touchInRect(161, 94, 284, 130)))
	{
		selectionX = 1;
		selectionY = 1;
		wait(1);
	
		FileOptions_Copy(COPY_KEEP_ON_FINISH);
		copy_enable = true;
	}
	
	else if ((copy_enable) && (input & KEY_TOUCH) && (touchInRect(161, 94, 284, 130)))
	{
		selectionX = 0;
		selectionY = 0;
		wait(1);

		if (FileOptions_Paste() == 0)
		{
			copy_enable = false;
			Dirlist_PopulateFiles(true);
			Dirlist_DisplayFiles();
		}
	}

	if ((CAN_CUT) && (input & KEY_TOUCH) && (touchInRect(161, 131, 284, 167)))
	{
		selectionX = 1;
		selectionY = 2;
		wait(1);

		FileOptions_Copy(COPY_DELETE_ON_FINISH);
		cut_enable = true;
	}
			
	else if ((cut_enable) && (input & KEY_TOUCH) && (touchInRect(161, 131, 284, 167)))
	{
		selectionX = 0;
		selectionY = 0;
		wait(1);

		if (FileOptions_Paste() == 0)
		{
			cut_enable = false;
			Dirlist_PopulateFiles(true);
			Dirlist_DisplayFiles();
		}
	}
}