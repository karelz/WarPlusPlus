@echo off

set cesta=D:\Data\Projekt\TheGame\TripleS
set adr=Compiler

c:\djgpp\bin\bison -d -l -v -p nsp %cesta%\%adr%\Bison\Nsp.y 

del %cesta%\%adr%\Src\Nsp.cpp
del %cesta%\%adr%\Src\Nsp.h

move %cesta%\%adr%\Bison\Nsp_tab.c %cesta%\%adr%\Src\Nsp.cpp
move %cesta%\%adr%\Bison\Nsp_tab.h %cesta%\%adr%\Src\Nsp.h

pause