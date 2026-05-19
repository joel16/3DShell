#include <3ds.h>
#include <cstdio>
#include <cstring>

#include "utils.h"

namespace Utils {
    bool IsNew3DS(void) {
        bool check = false;

        if (R_FAILED(APT_CheckNew3DS(std::addressof(check)))) {
            return false;
        }
        
        return check;
    }

    // Thanks TheOfficialFloW
    void GetSizeString(char *string, u64 size) {
        double double_size = static_cast<double>(size);
        int i = 0;
        const char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
        
        while (double_size >= 1024.0f) {
            double_size /= 1024.0f;
            i++;
        }
        
        std::sprintf(string, "%.*f %s", (i == 0) ? 0 : 2, double_size, units[i]);
    }

    static size_t UTF16Strlen(const u16* str) {
        const u16* s = str;
        
        while (*s) {
            ++s;
        }

        return s - str;
    }

    void UTF16ToUTF8(u8 *outBuf, const u16 *data, size_t maxLen) {
        ssize_t units = utf16_to_utf8(outBuf, data, maxLen - 1);
        if (units < 0) {
            units = 0;
        }
        
        outBuf[units] = '\0';
    }
    
    std::string UTF16ToUTF8(const u16 *data) {
        const size_t utf8BufSize = 3 * UTF16Strlen(data) + 1;
        std::string result;
        result.resize(utf8BufSize);
        
        ssize_t units = utf16_to_utf8(reinterpret_cast<u8*>(&result[0]), data, utf8BufSize - 1);

        if (units < 0) {
            units = 0;
        }
        
        result[units] = '\0';
        result.resize(units);
        return result;
    }
    
    std::u16string UTF8ToUTF16(const char *data) {
        const size_t utf16BufSize = strlen(data) + 1;
        std::u16string result;
        result.resize(utf16BufSize);
        
        ssize_t units = utf8_to_utf16(reinterpret_cast<u16*>(&result[0]), reinterpret_cast<const u8*>(data), utf16BufSize - 1);
        
        if (units < 0) {
            units = 0;
        }
        
        result[units] = u'\0';
        result.resize(units);
        return result;
    }
    
    bool IsCancelButtonPressed(void) {
        hidScanInput();
        return hidKeysDown() & KEY_B;
    }
    
    void SetMax(int& set, int value, int max) {
        if (set > max) {
            set = value;
        }
    }
    
    void SetMin(int& set, int value, int min) {
        if (set < min) {
            set = value;
        }
    }
    
    void Wrap(int& value, int min, int max) {
        if (max < min) {
            return;
        }

        int range = max - min + 1;
        value = (value - min) % range;
        
        if (value < 0) {
            value += range;
        }
        
        value += min;
    }
    
    void SafeCopy(char *dest, const char *src, std::size_t size) {
        if (!src) {
            return;
        }
        
        std::strncpy(dest, src, size - 1);
        dest[size - 1] = '\0';
    }
}
