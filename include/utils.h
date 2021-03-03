#ifndef _3D_SHELL_UTILS_H
#define _3D_SHELL_UTILS_H

#include <setjmp.h>
#include <string>

extern std::string __application_path__;
extern jmp_buf exit_jmp;

namespace Utils {
    void GetSizeString(char *string, double size);
    void SetBounds(int *set, int min, int max);
    void SetMax(int *set, int value, int max);
    void SetMin(int *set, int value, int min);
}

#endif
