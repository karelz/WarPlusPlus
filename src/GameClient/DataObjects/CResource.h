// CResource.h: interface for the CCResource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRESOURCE_H__CD20CAE3_58AC_11D4_B0B6_004F49068BD6__INCLUDED_)
#define AFX_CRESOURCE_H__CD20CAE3_58AC_11D4_B0B6_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Class representing one type of resource on the map
// it has name and image
class CCResource : public CObject  
{
  DECLARE_DYNAMIC(CCResource);

public:
  // constructor & destructor
	CCResource();
	virtual ~CCResource();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Returns the name of the resource
  CString GetName(){ ASSERT_VALID(this); return m_strName; }
  // Returns the icon
  CDDrawSurface *GetIcon(){ ASSERT_VALID(this); return &m_Icon; }

  // Creates the resource object from the map file (loads it)
  void Create(CArchiveFile MapFile);
  // Deletes the object
  virtual void Delete();

private:
  // The name of the resource
  CString m_strName;

  // The image (icon)
  CScratchSurface m_Icon;
};

#endif // !defined(AFX_CRESOURCE_H__CD20CAE3_58AC_11D4_B0B6_004F49068BD6__INCLUDED_)
