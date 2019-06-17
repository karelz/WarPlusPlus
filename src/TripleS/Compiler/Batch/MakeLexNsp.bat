@echo off
set cesta=D:\Data\Projekt\TheGame\TripleS
set adr=Compiler

c:\djgpp\bin\flex -L %cesta%\%adr%\Flex\LexNsp.y

cd %cesta%\%adr%\Src

del LexNsp.cpp

rem %cesta%\%adr%\Flex\RemoveUnistd.exe <%cesta%\%adr%\Batch\LexNsp.cc >LexNsp.cpp
rem del %cesta%\%adr%\Batch\LexNsp.cc

move %cesta%\%adr%\Batch\LexNsp.cc LexNsp.cpp

pause