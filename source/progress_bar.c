#include <3ds.h>

#include "common.h"
#include "config.h"
#include "progress_bar.h"
#include "C2D_helper.h"
#include "textures.h"

void ProgressBar_DisplayProgress(char *msg, char *src, u32 offset, u32 size)
{
	float text_width = 0;
	Draw_GetTextSize(0.48f, &text_width, NULL, src);

	Draw_Image(config_dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

	Draw_Text(((320 - (dialog.subtex->width)) / 2) + 80, ((240 - (dialog.subtex->height)) / 2) + 45, 0.48f, config_dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, msg);

	Draw_Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 111, 0.48f, config_dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, src);

	Draw_Rect(((320 - (dialog.subtex->width)) / 2) + 80, ((240 - (dialog.subtex->height)) / 2) + 178, 240, 12, config_dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	Draw_Rect(((320 - (dialog.subtex->width)) / 2) + 80, ((240 - (dialog.subtex->height)) / 2) + 178, (double)offset / (double)size * 240.0, 12, config_dark_theme? TITLE_COLOUR : PROGRESS_COLOUR);
}