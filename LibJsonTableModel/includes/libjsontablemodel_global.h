#pragma once

#if ((defined __WIN32__) || (defined _WIN32)) && (!defined LIBJSONTABLEMODEL_STATIC)
    #ifdef LIBJSONTABLEMODEL_LIBRARY
        #define LIBJSONTABLEMODEL_EXPORT	__declspec(dllexport)
    #else
        #define LIBJSONTABLEMODEL_EXPORT	__declspec(dllimport)
    #endif
#else
/* Static libraries or non-Windows needs no special declaration. */
    # define LIBJSONTABLEMODEL_EXPORT
#endif
