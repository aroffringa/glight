cmake_minimum_required(VERSION 3.5)

project(glight)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0>=3.0.0 REQUIRED)
pkg_check_modules(SIGCXX2 sigc++-2.0 REQUIRED)
pkg_check_modules(FLACPP flac++ REQUIRED)
pkg_check_modules(ALSA alsa REQUIRED)
pkg_check_modules(LIBOLA libola REQUIRED)

find_package(Threads REQUIRED)

find_library(AUBIO_LIBRARY NAMES aubio)
find_path(AUBIO_INCLUDE_DIR NAMES aubio/aubio.h)

if(False)
set(CMAKE_CXX_CLANG_TIDY "clang-tidy;--fix-errors;-checks=*,-llvmlibc*,\
  -fuchsia-default-arguments-calls,\
  -cppcoreguidelines-avoid-magic-numbers,\
  -readability-magic-numbers,\
  -readability-implicit-bool-conversion,\
  -hicpp-use-auto,\
  -modernize-use-auto,\
  -google-readability-braces-around-statements,\
  -hicpp-braces-around-statements,\
  -readability-braces-around-statements,\
  -modernize-use-trailing-return-type,\
  -readability-identifier-length,\
  -llvm-else-after-return,\
  -readability-else-after-return,\
  -altera-id-dependent-backward-branch")
endif()
  
add_compile_options(
  -DGTKMM_DISABLE_DEPRECATED
  -DGDKMM_DISABLE_DEPRECATED
  -DGLIBMM_DISABLE_DEPRECATED
  -DGIOMM_DISABLE_DEPRECATED
  -O3
  -Wall
  -Wvla
  -Wzero-as-null-pointer-constant
  -Wnon-virtual-dtor
  -Wduplicated-branches
  -Wundef
  -Wvla
  -Wpointer-arith
  -Wextra
  -Wno-unused-parameter
  -march=native)
  
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
  add_compile_options(-Wno-psabi)
endif()

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-DNDEBUG)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_definitions(-DGLIGHT_INSTALL_PATH="${CMAKE_INSTALL_PREFIX}")

include_directories(SYSTEM ${GTKMM_INCLUDE_DIRS})
include_directories(SYSTEM ${SIGCXX2_INCLUDE_DIRS})
include_directories(SYSTEM ${FLACPP_INCLUDE_DIRS})
include_directories(SYSTEM ${ALSA_INCLUDE_DIRS})
include_directories(SYSTEM ${LIBOLA_INCLUDE_DIRS})
include_directories(SYSTEM ${AUBIO_INCLUDE_DIR})

set(GUIFILES
  gui/application.cpp
  gui/components/nameframe.cpp
  gui/functions.cpp
  gui/instance.cpp
  gui/renderengine.cpp
  gui/components/audiowidget.cpp
  gui/components/beatinput.cpp
  gui/components/colorselectwidget.cpp
  gui/components/colorsequencewidget.cpp
  gui/components/durationinput.cpp
  gui/components/fixturelist.cpp
  gui/components/foldercombo.cpp
  gui/components/iconbutton.cpp
  gui/components/objectlist.cpp
  gui/components/powermonitor.cpp
  gui/components/propertiesbox.cpp
  gui/components/reorderwidget.cpp
  gui/components/visualizationmenu.cpp
  gui/components/visualizationwidget.cpp
  gui/dialogs/createchasedialog.cpp
  gui/dialogs/inputselectdialog.cpp
  gui/dialogs/multicontrollableselectiondialog.cpp
  gui/dialogs/sceneselect.cpp
  gui/faders/colorcontrolwidget.cpp
  gui/faders/combocontrolwidget.cpp
  gui/faders/controlmenu.cpp
  gui/faders/controlwidget.cpp
  gui/faders/faderwidget.cpp
  gui/faders/faderwindow.cpp
  gui/faders/moverwidget.cpp
  gui/faders/togglewidget.cpp
  gui/mainwindow/actions.cpp
  gui/mainwindow/mainmenu.cpp
  gui/mainwindow/mainwindow.cpp
  gui/mainwindow/objectlistframe.cpp
  gui/state/faderstate.cpp
  gui/windows/addfixturewindow.cpp  
  gui/windows/chasepropertieswindow.cpp
  gui/windows/designwizard.cpp
  gui/windows/editcolorrange.cpp
  gui/windows/effectpropertieswindow.cpp
  gui/windows/fixturelistwindow.cpp
  gui/windows/fixtureproperties.cpp
  gui/windows/fixturetypeswindow.cpp
  gui/windows/fixturetypefunctionsframe.cpp
  gui/windows/groupwindow.cpp
  gui/windows/presetcollectionwindow.cpp
  gui/windows/scenewindow.cpp
  gui/windows/settingswindow.cpp
  gui/windows/theatredimensions.cpp
  gui/windows/timesequencepropertieswindow.cpp)

set(SYSTEMFILES
  system/audioplayer.cpp
  system/flacdecoder.cpp
  system/jsonreader.cpp
  system/jsonwriter.cpp
  system/openfixturereader.cpp
  system/reader.cpp
  system/settings.cpp
  system/writer.cpp
  system/midi/controller.cpp
  system/midi/manager.cpp
)

