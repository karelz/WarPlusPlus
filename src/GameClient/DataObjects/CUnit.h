#ifndef CUNIT_H_
#define CUNIT_H_

#include "CUnitType.h"

struct SCCheckPoint {
  // position in mapcells
	DWORD m_dwX, m_dwY;

  // time in timeslices
  DWORD m_dwTime;
};


class CCMap;
class CCMapSquare;
class CUnitCache;

// one unit instance on the visible part of the map
class CCUnit : public CMultithreadNotifier
{
  DECLARE_DYNAMIC(CCUnit)

	friend class CCTimestampedVirtualConnection;

public:
  CCUnit();
  virtual ~CCUnit();

  // some create and delete
  void Create(CCUnitType *pUnitType, CUnitCache *pUnitCache);
  virtual void Delete();

  enum Events
  {
    // Position of the unit has changed
    // (In the dwParam is pointer to the unit... )
    E_PositionChanged = 1,
    // Brief info has changed (for example life bars...)
    // (In the dwParam is pointer to the unit... )
    E_BriefInfoChanged = 2,
    // Full info has changed
    // (In the dwParam is pointer to the unit... )
    E_FullInfoChanged = 3,
    // Unit has disappeared from the map, it means, its no longer visible
    // This can have two reasons :
    //   - unit has died
    //   - it's the enemy unit, and I can'n see it any more
    //     (out of my viewrange)
    // (In the dwParam is pointer to the unit... )
    E_UnitDisappeared = 4,
  };

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

//-------------------------------------------------------
  // Locks
    // Returns the read write lock
    CReadWriteLock *GetLock(){ return &m_lockUnit; }
    
    // Reference count ++
    void AddRef();
    // Reference cout --
    void Release();

//-------------------------------------------------------
  // Get functions
    // returns unit type object for this unit
    CCUnitType *GetUnitType(){ ASSERT_VALID(this); return m_pUnitType; }

	// returns full info on the unit
    BYTE * GetFullInfo (){ ASSERT_VALID ( this ); return m_pFullInfo; }
	// returns size of the full info on the unit
	DWORD GetFullInfoSize (){ ASSERT_VALID ( this ); return m_dwFullInfoLength; }

  // ID
    // returns unit's ID
    DWORD GetID(){ ASSERT_VALID(this); return m_dwID; }
    // sets the ID
    void SetID(DWORD dwID){ m_dwID = dwID; }

  // Map squares
    // returns next unit in the mapsquare list
    CCUnit *GetMapSquareNext(){ ASSERT_VALID(this); return m_pMapSquareNext; }

	  // Puts the unit into correct list for correct mapsquare
  	void UpdateMapSquare(CCMap *pMap);

  // Mode
    // Returns current mode for the unit
    BYTE GetMode () { ASSERT_VALID ( this ); return m_nMode; }


//-------------------------------------------------------
  // Position
    // returns position in pixels on the map
    CRect GetPositionRect(){ ASSERT_VALID(this); return m_rcPosition; }

    // Sets unit position
    // The unit must be locked for writing
  	void SetPosition(DWORD dwX, DWORD dwY, DWORD dwZ, CCMap *pMap);

    // Sets new checkpoints
    // The unit must be locked for writing
    void SetCheckPoints(SCCheckPoint *pFirstCheckPoint, SCCheckPoint *pSecondCheckPoint, CCMap *pMap);

    // returns position in mapcells
    DWORD GetXPosition(){ ASSERT_VALID(this); return m_dwXPos; }
    DWORD GetYPosition(){ ASSERT_VALID(this); return m_dwYPos; }
    DWORD GetZPosition(){ ASSERT_VALID(this); return m_dwZPos; }

    // returns position of the unit's square on the map in pixels
    DWORD GetXPixelPosition(){ ASSERT_VALID(this); return m_dwXPixelPos; }
    DWORD GetYPixelPosition(){ ASSERT_VALID(this); return m_dwYPixelPos; }

    // returns number of lives
    DWORD GetLives(){ ASSERT_VALID(this); return m_dwLives; }
    // returns maximum number of lives the unit can have
    DWORD GetMaxLives(){ ASSERT_VALID(this); return m_dwMaxLives; }

