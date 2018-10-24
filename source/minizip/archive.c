#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

#include "archive.h"
#include "fs.h"
#include "progress_bar.h"
#include "unzip.h"
#include "utils.h"

#include "dmc_unrar.c"

static char *Archive_GetDirPat(char *path) {
	char *e = strrchr(path, '/');

	if (!e) {
		char* buf = strdup(path);
		return buf;
	}

	int index = (int)(e - path);
	char *str = (char *) malloc(sizeof(char) * (index + 1));
	strncpy(str, path, index);
	str[index] = '\0';

	return str;
}

static char *Archive_GetFilename(dmc_unrar_archive *archive, size_t i) {
	size_t size = dmc_unrar_get_filename(archive, i, 0, 0);
	if (!size)
		return 0;

	char *filename = (char *)malloc(size);
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

/*static const char *Archive_GetFilenameWithoutDir(const char *filename) {
	if (!filename)
		return 0;

	char *p = strrchr(filename, '/');
	if (!p)
		return filename;

	if (p[1] == '\0')
		return 0;

	return p + 1;
}*/

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
		return -1;
	}

	void *buf = (void *)malloc(bufsize);
	if (!buf)
		return -2;

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
			return res;
		}

		FILE *out = fopen(write, "wb");

		if ((out == NULL) && ((*path) == 0) && (filenameWithoutPath != (char *)filename)) {
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

		res = unzCloseCurrentFile(unzHandle);
	}
	
	if (buf)
		free(buf);
	
	return res;
}

static Result unzExtractAll(const char *src, unzFile *unzHandle) {
	Result res = 0;
	int path = 0;
	char *filename = Utils_Basename(src);
	
	unz_global_info global_info;
	memset(&global_info, 0, sizeof(unz_global_info));
	
	if ((res = unzGetGlobalInfo(unzHandle, &global_info)) != UNZ_OK) { // Get info about the zip file.
		unzClose(unzHandle);
		return res;
	}

	for (unsigned int i = 0; i < global_info.number_entry; i++) {
		ProgressBar_DisplayProgress("Extracting", filename, i, global_info.number_entry);

		if ((res = unzExtractCurrentFile(unzHandle, &path)) != UNZ_OK)
			break;

		if ((i + 1) < global_info.number_entry) {
			if ((res = unzGoToNextFile(unzHandle)) != UNZ_OK) { // Could not read next file.
				unzClose(unzHandle);
				return res;
			}
		}
	}

	return res;
}

Result Archive_ExtractZIP(const char *src, const char *dst) {
	char temp_file[512];
	char temp_path[512];

	FS_MakeDir(archive, dst);

	strncpy(temp_path, "sdmc:", sizeof(temp_path));
	strncat(temp_path, (char *)dst, (1024 - strlen(temp_path) - 1));
	chdir(temp_path);
	
	strncpy(temp_file, "sdmc:", sizeof(temp_file));
	strncat(temp_file, (char*)src, (1024 - strlen(temp_file) - 1));

	unzFile *unzHandle = unzOpen(temp_file); // Open zip file

	if (unzHandle == NULL) // not found
		return -1;

	Result res = unzExtractAll(src, unzHandle);
	res = unzClose(unzHandle);

	return res;
}

Result Archive_ExtractRAR(const char *src, const char *dst) {
	char temp_file[512];
	char temp_path[512];

	FS_MakeDir(archive, dst);

	strncpy(temp_path, "sdmc:", sizeof(temp_path));
	strncat(temp_path, (char *)dst, (1024 - strlen(temp_path) - 1));
	chdir(temp_path);
	
	strncpy(temp_file, "sdmc:", sizeof(temp_file));
	strncat(temp_file, (char*)src, (1024 - strlen(temp_file) - 1));

	dmc_unrar_archive rar_archive;
	dmc_unrar_return ret;

	ret = dmc_unrar_archive_init(&rar_archive);
	if (ret != DMC_UNRAR_OK)
		return -1;

	ret = dmc_unrar_archive_open_path(&rar_archive, temp_file);
	if (ret != DMC_UNRAR_OK)
		return -1;

	size_t count = dmc_unrar_get_file_count(&rar_archive);
	for (size_t i = 0; i < count; i++) {
		char *name = Archive_GetFilename(&rar_archive, i);
		char temp[512];
		snprintf(temp, 512, "%s%s", dst, Archive_GetDirPat(name));
		
		if (!FS_DirExists(archive, temp)) {
			if (strcmp(Archive_GetFileExt(temp), "") == 0)
				FS_MakeDir(archive, temp);
		}

		//const dmc_unrar_file *file = dmc_unrar_get_file_stat(&rar_archive, i);

		ProgressBar_DisplayProgress("Extracting", Utils_Basename(name), i, count);

		//const char *filename = Archive_GetFilenameWithoutDir(name);

		if (name && !dmc_unrar_file_is_directory(&rar_archive, i)) {
			dmc_unrar_return supported = dmc_unrar_file_is_supported(&rar_archive, i);
			
			if (supported == DMC_UNRAR_OK) {
				dmc_unrar_return extracted = dmc_unrar_extract_file_to_path(&rar_archive, i, name, NULL, true);
				
				if (extracted != DMC_UNRAR_OK) {
					free(name);
					return -1;
				}

			}
			else {
				free(name);
				return -1;
			}
		}

		free(name);
	}

	dmc_unrar_archive_close(&rar_archive);
	return 0;
}
