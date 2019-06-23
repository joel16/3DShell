#include <malloc.h>
#include <minizip/unzip.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "archive.h"
#include "C2D_helper.h"
#include "config.h"
#include "fs.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

#include "dmc_unrar.c"

static char *Archive_GetDirPath(char *path) {
	char *e = strrchr(path, '/');

	if (!e) {
		char* buf = strdup(path);
		return buf;
	}

	int index = (int)(e - path);
	char *str = malloc(sizeof(char) * (index + 1));
	strncpy(str, path, index);
	str[index] = '\0';

	return str;
}

static char *Archive_GetFilename(dmc_unrar_archive *archive, size_t i) {
	size_t size = dmc_unrar_get_filename(archive, i, 0, 0);
	if (!size)
		return 0;

	char *filename = malloc(size);
	if (!filename)
		return 0;

	size = dmc_unrar_get_filename(archive, i, filename, size);
	if (!size) {
		free(filename);
		return 0;
	}

	dmc_unrar_unicode_make_valid_utf8(filename);
	if (filename[0] == '\0') {
		free(filename);
		return 0;
	}

	return filename;
}

static char *Archive_RemoveFileExt(char *filename) {
	char *ret, *lastdot;

   	if (filename == NULL)
   		return NULL;
   	if ((ret = malloc(strlen(filename) + 1)) == NULL)
   		return NULL;

   	strcpy(ret, filename);
   	lastdot = strrchr(ret, '.');

   	if (lastdot != NULL)
   		*lastdot = '\0';

   	return ret;
}

static const char *Archive_GetFileExt(const char *filename) {
	const char *ext = strrchr(filename, '.');
	return (ext && ext != filename) ? ext : (filename + strlen(filename));
}

static Result unzExtractCurrentFile(unzFile *unzHandle, int *path) {
	Result res = 0;
	char filename[256];
	unsigned int bufsize = (64 * 1024);

	unz_file_info file_info;
	if ((res = unzGetCurrentFileInfo(unzHandle, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0)) != RL_SUCCESS) {
		unzClose(unzHandle);
		Menu_DisplayError("unzGetCurrentFileInfo failed:", res);
		return res;
	}

	void *buf = malloc(bufsize);
	if (!buf)
		return -1;

	char *filenameWithoutPath = Utils_Basename(filename);

	if ((*filenameWithoutPath) == '\0') {
		if ((*path) == 0)
			mkdir(filename, 0777);
	}
	else {
		const char *write;

		if ((*path) == 0)
			write = filename;
		else
			write = filenameWithoutPath;
		
		if ((res = unzOpenCurrentFile(unzHandle)) != UNZ_OK) {
			unzClose(unzHandle);
			free(buf);
			Menu_DisplayError("unzOpenCurrentFile failed:", res);
			return res;
		}

		FILE *out = fopen(write, "wb");
		char *string = strdup(filename);

		if ((out == NULL) && ((*path) == 0) && (filenameWithoutPath != string)) {
			char c = *(filenameWithoutPath - 1);
			*(filenameWithoutPath - 1) = '\0';
			mkdir(write, 0777);
			*(filenameWithoutPath - 1) = c;
			out = fopen(write, "wb");
		}

		do {
			res = unzReadCurrentFile(unzHandle, buf, bufsize);

			if (res < 0)
				break;

			if (res > 0)
				fwrite(buf, 1, res, out);
		} 
		while (res > 0);

		fclose(out);

		if ((res = unzCloseCurrentFile(unzHandle)) != UNZ_OK) {
			free(buf);
			Menu_DisplayError("unzCloseCurrentFile failed:", res);
			return res;
		}
	}
	
	if (buf)
		free(buf);
	
	return 0;
}

static Result unzExtractAll(const char *src, unzFile *unzHandle) {
	Result res = 0;
	int path = 0;
	char *filename = Utils_Basename(src);
	
	unz_global_info global_info;
	memset(&global_info, 0, sizeof(unz_global_info));
	
	if ((res = unzGetGlobalInfo(unzHandle, &global_info)) != UNZ_OK) { // Get info about the zip file.
		unzClose(unzHandle);
		Menu_DisplayError("unzGetGlobalInfo failed:", res);
		return res;
	}

	for (unsigned int i = 0; i < global_info.number_entry; i++) {
		ProgressBar_DisplayProgress("Extracting", filename, i, global_info.number_entry);

		if ((res = unzExtractCurrentFile(unzHandle, &path)) != UNZ_OK)
			break;

		if ((i + 1) < global_info.number_entry) {
			if ((res = unzGoToNextFile(unzHandle)) != UNZ_OK) { // Could not read next file.
				unzClose(unzHandle);
				Menu_DisplayError("unzGoToNextFile failed:", res);
				return res;
			}
		}
	}

	return res;
}

static Result Archive_ExtractZIP(const char *src) {
	char *path = malloc(256);
	char *src_string = strdup(src);
	char *dirname_without_ext = Archive_RemoveFileExt(src_string);

	snprintf(path, 512, "%s/", dirname_without_ext);
	FS_MakeDir(archive, path);
	chdir(path);

	unzFile *unzHandle = unzOpen(src); // Open zip file

	if (unzHandle == NULL) {// not found
		free(path);
		free(dirname_without_ext);
		return -1;
	}

	Result res = unzExtractAll(src, unzHandle);

	if ((res = unzClose(unzHandle)) != UNZ_OK) {
		Menu_DisplayError("unzClose failed:", res);
		return res;
	}

	return res;
}

