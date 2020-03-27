#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void setGLogDirectory(const char* directory);
void initGoogleLogging(const char* argv0);
void glog_debug(const char* message);
void glog_info(const char* message);
void glog_warning(const char* message);
void glog_error(const char* message);

#ifdef __cplusplus
}
#endif
