#include <stdlib.h>

#include "cia.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "utils.h"

static Result CIA_RemoveTitle(u64 titleID, FS_MediaType media) {
	Result ret = 0;
	u32 count = 0;

	if (R_FAILED(ret = AM_GetTitleCount(media, &count))) {
		Menu_DisplayError("AM_GetTitleCount failed:", ret);
		return ret;
	}
	
	u32 read = 0;
	u64 *titleIDs = malloc(count *sizeof(u64));
	
	if (R_FAILED(ret = AM_GetTitleList(&read, media, count, titleIDs))) {
		Menu_DisplayError("AM_GetTitleList failed:", ret);
		free(titleIDs);
		return ret;
	}
	
	for (unsigned int i = 0; i < read; i++) {
		if (titleIDs[i] == titleID) {
			if (R_FAILED(ret = AM_DeleteAppTitle(media, titleID))) {
				Menu_DisplayError("AM_DeleteAppTitle failed:", ret);
				free(titleIDs);
				return ret;
			}

			break;
		}
	}
	
	free(titleIDs);
	return 0;
}

static Result CIA_LaunchTitle(u64 titleId, FS_MediaType mediaType) {
	Result ret = 0;
	u8 param[0x300];
	u8 hmac[0x20];
	
	if (R_FAILED(ret = APT_PrepareToDoApplicationJump(0, titleId, mediaType))) {
		Menu_DisplayError("APT_PrepareToDoApplicationJump failed:", ret);
		return ret;
	}
	if (R_FAILED(ret = APT_DoApplicationJump(param, sizeof(param), hmac))) {
		Menu_DisplayError("APT_DoApplicationJump failed:", ret);
		return ret;
	}

	return 0;
}

Result CIA_InstallTitle(const char *path, FS_MediaType media, bool update) {
	u32 bytesRead = 0, bytesWritten = 0;
	u64 fileSize = 0, offset = 0;
	Handle ciaHandle, fileHandle;
	AM_TitleEntry title;
	
	Result ret = 0;

	char *filename = Utils_Basename(path);

	if (R_FAILED(ret = FSUSER_OpenFileDirectly(&fileHandle, ARCHIVE_SDMC, fsMakePath(PATH_ASCII, ""), fsMakePath(PATH_ASCII, path), FS_OPEN_READ, 0))) {
		Menu_DisplayError("FSUSER_OpenFileDirectly failed:", ret);
		return ret;
	}
	
	if (R_FAILED(ret = AM_GetCiaFileInfo(media, &title, fileHandle))) {
		Menu_DisplayError("AM_GetCiaFileInfo failed:", ret);
		return ret;
	}
	
	if (!update) { // As long as we aren't updating 3DShell, remove the title before installing.
		if (R_FAILED(ret = CIA_RemoveTitle(title.titleID, media)))
			return ret;
	}
	
	if (R_FAILED(ret = FSFILE_GetSize(fileHandle, &fileSize))) {
		Menu_DisplayError("FSFILE_GetSize failed:", ret);
		return ret;
	}
	
	if (R_FAILED(ret = AM_StartCiaInstall(media, &ciaHandle))) {
		Menu_DisplayError("AM_StartCiaInstall failed:", ret);
		return ret;
	}
	
	u8 *cia_buffer = malloc((512 *1024));

	while (offset < fileSize) {
		ProgressBar_DisplayProgress("Installing", filename, bytesRead, fileSize);

		ret = FSFILE_Read(fileHandle, &bytesRead, offset, cia_buffer, (512 *1024)); // (512 *1024) - chunk size
		ret = FSFILE_Write(ciaHandle, &bytesWritten, offset, cia_buffer, bytesRead, 0);

		if (bytesRead != bytesWritten) {
			AM_CancelCIAInstall(ciaHandle);
			return ret;
		}

		offset += bytesWritten;
	}
	
	free(cia_buffer);

	if (R_FAILED(ret = AM_FinishCiaInstall(ciaHandle))) {
		Menu_DisplayError("AM_FinishCiaInstall failed:", ret);
		return ret;
	}

	if (R_FAILED(ret = svcCloseHandle(fileHandle))) {
		Menu_DisplayError("svcCloseHandle failed:", ret);
		return ret;
	}
	
	if (update) {
		if (R_FAILED(ret = CIA_LaunchTitle(title.titleID, MEDIATYPE_SD)))
			return ret;
	}

	return 0;
}
