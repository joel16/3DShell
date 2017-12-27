#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "file/dirlist.h"
#include "file/file_operations.h"
#include "file/fs.h"
#include "graphics/screen.h"
#include "keyboard.h"
#include "menus/menu_main.h"
#include "theme.h"
#include "utils.h"

struct colour Storage_colour;
struct colour Settings_title_text_colour;
struct colour BottomScreen_colour;
struct colour Options_title_text_colour;

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
int copymode = NOTHING_TO_COPY;

void drawProgress(char * msg, char * src, u32 offset, u32 size)
{
	screen_begin_frame();
	screen_select(GFX_BOTTOM);

	screen_draw_rect(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

	screen_draw_texture(TEXTURE_DELETE, ((320 - (screen_get_texture_width(TEXTURE_DELETE))) / 2), 
		((240 - (screen_get_texture_height(TEXTURE_DELETE))) / 2));

	screen_draw_string(((320 - (screen_get_texture_width(TEXTURE_DELETE))) / 2) + 10, ((240 - (screen_get_texture_height(TEXTURE_DELETE))) / 2) + 20, 
		0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), msg);

	screen_draw_stringf(((320 - (screen_get_string_width(src, 0.45f, 0.45f))) / 2), ((240 - (screen_get_texture_height(TEXTURE_DELETE))) / 2) + 45, 
		0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), "%s", src);

	screen_draw_rect(((320 - (screen_get_texture_width(TEXTURE_DELETE))) / 2) + 20, ((240 - (screen_get_texture_height(TEXTURE_DELETE))) / 2) + 70, 
		240, 4, RGBA8(200, 200, 200, 255));
	screen_draw_rect(((320 - (screen_get_texture_width(TEXTURE_DELETE))) / 2) + 20, ((240 - (screen_get_texture_height(TEXTURE_DELETE))) / 2) + 70, 
		(double)offset / (double)size * 240.0, 4, RGBA8(Storage_colour.r, Storage_colour.g, Storage_colour.b, 255));

	screen_end_frame();
}

Result createFolder(void)
{
	char tempFolder[256];
	strcpy(tempFolder, keyboard_3ds_get(256, "", "Enter name"));

	if (strncmp(tempFolder, "", 1) == 0)
		menu_main(KEEP);

	char path[500];
	strcpy(path, cwd);
	strcat(path, tempFolder);

	if (R_SUCCEEDED(makeDir(fsArchive, path)))
		menu_main(CLEAR);

	return 0;
}

Result renameFile(void)
{
	Result ret = 0;
	File * file = getFileIndex(position);

	if (file == NULL)
		return -1;

	if (strncmp(file->name, "..", 2) == 0)
		return -2;

	char oldPath[500], newPath[500], name[255];

	strcpy(oldPath, cwd);
	strcpy(newPath, cwd);
	strcat(oldPath, file->name);

	strcpy(name, keyboard_3ds_get(255, file->name, "Enter name"));
	strcat(newPath, name);

	if (file->isDir)
	{
		if (R_FAILED(ret = fsRenameDir(fsArchive, oldPath, newPath)))
			return ret;
	}
	else
	{
		if (R_FAILED(ret = fsRenameFile(fsArchive, oldPath, newPath)))
			return ret;
	}
	
	menu_main(CLEAR);
	return 0;
}

Result delete(void)
{
	File * file = getFileIndex(position);

	if (file == NULL)
		return -1;
	
	if (strncmp(file->name, "..", 2) == 0)
			return -2;

	if (sysProtection)
	{
		if (SYS_FILES)
			return -2;
	}
	
	if ((recycleBin) && !(strstr(cwd, "/3ds/3DShell/bin/") != NULL))
	{
		char oldCWD[1024];
		copy(COPY_DELETE_ON_FINISH);
		strcpy(oldCWD, cwd);
		memset(cwd, 0, sizeof(cwd));
		sprintf(cwd, "/3ds/3DShell/bin/");
		paste();
		strcpy(cwd, oldCWD);
		return 0;
	}	

	char path[1024]; // File path

	// Puzzle path
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	Result ret = 0;

	if (file->isDir) // Delete folder
	{
		if (R_FAILED(ret = fsRmdirRecursive(fsArchive, path)))
			return ret;
	}
	else // Delete file
	{
		if (R_FAILED(ret = fsRemove(fsArchive, path)))
			return ret;
	}
	
	return 0;
}

// Copy file or folder
void copy(int flag)
{
	File * file = getFileIndex(position);
	
	if (file == NULL)
		return;

	// Copy file source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);

	if ((file->isDir) && (strncmp(file->name, "..", 2) != 0)) // If directory, add recursive folder flag
		flag |= COPY_FOLDER_RECURSIVE;

	copymode = flag; // Set copy flags
}

