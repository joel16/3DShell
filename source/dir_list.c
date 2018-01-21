#include <stdlib.h>
#include <string.h>

#include "archive.h"
#include "cia.h"
#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "language.h"
#include "menu_gallery.h"
#include "menu_music.h"
#include "pp2d.h"
#include "textures.h"
#include "utils.h"

int position = 0; // menu position
int fileCount = 0; // file count
File * files = NULL; // file list

static void recursiveFree(File * node)
{
   if (node == NULL) // End of list
      return;
   
   recursiveFree(node->next); // Nest further
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
   		if (sortBy == 0)
   			return 0;
   		else if (sortBy == 1) // Sort alphabetically (ascending - A to Z)
   	    {
   	    	char entryNameA[0x200] = {'\0'}, entryNameB[0x200] = {'\0'};
   	    	u16_to_u8((u8 *) entryNameA, entryA->name, sizeof(entryNameA) - 1);
   	    	u16_to_u8((u8 *) entryNameB, entryB->name, sizeof(entryNameB) - 1);
   	    	return strcasecmp(entryNameA, entryNameB);
   	    }
   	    else if (sortBy == 2) // Sort alphabetically (descending - Z to A)
   	    {
   	    	char entryNameA[0x200] = {'\0'}, entryNameB[0x200] = {'\0'};
   	    	u16_to_u8((u8 *) entryNameA, entryA->name, sizeof(entryNameA) - 1);
   	    	u16_to_u8((u8 *) entryNameB, entryB->name, sizeof(entryNameB) - 1);
   	    	return strcasecmp(entryNameB, entryNameA);
   	    }
   	    else if (sortBy == 3) // Sort by file size (largest first)
   	    {
   	    	u64 sizeA = entryA->fileSize;
   	    	u64 sizeB = entryB->fileSize;
   	    	return sizeA > sizeB ? -1 : sizeA < sizeB ? 1 : 0;
   	    }
   	    else if (sortBy == 4) // Sort by file size (smallest first)
   	    {
   	    	u64 sizeA = entryA->fileSize;
   	    	u64 sizeB = entryB->fileSize;
   	    	return sizeB > sizeA ? -1 : sizeB < sizeA ? 1 : 0;
   	    }
   	}

   	return 0;
}

Result Dirlist_PopulateFiles(bool clear)
{
   recursiveFree(files);
   files = NULL;
   fileCount = 0;
   
   Handle dirHandle;
   Result ret = 0;

   if (R_SUCCEEDED(ret = FSUSER_OpenDirectory(&dirHandle, archive, fsMakePath(PATH_ASCII, cwd))))
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
      
      if (R_SUCCEEDED(ret = FSDIR_Read(dirHandle, &entryCount, MAX_FILES, entries)))
      {
         qsort(entries, entryCount, sizeof(FS_DirectoryEntry), cmpstringp);
         u8 name[255] = {'\0'};

         for (u32 i = 0; i < entryCount; i++) 
         {
            u16_to_u8(&name[0], entries[i].name, 254);

            if (name[0] == '\0') // Ingore null filenames
               continue;
                  
            if ((!isHiddenEnabled) && (strncmp(name, ".", 1) == RL_SUCCESS)) // Ignore "." in all Directories
               continue;

            if (strcmp(cwd, ROOT_PATH) == RL_SUCCESS && strncmp(name, "..", 2) == RL_SUCCESS) // Ignore ".." in Root Directory
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
      else
      {
         free(entries);
         return ret;
      }

      free(entries);

      if (R_FAILED(ret = FSDIR_Close(dirHandle))) // Close directory
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

void Dirlist_DisplayFiles(void)
{  
   pp2d_draw_textf(84, 28, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), "%.33s", cwd); // Display current path

   u64 totalStorage = FS_GetTotalStorage(BROWSE_STATE? SYSTEM_MEDIATYPE_CTR_NAND : SYSTEM_MEDIATYPE_SD);
   u64 usedStorage = FS_GetUsedStorage(BROWSE_STATE? SYSTEM_MEDIATYPE_CTR_NAND : SYSTEM_MEDIATYPE_SD);
   double fill = (((double)usedStorage / (double)totalStorage) * 209.0);

   pp2d_draw_rectangle(82, 47, fill, 2, RGBA8(48, 174, 222, 255)); // Draw storage bar

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
            pp2d_draw_texture(TEXTURE_SELECTOR, 0, 53 + (38 * printed)); // Draw selector

         char path[500];
         strcpy(path, cwd);
         strcpy(path + strlen(path), file->name);

         pp2d_draw_texture(TEXTURE_ICON_UNCHECK, 8, 66 + (38 * printed));

         if (file->isDir)
            pp2d_draw_texture(TEXTURE_ICON_FOLDER, 30, 58 + (38 * printed));
         else if ((strncasecmp(file->ext, "3ds", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "cia", 3) == RL_SUCCESS))
            pp2d_draw_texture(TEXTURE_ICON_APP, 30, 58 + (38 * printed));
         else if ((strncasecmp(file->ext, "mp3", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "ogg", 3) == RL_SUCCESS) || 
            (strncasecmp(file->ext, "wav", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "fla", 3) == RL_SUCCESS) || 
            (strncasecmp(file->ext, "bcs", 3) == RL_SUCCESS))
            pp2d_draw_texture(TEXTURE_ICON_AUDIO, 30, 58 + (38 * printed));
         else if ((strncasecmp(file->ext, "jpg", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "png", 3) == RL_SUCCESS) || 
            (strncasecmp(file->ext, "gif", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "bmp", 3) == RL_SUCCESS))
            pp2d_draw_texture(TEXTURE_ICON_IMG, 30, 58 + (38 * printed));
         else if ((strncasecmp(file->ext, "bin", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "fir", 3) == RL_SUCCESS))
            pp2d_draw_texture(TEXTURE_ICON_SYSTEM, 30, 58 + (38 * printed));
         else if (strncasecmp(file->ext, "txt", 3) == RL_SUCCESS)
            pp2d_draw_texture(TEXTURE_ICON_TEXT, 30, 58 + (38 * printed));
         else if (strncasecmp(file->ext, "zip", 3) == RL_SUCCESS)
            pp2d_draw_texture(TEXTURE_ICON_ARCHIVE, 30, 58 + (38 * printed));
         else
            pp2d_draw_texture(TEXTURE_ICON_FILE, 30, 58 + (38 * printed));

         char buf[64], size[16];

         strncpy(buf, file->name, sizeof(buf));
         buf[sizeof(buf) - 1] = '\0';

         pp2d_draw_textf(70, 58 + (38 * printed), 0.45f, 0.45f, RGBA8(0, 0, 0, 255), "%.45s", buf); // Display file name

         if ((file->isDir) && (strncmp(file->name, "..", 2) != RL_SUCCESS))
         {
            if (file->isReadOnly)
               pp2d_draw_textf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(95, 95, 95, 255), "%s dr-xr-xr-x", FS_GetFileModifiedTime(path));
            else
               pp2d_draw_textf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(95, 95, 95, 255), "%s drwxrwxrwx", FS_GetFileModifiedTime(path));

         }
         else if (strncmp(file->name, "..", 2) == RL_SUCCESS)
            pp2d_draw_text(70, 76 + (38 * printed), 0.45f, 0.45f, RGBA8(95, 95, 95, 255), lang_files[language][0]);
         else
         {
            Utils_GetSizeString(size, file->size);

            if (file->isReadOnly)
               pp2d_draw_textf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(95, 95, 95, 255), "%s -r--r--r--", FS_GetFileModifiedTime(path));
            else
               pp2d_draw_textf(70, 76 + (38 * printed), 0.42f, 0.42f, RGBA8(95, 95, 95, 255), "%s -rw-rw-rw-", FS_GetFileModifiedTime(path));

            pp2d_draw_textf(395 - pp2d_get_text_width(size, 0.42f, 0.42f), 76 + (38 * printed), 0.42f, 0.42f, RGBA8(0, 0, 0, 255), "%s", size);
         }

         printed++; // Increase printed counter
      }

      i++; // Increase counter
   }
}

