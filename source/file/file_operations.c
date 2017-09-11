#include "common.h"
#include "file/dirlist.h"
#include "file/file_operations.h"
#include "file/fs.h"
#include "keyboard.h"
#include "main.h"
#include "graphics/screen.h"
#include "utils.h"

#include <fcntl.h>
#include <unistd.h>

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
int copymode = NOTHING_TO_COPY;

Result createFolder(void)
{
	char tempFolder[256];
	strcpy(tempFolder, keyboard_3ds_get(256, "", "Enter name"));

	if (strncmp(tempFolder, "", 1) == 0)
		mainMenu(KEEP);

	char path[500];
	strcpy(path, cwd);

	strcat(path, tempFolder);

	if (R_SUCCEEDED(makeDir(fsArchive, path)))
		mainMenu(CLEAR);

	return 0;
}

Result renameFile(void)
{
	File * file = findindex(position);

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

	Result ret = 0;

	if (file->isDir)
		ret = fsRenameDir(fsArchive, oldPath, newPath);
	else
		ret = fsRenameFile(fsArchive, oldPath, newPath);

	if (R_SUCCEEDED(ret))
		mainMenu(CLEAR);

	return 0;
}

Result delete(void)
{
	// Find file
	File * file = findindex(position);

	// Not found
	if (file == NULL)
		return -1;

	if (sysProtection)
	{
		if ((strncmp(file->name, "..", 2) == 0) || (SYS_FILES))
			return -2;
	}
	else
	{
		if (strncmp(file->name, "..", 2) == 0)
			return -2;
	}

	// File path
	char path[1024];

	// Puzzle path
	strcpy(path, cwd);
	strcpy(path + strlen(path), file->name);

	Result ret = 0;

	// Delete folder
	if (file->isDir)
		ret = fsRmdirRecursive(fsArchive, path);
	// Delete file
	else
		ret = fsRemove(fsArchive, path);

	if (R_FAILED(ret))
		return ret;

	return 0;
}

// Copy file or folder
void copy(int flag)
{
	// Find file
	File * file = findindex(position);

	// Not found
	if (file == NULL)
		return;

	// Copy file source
	strcpy(copysource, cwd);
	strcpy(copysource + strlen(copysource), file->name);

	// Add recursive folder flag
	if ((file->isDir) && (strncmp(file->name, "..", 2) != 0))
		flag |= COPY_FOLDER_RECURSIVE;

	// Set copy flags
	copymode = flag;
}

// Copy file from A to B
int copy_file(char * a, char * b)
{
	// Chunk size
	int chunksize = (512 * 1024);

	// Reading buffer
	char * buffer = (char *)malloc(chunksize);

	// Accumulated writing
	u64 totalwrite = 0;

	// Accumulated reading
	u64 totalread = 0;

	// Result
	int result = 0;

	// Open file for reading
	int in = open(a, O_RDONLY, 0777);

	// Opened file for reading
	if (in >= 0)
	{
		// Delete output file (if existing)
		fsRemove(fsArchive, b);

		// Open file for writing
		int out = open(b, O_WRONLY | O_CREAT | O_TRUNC, 0777);

		// Opened file for writing
		if (out >= 0)
		{
			// Read byte count
			u64 b_read = 0;

			// Copy loop (512KB at a time)
			while((b_read = read(in, buffer, chunksize)) > 0)
			{
				// Accumulate read data
				totalread += b_read;

				// Write data
				totalwrite += write(out, buffer, b_read);
			}

			// Close output file
			close(out);

			// Insufficient copy
			if (totalread != totalwrite)
				result = -3;
		}

		// Output open error
		else
			result = -2;

		// Close input file
		close(in);
	}

	// Input open error
	else
		result = -1;

	// Free memory
	free(buffer);

	// Return result
	return result;
}

