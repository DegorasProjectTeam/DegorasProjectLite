#pragma once

#if ((defined __WIN32__) || (defined _WIN32)) && (!defined SP_CORE_STATIC)
    #ifdef SP_CORE_LIBRARY
        #define SP_CORE_EXPORT	__declspec(dllexport)
    #else
        #define SP_CORE_EXPORT	__declspec(dllimport)
    #endif
#else
/* Static libraries or non-Windows needs no special declaration. */
    # define SP_CORE_EXPORT
#endif
