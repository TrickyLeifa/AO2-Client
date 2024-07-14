cmake_minimum_required(VERSION 3.7.0)

project(SpriteChat VERSION 2.11.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(KAL_SPRITECHAT_ENABLE_DISCORD_RPC "Enable Discord Rich Presence" ON)

if(NOT KAL_BUILD_OUTPUT)
set(KAL_BUILD_OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/bin")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Network Widgets Concurrent WebSockets UiTools)

if (Qt5Core_FOUND)
  if (Qt5Core_VERSION VERSION_LESS 5.15.0)
    message(FATAL_ERROR "Minimum supported Qt5 version is 5.15.0!")
  endif()
endif()

qt_add_resources(KAL_SPRITECHAT_RESOURCES
  spritechat.qrc
)

add_executable(spritechat
  ${KAL_SPRITECHAT_RESOURCES}
  src/aoapplication.cpp
  src/aoapplication.h
  src/aoblipplayer.cpp
  src/aoblipplayer.h
  src/aobutton.cpp
  src/aobutton.h
  src/aocharbutton.cpp
  src/aocharbutton.h
  src/aoclocklabel.cpp
  src/aoclocklabel.h
  src/aoemotebutton.cpp
  src/aoemotebutton.h
  src/aoemotepreview.cpp
  src/aoemotepreview.h
  src/aoevidencebutton.cpp
  src/aoevidencebutton.h
  src/aoevidencedisplay.cpp
  src/aoevidencedisplay.h
  src/aoimage.cpp
  src/aoimage.h
  src/animationlayer.cpp
  src/animationlayer.h
  src/animationloader.h
  src/animationloader.cpp
  src/aomusicplayer.cpp
  src/aomusicplayer.h
  src/aopacket.cpp
  src/aopacket.h
  src/aosfxplayer.cpp
  src/aosfxplayer.h
  src/aotextarea.cpp
  src/aotextarea.h
  src/aotextboxwidgets.cpp
  src/aotextboxwidgets.h
  src/aoutils.cpp
  src/aoutils.h
  src/charselect.cpp
  src/chatlogpiece.cpp
  src/chatlogpiece.h
  src/courtroom.cpp
  src/courtroom.h
  src/datatypes.h
  src/debug_functions.cpp
  src/debug_functions.h
  src/demoserver.cpp
  src/demoserver.h
  src/discord_rich_presence.cpp
  src/discord_rich_presence.h
  src/emotes.cpp
  src/eventfilters.cpp
  src/eventfilters.h
  src/evidence.cpp
  src/file_functions.cpp
  src/file_functions.h
  src/gui_utils.h
  src/hardware_functions.cpp
  src/hardware_functions.h
  src/lobby.cpp
  src/lobby.h
  src/main.cpp
  src/network/websocketconnection.cpp
  src/network/websocketconnection.h
  src/networkmanager.cpp
  src/networkmanager.h
  src/options.cpp
  src/options.h
  src/packet_distribution.cpp
  src/path_functions.cpp
  src/scrolltext.cpp
  src/scrolltext.h
  src/serverdata.cpp
  src/serverdata.h
  src/text_file_functions.cpp
  src/widgets/aooptionsdialog.cpp
  src/widgets/aooptionsdialog.h
  src/widgets/direct_connect_dialog.cpp
  src/widgets/direct_connect_dialog.h
  src/widgets/server_editor_dialog.cpp
  src/widgets/server_editor_dialog.h
  src/widgets/playerlistwidget.h src/widgets/playerlistwidget.cpp
  src/widgets/moderator_dialog.h src/widgets/moderator_dialog.cpp
  src/screenslidetimer.h src/screenslidetimer.cpp
  src/moderation_functions.h src/moderation_functions.cpp
  src/network/serverinfo.h src/network/serverinfo.cpp
  src/main.cpp
)

target_compile_definitions(spritechat PUBLIC QT_NO_KEYWORDS QT_MESSAGELOGCONTEXT)
target_include_directories(spritechat PUBLIC src external)
target_link_directories(spritechat PUBLIC external)
target_link_libraries(spritechat PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Concurrent
  Qt${QT_VERSION_MAJOR}::WebSockets
  Qt${QT_VERSION_MAJOR}::UiTools
  bass
  bassopus
)

if(KAL_CLIENT_ENABLE_DISCORD_RPC)
  target_compile_definitions(spritechat PRIVATE KAL_CLIENT_ENABLE_DISCORD_RPC)
  target_link_libraries(spritechat PUBLIC discord-rpc)
endif()

set_target_properties(spritechat PROPERTIES RUNTIME_OUTPUT_NAME "SpriteChat")
set_target_properties(spritechat PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${KAL_BUILD_OUTPUT}>)

if(WIN32)
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET spritechat PROPERTY WIN32_EXECUTABLE true)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/resource/logo-client.rc")
    target_sources(spritechat PRIVATE ${APP_ICON_RESOURCE_WINDOWS})
  endif()
endif()
