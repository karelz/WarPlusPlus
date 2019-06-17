// KeyAccelerators.cpp: implementation of the CKeyAccelerators class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KeyAccelerators.h"
#include "Keyboard.h"

#include "..\Window\Window.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CKeyAccelerators, CNotifier)

extern CKeyAccelerators *g_pKeyAccelerators = NULL;

CKeyAccelerators::CKeyAccelerators()
{
}

CKeyAccelerators::~CKeyAccelerators()
{
}

#ifdef _DEBUG

void CKeyAccelerators::AssertValid() const
{
  CNotifier::AssertValid();
}

void CKeyAccelerators::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif


BOOL CKeyAccelerators::Create()
{
  // set us as the active one
  m_pPrevKeyAcc = g_pKeyAccelerators;
  g_pKeyAccelerators = this;

  return CNotifier::Create();
}

void CKeyAccelerators::Delete()
{
  // if we were active -> set the previous acc
  if(g_pKeyAccelerators == this)
    g_pKeyAccelerators = m_pPrevKeyAcc;

  CNotifier::Delete();
}

void CKeyAccelerators::Connect(CWindow *pWindow, UINT nChar, DWORD dwFlags, DWORD dwNotID)
{

  CNotifier::SObserverNode *pNode;
  pNode = CNotifier::FindObserverNode(pWindow);

  SKeyAccConnectionData *pConData;

  if(pNode == NULL){
    CNotifier::Connect(pWindow, dwNotID);

    pNode = CNotifier::FindObserverNode(pWindow);
    if(pNode == NULL) return;

    pConData = new SKeyAccConnectionData();
    pNode->m_pConnectionData = pConData;
    pConData->pNodes = NULL;
  }
  else{
    pConData = (SKeyAccConnectionData *)pNode->m_pConnectionData;
  }

  // create new key node
  SKeyAccNode *pKeyNode = new SKeyAccNode();
  // fill it with given data
  pKeyNode->nChar = nChar;
  pKeyNode->dwFlags = dwFlags;

  // add it to the beggining of the list
  pKeyNode->pNext = pConData->pNodes;
  pConData->pNodes = pKeyNode;
}

void CKeyAccelerators::DeleteNode(SObserverNode *pNode)
{
  SKeyAccConnectionData *pConData;

  pConData = (SKeyAccConnectionData *)pNode->m_pConnectionData;

  if(pConData != NULL){
    SKeyAccNode *pKeyNode, *pKeyNode2;

    // remove all key nodes
    pKeyNode = pConData->pNodes;
    while(pKeyNode != NULL){
      pKeyNode2 = pKeyNode;
      pKeyNode = pKeyNode2->pNext;
      delete pKeyNode2;
    }

    // delete the connection data
    delete pConData;
  }

  pNode->m_pConnectionData = NULL;

  CNotifier::DeleteNode(pNode);
}

BOOL CKeyAccelerators::DoAccelerator(CKeyboard::SKeyInfo *pInfo)
{
  SObserverNode *pNode;
  SKeyAccConnectionData *pConData;
  SKeyAccNode *pKeyNode;
  DWORD dwMask;

  for(pNode = CNotifier::OpenConnections();pNode != NULL; pNode = CNotifier::GetNextConnection(pNode)){
    if(pNode == NULL) continue;

    pConData = (SKeyAccConnectionData *)pNode->m_pConnectionData;
    if(pConData == NULL) continue;
    
    pKeyNode = pConData->pNodes;
    for(; pKeyNode != NULL; pKeyNode = pKeyNode->pNext){
      if((pKeyNode->nChar == pInfo->nChar)){
        dwMask = pInfo->dwFlags & CKeyboard::ShiftDown;
        if((pKeyNode->dwFlags & CKeyboard::ShiftDown) == CKeyboard::ShiftDown){
          if(!dwMask) continue; // I want some of them -> but non is pressed
        }
        else{
          // if thier differs in some bit -> no match
          if(dwMask ^ (pKeyNode->dwFlags & CKeyboard::ShiftDown)) continue;
        }
        
        dwMask = pInfo->dwFlags & CKeyboard::CtrlDown;
        if((pKeyNode->dwFlags & CKeyboard::CtrlDown) == CKeyboard::CtrlDown){
          if(!dwMask) continue; // I want some of them -> but non is pressed
        }
        else{
          // if thier differs in some bit -> no match
          if(dwMask ^ (pKeyNode->dwFlags & CKeyboard::CtrlDown)) continue;
        }

        dwMask = pInfo->dwFlags & CKeyboard::AltDown;
        if((pKeyNode->dwFlags & CKeyboard::AltDown) == CKeyboard::AltDown){
          if(!dwMask) continue; // I want some of them -> but non is pressed
        }
        else{
          // if thier differs in some bit -> no match
          if(dwMask ^ (pKeyNode->dwFlags & CKeyboard::AltDown)) continue;
        }

        // OK the test were successfull
        // just make sure the window is enabled
        if(((CWindow *)pNode->m_pObserver)->m_bInternalyEnabled){
          if(CNotifier::SendEvent(CKeyboard::E_KEYACCELERATOR, (DWORD)pInfo, pNode->m_pObserver)){
            CNotifier::CloseConnections();
            return TRUE;
          }
        }
      }
    }
  }
  CNotifier::CloseConnections();
  return FALSE;
}
