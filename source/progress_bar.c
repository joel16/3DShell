#include <3ds.h>

#include "common.h"
#include "config.h"
#include "progress_bar.h"
#include "C2D_helper.h"
#include "textures.h"

void ProgressBar_DisplayProgress(char *msg, const char *src, u32 offset, u32 size) {
	C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
	C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
	C2D_SceneBegin(RENDER_BOTTOM);

	Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
	Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar

	float text_width = 0;
	Draw_GetTextSize(0.45f, &text_width, NULL, src);

	Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));

	Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6, 0.45f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, msg);

	Draw_Text(((320 - (text_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 30, 0.45f, config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, src);

	Draw_Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 60, 240, 4, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
	Draw_Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 60, (double)offset / (double)size * 240.0, 4, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);

	Draw_EndFrame();
}
