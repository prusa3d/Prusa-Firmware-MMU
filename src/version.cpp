#include <version.hpp>

struct Signatures {
    uint8_t project_major;
    uint8_t project_minor;
    uint16_t project_revision;
    uint16_t project_build_number;
} const signatures __attribute__((section(".user_signatures"), used)) = {
    PROJECT_VERSION_MAJOR,
    PROJECT_VERSION_MINOR,
    PROJECT_VERSION_REV,
    PROJECT_BUILD_NUMBER,
};
