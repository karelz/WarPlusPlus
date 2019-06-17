// EResource.h: interface for the CEResource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERESOURCE_H__25FAD957_557F_11D4_B0B1_004F49068BD6__INCLUDED_)
#define AFX_ERESOURCE_H__25FAD957_557F_11D4_B0B1_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEResource : public CObject  
{
  DECLARE_DYNAMIC(CEResource);

public:
	CEResource();
	virtual ~CEResource();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  BOOL CheckValid();

  void Create(DWORD dwIndex, CString strName);
  void Create(DWORD dwIndex, CArchiveFile file);
  void Save(CArchiveFile file);
  virtual void Delete();

  // return name of the resource
  CString GetName(){ return m_strName; }
  // sets new resource name
  void SetName(CString strName){ m_strName = strName; }

  // Sets new icon
  void SetIcon(CDDrawSurface *pIcon);
  CDDrawSurface *GetIcon(){ return &m_Icon; }

  // Returns the index of the resource on the map
  DWORD GetIndex(){ return m_dwIndex; }

private:
  // The resource icon
  CScratchSurface m_Icon;

  // The name
  CString m_strName;

  // Index of the resource on the map
  DWORD m_dwIndex;
};

#endif // !defined(AFX_ERESOURCE_H__25FAD957_557F_11D4_B0B1_004F49068BD6__INCLUDED_)
