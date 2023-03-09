@echo off
del .\bin\2DTree.exe

@echo.
@echo Trying to Compile App Resource File...
rc.exe App.rc

@echo Trying to Compile App...
cl.exe /c /EHsc src/2DTree.c src/LS/ls.c

@echo.
@echo Trying to Link App...
link.exe 2DTree.obj ls.obj App.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /MACHINE:x64

@echo Trying to move built App to .\bin\
mkdir bin
move 2DTree.exe .\bin\

@echo.
@echo Cleaning up...
del 2DTree.obj ls.obj
del App.res