static Result Dirlist_SaveLastDirectory(void)
{
   return FS_Write(archive, "/3ds/3DShell/lastdir.txt", cwd);
}

// Get file index
File * Dirlist_GetFileIndex(int index)
{
   int i = 0;

   File * file = files; // Find file Item

   for(; file != NULL && i != index; file = file->next)
      i++;
   
   return file; // Return file
}

/**
 * Executes an operation on the file depending on the filetype.
 */
void Dirlist_OpenFile(void)
{
   char path[512];
   File * file = Dirlist_GetFileIndex(position);

   if (file == NULL)
      return;

   strcpy(fileName, file->name);
   strcpy(path, cwd);
   strcpy(path + strlen(path), file->name);

   if (file->isDir)
   {
      // Attempt to navigate to target
      if (R_SUCCEEDED(Dirlist_Navigate(true)))
      {
         if (BROWSE_STATE != STATE_NAND)
            Dirlist_SaveLastDirectory();

         Dirlist_PopulateFiles(true);
         Dirlist_DisplayFiles();
      }
   }
   else if ((strncasecmp(file->ext, "png", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "jpg", 3) == RL_SUCCESS) || 
      (strncasecmp(file->ext, "gif", 3) == RL_SUCCESS) || (strncasecmp(file->ext, "bmp", 3) == RL_SUCCESS))
      Gallery_DisplayImage(path);
   else if (Music_GetMusicFileType(path) != RL_SUCCESS)
      Music_Player(path);
   else if (strncasecmp(file->ext, "zip", 3) == RL_SUCCESS)
   {
      Archive_ExtractZip(path, cwd);
      Dirlist_PopulateFiles(true);
      Dirlist_DisplayFiles();
   }
   /*else if (strncasecmp(file->ext, "txt", 3) == RL_SUCCESS)
      menu_displayText(path);*/
}

// Navigate to Folder
int Dirlist_Navigate(bool folder)
{
   File * file = Dirlist_GetFileIndex(position); // Get index

   if ((file == NULL) || (!file->isDir)) // Not a folder
      return -1;

   // Normal folder
   if (folder)
   {
      // Append folder to working directory
      strcpy(cwd + strlen(cwd), file->name);
      cwd[strlen(cwd) + 1] = 0;
      cwd[strlen(cwd)] = '/';

      if (BROWSE_STATE != STATE_NAND)
         Dirlist_SaveLastDirectory();
   }

   else
   {
      // Special case ".."
      if (strncmp(file->name, "..", 2) == RL_SUCCESS)
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
            Dirlist_SaveLastDirectory();
      }
   }

   return 0; // Return success
}