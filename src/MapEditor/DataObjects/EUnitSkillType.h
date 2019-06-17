// EUnitSkillType.h: interface for the CEUnitSkillType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNITSKILLTYPE_H__D86699D5_94F0_11D3_A0D2_9EADB408E531__INCLUDED_)
#define AFX_EUNITSKILLTYPE_H__D86699D5_94F0_11D3_A0D2_9EADB408E531__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEUnitType;

class CEUnitSkillType : public CObject  
{
  DECLARE_DYNAMIC(CEUnitSkillType)

public:
  BOOL CheckValid();

  DWORD GetDataSize(){ return m_dwDataSize; }
	BYTE * GetData();
	void FreeData();
  BYTE * ReallocateData(DWORD dwNewSize);
	BYTE * AllocateData(DWORD dwSize);
  // saves the skill to the file
	void SaveToFile(CArchiveFile file);
  // deletes the object
	virtual void Delete();
  // creates the skill from file
	BOOL Create(CArchiveFile file, CEUnitType *pUnitType);
  // creates the new skill of given name (identifier)
	BOOL Create(CString strName, CEUnitType *pUnitType);

  // constructor & destructor
	CEUnitSkillType();
	virtual ~CEUnitSkillType();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // get name of the skill
  // the name can't change, cause the skill type is identified by it
  CString GetName(){ return m_strName; }

  // get/set enable flag
  BOOL GetEnabled(){ return m_bEnabled; }
  void SetEnabled(BOOL bValue){ m_bEnabled = bValue; }

  // get/set allowed mode
  BOOL GetAllowedMode(DWORD dwMode){ ASSERT(dwMode < 8);
    if(m_nAllowedModes & (1 << dwMode)) return TRUE; else return FALSE; }
  void SetAllowedMode(DWORD dwMode, BOOL bValue){ ASSERT(dwMode < 8);
    if(bValue) m_nAllowedModes |= 1 << dwMode;
    else m_nAllowedModes &= ~(1 << dwMode); }

  // returns the unit type for this skill
  CEUnitType *GetUnitType(){ return m_pUnitType; }

private:
  // bit array - if 1, the skill is allowed for the mode
  BYTE m_nAllowedModes;

  // TRUE - if the skill is enabled at start time
  BOOL m_bEnabled;

  // the name of the skill - this is the identifier of the skill
  CString m_strName;

  // the skill data
  DWORD m_dwDataSize;
  BYTE *m_pData;

  // Pointer to unit type to which we're applied to 
  CEUnitType *m_pUnitType;
};

#endif // !defined(AFX_EUNITSKILLTYPE_H__D86699D5_94F0_11D3_A0D2_9EADB408E531__INCLUDED_)
