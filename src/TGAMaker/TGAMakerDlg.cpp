// TGAMakerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TGAMaker.h"
#include "TGAMakerDlg.h"
#include "ImageReader2.h"
#include "TIFFReader2.h"
#include "TGAReader2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTGAMakerDlg dialog

CTGAMakerDlg::CTGAMakerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTGAMakerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTGAMakerDlg)
	m_strAlpha = _T("");
	m_strBitmap = _T("");
	m_strResult = _T("");
	m_bAutomaticResultName = FALSE;
	m_bAutomaticTransparentName = FALSE;
	m_bAlphaInverse = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  m_bOnTop = FALSE;
}

void CTGAMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTGAMakerDlg)
	DDX_Control(pDX, IDC_BROWSERESULT, m_wndBrowseResult);
	DDX_Control(pDX, IDC_BROWSEALPHA, m_wndBrowseAlpha);
	DDX_Control(pDX, IDC_ALPHA, m_wndAlpha);
	DDX_Control(pDX, IDC_RESULT, m_wndResult);
	DDX_Control(pDX, IDC_ALWAYSONTOP, m_wndAlwaysOnTop);
	DDX_Text(pDX, IDC_ALPHA, m_strAlpha);
	DDX_Text(pDX, IDC_BITMAP, m_strBitmap);
	DDX_Text(pDX, IDC_RESULT, m_strResult);
	DDX_Check(pDX, IDC_AUTOMATIC_RESULT_NAME, m_bAutomaticResultName);
	DDX_Check(pDX, IDC_AUTOMATIC_TRANSPARENT_NAME, m_bAutomaticTransparentName);
	DDX_Check(pDX, IDC_ALPHAINVERSE, m_bAlphaInverse);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTGAMakerDlg, CDialog)
	//{{AFX_MSG_MAP(CTGAMakerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSEALPHA, OnBrowseAlpha)
	ON_BN_CLICKED(IDC_BROWSEBITMAP, OnBrowseBitmap)
	ON_BN_CLICKED(IDC_BROWSERESULT, OnBrowseResult)
	ON_BN_CLICKED(IDC_CONVERT, OnConvert)
	ON_BN_CLICKED(IDC_ALWAYSONTOP, OnAlwaysOnTop)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_AUTOMATIC_RESULT_NAME, OnAutomaticResultName)
	ON_BN_CLICKED(IDC_AUTOMATIC_TRANSPARENT_NAME, OnAutomaticTransparentName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTGAMakerDlg message handlers

BOOL CTGAMakerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
  m_wndAlwaysOnTop.SetCheck(0);
  m_bOnTop = FALSE;

  CWnd *pStatic;
  CRect rcStatic;
  DWORD dwStyle, dwExStyle;
  pStatic = GetDlgItem(IDC_BKGCOLOR);
  pStatic->GetWindowRect(&rcStatic);
  ScreenToClient(&rcStatic);
  dwStyle = pStatic->GetStyle();
  dwExStyle = pStatic->GetExStyle();
  pStatic->DestroyWindow();
  
  m_BkgColor.Create("", dwStyle, rcStatic, this, IDC_BKGCOLOR);

  m_pPreview = (CPreviewWindow *)RUNTIME_CLASS(CPreviewWindow)->CreateObject();
  m_pPreview->Create(NULL, "");
  m_pPreview->ShowWindow(SW_SHOW);
  m_pPreview->UpdateWindow();

  m_BkgColor.SetPreview(m_pPreview);
  
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTGAMakerDlg::OnPaint() 
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
HCURSOR CTGAMakerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTGAMakerDlg::OnBrowseAlpha() 
{
  UpdateData(TRUE);
  CFileDialog fd(TRUE, "*.tif", m_strAlpha, OFN_FILEMUSTEXIST,
    "TIFF or Targa (*.tif, *.tga)|*.tif;*.tga|Všechny soubory|*.*||",
    this);

  fd.DoModal();

  m_strAlpha = fd.GetPathName();
  UpdateData(FALSE);
}

void CTGAMakerDlg::OnBrowseBitmap() 
{
  UpdateData(TRUE);
  CFileDialog fd(TRUE, "*.tif", m_strBitmap, OFN_FILEMUSTEXIST,
    "TIFF or Targa (*.tif, *.tga)|*.tif;*.tga|Všechny soubory|*.*||",
    this);

  fd.DoModal();

  m_strBitmap = fd.GetPathName();
  UpdateData(FALSE);
}

void CTGAMakerDlg::OnBrowseResult() 
{
  UpdateData(TRUE);

  CFileDialog fd(FALSE, "*.tga", m_strResult, OFN_FILEMUSTEXIST,
    "Truevision Targa (*.tga)|*.tga||",
    this);

  if(fd.DoModal() != IDOK) return;

  m_strResult = fd.GetPathName();

  UpdateData(FALSE);
  m_pPreview->SetPicture(m_strResult);
}

#pragma pack(1)
typedef struct{
  BYTE m_nIDFieldLength;
  BYTE m_nColorMapType;
  BYTE m_nImageType;
  WORD m_wColorMapOrigin;
  WORD m_wColorMapLength;
  BYTE m_nColorMapEntrySize;
  WORD m_wXOrigin;
  WORD m_wYOrigin;
  WORD m_wWidth;
  WORD m_wHeight;
  BYTE m_nPixelSize;
  BYTE m_nFlags;
}TGAHeader;
#pragma pack()

void CTGAMakerDlg::OnConvert() 
{
  BYTE *pBitmap = NULL;
  BYTE *pAlpha = NULL;
  BYTE *pResult = NULL;

  UpdateData(TRUE);

  if(m_bAutomaticTransparentName)
    ComputeAutomaticAlphaName();
  if(m_bAutomaticResultName)
    ComputeAutomaticResultName();

  if(!strcmpi((LPCSTR)m_strBitmap, (LPCSTR)m_strAlpha)){
    AfxMessageBox("Obrázek a prùsvitnost musí být rúzné soubory.");
    return;
  }

  CImageReader2 *pBitmapReader = NULL, *pAlphaReader = NULL;

  char *hlp;
  pBitmapReader = NULL;
  hlp = strrchr((LPCSTR)m_strBitmap, '.');
  if(hlp == NULL){
    AfxMessageBox("Obrázek : Nesprávný soubor.");
    return;
  }
  hlp++;
  if(!strcmpi(hlp, "tif")){
    pBitmapReader = new CTIFFReader2();
  }
  if(!strcmpi(hlp, "tga")){
    pBitmapReader = new CTGAReader2();
  }
  if(pBitmapReader == NULL){
    AfxMessageBox("Obrázek : Nesprávný typ souboru.");
    goto EndIt;
  }

  pAlphaReader = NULL;
  hlp = strrchr((LPCSTR)m_strAlpha, '.');
  if(hlp == NULL){
    AfxMessageBox("Prùsvitnost : Nesprávný soubor.");
    goto EndIt;
  }
  hlp++;
  if(!strcmpi(hlp, "tif")){
    pAlphaReader = new CTIFFReader2();
  }
  if(!strcmpi(hlp, "tga")){
    pAlphaReader = new CTGAReader2();
  }
  if(pBitmapReader == NULL){
    AfxMessageBox("Prùsvitnost : Nesprávný typ souboru.");
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
  if(!pBitmapReader->ReadImage(pBitmap, pBitmapReader->m_dwWidth * 3)){
    AfxMessageBox("Chyba pøi ètení obrázku.");
    goto EndIt;
  }
  pBitmapReader->Close();

  pAlphaReader->Open(m_strAlpha);
  if(!pAlphaReader->ReadInformations()){
    AfxMessageBox("Chyba pøi ètení prùsvitnosti.");
    goto EndIt;
  }
  if((pAlphaReader->m_dwWidth != pBitmapReader->m_dwWidth) ||
    (pAlphaReader->m_dwHeight != pBitmapReader->m_dwHeight)){
    AfxMessageBox("Obrázek a prùsvitnost nejsou stejnì velké.");
    goto EndIt;
  }
  pAlpha = new BYTE[pAlphaReader->m_dwWidth * pAlphaReader->m_dwHeight * 3];
  if(pBitmap == NULL){
    AfxMessageBox("Nedostatek pamìti.");
    goto EndIt;
  }
  if(!pAlphaReader->ReadImage(pAlpha, pAlphaReader->m_dwWidth * 3)){
    AfxMessageBox("Chyba pøi ètení prùsvitnosti.");
    goto EndIt;
  }
  pAlphaReader->Close();

  // Do the conversion
  DWORD size = pBitmapReader->m_dwWidth * pBitmapReader->m_dwHeight * 4;
  pResult = new BYTE[size];
  if(pResult == NULL){
    AfxMessageBox("Nedostatek pamìti.");
    goto EndIt;
  }

  CFile File;

  CFileException e;
  if(!File.Open(m_strResult, CFile::modeWrite | CFile::modeCreate, &e)){
    THROW(&e);
  }

  if(m_bAlphaInverse){
    DWORD i, j, pos1, pos2;
    for(i = 0; i < pBitmapReader->m_dwHeight; i++){
      pos1 = (pBitmapReader->m_dwHeight - i - 1) * pBitmapReader->m_dwWidth * 4;
      pos2 = i * pBitmapReader->m_dwWidth * 3;
      for(j = 0; j < pBitmapReader->m_dwWidth; pos1 += 4, pos2 += 3, j++){
        pResult[pos1] = pBitmap[pos2];
        pResult[pos1+1] = pBitmap[pos2+1];
        pResult[pos1+2] = pBitmap[pos2+2];
        pResult[pos1+3] = (pAlpha[pos2] + pAlpha[pos2+1] + pAlpha[pos2+2])/3;
      }
    }
  }
  else{
    DWORD i, j, pos1, pos2;
    for(i = 0; i < pBitmapReader->m_dwHeight; i++){
      pos1 = (pBitmapReader->m_dwHeight - i - 1) * pBitmapReader->m_dwWidth * 4;
      pos2 = i * pBitmapReader->m_dwWidth * 3;
      for(j = 0; j < pBitmapReader->m_dwWidth; pos1 += 4, pos2 += 3, j++){
        pResult[pos1] = pBitmap[pos2];
        pResult[pos1+1] = pBitmap[pos2+1];
        pResult[pos1+2] = pBitmap[pos2+2];
        pResult[pos1+3] = 255 - (pAlpha[pos2] + pAlpha[pos2+1] + pAlpha[pos2+2])/3;
      }
    }
  }

  TGAHeader h;

  h.m_nIDFieldLength = 15;
  h.m_nColorMapType = 0;
  h.m_nImageType = 2;
  h.m_wColorMapOrigin = 0;
  h.m_wColorMapLength = 0;
  h.m_nColorMapEntrySize = 0;
  h.m_wXOrigin = 0;
  h.m_wYOrigin = 0;
  h.m_wWidth = (WORD)pBitmapReader->m_dwWidth;
  h.m_wHeight = (WORD)pBitmapReader->m_dwHeight;
  h.m_nPixelSize = 32;
  h.m_nFlags = 8;

  File.Write(&h, 18);
  File.Write("TheGame format", 15);

  File.Write(pResult, size);

  File.Close();

  m_pPreview->SetPicture(m_strResult);

  }
  catch(...){
    AfxMessageBox("Some error ocured.");
    goto EndIt;
  }

EndIt:;
  if(pBitmap) delete [] pBitmap;
  if(pAlpha) delete [] pAlpha;
  if(pResult) delete [] pResult;
  if(pBitmapReader) delete pBitmapReader;
  if(pAlphaReader) delete pAlphaReader;
}

void CTGAMakerDlg::OnAlwaysOnTop() 
{
  m_bOnTop = ~m_bOnTop;
  if(m_bOnTop)
    SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
  else
    SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CTGAMakerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
  m_pPreview->DestroyWindow();	
}

void CTGAMakerDlg::OnAutomaticResultName() 
{
  UpdateData(TRUE);

  if(m_bAutomaticResultName){
    m_wndResult.EnableWindow(FALSE);
    ComputeAutomaticResultName();
  }
  else{
    m_wndResult.EnableWindow(TRUE);
  }
}

void CTGAMakerDlg::OnAutomaticTransparentName() 
{
	UpdateData(TRUE);

  if(m_bAutomaticTransparentName){
    m_wndAlpha.EnableWindow(FALSE);
    m_wndBrowseAlpha.EnableWindow(FALSE);
    ComputeAutomaticAlphaName();
  }
  else{
    m_wndAlpha.EnableWindow(TRUE);
    m_wndBrowseAlpha.EnableWindow(TRUE);
  }
}

void CTGAMakerDlg::ComputeAutomaticResultName()
{
  UpdateData(TRUE);

  int nPos = m_strResult.ReverseFind('\\');
  CString strName;
  int nPos2 = m_strBitmap.ReverseFind('\\');
  if(nPos2 < 0){
    strName = m_strBitmap;
  }
  else{
    strName = m_strBitmap.Mid(nPos2 + 1);
  }


  if(nPos < 0){
    m_strResult = strName;
  }
  else{
    m_strResult = m_strResult.Left(nPos) + "\\" + strName;
  }
  
  UpdateData(FALSE);
}

void CTGAMakerDlg::ComputeAutomaticAlphaName()
{
  UpdateData(TRUE);

  int nPos = m_strBitmap.ReverseFind('.');
  if(nPos < 0)
    m_strAlpha = m_strBitmap;
  else{
    m_strAlpha = m_strBitmap.Left(nPos);
    m_strAlpha += "T";
    m_strAlpha += m_strBitmap.Mid(nPos);
  }
  
  UpdateData(FALSE);
}
