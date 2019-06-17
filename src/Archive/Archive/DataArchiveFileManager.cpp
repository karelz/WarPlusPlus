// DataArchiveFileManager.cpp: implementation of the CDataArchiveFileManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <errno.h>
#include <iostream.h>
#include <direct.h>
#include "DataArchiveFileManager.h"
#include "DataArchiveDirContents.h"
#include "../Compression/CompressEngine.h"
#include "ArchiveUtils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDataArchiveFileManager::CDataArchiveFileManager()
{
	m_bRecurse=FALSE;
	m_strPath=".";
	m_cCommand='t';
}

CDataArchiveFileManager::~CDataArchiveFileManager()
{
}

int CDataArchiveFileManager::Parse(int argc, TCHAR *argv[]) {
	ASSERT(argc>0);

	if(argc==1) {
		PrintUsage();
		return 0;
	}

	int state=0; // Nejprve switche
	int i=1, j; // Poradi argumentu
	CString arg;
	while(i<argc) { // Zpracovani argumentu
		arg=argv[i];
		switch(state) {
		case 0:	// Switches
			if(arg[0]!='-') { // Konec switchu
				state=2; // Prechazime na command
			} else {
				// Parsovani switchu
				arg.MakeLower();
				for(j=1; j<arg.GetLength(); j++) {
					switch(arg[j]) {
					case 'r': // Recurse subdirectories
						m_bRecurse=TRUE;
						break;
					case 'd': // Path to directory
						state=1;
						break;
					default:
						cerr << "ERROR: Unknown switch '" << arg[j] << "'\n";
						return 1;
						break;
					}					
				}
				i++;
			}
			break;

		case 1: // Switch parameter
			m_strPath=arg;
			m_strPath.Replace('/', '\\');
			m_strPath.TrimRight();
			m_strPath.TrimRight('\\');
			state=0;
			i++;
			break;

		case 2: // Command
			if(arg.GetLength()>1) { // Neni to command				
				state=3;
			} else {
				state=3;
				m_cCommand=tolower(arg[0]);
				i++;
			}
			break;

		case 3:	// Archive name			
			if(FileExists(arg)) {
				// Otevirame stary archiv
				m_Archive.Create(arg, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveFile);
			}
			else {
				// Vytvarime novy archiv
				m_Archive.Create(arg, CArchiveFile::modeReadWrite | CArchiveFile::modeCreate, CDataArchiveInfo::archiveFile);
			}
			state=4;
			i++;
			break;
		
		case 4:	// File list			
		case 5:
			switch(tolower(m_cCommand)) {
			case 'a':	// Append
				Append(arg, "");
				break;
			case 'd':	// Delete
				Delete(arg);
				break;
			case 'x':	// Extract
				Extract(arg, m_strPath);
				break;
			default:
				cerr << "ERROR: Wrong command '" << m_cCommand << "'.\n";
				return 1;
			}
			i++;
			state=5;
			break;
		}	
	}
	
	if(state<4) {
		cerr << "ERROR: Archive name has to be specified.\n";
		return 1;
	}

	if(state==4) {
		switch(tolower(m_cCommand)) {
		case 'x':	// Extract			
			Extract("", m_strPath);
			break;
		case 't':	// Print
			PrintContents(m_Archive, m_strPath);
			break;
		case 'r':	// Rebuild
			Rebuild();
			break;
		default:
			cerr << "ERROR: Wrong command '" << m_cCommand << "'.\n";
			return 1;
		}
	}

	return 0;
}

void CDataArchiveFileManager::PrintUsage()
{
	cout << "WAR++ Archive File Manager\n\n";
	cout << "Usage: warman [SWITCHES] [COMMAND] [ARCHIVE NAME] [FILES]\n\n";
	cout << "Switches\n--------\n";
	cout << "-r\t\tRecurse subdirectories\n";
	cout << "-d <path>\tPath to directory (default '.')\n";
	cout << "\nCommands\n--------\n";
	cout << "A\t\tAppend to archive\n";	
	cout << "D\t\tDelete from archive\n";
	cout << "X\t\teXtract from archive\n";
	cout << "T\t\tprinT contents of archive (default if unspecified command)\n";
	cout << "R\t\tRebuild archive\n";
}

BOOL CDataArchiveFileManager::FileExists(CString strFileName)
{
	CFileFind find;
	if(!find.FindFile(strFileName)) 
		return FALSE;
	else
		return TRUE;
}

void CDataArchiveFileManager::PrintContents(CDataArchive archive, CString strPath)
{
	CDataArchiveDirContents *contents=m_Archive.GetDirContents(strPath);

	if(contents==NULL) return;
	
	BOOL bWorking=contents->MoveFirst();
	while(bWorking) {
		CDataArchiveFileDirectoryItem *item=contents->GetInfo();
		cout << strPath << '\\' << item->GetName() << "\n" << flush;
		if(item->IsDirectory()) {
			if(m_bRecurse) {
				PrintContents(archive, strPath+'\\'+item->GetName());
			}
		}
		bWorking=contents->MoveNext();
	}

	delete contents;
}

