#include "bmp.h"
#include "libnsbmp.h"

/*
	Modified version of SDL-3DS's video/SDL_bmp.c by nop90
	All code written here is mostly his, this is basically modified to run without SDL.
*/

#define BYTES_PER_PIXEL 4

void * bitmap_create(int width, int height, unsigned int state)
{
	(void) state;  /* unused */
	return calloc(width * height, BYTES_PER_PIXEL);
}

unsigned char * bitmap_get_buffer(void * bitmap)
{
	return (unsigned char *)bitmap;
}

size_t bitmap_get_bpp(void * bitmap)
{
	(void) bitmap;  /* unused */
	return BYTES_PER_PIXEL;
}

void bitmap_destroy(void * bitmap)
{
	free(bitmap);
}

FILE* fopenBuffer(const char * fn, const char * mode)
{
	if ((!fn) || (!mode))
		return NULL;
	
	return fopen(fn, mode);
}

void * bufferizeFile(const char * filename, u32 * size, bool binary, bool linear)
{
	FILE * file;
	
	if(!binary)
		file = fopenBuffer(filename, "r");
	else 
		file = fopenBuffer(filename, "rb");
	
	if(!file) 
		return NULL;
	
	u8* buffer;
	long lsize;
	fseek (file, 0 , SEEK_END);
	lsize = ftell (file);
	rewind (file);
	
	if (linear)
		buffer = (u8*)linearMemAlign(lsize, 0x80);
	else 
		buffer=(u8*)malloc(lsize);
	
	if (size)
		*size=lsize;
	
	if(!buffer)
	{
		fclose(file);
		return NULL;
	}
		
	fread(buffer, 1, lsize, file);
	fclose(file);
	return buffer;
}

sf2d_texture * sfil_load_BMP(unsigned char *data, size_t size, unsigned int x)
{
	bmp_bitmap_callback_vt bitmap_callbacks = 
	{
		bitmap_create,
		bitmap_destroy,
		bitmap_get_buffer,
		bitmap_get_bpp
	};
	
	bmp_result code;
	bmp_image bmp;

	/* create our bmp image */
	bmp_create(&bmp, &bitmap_callbacks);

	/* analyse the BMP */
	code = bmp_analyse(&bmp, size, data);
	
	if (code != BMP_OK) 
	{
		bmp_finalise(&bmp);
		return NULL;
	}

	/* decode the image */
	code = bmp_decode(&bmp);
	
	if (code != BMP_OK) 
	{
		bmp_finalise(&bmp);
		return NULL;
	}
	
	sf2d_texture * texture = NULL;
	texture = sf2d_create_texture(bmp.width, bmp.height, TEXFMT_RGBA8, SF2D_PLACE_RAM);
	
	if (texture) 
	{
		sf2d_texture_set_params(texture, GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)); 
		sf2d_texture_tile32(texture);
	} 
	else 
	{
		bmp_finalise(&bmp);
		return NULL;
	} 

	uint16_t row, col;
	uint8_t *image;
	image = (uint8_t *) bmp.bitmap;
	
	for (row = 0; row != bmp.height; row++) 
	{
		for (col = 0; col != bmp.width; col++) 
		{
			size_t z = (row * bmp.width + col) * BYTES_PER_PIXEL;
			sf2d_set_pixel(texture, col, row, RGBA8(image[z + 3], image[z + 2], image[z + 1], image[z])); 	
		}
	}
	
	/* clean up */
	bmp_finalise(&bmp);
	
	return texture;
}

sf2d_texture * sfil_load_BMP_file2(const char * file)
{
	u32 size;
	u8 * buf = (u8*)bufferizeFile(file, &size, true, false);
	
	sf2d_texture * texture = sfil_load_BMP(buf, size, 0);
	
	free(buf);
	return texture;  
}