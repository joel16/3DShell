#include "common.h"
#include "dirlist.h"
#include "file_operations.h"
#include "fs.h"
#include "keyboard.h"
#include "main.h"
#include "utils.h"

/*
*	Copy Mode
*	-1 : Nothing
*	0  : Copy
*	1  : Move
*/
int copymode = NOTHING_TO_COPY;

void newFolder()
{
	char tempFolder[256];
	strcpy(tempFolder, keyboard_3ds_get(256, "Enter name"));
	
	if (strncmp(tempFolder, "", 1) == 0)
		mainMenu(KEEP);
	
	char path[500];
	strcpy(path, cwd);
	
	strcat(path, tempFolder);
	
	makeDir(sdmcArchive, path);
	
	mainMenu(CLEAR);	
}

int renameFile()
{
	File * file = findindex(position);

	if(file == NULL) 
		return -1;

	if(strncmp(file->name, "..", 2) == 0) 
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
	
	if (strncmp(get_filename_ext(name), "", 1) == 0) // Second check for ext, this is if the user does not specify a file extension.
		strcat(newPath, ext);
	
	fsRename(sdmcArchive, oldPath, newPath);
	
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
				if((strncmp(info->d_name, ".", 1) == 0) || (strncmp(info->d_name, "..", 2) == 0))
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
			fsRemove(sdmcArchive, buffer);

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
		if((strncmp(file->name, "..", 2) == 0) || (SYS_FILES)) 
			return -2;
	}
	else
	{
		if(strncmp(file->name, "..", 2) == 0) 
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
		return fsRemove(sdmcArchive, path);
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
	if ((file->isFolder) && (strncmp(file->name, "..", 2) != 0))
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
		fsRemove(sdmcArchive, b);

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
		makeDir(sdmcArchive, b);
		
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
			fsRemove(sdmcArchive, copysource); // Delete File
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

void saveLastDirectory()
{
	char buf[250];
	strcpy(buf, cwd);
			
	FILE * write = fopen("/3ds/3DShell/lastdir.txt", "w");
	fprintf(write, "%s", buf);
	fclose(write);
}