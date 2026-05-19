#include <3ds.h>

#include "gui.h"
#include "osk.h"
#include "utils.h"

namespace OSK {
    std::u16string GetText(const std::string& initialText, const std::string& hintText) {
        GUI::End();

        static SwkbdState swkbd;
        static SwkbdStatusData swkbdStatus;
        static SwkbdLearningData swkbdLearning;
        bool reload = false;
        char text[256];
        
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 256);
        
        if (hintText.length() != 0) {
            swkbdSetHintText(&swkbd, hintText.c_str());
        }
            
        if (initialText.length() != 0) {
            swkbdSetInitialText(&swkbd, initialText.c_str());
        }
            
        swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
        swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Confirm", true);
        
        swkbdSetFeatures(&swkbd, SWKBD_ALLOW_HOME);
        swkbdSetFeatures(&swkbd, SWKBD_ALLOW_RESET);
        swkbdSetFeatures(&swkbd, SWKBD_ALLOW_POWER);
        swkbdSetFeatures(&swkbd, SWKBD_PREDICTIVE_INPUT);
        
        swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
        
        SwkbdDictWord words[3];
        swkbdSetDictWord(&words[0], ".3dsx", ".3dsx");
        swkbdSetDictWord(&words[1], ".cia", ".cia");
        swkbdSetDictWord(&words[2], "/3ds/", "/3ds/");
        swkbdSetDictionary(&swkbd, words, 3);
        
        swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
        swkbdSetLearningData(&swkbd, &swkbdLearning, reload, true);
        reload = true;
        
        swkbdInputText(&swkbd, text, 256);
        return Utils::UTF8ToUTF16(text);
    }
}
