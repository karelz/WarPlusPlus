// Implementation of Special_Blt() function used by GameClient
//
//    author:  Karby
//      date:  2000-05-04
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SpecialBlt.h"

//////////////////////////////////////////////////////////////////////
void __forceinline __cdecl Special_BltLine24(BYTE *pSrc, BYTE *pDest, DWORD dwBytes)
{
    __asm 
    {
        pushf               ; need to restore direction flag at the end
        cld                 ; clear direction flag to increase addresses during string operations
        mov ecx, dwBytes    ; copy the number of bytes
        mov esi, pSrc       ; source
        mov edi, pDest      ; destination
        shr ecx, 1          ; shift the number of bytes by 1
        jnc skipbyte      ; jump if the bit was not set
        movsb               ; the bit was set, copy 1 byte
      skipbyte:
        shr ecx, 1          ; shift the number of bytes by 1
        jnc skipword      ; jump if the bit was not set
        movsw               ; the bit was set, copy 1 word
      skipword:
        test ecx, ecx       ; test ecx against zero
        jz skipall        ; jump to end if ecx is zero
        rep movsd           ; copy the rest of scanline
      skipall:
        popf                ; restore direction flag
    }
}


void __forceinline __cdecl Special_BltLine24R(BYTE *pSrc, BYTE *pDest, DWORD dwBytes)
{
    __asm 
    {
        pushf               ; need to restore direction flag at the end
        std                 ; set direction flag to decrease addresses during string operations
        mov ecx, dwBytes    ; copy the number of bytes
        mov esi, pSrc       ; source
        mov edi, pDest      ; destination
        shr ecx, 1          ; shift the number of bytes by 1
        jnc skipbyte      ; jump if the bit was not set
        movsb               ; the bit was set, copy 1 byte
      skipbyte:
        dec esi
        dec edi
        shr ecx, 1          ; shift the number of bytes by 1
        jnc skipword      ; jump if the bit was not set
        movsw               ; the bit was set, copy 1 word
      skipword:
        sub esi, 2
        sub edi, 2
        test ecx, ecx       ; test ecx against zero
        jz skipall        ; jump to end if ecx is zero
        rep movsd           ; copy the rest of scanline
      skipall:
        popf                ; restore direction flag
    }
}


void __forceinline __cdecl Special_BltLine32(BYTE *pSrc, BYTE *pDest, DWORD dwDWords)
{
    __asm 
    {
        pushf               ; need to restore direction flag at the end
        cld                 ; clear direction flag to increase addresses during string operations
        mov ecx, dwDWords   ; copy the number of DWords
        mov esi, pSrc       ; source
        mov edi, pDest      ; destination
        rep movsd           ; copy the rest of scanline
        popf                ; restore direction flag
    }
}


void __forceinline __cdecl Special_BltLine32R(BYTE *pSrc, BYTE *pDest, DWORD dwDWords)
{
    __asm 
    {
        pushf               ; need to restore direction flag at the end
        std                 ; set direction flag to decrease addresses during string operations
        mov ecx, dwDWords   ; copy the number of DWords
        mov esi, pSrc       ; source
        mov edi, pDest      ; destination
        rep movsd           ; copy the rest of scanline
        popf                ; restore direction flag
    }
}


