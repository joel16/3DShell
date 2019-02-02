#include <curl/curl.h>
#include <malloc.h>
#include <string.h>

#include "fs.h"
#include "progress_bar.h"
#include "net.h"
#include "utils.h"

#define  USER_AGENT APP_TITLE

static char *result_buf = NULL;
static size_t result_sz = 0;
static size_t result_written = 0;

// following function is from 
// https://github.com/angelsl/libctrfgh/blob/master/curl_test/src/main.c
static size_t Net_HandleData(char *ptr, size_t size, size_t nmemb, void *userdata) {
	(void) userdata;
	const size_t bsz = size*nmemb;

	if (result_sz == 0 || !result_buf) {
		result_sz = 0x1000;
		result_buf = (char*)malloc(result_sz);
	}

	bool need_realloc = false;
	while (result_written + bsz > result_sz) {
		result_sz <<= 1;
		need_realloc = true;
	}

	if (need_realloc) {
		char *new_buf = (char*)realloc(result_buf, result_sz);

		if (!new_buf)
			return 0;

		result_buf = new_buf;
	}

	if (!result_buf)
		return 0;

	memcpy(result_buf + result_written, ptr, bsz);
	result_written += bsz;
	return bsz;
}

// From MultiUpdater
static Result Net_SetupContext(CURL *hnd, const char  *url) {
	curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
	curl_easy_setopt(hnd, CURLOPT_URL, url);
	curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(hnd, CURLOPT_USERAGENT, USER_AGENT);
	curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
	curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, Net_HandleData);
	curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(hnd, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(hnd, CURLOPT_STDERR, stdout);
	return 0;
}

// From MultiUpdater
Result Net_DownloadFile(const char *url, const char *path) {
	Result ret = 0;
	printf("Downloading from:\n%s\nto:\n%s\n", url, path);

	void *socubuf = memalign(0x1000, 0x100000);
	if (!socubuf)
		return -1;

	if (R_FAILED(ret = socInit((u32*)socubuf, 0x100000))) {
		free(socubuf);
		return ret;
	}

	CURL *hnd = curl_easy_init();
	ret = Net_SetupContext(hnd, url);
	
	if (ret != 0) {
		socExit();
		free(result_buf);
		free(socubuf);
		result_buf = NULL;
		result_sz = 0;
		result_written = 0;
		return ret;
	}
	
	Handle handle;
	u64 offset = 0;
	u32 bytesWritten = 0;

	ret = FS_OpenFile(&handle, archive, path, (FS_OPEN_WRITE | FS_OPEN_CREATE), 0);

	if (R_FAILED(ret)) {
		printf("Error: couldn't open file to write.\n");
		socExit();
		free(result_buf);
		free(socubuf);
		result_buf = NULL;
		result_sz = 0;
		result_written = 0;
		return -1;
	}

	u64 startTime = osGetTime();

	CURLcode cres = curl_easy_perform(hnd);
	curl_easy_cleanup(hnd);

	if (cres != CURLE_OK) {
		printf("Error in:\ncurl\n");
		socExit();
		free(result_buf);
		free(socubuf);
		result_buf = NULL;
		result_sz = 0;
		result_written = 0;
		return -1;
	}

	FSFILE_Write(handle, &bytesWritten, offset, result_buf, result_written, 0);

	//ProgressBar_DisplayProgress("Downloading", Utils_Basename(path), 0, 0);

	u64 endTime = osGetTime();
	u64 totalTime = endTime - startTime;
	printf("Download took %llu milliseconds.\n", totalTime);

	socExit();
	free(result_buf);
	free(socubuf);
	result_buf = NULL;
	result_sz = 0;
	result_written = 0;
	FSFILE_Close(handle);
	return 0;
}
