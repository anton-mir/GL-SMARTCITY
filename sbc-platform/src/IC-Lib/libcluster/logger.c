#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __ANDROID__
#define USE_ANDROID_LOG
#endif

#ifdef USE_ANDROID_LOG
#include <android/log.h>
#define ANDROID_LOG_TAG "logger"
#endif

#include "logger.h"
#include "glogwrapper.h"

const int max_log_message_size = 16384;

void setLogDirectory(const char* directory)
{
    setGLogDirectory(directory);
}

void initLogger()
{
    initGoogleLogging("v=3");
}

void log_debug(const char* fmt, ...)
{
    char message[max_log_message_size];
    va_list arg;
    va_start(arg, fmt);
    vsprintf(message, fmt, arg);
    va_end(arg);

    #ifdef USE_ANDROID_LOG
    __android_log_print(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, "%s", message);
    #endif
    glog_debug(message);
}

void log_info(const char* fmt, ...)
{
    char message[max_log_message_size];
    va_list arg;
    va_start(arg, fmt);
    vsprintf(message, fmt, arg);
    va_end(arg);

    #ifdef USE_ANDROID_LOG
    __android_log_print(ANDROID_LOG_INFO, ANDROID_LOG_TAG, "%s", message);
    #endif
    glog_info(message);
}

void log_warning(const char* fmt, ...)
{
    char message[max_log_message_size];
    va_list arg;
    va_start(arg, fmt);
    vsprintf(message, fmt, arg);
    va_end(arg);

    #ifdef USE_ANDROID_LOG
    __android_log_print(ANDROID_LOG_WARN, ANDROID_LOG_TAG, "%s", message);
    #endif
    glog_warning(message);
}

void log_error(const char* fmt, ...)
{
    char message[max_log_message_size];
    va_list arg;
    va_start(arg, fmt);
    vsprintf(message, fmt, arg);
    va_end(arg);

    #ifdef USE_ANDROID_LOG
    __android_log_print(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, "%s", message);
    #endif
    glog_error(message);
}