// Copy folder from A to B
Result copy_folder_recursive(char * a, char * b)
{
	// Open working Directory
	Handle dirHandle;
	Result directory = FSUSER_OpenDirectory(&dirHandle, fsArchive, fsMakePath(PATH_ASCII, a));

	u32 entriesRead;
	static char dname[1024];

	// Opened directory
	if (R_SUCCEEDED(directory))
	{
		// Create output directory (is allowed to fail, we can merge folders after all)
		makeDir(fsArchive, b);

		// Iterate files
		do
		{
			static FS_DirectoryEntry info;
			memset(&info, 0, sizeof(FS_DirectoryEntry));

			entriesRead = 0;

			if (R_SUCCEEDED(FSDIR_Read(dirHandle, &entriesRead, 1, &info)))
			{
				if (entriesRead)
				{
					u16_to_u8(&dname[0], info.name, 0xFF);

					// Valid filename
					if (strlen(dname) > 0)
					{
						// Calculate buffer size
						int insize = strlen(a) + strlen(dname) + 2;
						int outsize = strlen(b) + strlen(dname) + 2;

						// Allocate buffer
						char * inbuffer = (char *)malloc(insize);
						char * outbuffer = (char *)malloc(outsize);

						// Puzzle input path
						strcpy(inbuffer, a);
						inbuffer[strlen(inbuffer) + 1] = 0;
						inbuffer[strlen(inbuffer)] = '/';
						strcpy(inbuffer + strlen(inbuffer), dname);

						// Puzzle output path
						strcpy(outbuffer, b);
						outbuffer[strlen(outbuffer) + 1] = 0;
						outbuffer[strlen(outbuffer)] = '/';
						strcpy(outbuffer + strlen(outbuffer), dname);

						// Another folder
						if (info.attributes & FS_ATTRIBUTE_DIRECTORY)
							copy_folder_recursive(inbuffer, outbuffer); // Copy folder (via recursion)

						// Simple file
						else
							copy_file(inbuffer, outbuffer); // Copy file

						// Free buffer
						free(inbuffer);
						free(outbuffer);
					}
				}
			}
		}
		while(entriesRead);

		// Close directory
		if (R_SUCCEEDED(FSDIR_Close(dirHandle)))
			return 0; // Return success
	}

	return directory;
}

// Paste file or folder
Result paste(void)
{
	// No copy source
	if (copymode == NOTHING_TO_COPY)
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

	// Source filename
	char * filename = lastslash + 1;

	// Required target path buffer size
	int requiredlength = strlen(cwd) + strlen(filename) + 1;

	// Allocate target path buffer
	char * copytarget = (char *)malloc(requiredlength);

	// Puzzle target path
	strcpy(copytarget, cwd);
	strcpy(copytarget + strlen(copytarget), filename);

	// Return result
	Result ret = -3;

	// Recursive folder copy
	if ((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE)
	{
		// Check files in current folder
		File * node = files; for(; node != NULL; node = node->next)
		{
			// Found a file matching the name (folder = ok, file = not)
			if (strcmp(filename, node->name) == 0 && !node->isDir)
				return -4; // Error out
		}

		// Copy folder recursively
		ret = copy_folder_recursive(copysource, copytarget);

		// Source delete
		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			fsRmdirRecursive(fsArchive, copysource);
	}

	// Simple file copy
	else
	{
		// Copy file
		ret = copy_file(copysource, copytarget);

		// Source delete
		if ((R_SUCCEEDED(ret)) && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
			fsRemove(fsArchive, copysource); // Delete File
	}

	// Paste success
	if (R_SUCCEEDED(ret))
	{
		// Erase cache data
		memset(copysource, 0, sizeof(copysource));
		copymode = NOTHING_TO_COPY;
	}

	// Free target path buffer
	free(copytarget);

	// Return result
	return ret;
}

Result saveLastDirectory(void)
{
	return writeFile("/3ds/data/3DShell/lastdir.txt", cwd);
}
