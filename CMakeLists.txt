project (indicator-location)
cmake_minimum_required (VERSION 2.8.9)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set (PROJECT_VERSION "13.10.0")
set (PACKAGE ${CMAKE_PROJECT_NAME})
set (GETTEXT_PACKAGE ${CMAKE_PROJECT_NAME})

option (enable_tests "Build the package's automatic tests." ON)
option (enable_lcov "Generate lcov code coverage reports." ON)

##
##  GNU standard paths
## 
include (GNUInstallDirs)
if (EXISTS "/etc/debian_version") # Workaround for libexecdir on debian
  set (CMAKE_INSTALL_LIBEXECDIR "${CMAKE_INSTALL_LIBDIR}")
  set (CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_FULL_LIBDIR}")
endif ()
set (CMAKE_INSTALL_PKGLIBEXECDIR "${CMAKE_INSTALL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}")
set (CMAKE_INSTALL_FULL_PKGLIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}/${CMAKE_PROJECT_NAME}")

##
##  Check for prerequisites
##

find_package (PkgConfig REQUIRED)

include (FindPkgConfig)
pkg_check_modules (SERVICE REQUIRED
                   url-dispatcher-1
                   gio-unix-2.0>=2.36
                   glib-2.0>=2.36)
                   #ubuntu-platform-api>=1.0.0

##
##
##

set (CMAKE_INCLUDE_CURRENT_DIR ON)
set (CC_WARNING_ARGS " -Wall -Wextra -Wno-missing-field-initializers")

# testing & coverage
if (${enable_tests})
  set (GTEST_SOURCE_DIR /usr/src/gtest/src)
  set (GTEST_INCLUDE_DIR ${GTEST_SOURCE_DIR}/..)
  set (GTEST_LIBS -lpthread)
  enable_testing ()
  if (${enable_lcov})
    include(GCov)
  endif ()
endif ()

add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (po)
if (${enable_tests})
  add_subdirectory (tests)
endif ()