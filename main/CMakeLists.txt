file(GLOB SOURCES 
    "main.c"
    "fs.c"
    "*.cpp"
    "esp32/*.cpp"
    "components/*.cpp"
    "components/objects/*.cpp"
)

idf_component_register(SRCS
SRCS ${SOURCES}
INCLUDE_DIRS "." esp32 components components/objects
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)
