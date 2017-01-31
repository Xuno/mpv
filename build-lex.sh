#!/bin/sh

export devtools=/home/lex/develop-tools
export PKG_CONFIG_PATH=${devtools}/opencv/opencv-3.1.0/build/lib/pkgconfig:${devtools}/ffmpeg/ffmpeg_sources/ffmpeg-build/lib/pkgconfig:${PKG_CONFIG_PATH}
export PATH=${devtools}/opencv/opencv-3.1.0/build/lib:${devtools}/ffmpeg/ffmpeg_sources/ffmpeg-build/lib:${PATH}
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${devtools}/opencv/opencv-3.1.0/build/lib
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${devtools}/ffmpeg/ffmpeg_sources/ffmpeg-build/lib

export |grep PATH

#./waf clean
./waf configure  --enable-libmpv-shared  --prefix=${devtools}/mpv-player/mpv-build/build-XunoMpv-20170131 #--enable-gpl3
./waf install