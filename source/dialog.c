#include <stdio.h>

#include "common.h"
#include "config.h"
#include "C2D_helper.h"
#include "textures.h"

static char message_resized[41];

static void Dialog_DisplayBoxAndMsg(const char *title, const char *msg_1, const char *msg_2, float msg_1_width, float msg_2_width, bool with_bg) {
    if (with_bg) {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(RENDER_BOTTOM, config.dark_theme? BLACK_BG : WHITE);
        C2D_SceneBegin(RENDER_BOTTOM);
        Draw_Rect(0, 0, 320, 20, config.dark_theme? STATUS_BAR_DARK : MENU_BAR_LIGHT); // Status bar
        Draw_Rect(0, 20, 320, 220, config.dark_theme? MENU_BAR_DARK : STATUS_BAR_LIGHT); // Menu bar
    }

    Draw_Image(config.dark_theme? dialog_dark : dialog, ((320 - (dialog.subtex->width)) / 2), ((240 - (dialog.subtex->height)) / 2));
    Draw_Text(((320 - (dialog.subtex->width)) / 2) + 6, ((240 - (dialog.subtex->height)) / 2) + 6 - 3, 0.42f, 
        config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, title);

    if (msg_1 && msg_2) {
        Draw_Text(((320 - (msg_1_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 34 - 3, 0.42f, 
            config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, msg_1);

        Draw_Text(((320 - (msg_2_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 50 - 4, 0.42f, 
            config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, msg_2);
    }
    else if (msg_1 && !msg_2)
        Draw_Text(((320 - (msg_1_width)) / 2), ((240 - (dialog.subtex->height)) / 2) + 40 - 3, 0.42f, 
            config.dark_theme? TEXT_MIN_COLOUR_DARK : TEXT_MIN_COLOUR_LIGHT, msg_1);
}

void Dialog_DisplayMessage(const char *title, const char *msg_1, const char *msg_2, bool with_bg) {
    float text_width1 = 0.0f, text_width2 = 0.0f, confirm_width = 0.0f, confirm_height = 0.0f;
    
    if (msg_1)
        Draw_GetTextSize(0.42f, &text_width1, NULL, msg_1);
        
    if (msg_2)
        Draw_GetTextSize(0.42f, &text_width2, NULL, msg_2);
    
    Dialog_DisplayBoxAndMsg(title, msg_1, msg_2, text_width1, text_width2, with_bg);

    Draw_GetTextSize(0.42f, &confirm_width, &confirm_height, "OK");

    Draw_Rect((288 - confirm_width) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, 
        config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    Draw_Text(288 - confirm_width, (159 - confirm_height) - 3, 0.42f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "OK");

    if (with_bg)
        Draw_EndFrame();
}

void Dialog_DisplayPrompt(const char *title, const char *msg_1, const char *msg_2, int *selection, bool with_bg) {
    float text_width1 = 0.0f, text_width2 = 0.0f, confirm_width = 0.0f, confirm_height = 0.0f, cancel_width = 0.0f, cancel_height = 0.0f;
    
    if (msg_1)
        Draw_GetTextSize(0.42f, &text_width1, NULL, msg_1);
        
    if (msg_2)
        Draw_GetTextSize(0.42f, &text_width2, NULL, msg_2);
        
    Dialog_DisplayBoxAndMsg(title, msg_1, msg_2, text_width1, text_width2, with_bg);
    
    Draw_GetTextSize(0.42f, &confirm_width, &confirm_height, "YES");
    Draw_GetTextSize(0.42f, &cancel_width, &cancel_height, "NO");
    
    if (*selection == 0)
        Draw_Rect((288 - cancel_width) - 5, (159 - cancel_height) - 5, cancel_width + 10, cancel_height + 10, 
            config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    else if (*selection == 1)
        Draw_Rect((248 - (confirm_width)) - 5, (159 - confirm_height) - 5, confirm_width + 10, confirm_height + 10, 
            config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
            
    Draw_Text(248 - (confirm_width), (159 - confirm_height) - 3, 0.42f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "YES");
    Draw_Text(288 - cancel_width, (159 - cancel_height) - 3, 0.42f, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR, "NO");
    
    if (with_bg)
        Draw_EndFrame();
}

void Dialog_DisplayProgress(const char *title, const char *message, u32 offset, u32 size) {
    snprintf(message_resized, 41, "%.40s", message);
    float text_width = 0.0f;
    Draw_GetTextSize(0.42f, &text_width, NULL, message_resized);
    
    Dialog_DisplayBoxAndMsg(title, message_resized, NULL, text_width, 0, true);
    
    Draw_Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 65, 240, 4, 
        config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);
    Draw_Rect(((320 - (dialog.subtex->width)) / 2) + 20, ((240 - (dialog.subtex->height)) / 2) + 65, 
        (double)offset / (double)size * 240.0, 4, config.dark_theme? TITLE_COLOUR_DARK : TITLE_COLOUR);
    
    Draw_EndFrame();
}
