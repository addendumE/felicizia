# Source - https://stackoverflow.com/a/33090791
# Posted by Arne, modified by community. See post 'Timeline' for change history
# Retrieved 2026-03-22, License - CC BY-SA 4.0

cmake_minimum_required(VERSION 3.0)
project(example_project)

###############################################################################
## file globbing ##############################################################
###############################################################################

# these instructions search the directory tree when CMake is
# invoked and put all files that match the pattern in the variables
# `sources` and `data`
file(GLOB_RECURSE sources      ../main/components/*.cpp ../main/components/objects/*.cpp ../main/linux/*.cpp ../main/Websocket.cpp ../main/ThingSpeak.cpp ../main/MainApp.cpp)
# you can use set(sources src/main.cpp) etc if you don't want to
# use globbing to find files automatically

###############################################################################
## target definitions #########################################################
###############################################################################

# add the data to the target, so it becomes visible in some IDE
add_executable(example ${sources})

# just for example add some compiler flags
target_compile_options(example PUBLIC -std=c++1y -Wall -Wfloat-conversion -DLINUX_PLATFORM)
target_link_libraries(example curl websockets cjson)

# this lets me include files relative to the root source directory with a <> pair
target_include_directories(example PUBLIC ../main/components ../main/components/objects ../main/linux /usr/include/cjson ../main)

###############################################################################
## packaging ##################################################################
###############################################################################

