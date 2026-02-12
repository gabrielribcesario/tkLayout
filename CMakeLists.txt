cmake_minimum_required(VERSION 3.26 FATAL_ERROR)
project(tkLayout LANGUAGES C CXX)

# Use GNU-style directory hierarchy
include(GNUInstallDirs)

# ---Dependencies-----------------------------------------------------------

# Find packages
if (NOT TARGET ROOT)
    find_package(ROOT COMPONENTS Geom RIO HistPainter REQUIRED)
endif()
include(${ROOT_USE_FILE})

if (NOT TARGET Boost)
    find_package(Boost COMPONENTS program_options filesystem system json REQUIRED)
endif()

include_directories(${BOOST_INCLUDE_DIRS}
                    ${ROOT_INCLUDE_DIRS} 
                    ${PROJECT_SOURCE_DIR}/include/
                    ${PROJECT_SOURCE_DIR}/include/AnalyzerVisitors
                    ${PROJECT_SOURCE_DIR}/include/InnerCabling
                    ${PROJECT_SOURCE_DIR}/include/OuterCabling
)

# ---Options and flags-----------------------------------------------------------

# Installation options
option(TKLAYOUT_WITH_DOC "Whether or not to create doxygen doc target." OFF)
option(TKG_UPDATE_CONFIG "Whether or not to update the configuration." OFF)
set(TKG_LAYOUTDIRECTORY "$ENV{HOME}/www/layouts" CACHE PATH "Web server output directory")
set(TKG_STANDARDDIRECTORY "$ENV{HOME}/tkgeometry" CACHE PATH "Standard output directory")
set(TKG_MOMENTA "1.00, 10.00, 100.00" CACHE STRING "List of transverse momenta for the tracking performance test (in GeV/c)")
set(TKG_TRIGGERMOMENTA "1.00, 2.00, 5.00, 10.00" CACHE STRING "List of transverse momenta for the efficiency performance test (in GeV/c)")
set(TKG_THRESHOLD_PROB "1.00, 50.00, 90.00, 95.00" CACHE STRING "List of trigger efficiency for the pt threshold find test (in %)")

# Define the config file path
set(TKG_CONFIG_FILE "$ENV{HOME}/.tkgeometryrc")

