#ifndef __MAPEDITOR_MOVE_SKILLTYPE__HEADER_INCLUDED__
#define __MAPEDITOR_MOVE_SKILLTYPE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Typ pohybové skilly z MapEditoru
typedef struct tagSMMoveSkillType
{
	// rychlost v mapcellech za 100 TimeSlicù
	DWORD m_dwSpeed;
  // vzhled pøi pohybu
  DWORD m_dwMoveAppearanceID;
} SMMoveSkillType;

#endif //__MAPEDITOR_MOVE_SKILLTYPE__HEADER_INCLUDED__
