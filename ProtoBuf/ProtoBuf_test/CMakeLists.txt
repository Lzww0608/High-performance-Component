cmake_minimum_required(VERSION 2.8)

project (pb)

ADD_DEFINITIONS(-O2 -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++11)

set(pb_srcs person.pb.cc)

# 指定protobuf链接库路径
link_directories(/usr/local/lib)
add_executable(person person.cpp ${pb_srcs})
TARGET_LINK_LIBRARIES(person protobuf pthread)





