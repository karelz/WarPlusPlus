#ifndef __MAPEDITOR_MOVE_SKILLTYPE__HEADER_INCLUDED__
#define __MAPEDITOR_MOVE_SKILLTYPE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Typ pohybov� skilly z MapEditoru
typedef struct tagSMMoveSkillType
{
	// rychlost v mapcellech za 100 TimeSlic�
	DWORD m_dwSpeed;
  // vzhled p�i pohybu
  DWORD m_dwMoveAppearanceID;
} SMMoveSkillType;

#endif //__MAPEDITOR_MOVE_SKILLTYPE__HEADER_INCLUDED__
