cmake_minimum_required(VERSION 3.0.0)
project(NoMalloc VERSION 0.1.0)

include(CTest)
enable_testing()

add_library(NoMalloc 
    Source/Heap.c 
    Source/Global.c
    Source/Loader/NFdeps.c
    Source/Loader/NFmap.c
    Source/Loader/NFopen.c
    Source/Loader/NFreloc.c
    Source/Loader/NFsym.c
    Source/Loader/NFusage.c
)
target_include_directories(NoMalloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

add_executable(TestHeap Tests/Heap.c)
target_include_directories(TestHeap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(TestHeap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
target_link_libraries(TestHeap NoMalloc)
add_test(NAME TestHeap COMMAND TestHeap)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
