#!/usr/bin/env bash

# removing the old files
del FinalProject.exe
del finalp2.txt
del a.cpp

# compiling files
g++ -o FinalProject FinalProject.cpp

# running program
.\FinalProject.exe
