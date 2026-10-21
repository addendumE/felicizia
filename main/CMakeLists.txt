file(GLOB SOURCES 
    "main.c"
    "partition_table_update.c"
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
target_compile_definitions(${COMPONENT_LIB} PRIVATE BUILD_VERSION=\"VER_3_12\")
set_source_files_properties("${CMAKE_BINARY_DIR}/partition_table/partition-table.bin" PROPERTIES GENERATED TRUE)
target_add_binary_data(${COMPONENT_LIB} "${CMAKE_BINARY_DIR}/partition_table/partition-table.bin" BINARY DEPENDS partition_table_bin)

