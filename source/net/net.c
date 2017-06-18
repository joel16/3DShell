#include "net.h"

Result downloadFile(const char * url, const char * path)
{
	if (url == NULL) // Invalid URL
		return 1;
	
	if (path == NULL) // Dir path does not exist
		return 1;
	
	httpcContext context;
	Result ret = 0;
	u32 statuscode = 0;
	u32 contentsize = 0;
	u8 *buf;
	
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 0);
	if (ret != 0) // httpcOpenContext failed
		return ret;
	
	ret = httpcAddRequestHeaderField(&context, "Updater-Agent", "3DShell");
	if (ret != 0) // httpcAddRequestHeaderField failed
		return ret;
	
	ret = httpcSetSSLOpt(&context, 1<<9);
	if (ret != 0) // httpcSetSSLOpt failed
		return ret;
	
	ret = httpcBeginRequest(&context);
	if(ret != 0) // httpcBeginRequest failed
		return ret;
	
	ret = httpcGetResponseStatusCode(&context, &statuscode);
	if (ret != 0) // httpcGetResponseStatusCode failed
	{
		httpcCloseContext(&context);
		return ret;
	}
	
	if (statuscode != 200)
	{
		if (statuscode >= 300 && statuscode < 400) 
		{
			char newUrl[1024];
			ret = httpcGetResponseHeader(&context, (char*)"Location", newUrl, 1024);
			if (ret != 0)
			
				return ret;
			
			httpcCloseContext(&context);
			ret = downloadFile(newUrl, path);
			return ret;
		}
		else
		{
			httpcCloseContext(&context);
			return -1;
		}
	}
	
	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret != 0) // httpcGetDownloadSizeState failed
	{
		httpcCloseContext(&context);
		return ret;
	}
	
	buf = (u8*)malloc(contentsize);
	if (buf == NULL) // Malloc failed
		return -2;
	
	memset(buf, 0, contentsize);
	
	ret = httpcDownloadData(&context, buf, contentsize, NULL);
	if(ret != 0) // httpcDownloadData failed
	{
		free(buf);
		httpcCloseContext(&context);
		return -3;
	}
	
	remove(path);
	
	FILE * file = fopen(path, "wb");
	fwrite(buf, 1, contentsize, file); // Success
	fclose(file);
	
	free(buf);
	httpcCloseContext(&context);
	
	return 0;
}