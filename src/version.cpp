#include <version.hpp>


// Define structure at a fixed address in flash which contains information about the version of the firmware
// This structure is read using the bootloader.
struct Signatures {
    uint8_t project_major;
    uint8_t project_minor;
    uint16_t project_revision;
    uint16_t project_build_number;
} static constexpr signatures __attribute__((section(".version"), used)) = {
    PROJECT_VERSION_MAJOR,
    PROJECT_VERSION_MINOR,
    PROJECT_VERSION_REV,
    PROJECT_BUILD_NUMBER,
};
