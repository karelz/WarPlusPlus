#ifndef _GAMECLIENT_HELPERFUNCTIONS_H_
#define _GAMECLIENT_HELPERFUNCTIONS_H_

// Reads windows position and show state from registry, from given section
// If there is no specified in the registry, uses given defaults
// Shows or hides the window...
void ReadWindowPositionFromRegistry ( CString strSection, CWindow * pWnd, CRect * pRectDefault, bool bShowDefault );

// Writes windows position and show state to registry to given section
void WriteWindowPositionToRegistry ( CString strSection, CWindow * pWnd );

#endif // _GAMECLIENT_HELPERFUNCTIONS_H_