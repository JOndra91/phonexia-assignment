include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty/Catch/include)

set(project_sources ${PROJECT_SOURCE_DIR}/src/billManager.cpp)
file(GLOB test_sources *.cpp)

add_executable(billing-test ${test_sources} ${project_sources})
set_property(TARGET billing-test PROPERTY RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

add_test(NAME billing COMMAND billing-test)
