cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
#list(APPEND EXTRA_COMPONENT_DIRS components/lvgl_esp32_drivers components/lvgl_esp32_drivers/lvgl_touch components/lvgl_esp32_drivers/lvgl_tft)

if (NOT DEFINED PROJECT_NAME)
	project(Felicizia)
endif (NOT DEFINED PROJECT_NAME)

get_property(target_names DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)
message(target_names="${target_names}")

add_custom_command(
  TARGET Felicizia.elf  PRE_BUILD
  COMMAND ../makefsdata
  VERBATIM)