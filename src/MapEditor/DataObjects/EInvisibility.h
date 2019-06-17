// EInvisibility.h: interface for the CEVisibility class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EINVISIBILITY_H__A8F87A67_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
#define AFX_EINVISIBILITY_H__A8F87A67_57D5_11D4_B0B4_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEInvisibility : public CObject  
{
  DECLARE_DYNAMIC(CEInvisibility);

public:
	CEInvisibility();
	virtual ~CEInvisibility();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  BOOL CheckValid();

  CString GetName(){ return m_strName; }
  void SetName(CString strName){ m_strName = strName; }
  DWORD GetIndex(){ return m_dwIndex; }

  void Create(DWORD dwIndex);
  void Create(DWORD dwIndex, CArchiveFile file);
  virtual void Delete();
  void Save(CArchiveFile file);

private:
  // Index in the invisibility flags field
  DWORD m_dwIndex;
  // Name
  CString m_strName;
};

#endif // !defined(AFX_EINVISIBILITY_H__A8F87A67_57D5_11D4_B0B4_004F49068BD6__INCLUDED_)
