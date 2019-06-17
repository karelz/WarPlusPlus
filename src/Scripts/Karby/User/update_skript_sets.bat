@echo off
cls

REM Jak opatchovat mapu - dodat ji novy skriptset
REM ---------------------------------------------

REM Cesta k mape, kterou chceme opatchovat
SET THEMAPPATH="E:\FTP\Projekt-root\projekt\Maps\Karbyho\Graf trava\Karbyho - Graf trava.TheMap"

REM Jmeno skriptsetu
SET SCRIPTSETNAME=User.ScriptSet

REM Cesta ke skriptsetu
SET SCRIPTSETPATH="E:\Projekt\Scripts\Karby"

REM Cesta ke gameclientovi
REM SET GAMECLIENTRUNTIMEPATH="E:\Projekt\GameClient\gameclientruntimedata\Maps"

REM -----
REM 1) Smazat stary skriptset z mapy
echo -----
echo Mazani stareho scriptsetu z mapy...
warman -r D %THEMAPPATH% ScriptSets\%SCRIPTSETNAME%

REM -----
REM 2) Rozpakovat novy skriptset
echo -----
echo Rozpakovani noveho scriptsetu...
warman -r X %SCRIPTSETPATH%\%SCRIPTSETNAME%

REM -----
REM 3) Presunout tyto soubory do struktury adresaru, ktera odpovida presne
REM    tomu, co je v .TheMap
echo -----
echo Presunuti noveho scriptsetu do prislusneho adresare...
mkdir ScriptSets
mkdir ScriptSets\%SCRIPTSETNAME%
move Source ScriptSets\%SCRIPTSETNAME%
move ByteCode ScriptSets\%SCRIPTSETNAME%

REM -----
REM 4) Pridat skriptset do TheMap
echo -----
echo Pridani noveho scriptsetu do mapy...
warman -r A %THEMAPPATH% ScriptSet*

REM -----
REM 5) Smazat rozbaleny skriptset
echo -----
echo Smazani rozbaleneho scriptsetu...
rmdir /s /q ScriptSets

REM -----
REM 6) Aby to game client nemusel furt kopirovat pri spusteni,
REM    soupnout vyslednou mapu k nemu
REM echo -----
REM echo Kopirovani nove mapy klientovi...
REM copy %THEMAPPATH% %GAMECLIENTRUNTIMEPATH%

echo -----
echo Hotovo...

