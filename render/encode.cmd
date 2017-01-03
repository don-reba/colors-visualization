@echo off
setlocal
setlocal EnableDelayedExpansion

set dir=animation

set PATH=%PATH%;"C:\Program Files\ffmpeg\bin"

rem get the first file from the target directory
for %%f in (%dir%\*) do set file=%%f & goto found_file
:found_file

if exist "%file%" (
	for /f "tokens=*" %%e in ('ffprobe -v quiet -of csv^=p^=0 -show_entries stream^=height %file%') do set height=%%e

	rem                 720p 1080p 2k    4k
	rem  youtube 30fps: 5    8     16    35-45
	rem  youtube 60fps: 7.5  12    24    53-68
	rem  vimeo:         5-10 10-20 20-30 30-60

	if !height!==2160 set bitrate=60m
	if !height!==1440 set bitrate=30m
	if !height!==1080 set bitrate=20m
	if !height!==720  set bitrate=10m
	if !height!==480  set bitrate=5m
	if !height!==360  set bitrate=5m

	ffmpeg                       ^
		-r 60                ^
		-i "%dir%\%%d.png"   ^
		-hide_banner         ^
		-movflags +faststart ^
		-c:v libx264         ^
		-profile:v high      ^
		-level 4.2           ^
		-crf 18              ^
		-coder 1             ^
		-b:v !bitrate!       ^
		-bf 2                ^
		-g 30                ^
		-pix_fmt yuv420p     ^
		-y "%dir%.mp4"
) else (
	echo nothing to encode
)
