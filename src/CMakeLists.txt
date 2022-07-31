target_sources(firmware PRIVATE application.cpp debug.cpp main.cpp registers.cpp version.hpp)

target_link_libraries(firmware LUFA)

target_compile_definitions(
  firmware
  PRIVATE
  PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  PROJECT_VERSION_REV=${PROJECT_VERSION_REV}
  PROJECT_BUILD_NUMBER=${BUILD_NUMBER}
  FW_VERSION_FULL_STR="${PROJECT_VERSION_FULL}"
  FW_VERSION_STR="${PROJECT_VERSION}"
  FW_VERSION_SUFFIX_STR="${PROJECT_VERSION_SUFFIX}"
  FW_VERSION_SUFFIX_SHORT_STR="${PROJECT_VERSION_SUFFIX_SHORT}"
  )
add_subdirectory(hal)
add_subdirectory(logic)
add_subdirectory(modules)
