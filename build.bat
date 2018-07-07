@echo off

mkdir build\
pushd build\

del "*.exe"
del "*.obj"

..\tools\ctime -begin psychosnake_build_timings.ctm

cl /nologo /c /Fopsychosnake.obj /Zi /Fdpsychosnake.pdb ..\code\main_win32.cpp
link /nologo /DEBUG /OUT:psychosnake.exe psychosnake.obj user32.lib gdi32.lib winmm.lib opengl32.lib 

REM /SUBSYSTEM:windows /NODEFAULTLIB glew32.lib glu32.lib opengl32.lib

..\tools\ctime -end psychosnake_build_timings.ctm
popd