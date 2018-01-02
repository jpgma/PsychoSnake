@echo off

mkdir build\
pushd build\

del "*.exe"
del "*.obj"

..\tools\ctime -begin renderer_build_timings.ctm

cl /nologo /c /Forenderer.obj /Zi /Fdrenderer.pdb ..\code\renderer.cpp
link /nologo /DEBUG /OUT:renderer.exe renderer.obj user32.lib gdi32.lib winmm.lib opengl32.lib

REM /SUBSYSTEM:windows /NODEFAULTLIB glew32.lib glu32.lib

..\tools\ctime -end renderer_build_timings.ctm
popd