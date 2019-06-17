// ToolbarButtonInfo.h: interface for the CToolbarButtonInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLBARBUTTONINFO_H__EDA298D6_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
#define AFX_TOOLBARBUTTONINFO_H__EDA298D6_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ToolbarsErrorOutput.h"

// Object representing one toolbar button
class CToolbarButtonInfo : public CObject  
{
  DECLARE_DYNAMIC(CToolbarButtonInfo);

public:
  // Enum for types of the parameters for function
  typedef enum{
    ParamType_None,
    ParamType_Integer,
    ParamType_Real,
    ParamType_String,
    ParamType_Bool,
  } EParamType;

  // Structure describing one functino parameter
  typedef struct tagSParameter{
    EParamType m_eParamType; // type of the parameter
    union{  // value of the parameter
      int m_nValue;
      double m_dbValue;
      char *m_lpszValue;
      BOOL m_bValue;
    };
  } SParameter;

  // Constructor & destructor
	CToolbarButtonInfo();
	virtual ~CToolbarButtonInfo();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Returns the caption for the button
  CString GetCaption(){ ASSERT_VALID(this); return m_strCaption; }
  // Returns the animation for the button
  CAnimation *GetIcon(){ ASSERT_VALID(this); return m_pButtonIcon; }
  // Returns the key accelerator char (if 0 -> no accelerator)
  UINT GetKeyAccChar () { ASSERT_VALID ( this ); return m_nKeyAccChar; }
  // Returns the key accelerator flags
  DWORD GetKeyAccFlags () { ASSERT_VALID ( this ); return m_dwKeyAccFlags; }
  // Returns TRUE if the button is separator
  BOOL IsSeparator(){ ASSERT_VALID(this); return m_strCaption.IsEmpty(); }
  // Returns name of the function
  CString GetFunctionName(){ ASSERT_VALID(this); return m_strFunctionName; }
  // Returns number of parameters
  DWORD GetParamCount(){ ASSERT_VALID(this); return m_dwParamsCount; }
  // Returns one parameter
  SParameter *GetParameter(DWORD dwParam){ ASSERT_VALID(this); ASSERT(dwParam < m_dwParamsCount);
    return &(m_pParameters[dwParam]); }

  // Creates the button info from config file section
  // Returns TRUE if the creation was successfull
  BOOL Create(CDataArchive Archive, CConfigFileSection *pSection, CToolbarsErrorOutput *pErrorOutput, bool bCanBeSeparator = true);

  // Deletes the object
  virtual void Delete();

  // Initialize the image manager
  static void Init();
  // Closes the image manager
  static void Close();

  // Class that stores all loaded images for toolbars
  // Used to load all images only once
  class CToolbarImageManager
  {
  public:
    CToolbarImageManager(){ m_pNodes = NULL; }
    ~CToolbarImageManager(){ ASSERT(m_pNodes == NULL); }

    // creates the manager
    void Create();
    // deletes the manager
    void Delete();

    // Adds image to the manager
    // if the image is already there -> returns existing one
    // if the image can't be create -> returns NULL
    CAnimation *AddImage(CDataArchive, CString strImageFileName);

  private:
    struct SImageNode{
      CAnimation m_Animation;
      CString m_strFilePath;
      struct SImageNode *m_pNext;
    };
    // list of loaded images
    struct SImageNode *m_pNodes;
  };

  // The only instance of the image manager
  static CToolbarImageManager *m_pImageManager;

private:
  // Caption of the button
  // If the caption is empty -> the button is separator
  CString m_strCaption;

  // Image to be displayed on the button
  // if NULL -> use default image
  CAnimation *m_pButtonIcon;

  // Key accelerator
  UINT m_nKeyAccChar;
  DWORD m_dwKeyAccFlags;

  // Name of the function to run
  CString m_strFunctionName;

  // Number of params specified for the function
  DWORD m_dwParamsCount;
  // Array of parameters
  SParameter *m_pParameters;
};

#endif // !defined(AFX_TOOLBARBUTTONINFO_H__EDA298D6_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
