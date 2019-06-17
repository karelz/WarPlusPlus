// FNTMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FNTMaker.h"
#include "FNTMakerDlg.h"

#include "ImageReader.h"
#include "TIFFReader.h"
#include "TGAReader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFNTMakerDlg dialog

CFNTMakerDlg::CFNTMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFNTMakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFNTMakerDlg)
	m_strASCIICodes = _T("");
	m_strBitmap = _T("");
	m_strResult = _T("");
	m_dwCharSpace = 1;
	m_dwLineSpace = 1;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_bOnTop = FALSE;
}

void CFNTMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFNTMakerDlg)
	DDX_Control(pDX, IDC_ALWAYSONTOP, m_wndAlwaysOnTop);
	DDX_Text(pDX, IDC_ASCIICODES, m_strASCIICodes);
	DDX_Text(pDX, IDC_BITMAP, m_strBitmap);
	DDX_Text(pDX, IDC_RESULT, m_strResult);
	DDX_Text(pDX, IDC_CHARSPACE, m_dwCharSpace);
	DDV_MinMaxDWord(pDX, m_dwCharSpace, 0, 100);
	DDX_Text(pDX, IDC_LINESPACE, m_dwLineSpace);
	DDV_MinMaxDWord(pDX, m_dwLineSpace, 0, 100);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFNTMakerDlg, CDialog)
	//{{AFX_MSG_MAP(CFNTMakerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSEASCIICODE, OnBrowseASCIICodes)
	ON_BN_CLICKED(IDC_BROWSEBITMAP, OnBrowseBitmap)
	ON_BN_CLICKED(IDC_BROWSERESULT, OnBrowseResult)
	ON_BN_CLICKED(IDC_ALWAYSONTOP, OnAlwaysOnTop)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFNTMakerDlg message handlers

BOOL CFNTMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, FALSE);		// Set small icon
	SetIcon(m_hIcon, TRUE);			// Set big icon
	
  m_wndAlwaysOnTop.SetCheck(0);
  m_bOnTop = FALSE;
  
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFNTMakerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFNTMakerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFNTMakerDlg::OnBrowseASCIICodes() 
{
  UpdateData(TRUE);
  CFileDialog fd(TRUE, "*.txt", m_strASCIICodes, OFN_FILEMUSTEXIST,
    "Textové soubory (*.txt)|*.txt|Všechny soubory|*.*||",
    this);

  fd.DoModal();

  m_strASCIICodes = fd.GetPathName();
  UpdateData(FALSE);
}

void CFNTMakerDlg::OnBrowseBitmap() 
{
  UpdateData(TRUE);
  CFileDialog fd(TRUE, "*.tif", m_strBitmap, OFN_FILEMUSTEXIST,
    "TIFF or Targa (*.tif, *.tga)|*.tif;*.tga|Všechny soubory|*.*||",
    this);

  fd.DoModal();

  m_strBitmap = fd.GetPathName();
  UpdateData(FALSE);
}

void CFNTMakerDlg::OnBrowseResult() 
{
  UpdateData(TRUE);
  CFileDialog fd(FALSE, "*.fnt", m_strResult, OFN_FILEMUSTEXIST,
    "Soubor fontu (*.fnt)|*.fnt||",
    this);

  fd.DoModal();

  m_strResult = fd.GetPathName();
  UpdateData(FALSE);
}

