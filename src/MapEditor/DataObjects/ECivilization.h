// ECivilization.h: interface for the CECivilization class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ECIVILIZATION_H__688CEF75_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
#define AFX_ECIVILIZATION_H__688CEF75_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common\Map\Map.h"
#include "EScriptSet.h"

class CEMap;

class CECivilization : public CObject  
{
  DECLARE_DYNAMIC(CECivilization)

public:
  BOOL CheckValid();

	void SetName(CString strName);
	void SetColor(DWORD dwColor);
	void SaveToFile(CArchiveFile file);
	virtual void Delete();
	BOOL Create(CArchiveFile file, CEMap *pMap);
	BOOL Create(DWORD dwID, CString strName, DWORD dwColor);
	CECivilization();
	virtual ~CECivilization();

  CEScriptSet *GetScriptSet(){ return m_pScriptSet; }
  void SetScriptSet(CEScriptSet *pScriptSet){ ASSERT_VALID(this); m_pScriptSet = pScriptSet; }

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // returns name
  CString GetName(){ return m_strName; }
  // returns ID
  DWORD GetID(){ return m_dwID; }
  // returns color
  DWORD GetColor(){ return m_dwColor; }
  // returns pointer to resources array
  int *GetResources(){ return m_aResources; }
  // sets resources (copies the values from given array)
  void SetResources(int *pResources){ memcpy(m_aResources, pResources, RESOURCE_COUNT * sizeof(int)); }

  void SetStartPosition(DWORD dwX, DWORD dwY){ m_dwStartPosX = dwX; m_dwStartPosY = dwY; }
  DWORD GetXStartPosition(){ return m_dwStartPosX; }
  DWORD GetYStartPosition(){ return m_dwStartPosY; }

private:
  // name of the civ.
  CString m_strName;
  // script set
  CEScriptSet *m_pScriptSet;
  // ID of the civilization
  DWORD m_dwID;
  // the color for this civilization
  DWORD m_dwColor;

  // Start position
  DWORD m_dwStartPosX, m_dwStartPosY;

  // resources
  int m_aResources[RESOURCE_COUNT];
};

#endif // !defined(AFX_ECIVILIZATION_H__688CEF75_8A12_11D3_A0A6_FEA9F52CDF31__INCLUDED_)
