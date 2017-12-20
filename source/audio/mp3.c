/* Obtained from ctrmus source with permission. */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio/mp3.h"

static size_t * buffSize;
static mpg123_handle * mh = NULL;
static u32 rate;
static u8 channels;

static int errors = 0;

static struct
{
	int store_pics;
	int do_scan;
} param =
{
	  false
	, true
};


/**
 * Set decoder parameters for MP3.
 *
 * \param	decoder Structure to store parameters.
 */
void setMp3(struct decoder_fn * decoder)
{
	decoder->init = &initMp3;
	decoder->rate = &rateMp3;
	decoder->channels = &channelMp3;
	/*
	 * buffSize changes depending on input file. So we set buffSize later when
	 * decoder is initialised.
	 */
	buffSize = &(decoder->buffSize);
	decoder->decode = &decodeMp3;
	decoder->exit = &exitMp3;
}

/* Split up a number of lines separated by \n, \r, both or just zero byte
   and print out each line with specified prefix. */
void print_lines(char * data, const char * prefix, mpg123_string * inlines)
{
	size_t i;
	int hadcr = 0, hadlf = 0;
	char * lines = NULL;
	char * line  = NULL;
	size_t len = 0;

	if (inlines != NULL && inlines->fill)
	{
		lines = inlines->p;
		len   = inlines->fill;
	}
	else 
		return;

	line = lines;
	for (i = 0; i < len; ++i)
	{
		if (lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0)
		{
			char save = lines[i]; /* saving, changing, restoring a byte in the data */
			if (save == '\n') 
				++hadlf;
			if (save == '\r') 
				++hadcr;
			if ((hadcr || hadlf) && (hadlf % 2 == 0) && (hadcr % 2 == 0)) 
				line = "";

			if (line)
			{
				lines[i] = 0;
				snprintf(data, 0x1E, "%s%s\n", prefix, line);
				line = NULL;
				lines[i] = save;
			}
		}
		else
		{
			hadlf = hadcr = 0;
			if (line == NULL) 
				line = lines + i;
		}
	}
}

/* Print out the named ID3v2  fields. */
void print_v2(ID3v2 * ID3tag, mpg123_id3v2 * v2)
{
	print_lines(ID3tag->title, "Title: ", v2->title);
	print_lines(ID3tag->artist, "", v2->artist);
	print_lines(ID3tag->album, "Album: ", v2->album);
	print_lines(ID3tag->year, "Year: ",    v2->year);
	print_lines(ID3tag->comment, "Comment: ", v2->comment);
	print_lines(ID3tag->genre, "Genre: ",   v2->genre);
}

/* Easy conversion to string via lookup. */
const char * pic_types[] = 
{
	 "other"
	,"icon"
	,"other icon"
	,"front cover"
	,"back cover"
	,"leaflet"
	,"media"
	,"lead"
	,"artist"
	,"conductor"
	,"orchestra"
	,"composer"
	,"lyricist"
	,"location"
	,"recording"
	,"performance"
	,"video"
	,"fish"
	,"illustration"
	,"artist logo"
	,"publisher logo"
};

static const char * pic_type(int id)
{
	return (id >= 0 && id < (sizeof(pic_types)/sizeof(char*))) ? pic_types[id] : "invalid type";
}

const char * unknown_end = "picture";

static char * mime2end(mpg123_string * mime)
{
	size_t len;
	char * end;
	if(strncasecmp("image/",mime->p,6))
	{
		len = strlen(unknown_end)+1;
		end = malloc(len);
		memcpy(end, unknown_end, len);
		return end;
	}

	/* Else, use fmt out of image/fmt ... but make sure that usage stops at
	   non-alphabetic character, as MIME can have funny stuff following a ";". */
	for(len = 1; len < mime-> fill - 6; ++len)
	{
		if(!isalnum(mime->p[len-1+6])) 
			break;
	}
	/* len now containing the number of bytes after the "/" up to the next
	   invalid char or null */
	if(len < 1) 
		return "picture";

	end = malloc(len);
	if(!end) exit(11); /* Come on, is it worth wasting lines for a message? 
	                      If we're so broke, fprintf will also likely fail. */

	memcpy(end, mime->p+6,len-1);
	end[len-1] = 0;
	return end;
}

/* Construct a sane file name without introducing spaces, then open.
   Example: /some/where/some.mp3.front_cover.jpeg
   If multiple ones are there: some.mp3.front_cover2.jpeg */
