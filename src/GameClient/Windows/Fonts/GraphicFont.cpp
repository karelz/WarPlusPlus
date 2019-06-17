// GraphicFont.cpp: implementation of the CGraphicFont class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GraphicFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAXFONTBUFFERWIDTH 320

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CGraphicFont, CFontObject)

CGraphicFont::CGraphicFont()
{
  m_dwHeight = 0;
  m_dwWidth = 0;
  m_dwMaxWidth = 0;
  m_dwCharsCount = 0;
  m_pCharHeader = NULL;
  m_pAlphaChannel = NULL;
  m_dwPrevColor = 0x0FFFFFFFF;
}

CGraphicFont::~CGraphicFont()
{

}

//////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CGraphicFont::AssertValid() const
{
  CFontObject::AssertValid();
}

void CGraphicFont::Dump(CDumpContext &dc) const
{
  CFontObject::Dump(dc);
}

#endif

BOOL CGraphicFont::Create(CArchiveFile File, DWORD dwColor)
{
  if(!CFontObject::Create(dwColor)) return FALSE;

  // if there is no direct draw - nothing to do
  // it's a fatal error
  ASSERT(g_pDirectDraw != NULL);
  ASSERT_VALID(g_pDirectDraw);

  // all fonts have the alpha channel
  m_CharSurface.SetAlphaChannel(TRUE);

  SFNTHeader h;
  DWORD i, j;

  File.Open();

  // read the header and copy the variables
  File.Read(&h, sizeof(h));
  m_dwWidth = h.m_dwWidth;
  m_dwHeight = h.m_dwHeight;
  m_dwCharsCount = h.m_dwCharsCount;
  m_dwCharSpace = h.m_dwCharSpace;
  m_dwLineSpace = h.m_dwLineSpace;

  // allocate the char headers structures
  m_pCharHeader = new SCharHeader[m_dwCharsCount];
  if(m_pCharHeader == NULL) AfxThrowMemoryException();

  // read the char headers structures
  File.Read(m_pCharHeader, sizeof(SCharHeader) * m_dwCharsCount);

  // find the most width character
  // we will use it to create our scratch surface
  // for drawing characters
  m_dwMaxWidth = 0;
  for(i = 0; i < m_dwCharsCount; i++){
    if(m_pCharHeader[i].m_dwWidth > m_dwMaxWidth)
      m_dwMaxWidth = m_pCharHeader[i].m_dwWidth;
  }

  // read the alpha channel from the file
  m_pAlphaChannel = new BYTE[m_dwHeight * m_dwWidth];
  if(m_pAlphaChannel == NULL) AfxThrowMemoryException();
  File.Read(m_pAlphaChannel, m_dwHeight * m_dwWidth);

  File.Close();

  // organize all characters in the font to our table
  // by ASCII code
  SCharHeader *pCharacter;
  for(i = 0; i < 256; i++){
    pCharacter = NULL;
    for(j = 0; j < m_dwCharsCount; j++){
      if(m_pCharHeader[j].m_wASCIICode == i){
        pCharacter = &m_pCharHeader[j];
        break;
      }
    }
    m_pCharacters[i] = pCharacter;
  }

  memset(m_pPrevColors, 0, sizeof(DWORD) * 256);
  {
    m_dwBufferLines = 0;
    int nXPos = 0, nPrevX = 0;
    int nChar;
    m_dwBufferWidth = 0;

    for(nChar = 0; nChar < (int)m_dwCharsCount; nChar++){
      nXPos += (m_pCharHeader[nChar].m_dwWidth + m_dwCharSpace);
      if(nXPos > MAXFONTBUFFERWIDTH){
        nPrevX = 0; nXPos = m_pCharHeader[nChar].m_dwWidth; m_dwBufferLines++;
      }
      m_pPositions[m_pCharHeader[nChar].m_wASCIICode].x = 
        nPrevX;
      m_pPositions[m_pCharHeader[nChar].m_wASCIICode].y =
        m_dwBufferLines * (m_dwHeight + m_dwLineSpace);
      if(nXPos > (int)m_dwBufferWidth) m_dwBufferWidth = nXPos;
      nPrevX = nXPos;
    }
		m_dwBufferLines++;
  }

  // set the sizes of our scratch surface
  // it will be used for drawing the characters
  m_CharSurface.SetWidth(m_dwBufferWidth);
  m_CharSurface.SetHeight(m_dwBufferLines * (m_dwHeight + m_dwLineSpace));
  m_CharSurface.SetAlphaChannel(TRUE);

  // create the surface
  if(!m_CharSurface.Create()) return FALSE;

  {
    LPBYTE pAlpha;
    DWORD dwOff;
    DWORD dwSOff;
    WORD ascii;
    int nChar;
    DWORD dwWidth;
    DWORD dwSource;
    DWORD dwY;

    pAlpha = m_CharSurface.GetAlphaChannel();

    for(nChar = 0; nChar < (int)m_dwCharsCount; nChar++){
      ascii = (WORD)m_pCharHeader[nChar].m_wASCIICode;
      dwWidth = m_pCharHeader[nChar].m_dwWidth;
      dwSource = m_pCharHeader[nChar].m_dwPosition;

      for(dwY = 0; dwY < m_dwHeight; dwY++){
        dwOff = (dwY + m_pPositions[ascii].y) * m_dwBufferWidth +
          m_pPositions[ascii].x;
        dwSOff = dwSource + dwY * m_dwWidth;
        memcpy(&pAlpha[dwOff], &m_pAlphaChannel[dwSOff], dwWidth);
      }
    }

    m_CharSurface.ReleaseAlphaChannel(TRUE);
  }

  return TRUE;
}

