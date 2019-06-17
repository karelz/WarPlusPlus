// CULMapexInstance.h: interface for the CCULMapexInstance class.
//
//////////////////////////////////////////////////////////////////////

// *ROMAN v--v 27.3.2000

#if !defined(AFX_CULMAPEXINSTANCE_H__552D2E63_03D3_11D4_801C_0000B4A08F9A__INCLUDED_)
#define AFX_CULMAPEXINSTANCE_H__552D2E63_03D3_11D4_801C_0000B4A08F9A__INCLUDED_

#include "CMapex.h"

class CCMap;

// Class representing one mapex instance (UL mapex)
class CCULMapexInstance : CCachedObject 
{
public:
	// ctor
	CCULMapexInstance();

	// dtor
	virtual ~CCULMapexInstance();

	//////////////////////////
	// Creation
public:
	// Creates the object from the file
	// need the map object to find the mapex by ID
	void Create(CArchiveFile MapFile, CCMap *pMap);

	// Deletes the object
	void Delete();

	//////////////////////
	// Get/Set methods
public:
	// returns the mapex (type) object
	CCMapex *GetMapex(){ return m_pMapex; }

	// returns the x position (in mapcells)
	DWORD GetXPosition(){ return m_dwXPosition; }
	
	// returns the y position (in mapcells)
	DWORD GetYPosition(){ return m_dwYPosition; }

	// returns the z position
	DWORD GetZPosition(){ return m_dwZPosition; }

	//////////////////////////////////////////
	// Drawing functions
public:
	// Draws the mapex into the given surface
	// must know the coords (top-left) of the surface in mapcells
	void Draw(DWORD dwXPos, DWORD dwYPos, CDDrawSurface *pSurface);

private:
	// pointer to the mapex (type) object
	CCMapex *m_pMapex;

	// position
	DWORD m_dwXPosition;
	DWORD m_dwYPosition;
	DWORD m_dwZPosition;

	// next unit-level mapex instance in the list of them at map square
	CCULMapexInstance *m_pNext;

	// map square must be friend to be able to manipulate with the list of us
	friend class CCMapSquare;
};

// *ROMAN ^--^ 27.3.2000

#endif // !defined(AFX_CULMAPEXINSTANCE_H__552D2E63_03D3_11D4_801C_0000B4A08F9A__INCLUDED_)
