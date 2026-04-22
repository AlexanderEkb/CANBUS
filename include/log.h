/**
 * 
 */
#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>

#define logV(T, F, ...) _log(LL_VERBOSE, T, F, ##__VA_ARGS__)
#define logI(T, F, ...) _log(LL_INFO, T, F, ##__VA_ARGS__)
#define logW(T, F, ...) _log(LL_WARNING, T, F, ##__VA_ARGS__)
#define logE(T, F, ...) _log(LL_ERROR, T, F, ##__VA_ARGS__)

typedef enum {
  LL_VERBOSE  = 0,
  LL_INFO     = 1,
  LL_WARNING  = 2,
  LL_ERROR    = 3
} LogLevel_t;

void logInit(void);
void logLevel(LogLevel_t l);
void _puts(char const * const s);

void _log(LogLevel_t const verbosity, char const * const tag, char const * const fmt, ...);

#endif /* __LOG_H__ */