// Copy file from src to dst
int copy_file(char * src, char * dst)
{
	int chunksize = (512 * 1024); // Chunk size
	char * buffer = (char *)malloc(chunksize); // Reading buffer

	u64 totalwrite = 0; // Accumulated writing
	u64 totalread = 0; // Accumulated reading

	int result = 0; // Result

	int in = open(src, O_RDONLY, 0777); // Open file for reading
	u64 size = getFileSize(fsArchive, src);

	// Opened file for reading
	if (in >= 0)
	{
		if (fileExists(fsArchive, dst))
			fsRemove(fsArchive, dst); // Delete output file (if existing)

		int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0777); // Open output file for writing

		if (out >= 0) // Opened file for writing
		{
			u64 b_read = 0; // Read byte count

			// Copy loop (512KB at a time)
			while((b_read = read(in, buffer, chunksize)) > 0)
			{
				totalread += b_read; // Accumulate read data
				totalwrite += write(out, buffer, b_read); // Write data
				drawProgress(copymode == 1? "Moving" : "Copying", basename(src), totalread, size);
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

Result copy_file_archive(FS_Archive srcArchive, FS_Archive destArchive, FS_ArchiveID srcArchiveID, FS_ArchiveID destArchiveID, char * src, char * dst)
{
	int chunksize = (512 * 1024);
	char * buffer = (char *)malloc(chunksize);

	u32 bytesWritten = 0, bytesRead = 0;
	u64 offset = 0;
	
	Handle inputHandle, outputHandle;

	Result in = 0, out = 0, ret = 0;
	
	u64 size = getFileSize(srcArchive, src);

	if (R_SUCCEEDED(in = FSUSER_OpenFileDirectly(&inputHandle, srcArchiveID, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_ASCII, src), FS_OPEN_READ, 0)))
	{
		if (fileExists(fsArchive, dst))
			fsRemove(fsArchive, dst); // Delete output file (if existing)
		
		if (R_SUCCEEDED(out = FSUSER_OpenFileDirectly(&outputHandle, destArchiveID, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_ASCII, dst), (FS_OPEN_CREATE | FS_OPEN_WRITE), 0)))
		{
			// Copy loop (512KB at a time)
			do
			{
				if (R_FAILED(ret = FSFILE_Read(inputHandle, &bytesRead, offset, buffer, chunksize)))
					return ret;
				
				bytesWritten += FSFILE_Write(outputHandle, &bytesWritten, offset, buffer, size, FS_WRITE_FLUSH);
				drawProgress(copymode == 1? "Moving" : "Copying", basename(src), bytesRead, size);
				
				if (bytesWritten == bytesRead)
					break;
			}
			while(bytesRead);

			if (R_FAILED(ret = FSFILE_Close(outputHandle)))
				return ret;
			
			if (bytesRead != bytesWritten) 
				return -3;
		}
		else 
			return out;

		if (R_FAILED(ret = FSFILE_Close(inputHandle)))
			return ret;
	}
	else 
		return in;

	free(buffer);
	return 0;
}

// Recursively copy file from src to dst
Result copy_folder_recursive(char * src, char * dst)
{
	Handle dirHandle;
	Result ret = 0; // Open working Directory

	// Opened directory
	if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dirHandle, fsArchive, fsMakePath(PATH_ASCII, src))))
	{
		makeDir(fsArchive, dst); // Create output directory (is allowed to fail, we can merge folders after all)

		u32 entryCount = 0;
		FS_DirectoryEntry* entries = (FS_DirectoryEntry*) calloc(MAX_FILES, sizeof(FS_DirectoryEntry));
		char name[255] = {'\0'};

		// Iterate files
		do
		{
			if (R_SUCCEEDED(ret = FSDIR_Read(dirHandle, &entryCount, 1, entries)))
			{
				if (entryCount)
				{
					u16_to_u8(&name[0], entries->name, 0xFF);

					// Valid filename
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

						if (entries->attributes & FS_ATTRIBUTE_DIRECTORY) // Another folder
							copy_folder_recursive(inbuffer, outbuffer); // Copy folder (via recursion)	
						
						else copy_file(inbuffer, outbuffer); // Copy file

						// Free buffer
						free(inbuffer);
						free(outbuffer);
					}
				}
			}
			else 
				return ret;
		}
		while(entryCount);

		free(entries);

		// Close directory
		if (R_FAILED(ret = FSDIR_Close(dirHandle)))
			return ret; // Return success
	}
	else
		return ret;

	return 0;
}

// Paste file or folder
Result paste(void)
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

		ret = copy_folder_recursive(copysource, copytarget); // Copy folder recursively

		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			fsRmdirRecursive(fsArchive, copysource); // Delete dir
	}

	// Simple file copy
	else
	{
		if (copyFromNand)
		{
			FS_Archive sd, nand;
			openArchive(&sd, ARCHIVE_SDMC);
			openArchive(&nand, ARCHIVE_NAND_CTR_FS);
			ret = copy_file_archive(nand, sd, ARCHIVE_NAND_CTR_FS, ARCHIVE_SDMC, copysource, copytarget);
			closeArchive(sd);
			closeArchive(nand);
		}
		else if ((copyFromSD) && (BROWSE_STATE == STATE_NAND))
		{
			FS_Archive sd, nand;
			openArchive(&sd, ARCHIVE_SDMC);
			openArchive(&nand, ARCHIVE_NAND_CTR_FS);
			ret = copy_file_archive(sd, nand, ARCHIVE_SDMC, ARCHIVE_NAND_CTR_FS, copysource, copytarget);
			closeArchive(sd);
			closeArchive(nand);
		}	
		else ret = copy_file(copysource, copytarget); // Copy file
		
		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			fsRemove(fsArchive, copysource); // Delete file
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

Result saveLastDirectory(void)
{
	return fsWrite(fsArchive, "/3ds/3DShell/lastdir.txt", cwd);
}
