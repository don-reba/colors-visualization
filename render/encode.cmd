@echo off
setlocal

set PATH=%PATH%;"C:\Program Files\ffmpeg\bin"

set in=animation\%%d.png
set out=animation.mp4

:: 5 mbps for 720p, 8 mbps for 1080p, 16 mbps for 2k, 35-45 mbps for 4k
:: presets: ultrafast,superfast, veryfast, faster, fast, medium, slow, slower, veryslow, placebo
ffmpeg -y -i %in% -c:v libx264 -preset slow -b:v 5000k -r 30 -crf 18 -pix_fmt yuv420p -movflags faststart %out%