    // returns current animation
	// Small hack, return the last good one
	// Cause we know, that almost everytime the last good is in the cache
	// If the current is in the cache, the last points to the same
    CCUnitAnimation * GetCurrentAnimation(){ ASSERT_VALID(this); return m_pLastGoodAnimation; }

    // returns current frame in the animation
    CCUnitSurface * GetCurrentFrame(){ ASSERT_VALID(this); return m_pCurrentAnimation->GetFrame(m_dwCurrentFrame); }

    // returns current frame index
    DWORD GetCurrentFrameNum(){ ASSERT_VALID(this); return m_dwCurrentFrame; }

    // Returns if the unit is moving
    BOOL IsMoving(){ ASSERT_VALID(this); return (m_dbXSpeed != 0) || (m_dbYSpeed != 0); }
    // Moves the unit along it's move path by given amount of animseconds
    void MoveUnit(double dbAmount, CCMap *pMap);

    // Returns current direction (0 if the direction is random)
    DWORD GetCurrentDirection(){ ASSERT_VALID(this); return (DWORD)((m_nCurrentDirection == 8) ? 0 : m_nCurrentDirection); }

	// Returns flag if the unit is currently watched for full infos
	BOOL GetFullInfoFlag() { ASSERT_VALID(this); return m_bFullInfoFlag; }

	// Sets flag if the unit is currently watched for full infos
	void SetFullInfoFlag(BOOL bFullInfoFlag=TRUE) { ASSERT_VALID(this); m_bFullInfoFlag = bFullInfoFlag; }


//-------------------------------------------------------
  // Appearance
    // Sets unit direction
    void SetDirection(BYTE nDirection);
    // Sets unit appearance
    void SetAppearance(DWORD dwAppearanceID, BYTE nDirection);

    // restart current animation from the begining
    void RestartAnimation();

    // returns number of animseconds to the next frame of animation
    // if 0 -> no animation (infinite time)
    DWORD GetNextFrameRemainingTime(){ ASSERT_VALID(this); return m_dwNextFrameRemainingTime; }
    // decreases number of remainnig animseconds by given amount
    // if it returns TRUE -> step to the next frame
    // elseway do nothing
    BOOL DecreaseNextFrameRemainingTime(DWORD dwAmount){
      ASSERT_VALID(this);
      if(m_dwNextFrameRemainingTime <= dwAmount) return TRUE;
      m_dwNextFrameRemainingTime -= dwAmount;
      return FALSE;
    }
    // steps to the next frame in current animation
    void SetNextFrame();

    // Returns TRUE if the unit has random direction
    BOOL HasRandomDirection(){ ASSERT_VALID(this); return (m_nWantedDirection == 8); }
    // Decreases the remaining time for direction change
    // and if it reaches the zero sets new random direction
    void DecreaseNextDirectionChangeRemainingTime(DWORD dwAmount){
	  ASSERT_VALID(this);
      if(m_dwNextDirectionChangeRemainingTime <= dwAmount) SetRandomDirection();
      m_dwNextDirectionChangeRemainingTime -= dwAmount;
    }
    // Sets new random direction and also sets the direction change timeout
    void SetRandomDirection();


  // removes the unit from the map
  void RemoveFromMap(CCMap *pMap);

	// Sets parent map square
	void SetParentMapSquare(CCMapSquare *pMapSquare) { m_pParentMapSquare=pMapSquare; }

	// Gets parent map square
	CCMapSquare *GetParentMapSquare() { return m_pParentMapSquare; }

	// Sets next map square unit
	void SetNextMapSquareUnit(CCUnit *pUnit) { m_pMapSquareNext=pUnit; };

	// Gets next map square unit
	CCUnit *GetNextMapSquareUnit() { return m_pMapSquareNext; };

	// Sets prev map square unit
	void SetPrevMapSquareUnit(CCUnit *pUnit) { m_pMapSquarePrev=pUnit; };

	// Gets prev map square unit
	CCUnit *GetPrevMapSquareUnit() { return m_pMapSquarePrev; };

private:
    // some unit data

