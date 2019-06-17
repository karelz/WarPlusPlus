#if !defined(CONFIGMACROS_H_)
#define CONFIGMACROS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Windows\Fonts.h"

// use it in the layout objects
// function which reads the config file could looks like this
// CFG_BEGIN(pCfgFile); // begin the cfg file 
//   this also defines variables
//     CConfigFile *cfg_pConfigFile - copy of the pointer given to the macro
//     CString cfg_strPath - path to the config file in archive
//     CDataArchive cfg_Archive - archive from which the config file is
//     POSITION cfg_pos - helper variable

#define CFG_BEGIN(pConfigFile) \
	{ \
	  CString cfg_strPath; \
		CDataArchive cfg_Archive; \
		\
		CConfigFileSection cfg_Section = pConfigFile->GetRootSection(); \
		cfg_strPath = pConfigFile->GetFile().GetFileDir();\
		if(!cfg_strPath.IsEmpty()) cfg_strPath += "\\";\
		\
		cfg_Archive = pConfigFile->GetFile().GetDataArchive();

// CFG_END(); // end the config file after this you shouldn't call any other macros from here
#define CFG_END() \
	}


// deletes the given pointer to some object
//  this tests if it isn't NULL , if so -> delete Variable and then set it to NULL
#define CFG_DELETE(Variable) \
  if(Variable != NULL){ Variable->Delete(); delete Variable; Variable = NULL; }

// reads the long variable from the config file - the variable must exists
#define CFG_LONG(VariableName, CLong) \
  CLong = cfg_Section.GetInteger(VariableName);

// optional read of the long as above (the varibale musn't exists)
// if the varible doesn't exists it leaves it untouched
#define CFG_LONG_O(VariableName, CLong) \
  if(cfg_Section.IsVariable(VariableName)){ \
    CLong = cfg_Section.GetInteger(VariableName); \
  }


// same as for long but with RGB colors
// the RGB color is given in hexa 0x0RRGGBB
#define CFG_COLOR(VariableName, CColor) \
  CColor = cfg_Section.GetInteger(VariableName);
// optional
#define CFG_COLOR_O(VariableName, CColor) \
  if(cfg_Section.IsVariable(VariableName)){ \
    CColor = cfg_Section.GetInteger(VariableName); \
  }

// same as for long but with point
// the point is created by two variables
//   VarName_X and VarName_Y both are longs
#define CFG_POINT(VariableName, Point) \
  Point.x = cfg_Section.GetInteger(CString(VariableName) + "_X"); \
  Point.y = cfg_Section.GetInteger(CString(VariableName) + "_Y");

#define CFG_POINT_O(VariableName, Point) \
  if(cfg_Section.IsVariable(CString(VariableName) + "_X")){ \
    Point.x = cfg_Section.GetInteger(CString(VariableName) + "_X"); \
    Point.y = cfg_Section.GetInteger(CString(VariableName) + "_Y"); \
  }

// same as for nog but with size
// as in the point this is composed by VarName_X and VarName_Y
#define CFG_SIZE(VariableName, Size) \
  Size.cx = cfg_Section.GetInteger(CString(VariableName) + "_X"); \
  Size.cy = cfg_Section.GetInteger(CString(VariableName) + "_Y");

#define CFG_SIZE_O(VariableName, Size) \
  if(cfg_Section.IsVariable(CString(VariableName) + "_X")){ \
    Size.cx = cfg_Section.GetInteger(CString(VariableName) + "_X"); \
    Size.cy = cfg_Section.GetInteger(CString(VariableName) + "_Y"); \
  }

// as long but this read the real number (double)
#define CFG_DOUBLE(VariableName, CDouble) \
  CDouble = cfg_Section.GetReal(VariableName);
// optional
#define CFG_DOUBLE_O(VariableName, CDouble) \
  if(cfg_Section.IsVariable(VariableName)){ \
    CDouble = cfg_Section.GetReal(VariableName); \
  }

// creates animation from given path to some graphics file
// the path is the value of the variable
// the animation object is created and set to contain the file
//  supported file are static images (TGA TIFF) and animations (ANIM)
#define CFG_ANIMATION(VariableName, PAnimation) \
  PAnimation = new CAnimation(); \
  PAnimation->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName)));
// optional
#define CFG_ANIMATION_O(VariableName, PAnimation) \
  if(cfg_Section.IsVariable(VariableName)){ \
    PAnimation = new CAnimation(); \
    PAnimation->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName))); \
  }\

// load the cursor from the CURSOR file
// se animation (it's similar)
#define CFG_CURSOR(VariableName, PCursor) \
  PCursor = new CCursor(); \
  PCursor->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName)));

// optional
#define CFG_CURSOR_O(VariableName, PCursor) \
  if(cfg_Section.IsVariable(VariableName)){ \
    PCursor = new CCursor(); \
    PCursor->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName))); \
  }\


// like the animation but for sound files (WAV)
#define CFG_SOUND(VariableName, PSound) \
  PSound = new CSoundBuffer(); \
  PSound->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName)));

// optional
#define CFG_SOUND_O(VariableName, PSound) \
  if(cfg_Section.IsVariable(VariableName)){ \
    PSound = new CSoundBuffer(); \
    PSound->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName))); \
  }\


// like the animation this creates the font object
// the variable should contain string
// which can mean:
//   FNT file path -> graphic font is created
//   Name of the Windows font - in this case the system font is created
//     and the VarName_Size variable is searched to determine the size of it (in points)
//     if the variable doesn't exists it use the third parameter of the macro instead
#define CFG_FONT(VariableName, PFont, DefaultSize) \
  PFont = new CGraphicFont(); \
  try{ \
    ((CGraphicFont *)PFont)->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName))); \
  } \
  catch(CException *){ \
    delete PFont; \
    int cfg_nPointSize = DefaultSize; \
    if(cfg_Section.IsVariable(CString(VariableName) + "Size")){ \
      cfg_nPointSize = cfg_Section.GetInteger(CString(VariableName) + "Size"); \
    } \
    PFont = new CWindowsFont(); \
    ((CWindowsFont *)PFont)->Create(cfg_Section.GetString(VariableName), cfg_nPointSize); \
  }

// as the font but if th variable doesn;t exists the font pointer is left untouched
// optional
#define CFG_FONT_O(VariableName, PFont, DefaultSize) \
if(cfg_Section.IsVariable(VariableName)){ \
    PFont = new CGraphicFont(); \
    try{ \
      ((CGraphicFont *)PFont)->Create(cfg_Archive.CreateFile(cfg_strPath + cfg_Section.GetString(VariableName))); \
    } \
    catch(CException *){ \
      delete PFont; \
      int cfg_nPointSize = DefaultSize; \
      if(cfg_Section.IsVariable(CString(VariableName) + "Size")){ \
        cfg_nPointSize = cfg_Section.GetInteger(CString(VariableName) + "Size"); \
      } \
      PFont = new CWindowsFont(); \
      ((CWindowsFont *)PFont)->Create(cfg_Section.GetString(VariableName), cfg_nPointSize); \
    } \
  }

#endif // !defined(CONFIGMACROS_H_)