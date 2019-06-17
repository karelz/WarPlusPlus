@echo off
set cesta=D:\Data\Projekt\TheGame\TripleS
set adr=Compiler

c:\djgpp\bin\flex -L %cesta%\%adr%\Flex\Lexyy1st.y

cd %cesta%\%adr%\Src

del Lexyy1st.cpp
rem %cesta%\%adr%\Flex\RemoveUnistd.exe <%cesta%\%adr%\Batch\Lexyy1st.cc >Lexyy1st.cpp
rem del %cesta%\%adr%\Batch\Lexyy1st.cc

move %cesta%\%adr%\Batch\Lexyy1st.cc Lexyy1st.cpp

pause