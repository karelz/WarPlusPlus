@echo off
set cesta=D:\Data\Projekt\TheGame\TripleS
set adr=Compiler

c:\djgpp\bin\flex -L %cesta%\%adr%\Flex\Lexyy2nd.y

cd %cesta%\%adr%\Src

del Lexyy2nd.cpp

rem %cesta%\%adr%\Flex\RemoveUnistd.exe <%cesta%\%adr%\Batch\Lexyy2nd.cc >Lexyy2nd.cpp
rem del %cesta%\%adr%\Batch\Lexyy2nd.cc

move %cesta%\%adr%\Batch\Lexyy2nd.cc Lexyy2nd.cpp

pause