static Result Archive_ExtractRAR(const char *src) {
	char *path = malloc(256);
	char *src_string = strdup(src);
	char *dirname_without_ext = Archive_RemoveFileExt(src_string);

	snprintf(path, 512, "%s/", dirname_without_ext);
	FS_MakeDir(archive, path);
	chdir(path);

	dmc_unrar_archive rar_archive;
	dmc_unrar_return ret;

	ret = dmc_unrar_archive_init(&rar_archive);
	if (ret != DMC_UNRAR_OK) {
		free(path);
		free(dirname_without_ext);
		Menu_DisplayError("dmc_unrar_archive_init failed:", ret);
		return -1;
	}

	ret = dmc_unrar_archive_open_path(&rar_archive, src);
	if (ret != DMC_UNRAR_OK) {
		free(path);
		free(dirname_without_ext);
		Menu_DisplayError("dmc_unrar_archive_open_path failed:", ret);
		return -1;
	}

	size_t count = dmc_unrar_get_file_count(&rar_archive);

	for (size_t i = 0; i < count; i++) {
		char *filename = Archive_GetFilename(&rar_archive, i);

		char unrar_path[512];
		snprintf(unrar_path, 512, "%s%s", path, Archive_GetDirPath(filename));

		ProgressBar_DisplayProgress("Extracting", Utils_Basename(filename), i, count);

		if (!FS_DirExists(archive, unrar_path)) {
			if ((strcmp(Archive_GetFileExt(unrar_path), "") == 0) || (dmc_unrar_file_is_directory(&rar_archive, i)))
				FS_MakeDir(archive, unrar_path);
		}
		if (filename && !dmc_unrar_file_is_directory(&rar_archive, i)) {
			dmc_unrar_return supported = dmc_unrar_file_is_supported(&rar_archive, i);
			
			if (supported == DMC_UNRAR_OK) {
				dmc_unrar_return extracted = dmc_unrar_extract_file_to_path(&rar_archive, i, filename, NULL, true);
				
				if (extracted != DMC_UNRAR_OK) {
					free(filename);
					free(path);
					free(dirname_without_ext);
					return -1;
				}

			}
			else {
				free(filename);
				free(path);
				free(dirname_without_ext);
				return -1;
			}
		}

		free(filename);
	}

	free(path);
	free(dirname_without_ext);
	dmc_unrar_archive_close(&rar_archive);
	return 0;
}

Result Archive_ExtractFile(const char *path) {
	int dialog_selection = 0;
	float text_width1 = 0, text_width2 = 0, confirm_width = 0, confirm_height = 0, cancel_width = 0, cancel_height = 0;
	
	Draw_GetTextSize(0.45f, &text_width1, NULL, "This may take a few minutes.");
	Draw_GetTextSize(0.45f, &text_width2, NULL, "Do you want to continue?");
	Draw_GetTextSize(0.45f, &confirm_width, &confirm_height, "YES");
	Draw_GetTextSize(0.45f, &cancel_width, &cancel_height, "NO");

	char extension[5] = {0};
	strncpy(extension, &path[strlen(path) - 4], 4);

	while(aptMainLoop()) {
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
		C2D_SceneBegin(RENDER_BOTTOM);
		Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
		Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar

		Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

		Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "Extract file");

		Draw_Text(((320 - (text_width1)) / 2), ((240 - (dialog.subtex->height)) / 2) + 35, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "This may take a few minutes.");
		Draw_Text(((320 - (text_width2)) / 2), ((240 - (dialog.subtex->height)) / 2) + 50, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, "Do you wish to continue?");

		if (dialog_selection == 0)
			Draw_Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
		else if (dialog_selection == 1)
			Draw_Rect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

		Draw_Text(248 - (confirm_width), (159 - confirm_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
		Draw_Text(288 - cancel_width, (159 - cancel_height), 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");

		Draw_EndFrame();

		hidScanInput();
		u32 kDown = hidKeysDown();

		if (kDown & KEY_RIGHT)
			dialog_selection++;
		else if (kDown & KEY_LEFT)
			dialog_selection--;

		Utils_SetMax(&dialog_selection, 0, 1);
		Utils_SetMin(&dialog_selection, 1, 0);

		if (kDown & KEY_B)
			break;

		if (kDown & KEY_A) {
			if (dialog_selection == 1) {
				if (!strncasecmp(extension, ".rar", 4))
					return Archive_ExtractRAR(path);
				else if (!strncasecmp(extension, ".zip", 4))
					return Archive_ExtractZIP(path);
			}
			else
				break;
		}

		if (TouchInRect((288 - cancel_width) - 5, (159 - cancel_height) - 5, ((288 - cancel_width) - 5) + cancel_width + 10, ((159 - cancel_height) - 5) + cancel_height + 10)) {
			dialog_selection = 0;

			if (kDown & KEY_TOUCH)
				break;
		}
		else if (TouchInRect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, ((248 - (confirm_width)) - 5) + confirm_width + 10, ((159 - confirm_height) - 5) + confirm_height + 10)) {
			dialog_selection = 1;

			if (kDown & KEY_TOUCH) {
				if (!strncasecmp(extension, ".rar", 4))
					return Archive_ExtractRAR(path);
				else if (!strncasecmp(extension, ".zip", 4))
					return Archive_ExtractZIP(path);
			}
		}
	}

	return -1;
}
