#include "pp2d.h"
#include "textures.h"
#include "theme.h"

struct colour BottomScreen_colour;
struct colour Options_title_text_colour;
struct colour Settings_title_text_colour;
struct colour Storage_colour;

void ProgressBar_DisplayProgress(char * msg, char * src, u32 offset, u32 size)
{
	pp2d_begin_draw(GFX_BOTTOM, GFX_LEFT);

	pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(BottomScreen_colour.r, BottomScreen_colour.g, BottomScreen_colour.b, 255));

	pp2d_draw_texture(TEXTURE_DIALOG, ((320 - (pp2d_get_texture_width(TEXTURE_DIALOG))) / 2), 
		((240 - (pp2d_get_texture_height(TEXTURE_DIALOG))) / 2));

	pp2d_draw_text(((320 - (pp2d_get_texture_width(TEXTURE_DIALOG))) / 2) + 10, ((240 - (pp2d_get_texture_height(TEXTURE_DIALOG))) / 2) + 20, 
		0.45f, 0.45f, RGBA8(Settings_title_text_colour.r, Settings_title_text_colour.g, Settings_title_text_colour.b, 255), msg);

	pp2d_draw_text(((320 - (pp2d_get_text_width(src, 0.45f, 0.45f))) / 2), ((240 - (pp2d_get_texture_height(TEXTURE_DIALOG))) / 2) + 45, 
		0.45f, 0.45f, RGBA8(Options_title_text_colour.r, Options_title_text_colour.g, Options_title_text_colour.b, 255), src);

	pp2d_draw_rectangle(((320 - (pp2d_get_texture_width(TEXTURE_DIALOG))) / 2) + 20, ((240 - (pp2d_get_texture_height(TEXTURE_DIALOG))) / 2) + 70, 
		240, 4, RGBA8(200, 200, 200, 255));
	pp2d_draw_rectangle(((320 - (pp2d_get_texture_width(TEXTURE_DIALOG))) / 2) + 20, ((240 - (pp2d_get_texture_height(TEXTURE_DIALOG))) / 2) + 70, 
		(double)offset / (double)size * 240.0, 4, RGBA8(Storage_colour.r, Storage_colour.g, Storage_colour.b, 255));

	pp2d_end_draw();
}