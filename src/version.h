/// @file version.h
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define project_version_major 2
#define project_version_minor 1
#define project_version_revision 1
#define project_version_build 634
#define FW_BUILD_NUMBER 0

/// Project's version (2.0.0)
extern const char project_version[];

/// Full project's version (2.0.0-BETA+1035.PR111.B4)
extern const char project_version_full[];

/// Project's version suffix (-BETA+1035.PR111.B4)
extern const char project_version_suffix[];

/// Project's short version suffix (+1035)
extern const char project_version_suffix_short[];

/// Project's major version
extern const uint8_t project_major;

/// Project's minor version
extern const uint8_t project_minor;

/// Project's revision number
extern const uint16_t project_revision;

/// Project's build number (number of commits in a branch)
extern const uint16_t project_build_number;

/// Firmware name
extern const char project_firmware_name[];

#ifdef __cplusplus
}
#endif //__cplusplus
