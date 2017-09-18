#include <stdlib.h>

#include "common.h"
#include "qr_decoder.h"
#include "quirc/quirc.h"
#include "screen.h"

#define IMAGE_WIDTH  400
#define IMAGE_HEIGHT 240

static u16 * buf;
static struct quirc * qrDec;

static Result qrInit(void)
{
	Result ret = 0;
	
	if (R_SUCCEEDED(ret = camInit())) 
	{
		if (R_FAILED(ret = CAMU_SetSize(SELECT_OUT1, SIZE_CTR_TOP_LCD, CONTEXT_A)))
			return ret;
		if (R_FAILED(ret = CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A)))
			return ret;
		if (R_FAILED(ret = CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_30)))
			return ret;
		if (R_FAILED(ret = CAMU_SetNoiseFilter(SELECT_OUT1, true)))
			return ret;
		if (R_FAILED(ret = CAMU_SetAutoExposure(SELECT_OUT1, true)))
			return ret;
		if (R_FAILED(ret = CAMU_SetAutoWhiteBalance(SELECT_OUT1, true)))
			return ret;
		if (R_FAILED(ret = CAMU_SetTrimming(PORT_CAM1, false)))
			return ret;
	}	

	buf = malloc(sizeof(u16) * IMAGE_WIDTH * IMAGE_HEIGHT);

	qrDec = quirc_new();
	
	if (quirc_resize(qrDec, IMAGE_WIDTH, IMAGE_HEIGHT) < 0)
		return 0;
	
	return 0;
}

static Result qrExit(void)
{
	Result ret = 0;

	if (R_SUCCEEDED(ret = CAMU_Activate(SELECT_NONE)))
		camExit();
	
	quirc_destroy(qrDec);
	
	free(buf);
	
	return ret;
}

static Result decodeQr(void)
{
	Handle handle;
	
	Result ret = 0;
	u32 maxBytes = 0;
	
	if (R_FAILED(ret = CAMU_GetMaxBytes(&maxBytes, IMAGE_WIDTH, IMAGE_HEIGHT)))
		return ret;
	if (R_FAILED(ret = CAMU_SetTransferBytes(PORT_CAM1, maxBytes, IMAGE_WIDTH, IMAGE_HEIGHT)))
		return ret;
	if (R_FAILED(ret = CAMU_Activate(SELECT_OUT1)))
		return ret;
	if (R_FAILED(ret = CAMU_ClearBuffer(PORT_CAM1)))
		return ret;
	if (R_FAILED(ret = CAMU_StartCapture(PORT_CAM1)))
		return ret;
	if (R_FAILED(ret = CAMU_SetReceiving(&handle, buf, PORT_CAM1, (IMAGE_WIDTH * IMAGE_HEIGHT * 2), maxBytes)))
		return ret;
	if (R_FAILED(ret = CAMU_StopCapture(PORT_CAM1)))
		return ret;
	if (R_FAILED(ret = CAMU_Activate(PORT_NONE)))
		return ret;
		
	return 0;
}

void drawQrDec(void)
{
	//Handle handle = 0;
	
	qrInit();
	
	//svcWaitSynchronization(handle, U64_MAX);
	screen_load_texture_untiled(TEXTURE_QR_DEC, buf, (IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(u16)), IMAGE_WIDTH, IMAGE_HEIGHT, GPU_RGB565, false);
	//svcCloseHandle(handle);
	
	while (aptMainLoop())
	{
		hidScanInput();

		screen_begin_frame();
		screen_select(GFX_TOP);
		
		decodeQr();
		
		screen_draw_texture(TEXTURE_QR_DEC, 0, 0);
		
		screen_end_frame();
		
		if (kPressed & KEY_B)
			break;
	}
	
	qrExit();
}