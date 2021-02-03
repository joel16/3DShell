#include <3ds.h>

#include "c2d_helper.h"
#include "osk.h"

namespace OSK {
    std::string GetText(const std::string &initial_text, const std::string &hint_text) {
        C2D::Render();

        static SwkbdState swkbd;
        static SwkbdStatusData swkbdStatus;
        static SwkbdLearningData swkbdLearning;
        bool reload = false;
        char input_string[256];
        
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, 256);
        
        if (hint_text.length() != 0)
            swkbdSetHintText(&swkbd, hint_text.c_str());
            
        if (initial_text.length() != 0)
            swkbdSetInitialText(&swkbd, initial_text.c_str());
            
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
        
        swkbdInputText(&swkbd, input_string, 256);
        return input_string;
    }
}