  // Recomputes new pixel position on the map
  // bIsLoaded means that the m_pCurrentAnimation is in the cache (loaded)
  // !!! AND it is LOCKED (at least for reading) !!!
  void RecomputePixelPosition(BOOL bIsLoaded = FALSE);
  // Recomputes interpolations speeds
  void RecomputeInterpolations(DWORD dwCurrentTimeslice);
  // Go to the next checkpoint
  void NextCheckPoint(CCMap *pMap);

//-------------------------------------------------------
  // Locks
    // The lock for the unit
    CReadWriteLock m_lockUnit;
    // reference count
    DWORD m_dwReferenceCount;
    // pointer to the unit cache (where we are)
    CUnitCache *m_pUnitCache;

//-------------------------------------------------------
  // ID
    DWORD m_dwID;

  // Current mode
    BYTE m_nMode;

  // type info
    // pointer to the unit type
    CCUnitType *m_pUnitType;

  // life info
    // lives count
    DWORD m_dwLives;

    // max lives count
    DWORD m_dwMaxLives;

  // View radius
    DWORD m_dwViewRadius;
	
  // Full info o jednotce (muze byt NULL)
    // Ukazatel na data FullInfa
    BYTE *m_pFullInfo;
    // delka
    DWORD m_dwFullInfoLength;

  // Invisibility
    // Co vsechno vidim
    DWORD m_dwInvisibilityDetection;
    // Kdy nejsem videt
    DWORD m_dwInvisibilityState;


//-------------------------------------------------------
  // position and moving
    // position on the map (in mapcells)
    DWORD m_dwXPos, m_dwYPos, m_dwZPos;
    // position of the unit's square (the one which is occupied by the unit) in pixels on the map
    // this is the precious position of the unit on the map
    // the m_rcPosition is just computed from this
    DWORD m_dwXPixelPos, m_dwYPixelPos;

  // position on the map in pixels
    // this rect must include all the units graphics
    CRect m_rcPosition;

  // checkpoints
    // next two checkpoints
    SCCheckPoint m_CurrentCheckPoint;
    SCCheckPoint m_NextCheckPoint;

  // interpolations
    // speed of moving in pixels/animsecond
    // it's the speed of move to m_CurrentCheckPoint;
    // If both are 0 -> no move
    double m_dbXSpeed;
    double m_dbYSpeed;

    // Precise position in pixels on the map
    // This is used only by interpolations (it must be very precise)
    double m_dbXPosition;
    double m_dbYPosition;

//-------------------------------------------------------
  // appearance and graphics
    // pointer to current appearance
    CCUnitAppearance *m_pCurrentAppearance;

    // Wanted direction (this can be 8 -> don't know, the client will randomly select one)
    BYTE m_nWantedDirection;
    // last used animation (means, last one wich we've successfuly drawn)
    // if the m_pCurrentAnimatino is not in the cache, we'll use this one (its first frame)
    // if even this animation is not in the cache -> the unit won't be drawn
    CCUnitAnimation *m_pLastGoodAnimation;
    // pointer to current direction (animation)
    CCUnitAnimation *m_pCurrentAnimation;
    // and number of this direction
    BYTE m_nCurrentDirection;
    // current frame in the animation
    DWORD m_dwCurrentFrame;
    // remaining animseconds to another frame in the animation
    // if it's 0 -> no animation (just static image)
    DWORD m_dwNextFrameRemainingTime;

    // remaining animseconds to another direction change
    // this member is valid only if the m_nWantedDirection == 8
    DWORD m_dwNextDirectionChangeRemainingTime;

//-------------------------------------------------------
  // Mapsqure
	  // Pointer to parent map square
	  CCMapSquare *m_pParentMapSquare;
	  // Pointer at next unit in map square
	  CCUnit *m_pMapSquareNext;
	  // Pointer at previous unit in map square
	  CCUnit *m_pMapSquarePrev;


//-------------------------------------------------------
  // Unit cache
    // Pointers for unit cache
    CCUnit *m_pUnitCacheNext;
    CCUnit *m_pUnitCachePrev;

	// Flag, when set, unit is receiving either full infos or enemy full infos
	BOOL m_bFullInfoFlag;

	// Timestamp - time of last update of brief info
	DWORD m_dwTimeStampBrief;

	// Timestamp - time of last update of checkpoint info
	DWORD m_dwTimeStampCheckPoint;

	// Timestamp - time of last update of full infos
	DWORD m_dwTimeStampFull;

//-------------------------------------------------------
  // Viewport
    // viewport helpers
    CCUnit *m_pViewportNext;

  friend class CUnitCache;
  friend class CViewport;
};

#endif