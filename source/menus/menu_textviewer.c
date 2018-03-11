#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "dir_list.h"
#include "fs.h"
#include "menu_textviewer.h"
#include "pp2d.h"
#include "screenshot.h"
#include "status_bar.h"
#include "textures.h"
#include "utils.h"

struct TextViewer 
{
	Handle handle;
	struct Text *text;
	char *path;
	int opened;
	u64 lines;
};

struct Text 
{
	int line;
	char *contents;
};

static Result TextViewer_ReadData(struct TextViewer *textViewer, char **contents) 
{
	Result ret = 0;
	char *temp;
	u32 bytesRead = 0;
	u64 offset = 1;

  	u64 fizeSize = FS_GetFileSize(archive, textViewer->path);

  	temp = (char*)malloc(sizeof(char) * (fizeSize + 10));
  	*contents = (char*)malloc(sizeof(char) * (fizeSize + 10));

  	if (temp == NULL)
    	return -1;

   	if (R_FAILED(ret = FSFILE_Read(textViewer->handle, &bytesRead, offset, temp, fizeSize)))
	{
		FSFILE_Close(textViewer->handle);
		return ret;
	}

	offset += bytesRead;

  	if (R_FAILED(ret = FSFILE_Close(textViewer->handle)))
    	return ret; 

  	strncpy(*contents, temp, fizeSize);
  	free(temp);

  	return 0;
}

static u64 TextViewer_CountLines(char *path) 
{
	FILE *file = fopen(path, "r");
	int ch = 0;
	u64 count = 0;
	
	do {
		ch = fgetc(file);
		if (ch == '\n') 
			count++;
	} while(ch != EOF);

	fclose(file);
	return count;
}

static u64 TextViewer_ApplyContents(struct TextViewer *textViewer, char *contents) 
{
	if (textViewer == NULL)
		return -1;

	char *token;
	int line_index = 0;

	token = strtok(contents, "\n");
	printf("token %s\n", token);

	if (token == NULL)
		return -1;

	if (token != NULL) 
	{
		textViewer->lines = TextViewer_CountLines(textViewer->path);
		textViewer->text = (struct Text*)malloc(sizeof(struct Text) * textViewer->lines);
		textViewer->text[line_index].contents = malloc(sizeof(char) * strlen(contents) + 1);
		textViewer->text[line_index].line = line_index;
		strcpy(textViewer->text[line_index].contents, token);
		line_index += 1;
	}

   	token = strtok(NULL, "\n");

   	while(token != NULL)
   	{
   		textViewer->text[line_index].contents = malloc(sizeof(char*) * (strlen(token) + 1));
   		strcpy(textViewer->text[line_index].contents, token);
   		textViewer->text[line_index].line = line_index;
   		line_index += 1;
   		token = strtok(NULL, "\n");
   	}

   	return 0;
}

static Result TextViewer_Init(struct TextViewer *textViewer, const char path[]) 
{  
	Result ret = 0;

  	if (textViewer == NULL)
    	return -1;

	if (R_FAILED(ret = FS_Open(&textViewer->handle, archive, path, FS_OPEN_READ))) 
	{
		FSFILE_Close(textViewer->handle);
		return ret;
	}

  	textViewer->path = (char*)malloc(sizeof(char) * (strlen(path) + 3));
  	strcpy(textViewer->path, path);

  	return 0;
}

static int TextViewer_GetLineIndex(struct TextViewer *textViewer, int index, char **buf, int *line_number) 
{
	*buf = (char*)malloc(sizeof(char) * (strlen(textViewer->text[index].contents) + 3));
		if (*buf == NULL)
			return -1;
	
	strcpy(*buf, textViewer->text[index].contents);
	*line_number = textViewer->text[index].line;

	return 0;
}

static int TextViewer_Update(struct TextViewer *textViewer) 
{
	if (textViewer == NULL)
  		return -1;

  	/* must free */
  	char *contents;
  	char **bak_contents = &contents;

  	if (TextViewer_ReadData(textViewer, &contents) == -1) 
  	{
  		printf("Failed to read file\n");
  		return -1;
  	}

   	if (TextViewer_ApplyContents(textViewer, contents) == -1) 
   	{
   		printf("Failed to apply\n");
   		return -1;
   	}

   	if (*bak_contents) 
   		free(*bak_contents);

  	return 0;
}

static void TextViewer_DrawUI(int current_line, struct TextViewer *textViewer) 
{
	int textline_index = 0;
	char *linebuf;
	int line_number;
	char *line_number_buf;

	line_number_buf = (char*)malloc(sizeof(char) * 100);

	pp2d_draw_texture(TEXTURE_TEXTVIEWER_BG, 0, 0);

	StatusBar_DisplayBar();

	if (line_number_buf == NULL) 
	{
		printf("Memory error\n");
		return ;
	}

	pp2d_draw_text(36, 28, 0.45f, 0.45f, RGBA8(255, 255, 255, 255), fileName);

	for (textline_index = current_line; textline_index < textViewer->lines; textline_index++) 
	{
		if ((textline_index-current_line) == 30)
			break;

		if ((current_line+30) > textViewer->lines)
			break;

		if (TextViewer_GetLineIndex(textViewer, textline_index, &linebuf, &line_number) == 0) 
		{
			memset(line_number_buf, '\0', 100);
			snprintf(line_number_buf, 10, "%d", line_number+1);

			pp2d_draw_text(10, 58 + ((textline_index - current_line) * 14), 0.45f, 0.45f, RGBA8(0, 0, 0, 255), linebuf);
			
			if (linebuf) 
			{
				free(linebuf);
				linebuf = NULL;
			}
		}
	}

	if (line_number_buf)
		free(line_number_buf);
}

void TextViewer_DisplayText(char * path)
{
	int current_line = 0;
	struct TextViewer textViewer;

	if (TextViewer_Init(&textViewer, path) == -1) 
	{
		printf("failed to open\n");
		return;
	}

	if (TextViewer_Update(&textViewer) == -1)
	{
		printf("failed to run\n");
		return;
	}

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		pp2d_begin_draw(GFX_TOP, GFX_LEFT);

			TextViewer_DrawUI(current_line, &textViewer);

			if (kDown & KEY_DUP)
			{
				if (current_line > 0) 
					current_line -= 1;
			}
		
			else if (kDown & KEY_DDOWN)
			{
				if (current_line+30 <= textViewer.lines-1) 
					current_line += 1;
			}

		pp2d_end_draw();

		if (kDown & KEY_B)
		{
			wait(10);
			break;
		}

		if (((kHeld & KEY_L) && (kDown & KEY_R)) || ((kHeld & KEY_R) && (kDown & KEY_L)))
			Screenshot_Capture();
	}
}