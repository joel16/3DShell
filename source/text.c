#include "clock.h"
#include "main.h"
#include "power.h"
#include "screenshot.h"
#include "text.h"
#include "utils.h"

enum TextMenuEntrys {
	TEXT_MENU_ENTRY_MARK_UNMARK_ALL,
	TEXT_MENU_ENTRY_CUT,
	TEXT_MENU_ENTRY_COPY,
	TEXT_MENU_ENTRY_PASTE,
	TEXT_MENU_ENTRY_DELETE,
	TEXT_MENU_ENTRY_INSERT_EMPTY_LINE,
	TEXT_MENU_ENTRY_SEARCH,
	TEXT_MENU_ENTRY_HEX_EDITOR,
};

char * displayFileContents(const char * filename)
{
    char * buffer = NULL;
	FILE *file = fopen(filename, "r");
	
	if (file != NULL) 
	{
		if (fseek(file, 0L, SEEK_END) == 0)  // Go to the end of the file.
		{
			long bufsize = ftell(file); // Get the size of the file.
			
			if (bufsize == -1) 
				return "Unexpected error: Failed to get file size.";

			buffer = malloc(sizeof(char) * (bufsize + 1 + 1)); // Allocate buffer
			
			if (fseek(file, 0L, SEEK_SET) != 0) // Go back to the start of the file.
				return "Unexpected error: Failed to read start of the file.";

			size_t newLen = fread(buffer, sizeof(char), bufsize, file); // Read the entire file into memory.
		
			if (ferror(file) != 0) 
				fputs("Error reading file", stderr);
			 
			else 
				buffer[newLen++] = '\0'; // Null terminate.
		}	
    
		fclose(file);
	}

	free(buffer);
	return buffer;
}

void displayText(char * path)
{
	sf2d_texture * textView = sfil_load_PNG_file("romfs:/res/textview.png", SF2D_PLACE_RAM); setBilinearFilter(textView);
	
	sf2d_set_clear_color(RGBA8(251, 251, 251, 255));
	
	while (aptMainLoop())
	{
		hidScanInput();
		u32 kPress = hidKeysDown();
		u32 kHeld = hidKeysDown();
		
		sf2d_start_frame(GFX_BOTTOM, GFX_LEFT); // Clear bottom screen
		sf2d_end_frame();
		
		sf2d_start_frame(GFX_TOP, GFX_LEFT);
		
		sf2d_draw_texture(textView, 0, 0);
		drawBatteryStatus(295, 2);
		digitalTime(346, 1);
		
		sftd_draw_textf(font, 40, 30, RGBA8(251, 251, 251, 255), 11, "%s", fileName);
		sftd_draw_textf(font, 5, 56, RGBA8(0, 0, 0, 255), 11, "%s\n", displayFileContents(path));
		
		endDrawing();
		
		if (kPress & KEY_B)
		{
			sf2d_free_texture(textView);
			mainMenu(KEEP);
		}
		
		if ((kHeld & KEY_L) && (kHeld & KEY_R))
			captureScreenshot();
	}
	
	sf2d_free_texture(textView);
}