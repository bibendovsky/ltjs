#ifndef __MFCS_MACROS_H__
#define __MFCS_MACROS_H__


#include <malloc.h>
#include <windows.h>


#ifndef USES_CONVERSION
#define USES_CONVERSION
#endif // USES_CONVERSION


inline LPWSTR T2W(
    LPWSTR s)
{
    return s;
}

inline LPWSTR T2W(
    LPSTR s)
{
    int s_len = lstrlenA(s) + 1;
    int w_len = MultiByteToWideChar(CP_THREAD_ACP, 0, s, s_len, NULL, 0);

    if (w_len > 0) {
        LPWSTR result = static_cast<LPWSTR>(_malloca(2 * w_len));
        w_len = MultiByteToWideChar(CP_THREAD_ACP, 0, s, s_len, result, w_len);

        if (w_len > 0)
            return result;
    }

    LPWSTR result = static_cast<LPWSTR>(_malloca(2));
    result[0] = L'\0';
    return result;
}


#endif // __MFCS_MACROS_H__
