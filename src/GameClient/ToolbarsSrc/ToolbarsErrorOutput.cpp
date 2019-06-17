// ToolbarsErrorOutput.cpp: implementation of the CToolbarsErrorOutput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolbarsErrorOutput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CToolbarsErrorOutput, CObject);

CToolbarsErrorOutput::CToolbarsErrorOutput()
{
}

CToolbarsErrorOutput::~CToolbarsErrorOutput()
{
}

#ifdef _DEBUG

void CToolbarsErrorOutput::AssertValid() const
{
  CObject::AssertValid();
}

void CToolbarsErrorOutput::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Creation
// Creates the error output file in the givven file (name and archive)
void CToolbarsErrorOutput::Create(CString strFileName, CDataArchive Archive)
{
  // First remove the old errors file
  try{
    Archive.RemoveFile(strFileName);
  }
  catch(CException *e){
    // if some exception -> the file didn't exists (doesn't matter)
    // just forget the exception
    e->Delete();
  }

  // open the file for writing
  m_File = Archive.CreateFile(strFileName, CArchiveFile::modeCreate | CArchiveFile::modeWrite | CArchiveFile::modeUncompressed);

  // write the first line (this is just information what the file is about)
  CString strHeader = "This file contains errors which occured while loading toolbars...\r\n\r\n";
  m_File.Write((LPCSTR)strHeader, strHeader.GetLength());
}

// Deletes the object (this will close the file)
void CToolbarsErrorOutput::Delete()
{
  m_File.Close();
}


// Writes error string to the file
// DO NOT include the EOL it will be added automaticaly
void CToolbarsErrorOutput::WriteError(CString strError)
{
  // Append the EOL
  strError = strError + "\r\n";
  
  // write it to file
  m_File.Write((LPCSTR)strError, strError.GetLength());
}

// Writes exception string to the file
void CToolbarsErrorOutput::WriteError(CException *pException)
{
  ASSERT(pException != NULL);

  // first get the exception string
  char txt[1024];

  // If some error -> better to write nothing
  if(!pException->GetErrorMessage(txt, 1023)) return;
  txt[1023] = 0;

  // write it to the file
  m_File.Write(txt, strlen(txt));
}