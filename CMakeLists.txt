cmake_minimum_required(VERSION 2.6)

project(glight)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0>=3.0.0 REQUIRED)
pkg_check_modules(FLACPP flac++ REQUIRED)
pkg_check_modules(ALSA alsa REQUIRED)
pkg_check_modules(LIBOLA libola REQUIRED)

find_package(Boost COMPONENTS system REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(Threads REQUIRED)

find_library(AUBIO_LIBRARY NAMES aubio)
find_path(AUBIO_INCLUDE_DIR NAMES aubio/aubio.h)

set(CMAKE_CXX_FLAGS "-O3 -Wall -DNDEBUG -march=native -std=c++11")
set(CMAKE_C_FLAGS "-O3 -Wall -DNDEBUG -march=native")

include_directories(${GTKMM_INCLUDE_DIRS})
include_directories(${FLACPP_INCLUDE_DIRS})
include_directories(${ALSA_INCLUDE_DIRS})
include_directories(${LIBOLA_INCLUDE_DIRS})
include_directories(${AUBIO_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIR})
include_directories(${LIBXML2_INCLUDE_DIR})

set(MAINFILES
  audioplayer.cpp
  beatfinder.cpp
  flacdecoder.cpp
  glight.cpp
  reader.cpp
  writer.cpp)

set(GUIFILES
  gui/application.cpp
  gui/audiowidget.cpp
  gui/chaseframe.cpp
  gui/chasewizard.cpp
  gui/configurationwindow.cpp
  gui/controlwidget.cpp
  gui/controlwindow.cpp
  gui/effectsframe.cpp
  gui/guistate.cpp
  gui/nameframe.cpp
  gui/presetsframe.cpp
  gui/sceneframe.cpp
  gui/sequenceframe.cpp
  gui/showwindow.cpp
  gui/visualizationwindow.cpp
  gui/components/colorselectwidget.cpp
  gui/components/controllableselectmenu.cpp
  gui/components/propertiesbox.cpp
  gui/properties/propertyset.cpp)

set(THEATREFILES
  libtheatre/defaultchase.cpp
  libtheatre/effect.cpp
  libtheatre/fixture.cpp
  libtheatre/fixturefunction.cpp
  libtheatre/management.cpp
  libtheatre/oladevice.cpp
  libtheatre/scene.cpp
  libtheatre/show.cpp
  libtheatre/theatre.cpp)
  
add_executable(glight ${MAINFILES} ${GUIFILES} ${THEATREFILES})

link_directories(${GTKMM_LIBDIR})

target_link_libraries(glight 
  ${Boost_SYSTEM_LIBRARY} 
  ${FLACPP_LIBRARIES} 
  ${ALSA_LIBRARIES} 
  ${LIBOLA_LIBRARIES} 
  ${AUBIO_LIBRARY}
  ${GTKMM_LIBRARIES} 
  ${LIBXML2_LIBRARIES} 
  ${LIBOLA_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS glight RUNTIME DESTINATION bin)
