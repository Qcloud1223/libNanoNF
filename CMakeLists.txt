cmake_minimum_required(VERSION 3.0.0)
project(NoMalloc VERSION 0.1.0)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

add_library(NoMalloc 
    Source/Heap.c 
    Source/Global.c
    Source/Loader/NFdeps.c
    Source/Loader/NFmap.c
    Source/Loader/NFopen.c
    Source/Loader/NFreloc.c
    Source/Loader/NFsym.c
    Source/Loader/NFusage.c
    Source/Loader/NFversion.c
    Source/Loader/NFinit.c
    Source/Loader.c
    Source/Box.c
)
target_include_directories(NoMalloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(NoMalloc dl)

add_executable(TestHeap Tests/Heap.c)
target_include_directories(TestHeap PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
target_link_libraries(TestHeap NoMalloc)
add_test(NAME TestHeap COMMAND TestHeap)

add_library(_1 SHARED Examples/Library/lib1.c) # maybe better name later

add_library(HashFunc SHARED Examples/Library/hash.cpp)
set_property(TARGET HashFunc PROPERTY CXX_STANDARD 14)

find_package(PCRE2 REQUIRED)
add_library(MatchFunc SHARED Examples/Library/Match.c)
target_include_directories(MatchFunc PUBLIC ${PCRE2_INCLUDE_DIRS})
target_link_libraries(MatchFunc ${PCRE2_LIBRARIES})

add_library(LinkFunc SHARED Examples/Library/Link.c)

add_executable(ori-malloc Examples/ori-malloc.c)
target_include_directories(ori-malloc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(ori-malloc NoMalloc)

add_executable(OneBoxOneFuncOnce Examples/OneBoxOneFuncOnce.c)
target_include_directories(OneBoxOneFuncOnce PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(OneBoxOneFuncOnce NoMalloc)

find_package(PCAP REQUIRED)
add_executable(NFVDemo Examples/nfv.c)
target_include_directories(NFVDemo PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${PCAP_INCLUDE_DIR}
)
target_link_libraries(NFVDemo NoMalloc ${CMAKE_DL_LIBS} ${PCAP_LIBRARY})

add_executable(UtilRun utilrun.c)
target_include_directories(UtilRun PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(UtilRun NoMalloc MatchFunc)