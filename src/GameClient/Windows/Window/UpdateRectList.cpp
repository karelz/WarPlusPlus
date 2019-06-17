// UpdateRectList.cpp: implementation of the CUpdateRectList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UpdateRectList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CUpdateRectList, CObject)

CUpdateRectList::CUpdateRectList() :
  m_RectNodePool(50)
{
  m_pFirstNode = NULL;
}

CUpdateRectList::~CUpdateRectList()
{

}

#ifdef _DEBUG

void CUpdateRectList::AssertValid() const
{
  CObject::AssertValid();
}

void CUpdateRectList::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CUpdateRectList::Create(double dbMinCoverCoeficient, double dbMinCoverDegradation)
{
  ASSERT(dbMinCoverCoeficient >= 0);
  ASSERT(dbMinCoverDegradation >= 0);

  m_pFirstNode = NULL;

  m_dbMinCoverCoeficient = dbMinCoverCoeficient;
  m_dbMinCoverDegradation = dbMinCoverDegradation;

  return TRUE;
}

void CUpdateRectList::Delete()
{
  RemoveAll();
}

void CUpdateRectList::AddRect(CRect *pRect)
{
  ASSERT_VALID(this);
  ASSERT(pRect != NULL);

  if(pRect->IsRectEmpty()) return; // empty rects are added automaticly

  SRectNode *pNode = m_RectNodePool.Allocate();

  pNode->m_rcRect = *pRect;
  pNode->m_nSize = pRect->Width() * pRect->Height();
  pNode->m_nCover = pNode->m_nSize;
  pNode->m_dbCoverCoeficient = 1;
  pNode->m_pNext = NULL;

  AddNode(pNode);
}

