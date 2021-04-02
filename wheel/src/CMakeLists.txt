set(Target udp.server)

message(STATUS "Start building : " ${Target})

# 设置包含路径
include_directories(${CMAKE_SOURCE_DIR}/src)

# 添加当前文件夹下所有文件为源文件
FILE(GLOB HTTP_SRCS "*.cpp" "*.h" "*.hpp" "*.c")

IF(NOT KCP_ENABLE)
	list(FILTER HTTP_SRCS EXCLUDE REGEX "ikcp")
ENDIF()


SOURCE_GROUP("src" FILES ${HTTP_SRCS})

# 生成动态库
add_library(${Target} STATIC ${HTTP_SRCS})

# target_link_libraries(${Target} stdc++ fs)

# 定义导出宏
# target_compile_definitions(${Target} PRIVATE LIBRARY_EXPORT)
                                
# 安装生成的动态库
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# install(TARGETS ${Target}
        # EXPORT ${Target}Tagets
        # RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        # LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        # ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        # )

# 安装头文件
# install(DIRECTORY ${CMAKE_SOURCE_DIR}/src/*.h* DESTINATION ${CMAKE_INSTALL_PREFIX})

# 将目标文件可导出文件
# install(EXPORT ${Target}Tagets DESTINATION ${CMAKE_SOURCE_DIR}/cmake)

SET_TARGET_PROPERTIES(${Target} PROPERTIES FOLDER "libs")
set_target_properties(${Target} PROPERTIES DEBUG_POSTFIX "_d")
