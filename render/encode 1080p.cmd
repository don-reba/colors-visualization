@echo off
setlocal

set PATH=%PATH%;"C:\Program Files\ffmpeg\bin"

set in="animation (1080p)\%%d.png"
set out=animation.mp4

::                720p 1080p 2k    4k
:: youtube 30fps: 5    8     16    35-45
:: youtube 60fps: 7.5  12    24    53-68
:: vimeo:         5-10 10-20 20-30 30-60
ffmpeg                       ^
        -r 60                ^
        -i %in%              ^
        -hide_banner         ^
        -movflags +faststart ^
        -c:v libx264         ^
        -profile:v high      ^
        -level 4.2           ^
        -crf 18              ^
        -coder 1             ^
        -b:v 15m             ^
        -bf 2                ^
        -g 30                ^
        -pix_fmt yuv420p     ^
        -y %out%