# Look for the tkLayout configuration file
if(NOT EXISTS "${TKG_CONFIG_FILE}" OR TKG_UPDATE_CONFIG)
  message(STATUS "Configuring new environment at ${TKG_CONFIG_FILE}...")

  # Set the .tkgeometryrc TKG_BINDIRECTORY so it matches the CMAKE_INSTALL_PREFIX
  set(TKG_BINDIRECTORY ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

  # Check if the directories exist and create them if not
  file(MAKE_DIRECTORY "${TKG_BINDIRECTORY}")
  file(MAKE_DIRECTORY "${TKG_LAYOUTDIRECTORY}")
  file(MAKE_DIRECTORY "${TKG_STANDARDDIRECTORY}")

  # Create the configuration file from the template
  configure_file("${PROJECT_SOURCE_DIR}/cmake/tkgeometryrc.in" 
                 "${TKG_CONFIG_FILE}" @ONLY)
else()
  message(STATUS "Reading existing configuration from ${TKG_CONFIG_FILE}...")

  # Read the file into a list of strings
  file(STRINGS "${TKG_CONFIG_FILE}" ConfigLines)
  # Loop through each line to parse "KEY=VALUE"
  foreach(LINE IN LISTS ConfigLines)
    # Regex matches: Optional 'export ', then KEY, then =, then VALUE
    if(LINE MATCHES "^(export[ \t]+)?([A-Za-z0-9_]+)=(.*)$")
      # CMAKE_MATCH_2 is the KEY (e.g., TKG_BINDIRECTORY)
      set(KEY "${CMAKE_MATCH_2}")
      # CMAKE_MATCH_3 is the VALUE (e.g., /home/user/bin)
      set(VAL "${CMAKE_MATCH_3}")
      # Optional: Remove surrounding quotes if bash used them (VAR="Val")
      string(REGEX REPLACE "^[\"']|[\"']$" "" VAL "${VAL}")
      # Set the variable in CMake
      set(${KEY} "${VAL}")
    endif()
  endforeach()

  # Set the CMAKE_INSTALL_PREFIX so it matches the .tkgeometryrc TKG_BINDIRECTORY
  cmake_path(GET TKG_BINDIRECTORY PARENT_PATH CMAKE_INSTALL_PREFIX)
endif()

# # Doxygen documentation
# if(TKLAYOUT_WITH_DOC)
#     add_subdirectory(doc)
# endif()

# Set the C++ standard to the one required by ROOT
# Note: ROOT_CXX_STANDARD not working for some reason
set(CMAKE_CXX_FLAGS ${ROOT_CXX_FLAGS})
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Export compile commands for clangd and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# List of targets
set(INSTALLABLE_TARGETS "")

# Compile options for all targets
add_compile_options(-Wall
                    -Werror
                    -fmax-errors=2
                    -O3
                    -g
                    )

add_link_options("-Wl,--copy-dt-needed-entries")

# ---Source groups-----------------------------------------------------------

# Core Objects (OBJS)
set(SOURCES_CORE
  src/Analyzer.cc
  src/AnalyzerTools.cc
  src/AnalyzerVisitor.cc
  src/Bag.cc
  src/Barrel.cc
  src/capabilities.cc
  src/ConversionStation.cc
  src/CoordinateOperations.cc
  src/DetectorModule.cc
  src/DetIdBuilder.cc
  src/Disk.cc
  src/Endcap.cc
  src/Extractor.cc
  src/GeometricModule.cc
  src/global_funcs.cc
  src/GraphVizCreator.cc
  src/Histo.cc
  src/Hit.cc
  src/InactiveElement.cc
  src/InactiveRing.cc
  src/InactiveSurfaces.cc
  src/InactiveTube.cc
  src/IrradiationMap.cc
  src/IrradiationMapsManager.cc
  src/Layer.cc
  src/MainConfigHandler.cc
  src/MatCalc.cc
  src/MatCalcDummy.cc
  src/MaterialBudget.cc
  src/MaterialObject.cc
  src/MaterialProperties.cc
  src/MaterialTab.cc
  src/MaterialTable.cc
  src/Materialway.cc
  src/MatParser.cc
  src/MessageLogger.cc
  src/ModuleCap.cc
  src/Module.cc
  src/Palette.cc
  src/PlotDrawer.cc
  src/Polygon3d.cc
  src/Property.cc
  src/PtError.cc
  src/PtErrorAdapter.cc
  src/ReportIrradiation.cc
  src/ReportModuleCount.cc
  src/Ring.cc
  src/RodPair.cc
  src/RootWeb.cc
  src/Sensor.cc
  src/SimParms.cc
  src/Squid.cc
  src/StopWatch.cc
  src/SummaryTable.cc
  src/SupportStructure.cc
  src/tk2CMSSW.cc
  src/Tracker.cc
  src/Track.cc
  src/Usher.cc
  src/Vizard.cc
  src/WeightDistributionGrid.cc
  src/XMLWriter.cc
)

# Analyzer Visitors
set(SOURCES_ANALYZER_VISITORS
  src/AnalyzerVisitors/Bandwidth.cc
  src/AnalyzerVisitors/GeometricInfo.cc
  src/AnalyzerVisitors/IrradiationPower.cc
  src/AnalyzerVisitors/MaterialBillAnalyzer.cc
  src/AnalyzerVisitors/TriggerDistanceTuningPlots.cc
  src/AnalyzerVisitors/TriggerFrequency.cc
  src/AnalyzerVisitors/TriggerProcessorBandwidth.cc
  src/AnalyzerVisitors/ModuleCount.cc
  src/AnalyzerVisitors/JsonVisitor.cc
)

# Outer Cabling
set(SOURCES_OUTER_CABLING
  src/OuterCabling/OuterBundle.cc
  src/OuterCabling/OuterCable.cc
  src/OuterCabling/outer_cabling_constants.cc
  src/OuterCabling/outer_cabling_functions.cc
  src/OuterCabling/OuterCablingMap.cc
  src/OuterCabling/OuterDTC.cc
  src/OuterCabling/OuterGBT.cc
  src/OuterCabling/ModulesToBundlesConnector.cc
  src/OuterCabling/PhiPosition.cc
  src/OuterCabling/ServicesChannel.cc
)

# Inner Cabling
set(SOURCES_INNER_CABLING
  src/InnerCabling/GBT.cc
  src/InnerCabling/HvLine.cc
  src/InnerCabling/inner_cabling_constants.cc
  src/InnerCabling/inner_cabling_functions.cc
  src/InnerCabling/InnerBundle.cc
  src/InnerCabling/InnerDTC.cc
  src/InnerCabling/InnerCablingMap.cc
  src/InnerCabling/ModulesToPowerChainsConnector.cc
  src/InnerCabling/PowerChain.cc
)

# ---SVN revision logic-----------------------------------------------------------

# The Makefile runs ./getRevisionDefine to generate a flag (e.g. -DREVISION=123)
# We execute this at CMake configure time.
execute_process(
    COMMAND "./getRevisionDefine"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE SVN_REV_DEFINE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "SVN Revision Flags: ${SVN_REV_DEFINE}")

# ---Build executables-----------------------------------------------------------

# Main Executable: tklayout
# Corresponds to dependencies: OBJS + ANALYZERVISITORS + OUTERCABLING + INNERCABLING + SvnRevision.o
add_executable(tklayout
  src/tklayout.cc
  src/SvnRevision.cc
  ${SOURCES_CORE}
  ${SOURCES_ANALYZER_VISITORS}
  ${SOURCES_OUTER_CABLING}
  ${SOURCES_INNER_CABLING}
)
target_link_libraries(tklayout
  PUBLIC
  ${ROOT_LIBRARIES}
  Boost::system
  Boost::filesystem
  Boost::program_options
  Boost::json
)
list(APPEND INSTALLABLE_TARGETS tklayout)

# Question, probably no longer needed
# add_executable(setup.bin ${SOURCE_SETUP} ${SOURCE_GRAPHVIZCREATOR} ${SOURCE_MAINHANDLER} ${SOURCE_GLOBALFUNCTIONS} ${HEADERS})
# target_link_libraries(setup.bin ${BOOST_LIBRARIES})

# Question, probably no longer needed
# target_link_libraries(delphize ${BOOST_LIBRARIES} ${ROOT_LIBRARIES})
# add_executable(delphize ${SOURCE_DELPHIZE})

# Setup Executable
# Corresponds to: MainConfigHandler.o global_funcs.o GraphVizCreator.o setup.cc
add_executable(setup
  src/setup.cc
  src/MainConfigHandler.cc
  src/global_funcs.cc
  src/GraphVizCreator.cc
)
target_link_libraries(setup ${ROOT_LIBRARIES} Boost::system Boost::filesystem)
list(APPEND INSTALLABLE_TARGETS setup)

# Lightweight Executables

# diskPlace
add_executable(diskPlace src/diskPlace.cc)
list(APPEND INSTALLABLE_TARGETS diskPlace)

# ringPlace
add_executable(ringPlace src/ringPlace.cc)
list(APPEND INSTALLABLE_TARGETS ringPlace)

# ringName
add_executable(ringName src/ringName.cc)
list(APPEND INSTALLABLE_TARGETS ringName)

# Apply the SVN flags specifically to SvnRevision.cc as done in the Makefile
# The script output likely contains the "-D" prefix, so we add it to compile options
set_source_files_properties(src/SvnRevision.cc PROPERTIES COMPILE_OPTIONS "${SVN_REV_DEFINE}")

# ---Tests-----------------------------------------------------------

if(EXISTS "${CMAKE_SOURCE_DIR}/test/testGraphVizCreator.cc")
  add_executable(testGraphVizCreator 
    test/testGraphVizCreator.cc 
    src/GraphVizCreator.cc
  )
  target_link_libraries(testGraphVizCreator 
    ${ROOT_LIBRARIES}
    Boost::filesystem
    Boost::system
  )
endif()

# ---Install-----------------------------------------------------------

# Install all targets under CMAKE_INSTALL_PREFIX
foreach(target ${INSTALLABLE_TARGETS})
  install(TARGETS "${target}"
          EXPORT "${PROJECT_NAME}Targets"
          LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
          ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
          RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endforeach()

# Write install instructions
install(EXPORT "${PROJECT_NAME}Targets"
        FILE "${PROJECT_NAME}Targets.cmake"
        NAMESPACE "${PROJECT_NAME}::"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# Install the style, xml and config dirs
install(DIRECTORY style
        DESTINATION ${TKG_LAYOUTDIRECTORY})
install(DIRECTORY xml
        DESTINATION ${TKG_STANDARDDIRECTORY})
install(DIRECTORY config
        DESTINATION ${TKG_STANDARDDIRECTORY})

# Uninstall target
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)