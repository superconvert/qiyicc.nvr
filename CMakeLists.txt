#1.cmake verson，指定cmake版本 
cmake_minimum_required (VERSION 3.2)

#2.project name，指定项目的名称，一般和项目的文件夹名称对应
project (nvrserver)

set (NVR_VERSION_MAJOR 1)
set (NVR_VERSION_MINOR 0)

include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG ("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG ("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if (COMPILER_SUPPORTS_CXX11)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (COMPILER_SUPPORTS_CXX0X)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else ()
    message (STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif ()

#3.head file path，头文件目录
include_directories (
    ${PROJECT_SOURCE_DIR}/include 
    ${PROJECT_SOURCE_DIR}/thrift/gen-cpp 
    ${PROJECT_SOURCE_DIR}/3part/jsoncpp-master/include 
    /opt/thrift/include
)

#file(GLOB abc ${PROJECT_SOURCE_DIR}/thrift/gen-cpp/*.cpp)
#message (STATUS ${abc})

#4.source directory，源文件目录
aux_source_directory (src DIR_SRCS)
aux_source_directory (${PROJECT_SOURCE_DIR}/thrift/gen-cpp THRIFT_SRCS)
list(REMOVE_ITEM THRIFT_SRCS 
    "${PROJECT_SOURCE_DIR}/thrift/gen-cpp/nvrWebService_server.skeleton.cpp"
    "${PROJECT_SOURCE_DIR}/thrift/gen-cpp/nvrWebService_async_server.skeleton.cpp"
)

#5.set environment variable，设置环境变量，编译用到的源文件全部都要放到这里，否则编译能够通过，但是执行的时候会出现各种问题，比如"symbol lookup error xxxxx , undefined symbol"
set (SRC_PATH ${DIR_SRCS})

#6.add executable file，添加要编译的可执行文件
add_executable (${PROJECT_NAME} ${SRC_PATH} ${THRIFT_SRCS})

#7.add link library，添加可执行文件所需要的库，比如我们用到了libm.so（命名规则：lib+name+.so），就添加该库的名称
# 官方不推荐使用 link_directories, 经常找不到，推荐用 find_library 
find_library(LIBTHRIFT libthrift.a /opt/thrift/lib)
if (NOT LIBTHRIFT)
    message (FATAL_ERROR "libthrift not found")
endif (NOT LIBTHRIFT) 

target_link_libraries (${PROJECT_NAME} pthread curl jsoncpp ${LIBTHRIFT})

install (TARGETS nvrserver DESTINATION bin)

# 构建一个 CPack 安装包
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
  "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${NVR_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${NVR_VERSION_MINOR}")
include (CPack)
