@echo off
setlocal

set PATH=%PATH%;"C:\Program Files\ffmpeg\bin"

set in=animation\%%d.png
set out=animation.mkv

ffmpeg -y -i %in% -c:v libx264 -b:v 5000k -r 30 -crf 18 -pix_fmt yuv420p %out%
