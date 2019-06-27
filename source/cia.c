#include <stdlib.h>
#include <string.h>

#include "cia.h"
#include "fs.h"
#include "menu_error.h"
#include "progress_bar.h"
#include "utils.h"

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
	Result ret = 0;
	u32 bytes_read = 0, bytes_written = 0;
	u64 size = 0, offset = 0;
	Handle dst_handle, src_handle;
	AM_TitleEntry title;

	if (R_FAILED(ret = FS_OpenFile(&src_handle, archive, path, FS_OPEN_READ, 0))) {
		Menu_DisplayError("FS_OpenFile failed:", ret);
		return ret;
	}
	
	if (R_FAILED(ret = AM_GetCiaFileInfo(media, &title, src_handle))) {
		Menu_DisplayError("AM_GetCiaFileInfo failed:", ret);
		return ret;
	}
	
	if (!update) { // As long as we aren't updating 3DShell, remove the title before installing.
		if (R_FAILED(ret = AM_DeleteAppTitle(media, title.titleID))) {
			Menu_DisplayError("AM_DeleteAppTitle failed:", ret);
			return ret;
		}
	}
	
	if (R_FAILED(ret = FSFILE_GetSize(src_handle, &size))) {
		Menu_DisplayError("FSFILE_GetSize failed:", ret);
		return ret;
	}
	
	if (R_FAILED(ret = AM_StartCiaInstall(media, &dst_handle))) {
		Menu_DisplayError("AM_StartCiaInstall failed:", ret);
		return ret;
	}

	size_t buf_size = 0x10000;
	u8 *buf = malloc(buf_size); // Chunk size

	do {
		memset(buf, 0, buf_size);

		if (R_FAILED(ret = FSFILE_Read(src_handle, &bytes_read, offset, buf, buf_size))) {
			free(buf);
			FSFILE_Close(src_handle);
			FSFILE_Close(dst_handle);
			Menu_DisplayError("FSFILE_Read failed:", ret);
			return ret;
		}
		if (R_FAILED(ret = FSFILE_Write(dst_handle, &bytes_written, offset, buf, bytes_read, FS_WRITE_FLUSH))) {
			free(buf);
			FSFILE_Close(src_handle);
			FSFILE_Close(dst_handle);
			Menu_DisplayError("FSFILE_Write failed:", ret);
			return ret;
		}

		offset += bytes_read;
		ProgressBar_DisplayProgress("Installing", Utils_Basename(path), offset, size);
	} while(offset < size);

	if (bytes_read != bytes_written) {
		AM_CancelCIAInstall(dst_handle);
		free(buf);
		Menu_DisplayError("CIA bytes written mismatch:", ret);
		return ret;
	}
	
	free(buf);

	if (R_FAILED(ret = AM_FinishCiaInstall(dst_handle))) {
		Menu_DisplayError("AM_FinishCiaInstall failed:", ret);
		return ret;
	}

	if (R_FAILED(ret = FSFILE_Close(src_handle))) {
		Menu_DisplayError("FSFILE_Close failed:", ret);
		return ret;
	}
	
	if (update) {
		FS_RemoveFile(archive, path);
		
		if (R_FAILED(ret = CIA_LaunchTitle(title.titleID, MEDIATYPE_SD)))
			return ret;
	}

	return 0;
}