void CGraphicFont::Delete()
{
  m_CharSurface.Delete();

  // clear the char headers
  if(m_pCharHeader){
    delete m_pCharHeader;
    m_pCharHeader = NULL;
  }

  // clear the alpha channel
  if(m_pAlphaChannel){
    delete m_pAlphaChannel;
    m_pAlphaChannel = NULL;
  }

  CFontObject::Delete();
}

// returns the width of drawen character
int CGraphicFont::PaintCharacter(char Character, DWORD dwX, DWORD dwY,
                                 CDDrawSurface *pDestSurface, DWORD dwColor)
{
  // take the character from our table
  SCharHeader *pCharacter = m_pCharacters[(unsigned char)Character];

  // if it's null we don"t have to do anything
  // the character is not in the font
  if(pCharacter == NULL) return 0;

  CRect rcSource;
  rcSource.left = m_pPositions[(unsigned char)Character].x;
  rcSource.right = rcSource.left + pCharacter->m_dwWidth;
  rcSource.top = m_pPositions[(unsigned char)Character].y;
  rcSource.bottom = rcSource.top + m_dwHeight;

  // if the color of wanted character is different from the current color
  // we have to refill the surface with the new color
  if(m_pPrevColors[(unsigned char)Character] != dwColor){
    m_CharSurface.Fill(dwColor, &rcSource);
    m_pPrevColors[(unsigned char)Character] = dwColor;
  }

  // now copy the alpha channel of wanted character to our surface
/*  BYTE *pAlphaChannel;

  DWORD dwSource = (DWORD)(&m_pAlphaChannel[pCharacter->m_dwPosition]);
  DWORD dwWidth = pCharacter->m_dwWidth;
  DWORD dwDestDif = m_CharSurface.GetWidth() - dwWidth;
  DWORD dwSourceDif = m_dwWidth - dwWidth;
  DWORD dwDest = (DWORD)(pAlphaChannel);
  DWORD dwHeight = m_dwHeight;

  _asm{
    mov edx, dwHeight
    
    mov esi, dwSource
    mov edi, dwDest
  }
Loop1:;
  _asm{
    mov ecx, dwWidth
    mov ebx, ecx

    shr ecx, 2
    rep movsd
    mov ecx, ebx
    and ecx, 0x03
    rep movsb

    add edi, dwDestDif
    add esi, dwSourceDif

    dec edx
    jnz Loop1
  }

  // paste the character to destination surface
  // before - prepare the rect of the character
  CRect rectCharacter(0, 0, pCharacter->m_dwWidth, m_dwHeight);

  pDestSurface->Paste(dwX, dwY, &m_CharSurface, &rectCharacter);*/

  pDestSurface->Paste(dwX, dwY, &m_CharSurface, &rcSource);

  return pCharacter->m_dwWidth;
}

void CGraphicFont::PaintText(int nX, int nY, CString strText, CDDrawSurface *pDestSurface, DWORD dwColor)
{
  int nXPos, i, nLen;
  LPCSTR pStr;
  CSize size;

  nLen = strText.GetLength();
  if(nLen == 0) return;

  if(dwColor == 0x0FFFFFFFF) dwColor = m_dwColor;

  pStr = strText;

  // try if the text is not out of the clip rect of the
  // destination surface
  // if so we don't have to draw anything
  size = GetTextSize(strText);
  CRect rc(nX, nY, nX + size.cx, nY + size.cy);
//  rc.OffsetRect(pDestSurface->GetTransformation());
  CRect rcClip; pDestSurface->GetClipRect(&rcClip);
  rc.IntersectRect(&rc, &rcClip);
  if(rc.IsRectEmpty()) return;

  nXPos = nX;
  for(i = 0; i < nLen; i++){
    if(m_pCharacters[(unsigned char)pStr[i]] == NULL) continue;
    if((nXPos + (int)(m_pCharacters[(unsigned char)pStr[i]]->m_dwWidth) >= rcClip.left) && (nXPos < rcClip.right))
      nXPos += PaintCharacter(pStr[i], nXPos, nY, pDestSurface, dwColor);
    else
      nXPos += m_pCharacters[(unsigned char)pStr[i]]->m_dwWidth;
    nXPos += m_dwCharSpace;
  }
}

CSize CGraphicFont::GetCharSize(char Character)
{
  if(m_pCharacters[(unsigned char)Character] == NULL) return CSize(0, 0);
  return CSize(m_pCharacters[(unsigned char)Character]->m_dwWidth + m_dwCharSpace,
    m_dwHeight + m_dwLineSpace);
}

CSize CGraphicFont::GetTextSize(CString strText)
{
  DWORD i, cx, len;
  LPCSTR lpszText = strText;

  len = strText.GetLength();

  for(i = 0, cx = 0; i < len; i++){
    if(m_pCharacters[(unsigned char)(lpszText[i])] == NULL) continue;
    cx += m_pCharacters[(unsigned char)(lpszText[i])]->m_dwWidth + m_dwCharSpace;
  }

  return CSize(cx, m_dwHeight + m_dwLineSpace);
}
