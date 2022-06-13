/// @file version.c
#include "version.h"

#define _STR(x) #x
#define STR(x) _STR(x)

const char project_version[] = STR(FW_VERSION);

const char project_version_full[] = STR(FW_VERSION_FULL);

const char project_version_suffix[] = STR(FW_VERSION_SUFFIX);

const char project_version_suffix_short[] = STR(FW_VERSION_SUFFIX_SHORT);

const uint8_t project_major = project_version_major;
const uint8_t project_minor = project_version_minor;
const uint16_t project_build_number = FW_BUILD_NUMBER;
