#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DISABLE_DEBUG

#define setLogDirectory(...)
#define initLogger(...)
#define log_debug(...)
#define log_info(...)
#define log_warning(...)

#else

void setLogDirectory(const char* directory);
void initLogger();
void log_debug(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_warning(const char* fmt, ...);

#endif // DISABLE_DEBUG

void log_error(const char* fmt, ...);

#ifdef __cplusplus
}
#endif
