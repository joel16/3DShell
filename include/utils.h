#pragma once

#include <setjmp.h>
#include <string>

extern std::string __application_path__;
extern jmp_buf exitJmp;

namespace Utils {
    bool IsNew3DS(void);
    void GetSizeString(char *string, u64 size);
    void UTF16ToUTF8(u8 *buf, const u16 *data, size_t length);
    std::string UTF16ToUTF8(const u16 *data);
    std::u16string UTF8ToUTF16(const char *data);
    bool IsCancelButtonPressed(void);
    void Wrap(int& value, int min, int max);
    void SafeCopy(char *dest, const char *src, std::size_t size);
}
