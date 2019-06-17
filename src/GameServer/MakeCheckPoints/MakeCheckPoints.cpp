
#include "StdAfx.h"
#include "MakeCheckPoints.h"
#include <math.h>

#define ARRAY_SIZE 100
#define DISTANCE_ORTH 100
#define DISTANCE_DIAG 141

DWORD aWaveResult[ARRAY_SIZE][ARRAY_SIZE];
BYTE aTemporary[ARRAY_SIZE][ARRAY_SIZE];

// dopredna deklarace
BYTE AvailableDirections(CPointDW pointStart);

/////////////////////////////////////////////////////////////////////
// Exportovana funkce, ktera vytvori seznam checkpointu

void MakeCheckPoints(CPointDW pointStart, CPointDW pointEnd)
{
    // vynulovani pole mezivysledku
    memset(aTemporary, 0, ARRAY_SIZE * ARRAY_SIZE);

    // inicializace posledniho checkpointu
    CPointDW pointLastCheckPoint = pointEnd;

    // TODO: do listu pridat checkpoint
    TRACE("Checkpoint %u, %u\n", pointLastCheckPoint.x, pointLastCheckPoint.y);

    while (pointLastCheckPoint != pointStart)
    {
        // inicializace smeru
        BYTE btActualDirection = AvailableDirections(pointEnd);
            
        // je to jednoduchy smer?
        if ((btActualDirection & (btActualDirection - 1)) == 0)
        {
            // je to jednoduchy smer

            // posun o jedno policko v tomto smeru
            DWORD dwXDelta = 0;
            DWORD dwYDelta = 0;

            // smer vlevo
            if (btActualDirection & 0xC1)
                dwXDelta = -1;
            // smer vpravo
            if (btActualDirection & 0x1C)
                dwXDelta = +1;
            // smer nahoru
            if (btActualDirection & 0x07)
                dwYDelta = -1;
            // smer dolu
            if (btActualDirection & 0x70)
                dwYDelta = +1;

            // aktualni pozice
            CPointDW pointActualPos = pointLastCheckPoint;

            for ( ; ; )
            {
                // posunti na dalsi policko v tomto smeru
                pointActualPos.x += dwXDelta;
                pointActualPos.y += dwYDelta;

                if ((pointActualPos == pointStart) ||
                    (AvailableDirections(pointActualPos) != btActualDirection))
                {
                    // vytvorime checkpoint
                    pointLastCheckPoint = pointActualPos;
                    TRACE("Checkpoint %u, %u\n", pointLastCheckPoint.x, pointLastCheckPoint.y);

                    // skoncime cyklus
                    break;
                }
            }
        }
        else
        {
            // je to dvojity smer; budeme delat "Pascaluv trojuhelnik"

            // nastaveni 1 do koncoveho bodu
            aTemporary[pointLastCheckPoint.y][pointLastCheckPoint.x] = 1;
            
            // musime vytvorit 2 vetve -- pro svisly nebo vodorovny pripad
            if ((btActualDirection & 0x22) != 0)
            {
                // svisly pripad, budeme postupovat po radcich
                
                // aktualne pocitana radka
                DWORD dwActualRow = pointLastCheckPoint.y;
                // predchozi radka
                DWORD dwLastRow;
                // nahoru nebo dolu?
                // DWORD je to kvuli pretypovani; trochu prasarna, no a co...
                DWORD dwRowDelta = ((btActualDirection & 0x02) != 0) ? -1 : +1;
                // promenne pro minimum a maximum na radku
                DWORD dwColumnMin = pointLastCheckPoint.x;
                DWORD dwColumnMax = pointLastCheckPoint.x;
                // promenne pro minimum a maximum na predposlednim zpracovanem radku
                DWORD dwColumnMinLast;
                DWORD dwColumnMaxLast;
                // zmena minima a maxima pri postupu na dalsi radek
                // DWORDy to jsou kvuli pretypovani; trochu prasarna, no a co...
                DWORD dwColumnMinDelta = ((btActualDirection & 0x41) != 0) ? -1 : 0;
                DWORD dwColumnMaxDelta = ((btActualDirection & 0x14) != 0) ? +1 : 0;
                // policko sikmo od aktualne pocitane pozice ma x-ovou slozku 
                // souradnice posunutou o nasledujici hodnotu
                DWORD dwColumnShift = ((btActualDirection & 0x41) != 0) ? -1 : +1;

                // maximum na predchozim radku
                DWORD dwMaximumValueColumn = pointLastCheckPoint.x;

                // posledni kandidat na checkpoint
                CPointDW pointCheckPointCandidate = pointLastCheckPoint;

                for ( ; ; )  // nekonecny cyklus
                {
                    // provedeme iteracni krok
                    dwLastRow = dwActualRow;
                    dwActualRow += dwRowDelta;
                    dwColumnMinLast = dwColumnMin;
                    dwColumnMaxLast = dwColumnMax;
                    dwColumnMin += dwColumnMinDelta;
                    dwColumnMax += dwColumnMaxDelta;

                    // spocitame hodnoty na novem radku
                    {
                        for (DWORD i = dwColumnMin; i <= dwColumnMax; i++)
                        {
                            // kdyz je dane policko na optimalni ceste, tak
                            // spocitame soucet hodnot na predchozim radku
                            if ((aWaveResult[dwActualRow][i] + DISTANCE_ORTH == aWaveResult[dwLastRow][i]) ||
                                (aWaveResult[dwActualRow][i] + DISTANCE_DIAG == aWaveResult[dwLastRow][i - dwColumnShift]))
                                aTemporary[dwActualRow][i] = aTemporary[dwLastRow][i] + aTemporary[dwLastRow][i - dwColumnShift];
                        }
                    }

                    // najdeme nove maximum
                    if (aTemporary[dwActualRow][dwMaximumValueColumn] < aTemporary[dwActualRow][dwMaximumValueColumn + dwColumnShift])
                    {
                        // hodnota svisle je mensi, pujdeme sikmo
                        dwMaximumValueColumn += dwColumnShift;
                    }

                    // zjistime nove dwColumnMin
                    dwColumnMin = dwMaximumValueColumn;
                    if (aTemporary[dwActualRow][dwColumnMin - 1] != 0)
                        dwColumnMin--;
                    if (aTemporary[dwActualRow][dwColumnMin - 1] != 0)
                        dwColumnMin--;
                    // zjistime nove dwColumnMAx
                    dwColumnMax = dwMaximumValueColumn;
                    if (aTemporary[dwActualRow][dwColumnMax + 1] != 0)
                        dwColumnMin++;
                    if (aTemporary[dwActualRow][dwColumnMin + 1] != 0)
                        dwColumnMin++;

                    // mame spocitane hodnoty na dalsim radku, mame nalezene maximum;
                    // ted zkusime z maxima udelat caru do posledniho znameho checkpointu
                    // (DDA algoritmus -- stejny jako v Move skille)
                    BOOL bPathFree = TRUE;
                    {
                        double dblX = (double)dwMaximumValueColumn + 0.5;
                        double dblXDelta = (double)(pointLastCheckPoint.x - dwMaximumValueColumn) / (double)abs(pointLastCheckPoint.y - dwActualRow);
                        
                        // prvni krok ("pixel") je samozrejmy, takze ho provedeme bez testu
                        dblX += dblXDelta;
                        
                        for (DWORD i = dwActualRow - dwRowDelta; i != pointLastCheckPoint.y; i - dwRowDelta, dblX += dblXDelta)
                        {
                            if (aTemporary[i][(DWORD)floor(dblX)] == 0)
                            {
                                bPathFree = FALSE;
                                break;
                            }
                        }
                    }
                        
                    // kdyz cesta neni pruchozi
                    if (!bPathFree)
                    {
                        // cyklus nedobehl cely; z posledniho kandidata na checkpoint
                        // opravdu vytvorime checkpoint, zacneme cely vypocet znova
                        
                        // TODO: do listu pridat checkpoint
                        TRACE("Checkpoint %u, %u\n", pointCheckPointCandidate.x, pointCheckPointCandidate.y);
                        
                        // nemelo by se stat, ze pridavany checkpoint je stejny, jako posledni,
                        // ktery tam uz je
                        ASSERT(pointCheckPointCandidate != pointLastCheckPoint);
                        
                        // nastavime posledni checkpoint                                
                        pointLastCheckPoint = pointCheckPointCandidate;
                        
                        // inicializujeme dalsi vypocet
                        dwActualRow = pointLastCheckPoint.y;
                        
                        // vynulujeme posledni radek mezivysledku
                        {
                            for (DWORD i = dwColumnMinLast; i <= dwColumnMaxLast; i++)
                                aTemporary[dwActualRow][i] = 0;
                        }
                        
                        // na misto prave vlozeneho checkpointu napiseme jednicku
                        aTemporary[pointLastCheckPoint.y][pointLastCheckPoint.x] = 1;
                        
                        // nastavime sirku radku na jedna
                        dwColumnMin = pointLastCheckPoint.x;
                        dwColumnMax = pointLastCheckPoint.x;

                        // pokracujeme dalsim radkem vypoctu
                        continue;
                    }

                    // kdyz jsme se dostali do uzkeho mista, tj. existuje pouze
                    // jeden mapcell, pres ktery se da jit, tak do nej dame checkpoint
                    if (dwColumnMin == dwColumnMax)
                    {
                        pointLastCheckPoint.y = dwActualRow;
                        pointLastCheckPoint.x = dwMaximumValueColumn;

                        // TODO: do listu pridat checkpoint
                        TRACE("Checkpoint %u, %u\n", pointLastCheckPoint.x, pointLastCheckPoint.y);

                        // vypocet dal nepokracuje
                        break;
                    }
                    
                    // kdyz hodnota v maximu prelezla 127 (neda se dal scitat do bajtu)
                    if (aTemporary[dwActualRow][dwMaximumValueColumn] > 127)
                    {
                        pointLastCheckPoint.y = dwActualRow;
                        pointLastCheckPoint.x = dwMaximumValueColumn;

                        // TODO: do listu pridat checkpoint
                        TRACE("Checkpoint %u, %u\n", pointLastCheckPoint.x, pointLastCheckPoint.y);

                        // vynulujeme posledni radek vypoctu
                        {
                            for (DWORD i = dwColumnMin; i <= dwColumnMax; i++)
                                aTemporary[dwActualRow][i] = 0;
                        }

                        // do posledniho checkpointu nastavime 1
                        aTemporary[pointLastCheckPoint.y][pointLastCheckPoint.x];

                        // nastavime sirku radku na jedna
                        dwColumnMin = pointLastCheckPoint.x;
                        dwColumnMax = pointLastCheckPoint.x;

                        // pokracujeme dalsim radkem vypoctu
                        continue;
                    }
                }
            }
            else
            {
                // vodorovny pripad

                // TODO: oprasknout svisly pripad a prohodit role x a y
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////
// V danem bode zjisti smery, kterymi lze jit

/*
     vysledek podle bitu v bajtu:
       nejnizsi bit  1  vlevo nahoru
                     2  nahoru
                     3  vpravo nahoru
                     4  vpravo
                     5  vpravo dolu
                     6  dolu
                     7  vlevo dolu
       nejvyssi bit  8  vlevo

      3       6      12      24      48      96     192     129
       xx.     .xx     ..x     ...     ...     ...     ...     x..
       .o.     .o.     .ox     .ox     .o.     .o.     xo.     xo.
       ...     ...     ...     ..x     .xx     xx.     x..     ...

      1       2       4       8      16      32      64     128
       x..     .x.     ..x     ...     ...     ...     ...     ...
       .o.     .o.     .o.     .ox     .o.     .o.     .o.     xo.
       ...     ...     ...     ...     ..x     .x.     x..     ...

   pokud lze jit do vice smeru, nez do vyse uvedenych, vybere se nektery z nich
*/

BYTE AvailableDirections(CPointDW pointStart)
{
    BYTE btResult = 0;
    DWORD dwCurrent = aWaveResult[pointStart.y][pointStart.x];
    
    // smer vlevo nahoru
    if (aWaveResult[pointStart.y - 1][pointStart.x - 1] <= dwCurrent - DISTANCE_DIAG)
        btResult |= 0x01;

    // smer nahoru
    if (aWaveResult[pointStart.y - 1][pointStart.x] <= dwCurrent - DISTANCE_ORTH)
        btResult |= 0x02;

    // smer vpravo nahoru
    if (aWaveResult[pointStart.y - 1][pointStart.x + 1] <= dwCurrent - DISTANCE_DIAG)
        btResult |= 0x04;

    // smer vpravo
    if (aWaveResult[pointStart.y][pointStart.x + 1] <= dwCurrent - DISTANCE_ORTH)
        btResult |= 0x08;

    // smer vpravo dolu
    if (aWaveResult[pointStart.y + 1][pointStart.x + 1] <= dwCurrent - DISTANCE_DIAG)
        btResult |= 0x10;

    // smer dolu
    if (aWaveResult[pointStart.y + 1][pointStart.x] <= dwCurrent - DISTANCE_ORTH)
        btResult |= 0x20;

    // smer vlevo dolu
    if (aWaveResult[pointStart.y + 1][pointStart.x - 1] <= dwCurrent - DISTANCE_DIAG)
        btResult |= 0x40;

    // smer vlevo
    if (aWaveResult[pointStart.y][pointStart.x - 1] <= dwCurrent - DISTANCE_ORTH)
        btResult |= 0x80;

/* ZBYTECNE
    switch (btResult)
    {
    // dva smery jsou preferovane
    case 0x03:
    case 0x06:
    case 0x0C:
    case 0x18:
    case 0x30:
    case 0x60:
    case 0xC0:
    case 0x81:
    // jeden smer je taky dobre
    case 0x01:
    case 0x02:
    case 0x04:
    case 0x08:
    case 0x10:
    case 0x20:
    case 0x40:
    case 0x80:
        return btResult;
    }
*/

    // smer vlevo nahoru a nahoru
    if (btResult & 0x03 == 0x03)
        return 0x03;

    // smer nahoru a vpravo nahoru
    if (btResult & 0x06 == 0x06)
        return 0x06;

    // smer vpravo nahoru a vpravo
    if (btResult & 0x0C == 0x0C)
        return 0x0C;
    
    // smer vpravo a vpravo dolu
    if (btResult & 0x18 == 0x18)
        return 0x18;

    // smer vpravo dolu a dolu
    if (btResult & 0x30 == 0x30)
        return 0x30;

    // smer dolu a vlevo dolu
    if (btResult & 0x60 == 0x60)
        return 0x60;

    // smer vlevo dolu a vlevo
    if (btResult & 0xC0 == 0xC0)
        return 0xC0;

    // smer vlevo a vlevo nahoru
    if (btResult & 0x81 == 0x81)
        return 0x81;

    // ----------------------------
    
    // smer vlevo nahoru
    if (btResult & 0x01 == 0x01)
        return 0x01;

    // smer nahoru
    if (btResult & 0x02 == 0x02)
        return 0x02;

    // smer vpravo nahoru
    if (btResult & 0x04 == 0x04)
        return 0x04;

    // smer vpravo
    if (btResult & 0x08 == 0x08)
        return 0x08;

    // smer vpravo dolu
    if (btResult & 0x10 == 0x10)
        return 0x10;

    // smer dolu
    if (btResult & 0x20 == 0x20)
        return 0x20;

    // smer vlevo dolu
    if (btResult & 0x40 == 0x40)
        return 0x40;

    // smer vlevo
    if (btResult & 0x80 == 0x80)
        return 0x80;

    // coze??!?  asi jsou spatne vysledky vlny
    ASSERT(FALSE);
    return 0;
}


/////////////////////////////////////////////////////////////////////
// Zjisti pruchodnost usecky

BOOL bArray[100][100];

BOOL IsPathFree(CPointDW pointStart, CPointDW pointEnd)
{
    // neni to ten samy bod?
    if (pointStart == pointEnd)
        return TRUE;
    
    // dva pripady
    if (abs((int)pointStart.x - (int)pointEnd.x) > abs((int)pointStart.y - (int)pointEnd.y))
    {
        // "vodorovna" cara
        double dblY = (double)pointStart.y + 0.5;
        double dblYDelta = (double)(pointEnd.y - pointStart.y) / (double)abs(pointEnd.x - pointStart.x);

        DWORD dwX = pointStart.x;
        DWORD dwXDelta = (pointEnd.x > pointStart.x) ? +1 : -1;

        while (dwX != pointEnd.x)
        {
            if (!bArray[(DWORD)floor(dblY)][dwX])
                return FALSE;

            dwX += dwXDelta;
            dblY += dblYDelta;
        }

        // posledni (cilovy) mapcell
        if (!bArray[(DWORD)floor(dblY)][dwX])
            return FALSE;

        return TRUE;
    }
    else
    {
        // "svisla" cara
        double dblX = (double)pointStart.x + 0.5;
        double dblXDelta = (double)(pointEnd.x - pointStart.x) / (double)abs(pointEnd.y - pointStart.y);
        
        DWORD dwY = pointStart.y;
        DWORD dwYDelta = (pointEnd.y > pointStart.y) ? +1 : -1;
        
        while (dwY != pointEnd.y)
        {
            if (!bArray[dwY][(DWORD)floor(dblX)])
                return FALSE;

            dblX += dblXDelta;
            dwY += dwYDelta;
        }

        // posledni (cilovy) mapcell
        if (!bArray[dwY][(DWORD)floor(dblX)])
            return FALSE;

        return TRUE;
    }
}