void CFNTMakerDlg::OnAlwaysOnTop() 
{
  m_bOnTop = ~m_bOnTop;
  if(m_bOnTop)
    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  else
    SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

typedef struct{
  char m_ID[16];
  DWORD m_dwWidth;  // width of the alpha channel image
  DWORD m_dwHeight; // height of the alpha channel image
  DWORD m_dwCharsCount; // number of chars in the font
  DWORD m_dwCharSpace;  // space between two characters
  DWORD m_dwLineSpace;  // space between two lines
  BYTE m_Reserved[36];
}FNTHeader;

typedef struct{
  DWORD m_wASCIICode; // ASCII code of this character
  DWORD m_dwWidth;   // width of the character
  DWORD m_dwPosition;  // x coord of the character in the alpha channel image
}CharHeader;

void CFNTMakerDlg::OnConvert() 
{
  CFileException *pFileException = new CFileException();
  
  UpdateData(TRUE);

  BYTE *pBitmap = NULL;
  BYTE *pResult = NULL;
  BYTE *pText = NULL, *pASCIICodes = NULL;
  CImageReader *pBitmapReader = NULL;
  CFile fileTXT, fileResult;
  DWORD dwTXTLen, dwASCIILen;
  FNTHeader h;
  memset(&h, 0, sizeof(h));
  CharHeader *pCharHeader = NULL;
  DWORD i, j;

  char *hlp;
  pBitmapReader = NULL;
  hlp = strrchr((LPCSTR)m_strBitmap, '.');
  if(hlp == NULL){
    AfxMessageBox("Obrázek : Nesprávný soubor.");
    return;
  }
  hlp++;
  if(!strcmpi(hlp, "tif")){
    pBitmapReader = new CTIFFReader();
  }
  if(!strcmpi(hlp, "tga")){
    pBitmapReader = new CTGAReader();
  }
  if(pBitmapReader == NULL){
    AfxMessageBox("Obrázek : Nesprávný typ souboru.");
    goto EndIt;
  }

  try{
  pBitmapReader->Open(m_strBitmap);
  if(!pBitmapReader->ReadInformations()){
    AfxMessageBox("Chyba pøi ètení obrázku.");
    goto EndIt;
  }
  pBitmap = new BYTE[pBitmapReader->m_dwWidth * pBitmapReader->m_dwHeight * 3];
  if(pBitmap == NULL){
    AfxMessageBox("Nedostatek pamìti.");
    goto EndIt;
  }

  h.m_dwHeight = pBitmapReader->m_dwHeight - 1;
  h.m_dwWidth = pBitmapReader->m_dwWidth;
  h.m_dwCharSpace = m_dwCharSpace;
  h.m_dwLineSpace = m_dwLineSpace;

  if(!pBitmapReader->ReadImage(pBitmap, pBitmapReader->m_dwWidth * 3)){
    AfxMessageBox("Chyba pøi ètení obrázku.");
    goto EndIt;
  }
  pBitmapReader->Close();

  
  if(!fileTXT.Open(m_strASCIICodes, CFile::modeRead, pFileException)){
    pFileException->ReportError();
    goto EndIt;
  }

  dwTXTLen = fileTXT.GetLength();
  pText = new BYTE[dwTXTLen];
  fileTXT.Read(pText, dwTXTLen);

  fileTXT.Close();

  pASCIICodes = new BYTE[dwTXTLen];
  for(i = 0, dwASCIILen = 0; i < dwTXTLen; i++){
    if(pText[i] == 0x0d) continue;
    if(pText[i] == 0x0a) continue;
    pASCIICodes[dwASCIILen++] = pText[i];
  }

  h.m_dwCharsCount = dwASCIILen;
  strcpy(h.m_ID, "TheGame font   ");
  pCharHeader = new CharHeader[dwASCIILen];

  DWORD dwCharWidth, dwPos = 0;
  DWORD dwTheLinePos = h.m_dwHeight * h.m_dwWidth * 3;
  while(pBitmap[dwTheLinePos + dwPos] < 128){
    dwPos += 3;
  }
  for(i = 0; i < dwASCIILen; i++){
    dwCharWidth = 0;
    pCharHeader[i].m_dwPosition = dwPos / 3;
    while(pBitmap[dwTheLinePos + dwPos] > 128){
      dwPos += 3; dwCharWidth++;
    }
    while(pBitmap[dwTheLinePos + dwPos] < 128){
      dwPos += 3;
    }
    pCharHeader[i].m_dwWidth = dwCharWidth;
    pCharHeader[i].m_wASCIICode = pASCIICodes[i];
  }

  pResult = new BYTE[h.m_dwHeight * h.m_dwWidth];
  for(i = 0; i < h.m_dwHeight; i++){
    for(j = 0; j < h.m_dwWidth; j++){
      dwPos = (i * h.m_dwWidth + j) * 3;
      pResult[i * h.m_dwWidth + j] =
        (pBitmap[dwPos] + pBitmap[dwPos + 1] + pBitmap[dwPos + 2]) / 3;
    }
  }

  if(!fileResult.Open(m_strResult, CFile::modeWrite | CFile::modeCreate, pFileException)){
    pFileException->ReportError();
    goto EndIt;
  }

  fileResult.Write(&h, sizeof(h));
  fileResult.Write(pCharHeader, sizeof(CharHeader) * dwASCIILen);
  fileResult.Write(pResult, h.m_dwHeight * h.m_dwWidth);

  fileResult.Close();

  }
  catch(...){
    AfxMessageBox("Some error occured.");
    goto EndIt;
  }

EndIt:
  if(pBitmapReader) delete pBitmapReader;
  if(pBitmap) delete pBitmap;
  if(pFileException) pFileException->Delete();
  if(pText) delete pText;
  if(pASCIICodes) delete pASCIICodes;
  if(pCharHeader) delete pCharHeader;
  if(pResult) delete pResult;
}
