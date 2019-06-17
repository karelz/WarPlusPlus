// IPersistentSaveLoadWaiting.h: interface for the CIPersistentSaveLoadWaiting class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PERSISTENTSAVELOADWAITING_H__58B2E28D_71B4_4ACE_800B_8B9B64393BB5__INCLUDED_)
#define AFX_PERSISTENTSAVELOADWAITING_H__58B2E28D_71B4_4ACE_800B_8B9B64393BB5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Zde jsou funkce pro ukladani trid zdedenych z CIWaitingInterface

void PersistentSaveWaiting( CIWaitingInterface *pWaitingInterface, CPersistentStorage &storage);

CIWaitingInterface* PersistentLoadWaiting( CPersistentStorage &storage);

void PersistentTranslatePointersWaiting( CIWaitingInterface *pWaitingInterface, CPersistentStorage &storage);

void PersistentInitWaiting( CIWaitingInterface *pWaitingInterface);

#endif // !defined(AFX_PERSISTENTSAVELOADWAITING_H__58B2E28D_71B4_4ACE_800B_8B9B64393BB5__INCLUDED_)
