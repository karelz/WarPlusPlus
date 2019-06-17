@echo off

set cesta=d:\Data\Projekt\TheGame\TripleS
set adr=Compiler

c:\djgpp\bin\bison.exe -d -l -v %cesta%\Compiler\Bison\Syntax.y 

del %cesta%\%adr%\Src\Syntax.cpp
del %cesta%\%adr%\Src\Syntax.h

move %cesta%\%adr%\Bison\Syntax_tab.c %cesta%\%adr%\Src\Syntax.cpp
move %cesta%\%adr%\Bison\Syntax_tab.h %cesta%\%adr%\Src\Syntax.h

pause