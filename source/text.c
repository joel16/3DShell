#include "common.h"
#include "clock.h"
#include "dirlist.h"
#include "power.h"
#include "screenshot.h"
#include "text.h"
#include "utils.h"
#include "wifi.h"

static long _file_get_size(FILE* fp);
static long _file_apply_contents(struct fileviewer *fv, char *contents);

static int _file_readall(FILE* fp, char **contents) 
{
	char *temp;

	if (fp == NULL)
		return -1;
	
	int file_size = (int)_file_get_size(fp);

	temp = (char*)malloc(sizeof(char) * (file_size + 10));
	*contents = (char*)malloc(sizeof(char) * (file_size + 10));

	if (temp == NULL)
		return -1;

	if (fread(temp, file_size, 1, fp) < 0)
		return -1;

	if (fclose(fp) == -1) 
		return -1;

	strncpy(*contents, temp, file_size);
	free(temp);
	
	return 0;
}

static long _file_get_size(FILE* fp) 
{
	if (fp == NULL)
		return -1;

	if (fseek(fp, 0, SEEK_END) == -1) 
		return -1;

	long fsize = ftell(fp);

	if (fsize == -1)
	{
		fseek(fp, 0, SEEK_SET);
		return -1;
	}

	if (fseek(fp, 0, SEEK_SET) == -1)
		return -1;

	return fsize;
}

static long countlines(char *path) 
{
	FILE *fp = fopen(path, "r");
	int ch;
	long count = 0;
	
	do 
	{
		ch = fgetc(fp);
		if(ch == '\n') 
			count++;
	} while(ch != EOF);

	return count;
}

static long _file_apply_contents(struct fileviewer *fv, char *contents) 
{
	if (fv == NULL) 
		return -1;

	char *token;
	int line_index = 0;

	token = strtok(contents, "\n");
  
	if (token == NULL) 
		return -1;

	if (token != NULL) 
	{
		fv->lines = countlines(fv->path);
		//fv->contents = (char**)malloc(sizeof(char**) * fv->lines);
		//fv->contents[line_index] = malloc(sizeof(char*) * (strlen(contents) + 1));
		fv->tv = (struct textviewer*)malloc(sizeof(struct textviewer) * fv->lines);
		fv->tv[line_index].contents = malloc(sizeof(char) * strlen(contents) + 1);
		fv->tv[line_index].line = line_index;
		strcpy(fv->tv[line_index].contents, token);
		line_index += 1;
	}

	token = strtok(NULL, "\n");

	while(token != NULL) 
	{
		fv->tv[line_index].contents = malloc(sizeof(char*) * (strlen(token) + 1));
		strcpy(fv->tv[line_index].contents, token);
		fv->tv[line_index].line = line_index;
		line_index += 1;
		token = strtok(NULL, "\n");
	}

	return 0;
}

int fileviewer_init(struct fileviewer *fv, const char path[]) 
{
	if (fv == NULL)
		return -1;

	fv->fp = fopen(path, "rb");

	fv->path = (char*)malloc(sizeof(char) * (strlen(path) + 3));
	strcpy(fv->path, path);

	if (fv->fp == NULL)
		return -1;

	return 0;
}

int fileviewer_getline_index(struct fileviewer *fv, int index, char **buf, int *line_number) 
{
	*buf = (char*)malloc(sizeof(char) * (strlen(fv->tv[index].contents) + 3));

	if (*buf == NULL)
		return -1;

	strcpy(*buf, fv->tv[index].contents);
	*line_number = fv->tv[index].line;
	
	return 0;
}

int fileviewer_run(struct fileviewer *fv) 
{
	if (fv == NULL) 
		return -1;

	/* must free */
	char *contents;
	char **bak_contents = &contents;

	if(_file_readall(fv->fp, &contents) == -1) 
		return -1;

	if (_file_apply_contents(fv, contents) == -1)
		return -1;

	if (*bak_contents)
		free(*bak_contents);

  return 0;
}

void draw_ui(const char *file_name, int current_line, struct fileviewer *fv) 
{
	int textline_index = 0;
	char *linebuf;
	int line_number;
	/*char *line_number_buf;
	char *page_number_buf;
	int page_number;

	line_number_buf = (char*)malloc(sizeof(char) * 100);
	page_number_buf = (char*)malloc(sizeof(char) * (strlen("") + 100));

	if (line_number_buf == NULL || page_number_buf == NULL) 
		return;*/

	/*page_number = current_line / 30;

	sprintf(page_number_buf, "%d / %d", page_number + 1, (int)(((fv->lines-30) / 30)+1));

	sftd_draw_textf(font, (390 - sftd_get_text_width(font, 11, page_number_buf)), 27, RGBA8(251, 251, 251, 255), 11, "%s", page_number_buf);*/

	for(textline_index = current_line; textline_index < fv->lines; textline_index++) 
	{
		if ((textline_index-current_line) == 30)
			break;

		if ((current_line + 30) > fv->lines)
			break;

		if (fileviewer_getline_index(fv, textline_index, &linebuf, &line_number) == 0) 
		{
			/*memset(line_number_buf, '\0', 100);
			snprintf(line_number_buf, 10, "%d", line_number+1);

			sftd_draw_textf(font, 10, 58 + ((textline_index - current_line) * 14), RGBA8(0, 0, 0, 255), 11, "%s", line_number_buf);*/
			sftd_draw_textf(font, 10, 58 + ((textline_index - current_line) * 14), RGBA8(0, 0, 0, 255), 11, "%s", linebuf);
			
			if(linebuf) 
			{
				free(linebuf);
				linebuf = NULL;
			}
		}
	}

	/*if (line_number_buf)
		free(line_number_buf);

	if (page_number_buf)
		free(page_number_buf);*/
}

int displayText(char * path)
{
	sf2d_texture * textView = sfil_load_PNG_file("romfs:/res/textview.png", SF2D_PLACE_RAM); setBilinearFilter(textView);
	
	sf2d_set_clear_color(RGBA8(251, 251, 251, 255));
	
	struct fileviewer fv;
	int current_line = 0;
	
	fileviewer_init(&fv, path);
		
	fileviewer_run(&fv);
			
	while (aptMainLoop())
	{
		hidScanInput();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT); // Clear bottom screen
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		
		sf2d_draw_texture(textView, 0, 0);
		
		drawWifiStatus(270, 2);
		drawBatteryStatus(295, 2);
		digitalTime(346, 1);
	
		sftd_draw_textf(font, 40, 27, RGBA8(251, 251, 251, 255), 11, "%s", fileName);
		draw_ui(path, current_line, &fv);
		
		if (kPressed & KEY_DUP)
		{
			if (current_line > 0) 
			{
				current_line -= 1;
				draw_ui(path, current_line, &fv);
			}
		}
		
		else if (kPressed & KEY_DDOWN)
		{
			if (current_line+30 <= fv.lines-1) 
			{
				current_line += 1;
				draw_ui(path, current_line, &fv);
			}
		}
		
		endDrawing();
		
		if (kPressed & KEY_B)
		{
			wait(100000000);
			break;
		}
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}
	
	sf2d_free_texture(textView);
	return 0;
}