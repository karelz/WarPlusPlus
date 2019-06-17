// CInvisibility.h: interface for the CCInvisibility class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CINVISIBILITY_H__CD20CAE6_58AC_11D4_B0B6_004F49068BD6__INCLUDED_)
#define AFX_CINVISIBILITY_H__CD20CAE6_58AC_11D4_B0B6_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Object representing one type of invisibility
class CCInvisibility : public CObject  
{
  DECLARE_DYNAMIC(CCInvisibility);

public:
  // constructor & destructor
	CCInvisibility();
	virtual ~CCInvisibility();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Returns the name of the invisibility flag
  CString GetName(){ ASSERT_VALID(this); return m_strName; }

  // Creates the object from the MapFile (loads it)
  void Create(CArchiveFile MapFile);
  // Deletes the object
  virtual void Delete();

private:
  // The name of the invisibility flag
  CString m_strName;
};

#endif // !defined(AFX_CINVISIBILITY_H__CD20CAE6_58AC_11D4_B0B6_004F49068BD6__INCLUDED_)
