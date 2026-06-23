cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(tkLayoutDD4hepPlugin)

# Locate DD4hep.  Point DD4hep_DIR at the build or install prefix:
#   cmake -DDD4hep_DIR=/path/to/dd4hep/build  ..
if(NOT TARGET DD4hep::DDCore)
  find_package(DD4hep REQUIRED COMPONENTS DDCore)
  include(${DD4hep_DIR}/cmake/DD4hep.cmake)
  include(${DD4hep_DIR}/cmake/DD4hepBuild.cmake)
  dd4hep_configure_output()
endif()

dd4hep_set_compiler_flags()

dd4hep_add_plugin(tkLayoutDD4hep
  SOURCES tkLayoutTracker_geo.cpp
  USES    DD4hep::DDCore
          ROOT::Core
          ROOT::Geom
          ROOT::GenVector
)

install(TARGETS tkLayoutDD4hep
        LIBRARY DESTINATION $ENV{DD4HEP_LIBRARY_PATH}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/libtkLayoutDD4hep.components
        DESTINATION $ENV{DD4HEP_LIBRARY_PATH}
)