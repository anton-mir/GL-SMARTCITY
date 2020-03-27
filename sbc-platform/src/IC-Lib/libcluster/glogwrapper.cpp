#include <dirent.h>
#include <iostream>
#include <mutex>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "glog/logging.h"
#include "glogwrapper.h"

#define MAX_LOG_FILE_SIZE_IN_MB 100

bool isGoogleLoggingInitialized = false;
bool hasLogDirectory = false;
const char* defaultLogDirectory = "/tmp/ic_lib_log/";
std::mutex accessToGlogMutex;

bool directoryExists(const char* path)
{
    if (path == NULL) {
        return false;
    }

    DIR* pDir;
    bool bExists = false;

    pDir = opendir(path);

    if (pDir != NULL) {
        bExists = true;
        (void)closedir(pDir);
    }

    return bExists;
}

void addDirectory(const char* path)
{
    std::string command = "mkdir -p ";
    command += path;

    const int dir_err = system(command.c_str());
    if (dir_err == -1) {
        printf("Error creating director\n");
    }
}

void setGLogDirectory(const char* logDirectory)
{
    if (!directoryExists(logDirectory)) {
        addDirectory(logDirectory);
    }

    google::SetLogDestination(google::GLOG_INFO, logDirectory);
    hasLogDirectory = true;

    std::string message = "Set Log Destination = " + std::string(logDirectory);
    glog_info(message.c_str());
}

void initGoogleLogging(const char* argv0)
{
    if (!isGoogleLoggingInitialized) {
        FLAGS_alsologtostderr = true;
        if (!hasLogDirectory) {
            setGLogDirectory(defaultLogDirectory);
        }
        google::InitGoogleLogging(argv0);
        isGoogleLoggingInitialized = true;

        fLI::FLAGS_max_log_size = MAX_LOG_FILE_SIZE_IN_MB;

        glog_info("GLog is initialized successfully");
    }
}

void glog_debug(const char* message)
{
#ifndef STOP_GLOG
    std::unique_lock<std::mutex> lock(accessToGlogMutex);
    LOG(INFO) << message << "\n"; //GLog doesn't have DEBUG
#endif //STOP_GLOG
}

void glog_info(const char* message)
{
#ifndef STOP_GLOG
    std::unique_lock<std::mutex> lock(accessToGlogMutex);
    LOG(INFO) << message << "\n";
#endif //STOP_GLOG
}

void glog_warning(const char* message)
{
#ifndef STOP_GLOG
    std::unique_lock<std::mutex> lock(accessToGlogMutex);
    LOG(WARNING) << message << "\n";
#endif //STOP_GLOG
}

void glog_error(const char* message)
{
#ifndef STOP_GLOG
    std::unique_lock<std::mutex> lock(accessToGlogMutex);
    LOG(ERROR) << message << "\n";
#endif //STOP_GLOG
}
