include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

# Disable async CSV reader
add_definitions(-DCSV_IO_NO_THREAD)

add_executable(billing main.cpp billManager.cpp)
target_compile_options(billing PRIVATE -Wall -Wextra)
set_property(TARGET billing PROPERTY CXX_STANDARD 11)
set_property(TARGET billing PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