void Special_Blt(BYTE *pBuffer, DWORD dwScanLineSize, const CRect &rectSrc, const CRect &rectDest, BOOL b32bit)
{
    // ###
    ASSERT(rectSrc.Width() == rectDest.Width());
    ASSERT(rectSrc.Height() == rectDest.Height());
    ASSERT(dwScanLineSize >= (b32bit ? 4 : 3) * (DWORD)rectSrc.Width());
    ASSERT(::AfxIsValidAddress(pBuffer, ((rectSrc.bottom > rectDest.bottom) ? rectSrc.bottom : rectDest.bottom) * (b32bit ? 4 : 3), TRUE));

    DWORD dwNumberOfScanLines = rectSrc.bottom - rectSrc.top;
    DWORD dwScanLineWidth = rectSrc.Width();

    if (rectDest.top < rectSrc.top)
    {
        /////////////////////////////////////
        // copy left to right, top to bottom

        BYTE *pSrc = pBuffer + rectSrc.top * dwScanLineSize;
        BYTE *pDest = pBuffer + rectDest.top * dwScanLineSize;
        
        if (b32bit)
        {
            // 32bit True Color
            pSrc += rectSrc.left * 4;
            pDest += rectDest.left * 4;
            for (DWORD i = 0; i < dwNumberOfScanLines; i++)
            {
                Special_BltLine32(pSrc, pDest, dwScanLineWidth);
                pSrc += dwScanLineSize;
                pDest += dwScanLineSize;
            }
        }
        else
        {
            // 24bit True Color
            pSrc += rectSrc.left * 3;
            pDest += rectDest.left * 3;
            dwScanLineWidth *= 3;
            for (DWORD i = 0; i < dwNumberOfScanLines; i++)
            {
                Special_BltLine24(pSrc, pDest, dwScanLineWidth);
                pSrc += dwScanLineSize;
                pDest += dwScanLineSize;
            }
        }
    }
    else
    {
        if (rectDest.top > rectSrc.top)
        {
            /////////////////////////////////////
            // copy left to right, bottom to top

            BYTE *pSrc = pBuffer + (rectSrc.bottom - 1) * dwScanLineSize;
            BYTE *pDest = pBuffer + (rectDest.bottom - 1) * dwScanLineSize;
            
            if (b32bit)
            {
                // 32bit True Color
                pSrc += rectSrc.left * 4;
                pDest += rectDest.left * 4;
                for (DWORD i = 0; i < dwNumberOfScanLines; i++)
                {
                    Special_BltLine32(pSrc, pDest, dwScanLineWidth);
                    pSrc -= dwScanLineSize;
                    pDest -= dwScanLineSize;
                }
            }
            else
            {
                // 24bit True Color
                pSrc += rectSrc.left * 3;
                pDest += rectDest.left * 3;
                dwScanLineWidth *= 3;
                for (DWORD i = 0; i < dwNumberOfScanLines; i++)
                {
                    Special_BltLine24(pSrc, pDest, dwScanLineWidth);
                    pSrc -= dwScanLineSize;
                    pDest -= dwScanLineSize;
                }
            }
        }
        else
        {
            // both rectangles have the same Y coordinate
            if (rectDest.left < rectSrc.left)
            {
                /////////////////////////////////////
                // copy left to right, top to bottom

                BYTE *pSrc = pBuffer + rectSrc.top * dwScanLineSize;
                BYTE *pDest = pBuffer + rectDest.top * dwScanLineSize;
                
                if (b32bit)
                {
                    // 32bit True Color
                    pSrc += rectSrc.left * 4;
                    pDest += rectDest.left * 4;
                    for (DWORD i = 0; i < dwNumberOfScanLines; i++)
                    {
                        Special_BltLine32(pSrc, pDest, dwScanLineWidth);
                        pSrc += dwScanLineSize;
                        pDest += dwScanLineSize;
                    }
                }
                else
                {
                    // 24bit True Color
                    pSrc += rectSrc.left * 3;
                    pDest += rectDest.left * 3;
                    dwScanLineWidth *= 3;
                    for (DWORD i = 0; i < dwNumberOfScanLines; i++)
                    {
                        Special_BltLine24(pSrc, pDest, dwScanLineWidth);
                        pSrc += dwScanLineSize;
                        pDest += dwScanLineSize;
                    }
                }
            }
            else
            {
                if (rectDest.left > rectSrc.left)
                {
                    /////////////////////////////////////
                    // copy right to left, top to bottom

                    BYTE *pSrc = pBuffer + rectSrc.top * dwScanLineSize;
                    BYTE *pDest = pBuffer + rectDest.top * dwScanLineSize;
                    
                    if (b32bit)
                    {
                        // 32bit True Color
                        pSrc += (rectSrc.right - 1) * 4;
                        pDest += (rectDest.right - 1) * 4;
                        for (DWORD i = 0; i < dwNumberOfScanLines; i++)
                        {
                            Special_BltLine32R(pSrc, pDest, dwScanLineWidth);
                            pSrc += dwScanLineSize;
                            pDest += dwScanLineSize;
                        }
                    }
                    else
                    {
                        // 24bit True Color
                        pSrc += (rectSrc.right - 1) * 3 + 2;
                        pDest += (rectDest.right - 1) * 3 + 2;
                        dwScanLineWidth *= 3;
                        for (DWORD i = 0; i < dwNumberOfScanLines; i++)
                        {
                            Special_BltLine24R(pSrc, pDest, dwScanLineWidth);
                            pSrc += dwScanLineSize;
                            pDest += dwScanLineSize;
                        }
                    }
                }
                else
                {
                    // DO NOTHING!
                    return;
                }
            }
        }
    }
}
