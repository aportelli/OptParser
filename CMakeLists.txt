cmake_minimum_required(VERSION 3.19.0)
project(OptParser VERSION 1.0.0)

include(GNUInstallDirs)
include(CTest)
include(CMakePackageConfigHelpers)

option(OPTPARSER_TEST "Compile unit tests" Off)

add_library(${PROJECT_NAME} INTERFACE OptParser/OptParser.hpp)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/OptParser>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/OptParser>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

if(OPTPARSER_TEST)
  enable_testing()
  add_subdirectory(tests)
endif()

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}_Targets
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
write_basic_package_version_file(
  "${PROJECT_NAME}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(
  EXPORT ${PROJECT_NAME}_Targets
  FILE ${PROJECT_NAME}Targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/OptParser
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