set(THEATREFILES
  theatre/color.cpp
  theatre/colordeduction.cpp
  theatre/effect.cpp
  theatre/fixture.cpp
  theatre/fixturefunction.cpp
  theatre/fixturemode.cpp
  theatre/fixturetype.cpp
  theatre/folder.cpp
  theatre/folderobject.cpp
  theatre/management.cpp
  theatre/managementtools.cpp
  theatre/presetcollection.cpp
  theatre/presetvalue.cpp
  theatre/sourcevaluestore.cpp
  theatre/theatre.cpp
  theatre/transition.cpp
  theatre/design/autodesign.cpp
  theatre/design/colorpreset.cpp
  theatre/design/designinfo.cpp
  theatre/design/rotation.cpp
  theatre/devices/beatfinder.cpp
  theatre/devices/olaconnection.cpp
  theatre/devices/universemap.cpp
  theatre/effects/hue_saturation_lightness_effect.cpp
  theatre/filters/filter.cpp
  theatre/properties/propertyset.cpp
  theatre/scenes/blackoutsceneitem.cpp
  theatre/scenes/scene.cpp)
  
set(EXTERNALFILES
  external/hsluv/hsluv.cpp)

include_directories(external/hsluv)

include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

add_library(glight-object OBJECT
  ${THEATREFILES} ${SYSTEMFILES} ${GUIFILES} ${EXTERNALFILES})

add_executable(glight $<TARGET_OBJECTS:glight-object> glight.cpp)

target_link_directories(glight PRIVATE ${GTKMM_LIBDIR} ${LIBOLA_LIBDIR})

set(GLIGHT_LIBRARIES
  ${FLACPP_LIBRARIES} 
  ${ALSA_LIBRARIES} 
  ${LIBOLA_LIBRARIES} 
  ${AUBIO_LIBRARY}
  ${GTKMM_LIBRARIES} 
  ${LIBOLA_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(glight ${GLIGHT_LIBRARIES})

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
    cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost COMPONENTS unit_test_framework)
if(Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  add_executable(runtests EXCLUDE_FROM_ALL
    $<TARGET_OBJECTS:glight-object>
    tests/runtests.cpp 
    tests/gui/tunits.cpp
    tests/system/tcolormap.cpp
    tests/system/tcolortemperature.cpp
    tests/system/tdeletableptr.cpp
    tests/system/tfileformat.cpp
    tests/system/tindifferentptr.cpp
    tests/system/tjsonreader.cpp
    tests/system/tjsonwriter.cpp
    tests/system/tmath.cpp
    tests/system/tsettings.cpp
    tests/system/ttrackableptr.cpp
    tests/system/topenfixturereader.cpp
    tests/system/toptionalnumber.cpp
    tests/system/tuniquewithoutordering.cpp
    tests/theatre/tchase.cpp
    tests/theatre/tcolordeduction.cpp
    tests/theatre/tcontrolvalue.cpp
    tests/theatre/tfixturecontrol.cpp
    tests/theatre/tfixturefunction.cpp
    tests/theatre/tfixturegroup.cpp
    tests/theatre/tfixturetype.cpp
    tests/theatre/tfixturetypefunction.cpp
    tests/theatre/tfolder.cpp
    tests/theatre/tfolderoperations.cpp
    tests/theatre/tfunctiontype.cpp
    tests/theatre/tmanagement.cpp
    tests/theatre/tpresetcollection.cpp
    tests/theatre/tpresetvalue.cpp
    tests/theatre/tscene.cpp
    tests/theatre/ttheatre.cpp
    tests/theatre/ttransition.cpp
    tests/theatre/tvaluesnapshot.cpp
    tests/theatre/effects/trgbmastereffect.cpp
    tests/theatre/filters/tautomasterfilter.cpp
    tests/theatre/filters/tmonochromefilter.cpp
    tests/theatre/filters/trgbfilter.cpp
    )
  target_link_directories(runtests PRIVATE ${GTKMM_LIBDIR} ${LIBOLA_LIBDIR})
  target_link_libraries(runtests ${GLIGHT_LIBRARIES})
  add_test(runtests runtests)
  add_custom_target(check COMMAND runtests -l unit_scope DEPENDS runtests)
else()
  message(STATUS "Boost testing framework not found.")
endif()

add_custom_target(
  coverage
  COMMAND gcovr -r .. -e '.*/gui/.*' -e '.*/tests/.*' -e '.*/CompilerIdCXX/.*'
  DEPENDS check
)

set(CPACK_PACKAGE_NAME "glight")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "André Offringa")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 9)
set(CPACK_PACKAGE_VERSION_PATCH 3)
set(CPACK_PACKAGE_DESCRIPTION "Glight is an application for live control of DMX lighting")
set(CPACK_DEBIAN_PACKAGE_SECTION misc)

include(CPack)

install(TARGETS glight RUNTIME DESTINATION bin)
install(DIRECTORY data/icons data/applications DESTINATION share )
