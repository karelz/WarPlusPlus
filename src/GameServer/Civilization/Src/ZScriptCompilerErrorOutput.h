// ZScriptCompilerErrorOutput.h: interface for the CZScriptCompilerErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZSCRIPTCOMPILERERROROUTPUT_H__0B79C004_0F38_11D4_8495_004F4E0004AA__INCLUDED_)
#define AFX_ZSCRIPTCOMPILERERROROUTPUT_H__0B79C004_0F38_11D4_8495_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CZScriptErrorOutput;

class CZScriptCompilerErrorOutput : public CCompilerErrorOutput  
{
public:
	CZScriptCompilerErrorOutput();
	virtual ~CZScriptCompilerErrorOutput();

  void Open(CZScriptErrorOutput *pErrorOutput);
  void Close();

	virtual void PutChars(const char *pBuffer, int nCount);
	virtual void Flush();

private:
  CZScriptErrorOutput *m_pErrorOutput;
};

#endif // !defined(AFX_ZSCRIPTCOMPILERERROROUTPUT_H__0B79C004_0F38_11D4_8495_004F4E0004AA__INCLUDED_)
