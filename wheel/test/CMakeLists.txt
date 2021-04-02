set(Target Test)
project(Target C CXX)

# 设置语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

message(STATUS "Start building : " ${Target})

# 设置包含路径
include_directories(${CMAKE_SOURCE_DIR}/src)

# find_package(${This} REQUIRED HINTS "${CMAKE_SOURCE_DIR}/cmake")
# include_directories(${${This}_INCLUDE_DIRS})

# 设置依赖动态库、静态库路径
link_directories(
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
        )

# link_directories(${This}_LINK_DIRS)

aux_source_directory(. SRCS)

add_executable(${Target} ${SRCS})

target_link_libraries(${Target} udp.server)
IF(UNIX)

target_link_libraries(${Target} pthread)
ENDIF()

SET_TARGET_PROPERTIES(${Target} PROPERTIES FOLDER "test")
set_target_properties(${Target} PROPERTIES DEBUG_POSTFIX "_d")

