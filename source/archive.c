#include "archive.h"
#include "file/fs.h"
#include "unzip/unzip.h"
#include "utils.h"

Result extractZip(const char * zipFile, const char * path)
{
	char tmpFile2[1024];
	char tmpPath2[1024];

	makeDir(fsArchive, path);

	strncpy(tmpPath2, "sdmc:", sizeof(tmpPath2));
	strncat(tmpPath2, (char *)path, (1024 - strlen(tmpPath2) - 1));
	chdir(tmpPath2);

	if (strncmp("romfs:/", zipFile, 7) == 0)
		strncpy(tmpFile2, zipFile, sizeof(tmpFile2));
	else
	{
		strncpy(tmpFile2, "sdmc:", sizeof(tmpFile2));
		strncat(tmpFile2, (char*)zipFile, (1024 - strlen(tmpFile2) - 1));
	}

	Zip * handle = ZipOpen(tmpFile2);

	if (handle == NULL)
		return 0;

	Result res = ZipExtract(handle, NULL);
	res = ZipClose(handle);

	return res;
}
