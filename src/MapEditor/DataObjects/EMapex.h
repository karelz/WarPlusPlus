// EMapex.h: interface for the CEMapex class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAPEX_H__43E086DE_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_EMAPEX_H__43E086DE_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\GameClient\Windows\Animation.h"

class CEMapex  
{
public:
  BOOL CheckValid();

  // sets new land type on given coordinates (topleft of the mapex is 0,0)
	void SetLandType(DWORD dwX, DWORD dwY, BYTE nLandType);
  // returns land type on givven coordinates (topleft of the mapex is 0,0)
	BYTE GetLandType(DWORD dwX, DWORD dwY);
  // return pointer to the graphics of this mapex
	CAnimation * GetGraphics();
  // sets new mapex name (user one)
	void SetName(CString strName);
  // returns mapex name (user one)
	CString GetName();
  // returns mapex files name (in the library archive)
	CString GetMapexFileName();

  // sets new graphics files (and copy them to the library archive)
	void SetGraphics(CString strGraphicsFile, CDataArchive Archive);
  // deletes the graphics files from givven library archive
	void RemoveGraphics(CDataArchive Archive);

  // saves mapex to a file
	void SaveMapex(CDataArchive Archive);

  // sets new size of the mapex (maintains all possible land type information)
	void SetSize(CSize sizeMapex);

  // creates new mapex from file
	BOOL Create(CArchiveFile MapexFile);
  // creates new empty mapex with givven ID and size (in mapcells)
	BOOL Create(DWORD dwID, CSize sizeMapex);
  // deletes the mapex
	void Delete();

  // constructor & destructor
	CEMapex();
	~CEMapex();

  // returns mapex ID
  DWORD GetID(){ return m_dwID; }
  // returns size of the mapex (in mapcells)
  CSize GetSize(){ return m_sizeMapex; }

  void SetModified(BOOL bMod = TRUE){ m_bModified = bMod; }
  BOOL IsModified(){ return m_bModified; }

private:
  // mapex ID
  DWORD m_dwID;

  // size of the mapex in mapcells
  CSize m_sizeMapex;

  // array of land types for this mapex
  // 2-dimensional array of sizes m_sizeMapex
  BYTE *m_pLandType;

  // animation for this mapex
  CAnimation m_Graphics;

  // file name of the graphics in the library
  CString m_strGraphicsFile;

  // file name for this mapex in the library
  CString m_strMapexFile;

  // name of the mapex
  CString m_strName;

  BOOL m_bModified;
};

#endif // !defined(AFX_EMAPEX_H__43E086DE_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
