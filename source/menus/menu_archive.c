#include <archive.h>
#include <archive_entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archive.h"
#include "C2D_helper.h"
#include "common.h"
#include "config.h"
#include "dialog.h"
#include "fs.h"
#include "menu_error.h"
#include "touch.h"
#include "utils.h"

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

static u64 Archive_CountFiles(const char *path) {
	int ret = 0;
	u64 count = 0;

    struct archive *handle = archive_read_new();

	ret = archive_read_support_format_all(handle);

    ret = archive_read_open_filename(handle, path, 1024);
    if (ret != ARCHIVE_OK) {
		Menu_DisplayError("archive_read_open_filename :", ret);
        return 1;
    }

	for (;;) {
		struct archive_entry *entry = NULL;
        int ret = archive_read_next_header(handle, &entry);
        if (ret == ARCHIVE_EOF)
            break;
		
		count++;
	}

	ret = archive_read_free(handle);
	return count;
}

static int Archive_WriteData(struct archive *src, struct archive *dst) {
	int ret = 0;
	
	for (;;) {
		const void *chunk = NULL;
		size_t length = 0;
		s64 offset = 0;
		
		ret = archive_read_data_block(src, &chunk, &length, &offset);
		if (ret == ARCHIVE_EOF)
			return ARCHIVE_OK;
		
		if (ret != ARCHIVE_OK)
			return ret;
			
		ret = archive_write_data_block(dst, chunk, length, offset);
		if (ret != ARCHIVE_OK)
			return ret;
	}
	
	return -1;
}

int Archive_ExtractArchive(const char *path) {
	char *dest_path = malloc(256);
	char *dirname_without_ext = Archive_RemoveFileExt((char *)path);

	snprintf(dest_path, 512, "%s/", dirname_without_ext);
	FS_MakeDir(archive, dest_path);

	int count = 0;
	u64 max = Archive_CountFiles(path);

    int ret = 0;
    struct archive *handle = archive_read_new();
    struct archive *dst = archive_write_disk_new();

	ret = archive_read_support_format_all(handle);

    ret = archive_read_open_filename(handle, path, 1024);
    if (ret != ARCHIVE_OK) {
		Menu_DisplayError("archive_read_open_filename :", ret);
        return 1;
    }

    for (;;) {
		Dialog_DisplayProgress("Extracting", path, count, max);

        struct archive_entry *entry = NULL;
        ret = archive_read_next_header(handle, &entry);

        if (ret == ARCHIVE_EOF)
            break;

        if (ret != ARCHIVE_OK) {
			Menu_DisplayError("archive_read_next_header failed:", ret);

            if (ret != ARCHIVE_WARN)
                break;
        }

        const char *entry_path = archive_entry_pathname(entry);

        char new_path[1024 * 4];
        ret = snprintf(new_path, sizeof(new_path), "%s/%s", dest_path, entry_path);

        ret = archive_entry_update_pathname_utf8(entry, new_path);
        if (!ret) {
			Menu_DisplayError("archive_entry_update_pathname_utf8:", ret);
            break;
        }

		ret = archive_write_disk_set_options(dst, ARCHIVE_EXTRACT_UNLINK);
        ret = archive_write_header(dst, entry);
        if (ret != ARCHIVE_OK) {
			Menu_DisplayError("archive_write_header failed:", ret);
            break;
        }

        ret = Archive_WriteData(handle, dst);
        ret = archive_write_finish_entry(dst);
		count++;
    }

    ret = archive_write_free(dst);
    ret = archive_read_free(handle);
	free(dest_path);
	free(dirname_without_ext);
    return ret;
}

Result Archive_ExtractFile(const char *path) {
	int dialog_selection = 0;

	float confirm_width = 0.0f, confirm_height = 0.0f, cancel_width = 0.0f, cancel_height = 0.0f;
	Draw_GetTextSize(0.42f, &confirm_width, &confirm_height, "YES");
	Draw_GetTextSize(0.42f, &cancel_width, &cancel_height, "NO");

	while(aptMainLoop()) {
		Dialog_DisplayPrompt("Extract file", "This may take a few minutes.", "Do you want to continue?", &dialog_selection, true);

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
			if (dialog_selection == 1)
				return Archive_ExtractArchive(path);
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

			if (kDown & KEY_TOUCH)
				return Archive_ExtractArchive(path);
		}
	}

	return -1;
}