int open_picfile(const char * prefix, mpg123_picture * pic)
{
	char * end, * typestr, * pfn;
	const char * pictype;
	size_t i, len;
	int fd;
	unsigned long count = 1;

	pictype = pic_type(pic->type);
	len = strlen(pictype);
	
	if (!(typestr = malloc(len+1)))
		return 0;
	
	memcpy(typestr, pictype, len);
	
	for(i = 0; i < len; ++i) 
		if(typestr[i] == ' ') 
			typestr[i] = '_';

	typestr[len] = 0;
	end = mime2end(&pic->mime_type);
	len = strlen(prefix)+1+strlen(typestr)+1+strlen(end);
	
	if(!(pfn = malloc(len+1))) 
		return 0;

	sprintf(pfn, "%s.%s.%s", prefix, typestr, end);
	pfn[len] = 0;

	errno = 0;
	fd = open(pfn, O_CREAT|O_WRONLY|O_EXCL);
	
	while((fd < 0) && (errno == EEXIST) && (++count < U64_MAX))
	{
		char dum;
		size_t digits;

		digits = snprintf(&dum, 11, "%lu", count);
		
		if(!(pfn = realloc(pfn, len+digits + 1)))
			return 0;

		sprintf(pfn, "%s.%s%lu.%s", prefix, typestr, count, end);
		pfn[len + digits] = 0;
		errno = 0;		
		fd = open(pfn, O_CREAT|O_WRONLY|O_EXCL);
	}
	
	if(fd < 0)
		++errors;

	free(end);
	free(typestr);
	free(pfn);
	return fd;
}

static void store_pictures(const char* prefix, mpg123_id3v2 *v2)
{
	int i;

	for(i=0; i<v2->pictures; ++i)
	{
		int fd;
		mpg123_picture* pic;

		pic = &v2->picture[i];
		fd = open_picfile(prefix, pic);
		if(fd >= 0)
		{ /* stream I/O for not having to care about interruptions */
			FILE * picfile = fdopen(fd, "w");
			if(picfile)
			{
				if(fwrite(pic->data, pic->size, 1, picfile) != 1)
					++errors;
				
				if(fclose(picfile))
					++errors;
				
			}
			else
				++errors;
			
		}
	}
}

/**
 * Initialise MP3 decoder.
 *
 * \param	file	Location of MP3 file to play.
 * \return			0 on success, else failure.
 */
int initMp3(const char * file)
{
	int err = 0;
	int encoding = 0;

	if ((err = mpg123_init()) != MPG123_OK)
		return err;

	if ((mh = mpg123_new(NULL, &err)) == NULL)
	{
		//printf("Error: %s\n", mpg123_plain_strerror(err));
		return err;
	}
	
	mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.);
	
	mpg123_id3v2 * v2;
	int meta;

	if (mpg123_open(mh, file) != MPG123_OK || mpg123_getformat(mh, (long *) &rate, (int *) &channels, &encoding) != MPG123_OK)
	{
		//printf("Trouble with mpg123: %s\n", mpg123_strerror(mh));
		return -1;
	}

	meta = mpg123_meta_check(mh);
	if (meta & MPG123_ID3 && mpg123_id3(mh, NULL, &v2) == MPG123_OK)
		if (v2 != NULL) 
		{
			print_v2(&ID3, v2);
			
			if(param.store_pics)
				store_pictures(id3_pic, v2);
		}
	
	/*
	 * Ensure that this output format will not change (it might, when we allow
	 * it).
	 */
	mpg123_format_none(mh);
	mpg123_format(mh, rate, channels, encoding);

	/*
	 * Buffer could be almost any size here, mpg123_outblock() is just some
	 * recommendation. The size should be a multiple of the PCM frame size.
	 */
	*buffSize = (mpg123_outblock(mh) * 16);

	return 0;
}

/**
 * Get sampling rate of MP3 file.
 *
 * \return	Sampling rate.
 */
u32 rateMp3(void)
{
	return rate;
}

/**
 * Get number of channels of MP3 file.
 *
 * \return	Number of channels for opened file.
 */
u8 channelMp3(void)
{
	return channels;
}

/**
 * Decode part of open MP3 file.
 *
 * \param buffer	Decoded output.
 * \return			Samples read for each channel.
 */
u64 decodeMp3(void * buffer)
{
	size_t done = 0;
	mpg123_read(mh, buffer, *buffSize, &done);
	return done / (sizeof(int16_t));
}

/**
 * Free MP3 decoder.
 */
void exitMp3(void)
{
	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
}