void CDataArchiveFileManager::Append(CString strPath, CString strWhere)
{
	CFileFind find;
	if(!find.FindFile(strPath) && !find.FindFile(strPath+CCompressEngine::GetDefaultArchiveExtension())) { // Soubor neni
		cerr << "ERROR: File '" << strPath << "' not found.\n";
		return;
	}

	BOOL bW=TRUE;
	while (bW) {
		bW=find.FindNextFile();
		CString strFound, strDir, strName;
		strFound=find.GetFilePath();
		SplitPath(strFound, strDir, strName);

		if(find.IsDirectory() && !m_bRecurse) { // Adresar, ale my nerekurzime
			cerr << "WARNING: Ommiting directory '" << strPath << "'.\n";
		} else if(find.IsDirectory() && m_bRecurse) { // Adresar, rekurzime
			strPath=find.GetFilePath();
			strPath.TrimRight();
			strPath.TrimRight('\\'); strPath.TrimRight('/');
			strWhere+='\\'+strName;
			try {
				m_Archive.MakeDirectory(strWhere);
			} catch (CDataArchiveException *pError) {
				if(pError->GetErrorCode()!=CDataArchiveException::alreadyExists) {
					throw;
				}
			}
			cout << "MKDIR:\t" << strWhere << "\n";
			CFileFind dirContents;
			BOOL bWorking=dirContents.FindFile(strPath+"\\*.*");
			while(bWorking) {
				bWorking=dirContents.FindNextFile();
				if(dirContents.IsDots()) { // Preskocit . a ..
					continue;
				}
				Append(dirContents.GetFilePath(), strWhere);	
			}
		} else { // Soubor
			CString gfp=find.GetFilePath();
			CArchiveUtils::ClipExtension(gfp);
			CArchiveUtils::ClipExtension(strName);
			cout << "APPEND:\t" << gfp << " -> " << strWhere+'\\'+strName << flush;
			m_Archive.AppendFile(gfp, strWhere+'\\'+strName);
			cout << " [OK]\n" << flush;
		}
	}
}

void CDataArchiveFileManager::Delete(CString strPath)
{
	CDataArchiveDirContents *contents=m_Archive.GetDirContents(strPath);

	if(contents==NULL) {
		cout << "REMOVING:" << strPath << "\n" << flush;
		m_Archive.RemoveFile(strPath);
	} else {	
		BOOL bWorking=contents->MoveFirst();
		while(bWorking) {
			CDataArchiveFileDirectoryItem *item=contents->GetInfo();
			if(item->IsDirectory()) {
				if(m_bRecurse) {
					Delete(strPath+'\\'+item->GetName());
				}				
			} else {
				cout << "REMOVING:" << strPath << '\\' << item->GetName() << "\n" << flush;
				m_Archive.RemoveFile(strPath+'\\'+item->GetName());
			}
			bWorking=contents->MoveNext();
		}
		cout << "REMOVING:" << strPath << "\n" << flush;
		m_Archive.RemoveDirectory(strPath);
		delete contents;
	}
}

void CDataArchiveFileManager::Extract(CString strPath, CString strWhere)
{
	CDataArchiveDirContents *contents=m_Archive.GetDirContents(strPath);

	CString strMyDir, strMyFile, strFrom, strTo;
	SplitPath(strPath, strMyDir, strMyFile);

	if(contents==NULL) {
		cout << "EXTRACT:\t" << strPath << flush;
		try {
			m_Archive.ExtractFile(strPath, strWhere+'\\'+strPath);
		} catch(CException *e) {
      char txt[1024];
      e->GetErrorMessage(txt, 1024);
      e->Delete();
			cout << " [ERROR]\n" << flush;
      cout << txt << flush;
			return;
		}
		cout << " [OK]\n" << flush;			
	} else {
		int err=_mkdir(strWhere /* + '\\' + strMyFile */);
		if(errno==EEXIST) err=0;
		if(err==0) {
			cout << "MAKEDIR:\t" << strWhere << /* '\\' << strMyFile << */ "\n" << flush;
		} else {
			cerr << "ERROR: Cannot create directory '" << strWhere /* + '\\' + strMyFile */ << "'.\n" << flush;
		}				

		BOOL bWorking=contents->MoveFirst();		
		while(bWorking) {
			CDataArchiveFileDirectoryItem *item=contents->GetInfo();
			strTo=strWhere + '\\' + item->GetName();
			strFrom=strPath + '\\' + item->GetName();
			if(item->IsDirectory()) {
				if(m_bRecurse) {
					Extract(strFrom, strTo);
				}
			} else {
				cout << "EXTRACT:\t" << strFrom << flush;
				try {
					m_Archive.ExtractFile(strTo, strFrom);
				} catch(CException *e) {
          char txt[1024];
          e->GetErrorMessage(txt, 1024);
          e->Delete();
					cout << " [ERROR]\n" << flush;
          cout << txt << flush;
					bWorking=contents->MoveNext();
					continue;
				}
				cout << " [OK]\n" << flush;			
			}
			bWorking=contents->MoveNext();
		}
		delete contents;
	}
}

void CDataArchiveFileManager::SplitPath(CString &strPath, CString &strDir, CString &strFile)
{
	strPath.Replace('/', '\\');
	strPath.TrimRight(); strPath.TrimRight('\\');
	strPath.TrimLeft(); strPath.TrimLeft('\\');

	// Oddelime jmeno od cesty
	int pos=strPath.ReverseFind('\\');
	if(pos>=0) { // Lomitko tam je
		strDir=strPath.Left(pos);
		strFile=strPath.Mid(pos+1);
	} else { // Lomitko tam neni, to co je zadane je cele jmeno adresare
		strDir="";
		strFile=strPath;
	}
}

void CDataArchiveFileManager::Rebuild()
{
	m_Archive.Rebuild();
}	
