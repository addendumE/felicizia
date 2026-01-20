cmake_minimum_required(VERSION 3.16)
project(tsd LANGUAGES C CXX)
set(CMAKE_BUILD_TYPE Debug)
add_compile_definitions(LINUX)
add_compile_definitions(LINUX_PLATFORM)

set (PC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/..)
include_directories(
	/usr/include/cjson
	/usr/local/include/lvgl
	${PC_SOURCE_DIR}
	${PC_SOURCE_DIR}/linux
	${PC_SOURCE_DIR}/Gui
	${PC_SOURCE_DIR}/LVobj
)

add_executable(tsd
	${PC_SOURCE_DIR}/linux/main.c
	${PC_SOURCE_DIR}/linux/Adc.cpp
	${PC_SOURCE_DIR}/linux/Nvs.cpp
	${PC_SOURCE_DIR}/linux/Thread.cpp
	${PC_SOURCE_DIR}/linux/Timer.cpp
	${PC_SOURCE_DIR}/linux/Lock.cpp
	${PC_SOURCE_DIR}/linux/Http.cpp
	${PC_SOURCE_DIR}/linux/WifiManager.cpp
	
	${PC_SOURCE_DIR}/linux/esp_log.cpp
	${PC_SOURCE_DIR}/startup.cpp
	${PC_SOURCE_DIR}/MainApp.cpp
	#${PC_SOURCE_DIR}/PushApp.cpp
	${PC_SOURCE_DIR}/DataManager.cpp
	${PC_SOURCE_DIR}/Websocket.cpp
	${PC_SOURCE_DIR}/ThingSpeak.cpp
	${PC_SOURCE_DIR}/Gui/Gui.cpp
	${PC_SOURCE_DIR}/Gui/Page.cpp
	${PC_SOURCE_DIR}/Gui/MainPage.cpp
	
	${PC_SOURCE_DIR}/LVobj/LVobj.cpp
	${PC_SOURCE_DIR}/LVobj/LVlabel.cpp
	${PC_SOURCE_DIR}/LVobj/LVcontainer.cpp	
)

target_link_libraries(tsd lvgl lvgl_thorvg  SDL2 pthread rt curl websockets cjson)

