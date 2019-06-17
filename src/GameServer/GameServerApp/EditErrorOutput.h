// EditErrorOutput.h: interface for the CEditErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITERROROUTPUT_H__0FE21CA7_780C_11D4_B0EA_004F49068BD6__INCLUDED_)
#define AFX_EDITERROROUTPUT_H__0FE21CA7_780C_11D4_B0EA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEditErrorOutput : public CCompilerErrorOutput  
{
public:
	CEditErrorOutput();
	virtual ~CEditErrorOutput();
  
  void SetEdit(CEdit *pEdit);

  // Zapsani znaku na vystup (-vola prekladac)
	virtual void PutChars(const char *pBuffer, int nCount);
  // Vyprazdneni cache (jsou-li) (-vola prekladac pred skoncenim)
	virtual void Flush();

private:
  CEdit *m_pEdit;
};

#endif // !defined(AFX_EDITERROROUTPUT_H__0FE21CA7_780C_11D4_B0EA_004F49068BD6__INCLUDED_)