void CUpdateRectList::AddNode(SRectNode *pNewNode)
{
  // go through all existing nodes and try to join some

  SRectNode *pNode, *pPrevNode = NULL;
  CRect rcUnion, rcIntersect;
  int nUnionSize, nUnionCover;
  double dbCoverCoeficient;

  while(1){ // endless cicle -> try adding till it's possible
    pNode = m_pFirstNode; pPrevNode = NULL;
    while(pNode != NULL){
      // Try if they intersects each other
/*      rcIntersect.IntersectRect(&(pNode->m_rcRect), &(pNewNode->m_rcRect));
      if(!rcIntersect.IsRectEmpty()){
        // We'll left the pNode as it is
        // we will cut some pieces from the new node
        if(rcIntersect.left <= pNode->m_rcRect.left){
          // The left side is equal to our left side (it must be)
          if(rcIntersect.top <= pNode->m_rcRect.top){
            // The top is equal to our (must be)
            if(rcIntersect.right >= pNode->m_rcRect.right){
              if(rcIntersect.bottom >= pNode->m_rcRect.bottom){
                // The whole new rect is inside the old one
                // forget it

                m_RectNodePool.Free(pNewNode);
                return;
              }
              else{
                // ------------------
                // |    :      :    |
                // |    --------    |
                // |    |######|    |
                // ------------------
                //      |######|
                //      --------
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, rcIntersect.left, pNewNode->m_rcRect.bottom);
                CRect rcNew2(rcIntersect.left, pNewNode->m_rcRect.top, rcIntersect.right, rcIntersect.top);
                CRect rcNew3(rcIntersect.right, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);
                AddRect(&rcNew3);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
            }
            else{
              if(rcIntersect.bottom >= pNode->m_rcRect.bottom){
                // ------
                // |    |
                // |..--|---
                // |  |#|##|
                // |  |#|##|
                // |..--|---
                // |    |
                // ------
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, rcIntersect.top);
                CRect rcNew2(pNewNode->m_rcRect.left, rcIntersect.top, rcIntersect.left, rcIntersect.bottom);
                CRect rcNew3(pNewNode->m_rcRect.left, rcIntersect.bottom, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);
                AddRect(&rcNew3);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
              else{
                // This is situation like this
                //  ------ <--- this is the new one
                //  |  : |
                //  |  --|----
                //  |  |#|###| <----- this is the old one
                //  ------###|
                //     -------
                // We'll cut it by the double dot line
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, rcIntersect.left, pNewNode->m_rcRect.bottom);
                CRect rcNew2(rcIntersect.left, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, rcIntersect.top);

                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
            }
          }
          else{
            // The top is below our
            if(rcIntersect.bottom >= pNode->m_rcRect.bottom){
              if(rcIntersect.right >= pNode->m_rcRect.right){
                //   -----------
                //   |#########|
                // ----------------
                // | |#########|  |
                // | -----------  |
                // | :         :  |
                // ----------------
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, rcIntersect.left, pNewNode->m_rcRect.bottom);
                CRect rcNew2(rcIntersect.left, rcIntersect.bottom , rcIntersect.right, pNewNode->m_rcRect.bottom);
                CRect rcNew3(rcIntersect.right, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);
                AddRect(&rcNew3);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
              else{
                // The bottom is equal to our
                //   ------
                //   |####|
                // -----##|
                // | |#|##|
                // | --|---
                // | : |
                // -----
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, rcIntersect.left, pNewNode->m_rcRect.bottom);
                CRect rcNew2(rcIntersect.left, rcIntersect.bottom, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
            }
            else{
              // The bottom is above our
              //   ------
              //   |####|
              // -----##|
              // | |#|##|
              // | |#|##|
              // -----##|
              //   |####|
              //   ------
              CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, rcIntersect.left, pNewNode->m_rcRect.bottom);
              // Add them
              AddRect(&rcNew1);

              // delete the new node
              m_RectNodePool.Free(pNewNode);
              return;
            }
          }
        }
        else{
          // The left side is to right from our left side
          if(rcIntersect.top <= pNode->m_rcRect.top){
            // The top is equal to our
            if(rcIntersect.right >= pNode->m_rcRect.right){
              // The right is equal to our
              if(rcIntersect.bottom >= pNode->m_rcRect.bottom){
                //    ------
                //    |    |
                // ---|--..|
                // |##|#|  |
                // |##|#|  |
                // ---|--..|
                //    |    |
                //    ------
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, rcIntersect.top);
                CRect rcNew2(rcIntersect.right, rcIntersect.top , pNewNode->m_rcRect.right, rcIntersect.bottom);
                CRect rcNew3(pNewNode->m_rcRect.left, rcIntersect.bottom, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);
                AddRect(&rcNew3);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
              else{
                //     ------
                //     | :  |
                // ----|--  |
                // |###|#|  |
                // |###------
                // |#####|
                // -------
                CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, rcIntersect.right, rcIntersect.top);
                CRect rcNew2(rcIntersect.right, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
            }
            else{
              // The right is to the left from our right
              //    ------
              //    |    |
              // ---|----|---
              // |##|####|##|
              // |##------##|
              // |##########|
              // ------------
              CRect rcNew1(pNewNode->m_rcRect.left, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, rcIntersect.top);
              // Add them
              AddRect(&rcNew1);

              // delete the new node
              m_RectNodePool.Free(pNewNode);
              return;
            }
          }
          else{
            // The top is below our
            if(rcIntersect.right >= pNode->m_rcRect.right){
              // The right is equal to our
              if(rcIntersect.bottom >= pNode->m_rcRect.bottom){
                // --------
                // |######|
                // |###-------
                // |###|##|  |
                // |###-------
                // |######|
                // --------
                CRect rcNew1(rcIntersect.right, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
              else{
                // -------
                // |#####|
                // |##------
                // |##|##| |
                // ---|--- |
                //    |  : |
                //    ------
                CRect rcNew1(rcIntersect.right, pNewNode->m_rcRect.top, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
                CRect rcNew2(pNewNode->m_rcRect.left, rcIntersect.bottom, rcIntersect.right, pNewNode->m_rcRect.bottom);
                // Add them
                AddRect(&rcNew1);
                AddRect(&rcNew2);

                // delete the new node
                m_RectNodePool.Free(pNewNode);
                return;
              }
            }
            else{
              // The right is to the left of our
              // ------------
              // |##########|
              // |###-----##|
              // |###|###|##|
              // ----|---|---
              //     |   |  
              //     -----
              CRect rcNew1(pNewNode->m_rcRect.left, rcIntersect.bottom, pNewNode->m_rcRect.right, pNewNode->m_rcRect.bottom);
              // Add them
              AddRect(&rcNew1);

              // delete the new node
              m_RectNodePool.Free(pNewNode);
              return;
            }
          }
        }
      }*/

      rcUnion.UnionRect(&pNode->m_rcRect, &pNewNode->m_rcRect);
      nUnionSize = rcUnion.Width() * rcUnion.Height();
      nUnionCover = pNode->m_nCover + pNewNode->m_nCover;
      dbCoverCoeficient = (double)nUnionCover / (double)nUnionSize;
      
      // if the cover coeficient is too small -> no join
      if(dbCoverCoeficient >= m_dbMinCoverCoeficient){
        
        // now try if cover this join wouldn't low the cover coeficint
        // too much
        // compare it only with the bigger one
        if(pNode->m_dbCoverCoeficient >= pNewNode->m_dbCoverCoeficient){
          if((dbCoverCoeficient / pNode->m_dbCoverCoeficient) < m_dbMinCoverDegradation){
            goto NoJoin;
          }
        }
        else{
          if((dbCoverCoeficient / pNewNode->m_dbCoverCoeficient) < m_dbMinCoverDegradation){
            goto NoJoin;
          }
        }
        
        // OK -> join it
        // so first remove the node from the list
        if(pPrevNode != NULL){
          pPrevNode->m_pNext = pNode->m_pNext;
        }
        else{
          m_pFirstNode = pNode->m_pNext;
        }
        
        // now join nodes
        pNode->m_rcRect = rcUnion;
        pNode->m_nSize = nUnionSize;
        pNode->m_nCover = nUnionCover;
        pNode->m_dbCoverCoeficient = dbCoverCoeficient;
        
        // delete the new one
        m_RectNodePool.Free(pNewNode);
        
        // and try to join add the new joined node to the list again
        pNewNode = pNode;
        goto TryItAgain;
      }
      
NoJoin:
      pPrevNode = pNode;
      pNode = pNode->m_pNext;
    }

    if(pNode == NULL){ // no possible join found -> just add it
      pNewNode->m_pNext = m_pFirstNode;
      m_pFirstNode = pNewNode;
      break; // OK -> everything is done -> quit
    }

TryItAgain:
    ;
  }
}

void CUpdateRectList::RemoveAll()
{
  SRectNode *pNode = m_pFirstNode, *pDelNode;
  while(pNode != NULL){
    pDelNode = pNode;
    pNode = pNode->m_pNext;
    m_RectNodePool.Free(pDelNode);
  }
  m_pFirstNode = NULL;
}

CUpdateRectList::SRectNode * CUpdateRectList::GetFirstRectNode()
{
  return m_pFirstNode;
}

BOOL CUpdateRectList::IsEmpty()
{
  return (m_pFirstNode == NULL);
}

void CUpdateRectList::RemoveFirstNode()
{
  SRectNode *pNode = m_pFirstNode;
  m_pFirstNode = m_pFirstNode->m_pNext;
  m_RectNodePool.Free(pNode);
}
