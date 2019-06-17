// EScriptSet.h: interface for the CEScriptSet class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ESCRIPTSET_H__1C8EC1B1_79AB_11D4_B0ED_004F49068BD6__INCLUDED_)
#define AFX_ESCRIPTSET_H__1C8EC1B1_79AB_11D4_B0ED_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEScriptSet : public CObject  
{
public:
	CEScriptSet();
	virtual ~CEScriptSet();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  CString GetName(){ return m_strName; }
  void SetName(CString strName){ m_strName = strName; }
  CString GetPath(){ return m_strFullName; }
  CString GetFileName(){ return m_strFileName; }
  void SetPath(CString strPath);

  DWORD GetID(){ return m_dwID; }

  void Create(CArchiveFile MapFile);
  void Create(DWORD dwID, CDataArchive Archive);
  virtual void Delete();

  void Save(CArchiveFile MapFile);

  void Export(CDataArchive TheMap);

private:
  DWORD m_dwID;
  CString m_strName;
  CString m_strFileName;
  CString m_strFullName;
};

#endif // !defined(AFX_ESCRIPTSET_H__1C8EC1B1_79AB_11D4_B0ED_004F49068BD6__INCLUDED_)
