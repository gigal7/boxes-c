#ifndef UTIL_H_
#define UTIL_H_


//#define DEBUG

/**
 * Print a log message
 */
#ifdef DEBUG
#define LOG(_format, ...) \
        printf("[%s:%d] " _format "\n", __FILE__, __LINE__, ## __VA_ARGS__);
#else
#define LOG(...)
#endif


#endif
