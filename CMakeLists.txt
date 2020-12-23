cmake_minimum_required(VERSION 2.6)

project(glight)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0>=3.0.0 REQUIRED)
pkg_check_modules(FLACPP flac++ REQUIRED)
pkg_check_modules(ALSA alsa REQUIRED)
pkg_check_modules(LIBOLA libola REQUIRED)

find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Threads REQUIRED)

find_library(AUBIO_LIBRARY NAMES aubio)
find_path(AUBIO_INCLUDE_DIR NAMES aubio/aubio.h)

add_compile_options(-O3 -Wall -DNDEBUG -std=c++17 -march=native)

include_directories(${GTKMM_INCLUDE_DIRS})
include_directories(${FLACPP_INCLUDE_DIRS})
include_directories(${ALSA_INCLUDE_DIRS})
include_directories(${LIBOLA_INCLUDE_DIRS})
include_directories(${AUBIO_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIR})
include_directories(${LIBXML2_INCLUDE_DIR})

set(GUIFILES
  gui/addfixturewindow.cpp  
  gui/application.cpp  
  gui/audiowidget.cpp
  gui/chasepropertieswindow.cpp
  gui/createchasedialog.cpp
  gui/designwizard.cpp
  gui/effectpropertieswindow.cpp
  gui/fixturelistwindow.cpp
  gui/guistate.cpp
  gui/nameframe.cpp
  gui/objectlistframe.cpp
  gui/presetcollectionwindow.cpp
  gui/sceneframe.cpp
  gui/showwindow.cpp
  gui/timesequencepropertieswindow.cpp
  gui/visualizationwindow.cpp
  gui/components/colorselectwidget.cpp
  gui/components/colorsequencewidget.cpp
  gui/components/durationinput.cpp
  gui/components/foldercombo.cpp
  gui/components/objectlist.cpp
  gui/components/propertiesbox.cpp
  gui/dialogs/inputselectdialog.cpp
  gui/faders/controlwidget.cpp
  gui/faders/faderwidget.cpp
  gui/faders/faderwindow.cpp
  gui/faders/togglewidget.cpp)

set(SYSTEMFILES
  system/audioplayer.cpp
  system/beatfinder.cpp
  system/flacdecoder.cpp
  system/reader.cpp
  system/writer.cpp
)

set(THEATREFILES
  theatre/autodesign.cpp
  theatre/effect.cpp
  theatre/fixture.cpp
  theatre/fixturefunction.cpp
  theatre/folderobject.cpp
  theatre/management.cpp
  theatre/oladevice.cpp
  theatre/presetvalue.cpp
  theatre/scene.cpp
  theatre/show.cpp
  theatre/theatre.cpp
  theatre/properties/propertyset.cpp)

add_library(glight-object OBJECT
  ${GUIFILES} ${SYSTEMFILES} ${THEATREFILES})

add_executable(glight $<TARGET_OBJECTS:glight-object> glight.cpp)

link_directories(${GTKMM_LIBDIR})

set(GLIGHT_LIBRARIES
  ${Boost_FILESYSTEM_LIBRARY} 
  ${Boost_SYSTEM_LIBRARY} 
  ${FLACPP_LIBRARIES} 
  ${ALSA_LIBRARIES} 
  ${LIBOLA_LIBRARIES} 
  ${AUBIO_LIBRARY}
  ${GTKMM_LIBRARIES} 
  ${LIBXML2_LIBRARIES} 
  ${LIBOLA_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(glight ${GLIGHT_LIBRARIES})

find_package(Boost COMPONENTS filesystem system unit_test_framework)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
  add_executable(runtests EXCLUDE_FROM_ALL
    $<TARGET_OBJECTS:glight-object>
    tests/runtests.cpp 
    tests/testchase.cpp
    tests/testfileformat.cpp
    tests/testfixturecontrol.cpp
    tests/testfolder.cpp
    tests/testmanagement.cpp
    tests/testpresetcollection.cpp
    tests/testpresetvalue.cpp
    )
  target_link_libraries(runtests ${GLIGHT_LIBRARIES})
  add_test(runtests runtests)
  add_custom_target(check COMMAND runtests -l unit_scope DEPENDS runtests)
else()
  message("Boost testing framework not found.")
endif()

add_custom_target(
  coverage
  COMMAND gcovr -r .. -e '.*/tests/.*' -e '.*/CompilerIdCXX/.*'
  DEPENDS check
)

install(TARGETS glight RUNTIME DESTINATION bin)
