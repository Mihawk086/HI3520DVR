cmake_minimum_required(VERSION 3.8)
project(hisi3520)

INCLUDE_DIRECTORIES(
        include
        include/live555
        include/hi_include
        include/json
        include/osd
        ./recordsdk
        ./ringfifo
        ./sqlite
        ./time
        ./util
        ./venc
        ./rtsp
        ./rtsp/xop
        ./ZLToolKit
        ./osd
        ./lib/ffmpeg/include
        ./include/libmpeg
        ./RecordSDK
)

#source directory
#AUX_SOURCE_DIRECTORY(src DIR_SRCS)

set(CMAKE_CXX_STANDARD 11)

#hisiv300
#set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/bin/arm-hisiv300-linux-g++)
#set(CMAKE_C_COMPILER /opt/hisi-linux/x86-arm/arm-hisiv300-linux/target/bin/arm-hisiv300-linux-gcc)
#set(CMAKE_CXX_COMPILER /opt/hisi-linux/x86-arm/arm-hisiv400-linux/target/bin/arm-hisiv400-linux-g++)
#set(CMAKE_C_COMPILER /opt/hisi-linux/x86-arm/arm-hisiv400-linux/target/bin/arm-hisiv400-linux-gcc)

LINK_DIRECTORIES(lib)
LINK_DIRECTORIES(lib/hisi)
#LINK_DIRECTORIES(lib/hisiv400lib)
LINK_DIRECTORIES(lib/live555)
LINK_DIRECTORIES(lib/osd)
LINK_DIRECTORIES(lib/mp4v2)
LINK_DIRECTORIES(lib/sqlite3)

#LINK_DIRECTORIES(lib/ffmpeg)
LINK_DIRECTORIES(lib/ffmpeg)
LINK_LIBRARIES(-lavformat -lavcodec -lavutil
-lpostproc  -lswresample -lswscale)


#链接库
LINK_LIBRARIES(-lsqlite3)
LINK_LIBRARIES(-lmp4v2)
LINK_LIBRARIES(-lmpi -lhdmi -lupvqe -ldnvqe -live -ljpeg -ljpeg6b -lmd -ltde)
LINK_LIBRARIES(-lVoiceEngine)
LINK_LIBRARIES(-lSDL_ttf -lSDL -lfreetype)
#live555库
#LINK_LIBRARIES(-lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment)
LINK_LIBRARIES(-lrt -pthread -lpthread -ldl -lm)

add_definitions("-Wall -g")


aux_source_directory(./hisi/common HISI_SRC)
aux_source_directory(./osd OSD_SRC)
aux_source_directory(./recordsdk RECORDSDK_SRC)
aux_source_directory(./ringfifo RINGFIFO_SRC)
aux_source_directory(./rtsp RTSP_SRC1)
aux_source_directory(./rtsp/xop RTSP_SRC2)
aux_source_directory(./sqlite SQLITE_SRC)
aux_source_directory(./time TIME_SRC)
aux_source_directory(./util UTIL_SRC)
aux_source_directory(./venc VENC_SRC)
aux_source_directory(./recordsdk/libmpeg/source MPEG_SRC1)
aux_source_directory(./recordsdk/libmpeg/test MPEG_SRC2)
aux_source_directory(./RecordSDK RECORDSDK_SRC2)


set(ToolKit_Root ./ZLToolKit)
file(GLOB ToolKit_src_list ${ToolKit_Root}/*/*.cpp ${ToolKit_Root}/*/*.h ${ToolKit_Root}/*/*.c)

#file(GLOB_RECURSE SRC_LIST /src*.cpp /src*.h /src*.c)

add_executable(hisi3520 main.cpp ${HISI_SRC} ${OSD_SRC} ${RECORDSDK_SRC} ${RINGFIFO_SRC} ${RTSP_SRC1}
        ${RTSP_SRC2} ${SQLITE_SRC} ${TIME_SRC} ${UTIL_SRC} ${VENC_SRC} ${ToolKit_src_list} ${MPEG_SRC1}
        ${MPEG_SRC2} ${RECORDSDK_SRC2})

add_executable(test1 test/test.cpp ${HISI_SRC} ${OSD_SRC} ${RECORDSDK_SRC} ${RINGFIFO_SRC} ${RTSP_SRC1}
        ${RTSP_SRC2} ${SQLITE_SRC} ${TIME_SRC} ${UTIL_SRC} ${VENC_SRC} ${ToolKit_src_list} ${MPEG_SRC1}
        ${MPEG_SRC2} ${RECORDSDK_SRC2})

add_executable(test2 test/test2.cpp ${HISI_SRC} ${OSD_SRC} ${RECORDSDK_SRC} ${RINGFIFO_SRC} ${RTSP_SRC1}
        ${RTSP_SRC2} ${SQLITE_SRC} ${TIME_SRC} ${UTIL_SRC} ${VENC_SRC} ${ToolKit_src_list} ${MPEG_SRC1}
        ${MPEG_SRC2} ${RECORDSDK_SRC2})


