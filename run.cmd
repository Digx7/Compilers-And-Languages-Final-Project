@ECHO OFF

ECHO Removing old files
del FinalProject.exe
del finalp2.txt
del a.cpp

ECHO Compiling files 
g++ -o FinalProject FinalProject.cpp

ECHO Running program
.\FinalProject.exe

ECHO Program has exited