// MapViewMessageBar.h: interface for the CMapViewMessageBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPVIEWMESSAGEBAR_H__325CAC03_7A95_11D4_B0EE_004F49068BD6__INCLUDED_)
#define AFX_MAPVIEWMESSAGEBAR_H__325CAC03_7A95_11D4_B0EE_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMapViewMessageBar : public CWindow  
{
  DECLARE_DYNAMIC(CMapViewMessageBar);
  DECLARE_OBSERVER_MAP(CMapViewMessageBar);

public:
	CMapViewMessageBar();
	virtual ~CMapViewMessageBar();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  void Create(CRect &rcWindow, CWindow *pParent, CDataArchive Archive);
  virtual void Delete();

  virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);
  virtual CWindow *WindowFromPoint(CPoint &pt);

  typedef enum {
    MsgType_User,
    MsgType_Info,
    MsgType_Warning, 
    MsgType_Error,
    MsgType_Admin,

    MsgType_Count
  } EMessageType;

  enum{
    E_UnitLocation = 1,  // Unit location selected (param is Unit ID)
    E_MapLocation = 2,   // Map location selected (param is X<<16 & Y)
  };

  void Message(CString strText, EMessageType eType);
  void Message(CString strText, EMessageType eType, DWORD dwXPos, DWORD dwYPos);
  void Message(CString strText, EMessageType eType, DWORD dwUnitID);

protected:
  void OnTimeTick(DWORD dwTime);
  void OnLButtonDown(CPoint pt);

private:

  static DWORD m_aMsgTypeColor[MsgType_Count];

  void ResetTimer();
  void DeleteLast();

  typedef struct tagSMessageLine
  {
    CString m_strText;
    DWORD m_dwColor;
    DWORD m_dwTime;
    DWORD m_dwXPos, m_dwYPos;
    DWORD m_dwUnitID;
    struct tagSMessageLine *m_pNext;
  } SMessageLine;

  SMessageLine * InternalMessage( CString strText, EMessageType eType );
  BOOL HasLineLocation(SMessageLine *pLine);
  BOOL IsInLineLocation(SMessageLine *pLine, CPoint &pt);

  // List of lines
  // The first one is the last displayed
  SMessageLine *m_pLines;
  DWORD m_dwLinesCount;

  // The location image
  CImageSurface m_LocationImage;
};

#endif // !defined(AFX_MAPVIEWMESSAGEBAR_H__325CAC03_7A95_11D4_B0EE_004F49068BD6__INCLUDED_)
