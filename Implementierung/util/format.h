#ifndef FORMAT_H
#define FORMAT_H

// TODO do we have to be portable to win or 32bit machines??

#ifdef _WIN32
    #define FORMAT_UINT_64 "%llu"
#else //Since we do not develop or indent on running in OS X, we'll assume linux
    #define FORMAT_UINT_64 "%lu"
#endif

#endif // FORMAT_H