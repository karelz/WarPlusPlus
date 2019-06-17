// Animation.h: interface for the CAnimation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ANIMATION_H__BE6D3E09_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
#define AFX_ANIMATION_H__BE6D3E09_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CAnimation : public CObject  
{
  DECLARE_DYNAMIC(CAnimation);

public:
	BOOL Create(CConfigFile *pAnimFile);
	// returns TRUE if at least one pixel in animation is transparent
  BOOL IsTransparent();
  // returns size in which will whole animation fit
	CSize GetSize();
  // creates the animation as a static image
  // it means an animation with one frame and speed equal to 0
  // the given file is the image file
  BOOL Create(CArchiveFile File);
	// sets the new speed for the animation
  // the speed is the number of miliseconds between two frames
  // if the speed is equal to 0 it means the animation
  // will not animate -> infinite time between frames
  void SetSpeed(DWORD dwSpeed);
	// sets the selected frame to given surface
  // !!!!!!! you have to create the surface object with
  // operator new but don't delete it
  // the animation object will delete it
  // when it won't use it any more
  void SetFrame(DWORD dwIndex, CDDrawSurface *pSurface);
  // creates the empty animation
  // just sets the number of frames and speed
	BOOL Create(DWORD dwFrameNum, DWORD dwSpeed, BOOL bLoop = TRUE);
  // returns the speed of the animation
  // it means number of miliseconds between two frames
	DWORD GetSpeed();
  // returns pointer to selected frame
	CDDrawSurface * GetFrame(DWORD dwIndex);
  // returns number of frames in the animation
  DWORD GetFrameNum();

  BOOL GetLoop(){ return m_bLoop; }
  void SetLoop(BOOL bLoop){ m_bLoop = bLoop; }
	// deletes the animation
  void Delete();
  // creates the animation (loads it from the disk to memory)
  // constructs the object
	CAnimation();
  // destructs the object
	virtual ~CAnimation();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	void RecomputeSize();
	CSize m_szSize;
  // Array of frames - surfaces
  CTypedPtrArray<CObArray, CDDrawSurface *> m_aFrames;
	// parses one line (format: 'VariableName=Value')
  // reads one line from the given buffer
  // moves the buffer pointer after the line
	// speed of the animation
  // delay between frames in miliseconds
  DWORD m_dwSpeed;
  // number of frames in the animation
	DWORD m_dwFrameNum;
  // default looping
  BOOL m_bLoop;
  // reads the .anim file
};

class CAnimationInstance : public CObserver
{
  DECLARE_DYNAMIC(CAnimationInstance);
  DECLARE_OBSERVER_MAP(CAnimationInstance);
public:
  // stops playing the animation
	void Stop();
  // returns the rectangle around the animation
  CRect *GetRect(){
    return &m_rcBound;
  }
  // sets the rectangle around the animation
  void SetRect(CRect &rcBound){
    m_rcBound = rcBound;
  }
  // sets the position of the animation
  // uses the size to create its rect
  void SetRect(CPoint &ptPosition){
    CSize sz = m_pAnimation->GetSize();
    m_rcBound.SetRect(ptPosition.x, ptPosition.y, ptPosition.x + sz.cx, ptPosition.y + sz.cy);
  }
  // starts animation
	void Play();

  // constructs the animation instance object
  CAnimationInstance();

  // constructs the animation instance object
  CAnimationInstance(CAnimation *pAnimation, CObserver *pObserver = NULL, int nLoop = -1, DWORD dwNotID = DefaultNotID);

  // destructor
  ~CAnimationInstance();

  // creates the animation instance
  BOOL Create(CAnimation *pAnimation, CObserver *pObserver = NULL, int nLoop = -1, DWORD dwNotID = DefaultNotID);
  
  // returns pointer to the animation object
  CAnimation *GetAnimation(){ return m_pAnimation; }

  // returns current frame of the animation
  CDDrawSurface *Frame(){
    return (m_pAnimation == NULL) ? NULL : m_pAnimation->GetFrame(m_dwFrame); }

  // sets new position in the animation
  void SetPosition(DWORD dwFrame){
    m_dwFrame = dwFrame;}

  // returns current position in the animation
  DWORD GetPosition(){
    return m_dwFrame;}

  // returns the speed of the animation
  DWORD GetSpeed(){
    ASSERT(m_pAnimation != NULL); return m_pAnimation->GetSpeed(); }
  // returns TRUE if the animation has some transparent pixels
  BOOL IsTransparent(){
    ASSERT(m_pAnimation != NULL); return m_pAnimation->IsTransparent(); }
  // returns the size of the animation biggest frame
  CSize GetSize(){
    ASSERT(m_pAnimation != NULL); return m_pAnimation->GetSize(); }

  // moves to the next frame in the animation
  void NextFrame(){
    ASSERT(m_pAnimation != NULL);
    if(m_pAnimation == NULL) return;
    m_dwFrame++;
    if(m_dwFrame >= m_pAnimation->GetFrameNum()){
      if(m_bLoop) m_dwFrame = 0; else m_dwFrame--;}
  }
  
  enum Events{
    E_REPAINT = 1 // event to repaint some rect
  };

  enum{
    DefaultNotID = 0x0FFFE000
  };

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // rectangle of the animation
  CRect m_rcBound;
  // ID of this 'notifier'
  DWORD m_dwNotID;
  // pointer to object to which send the repaint message
  CObserver *m_pObserver;
  // pointer to the animation object
  CAnimation *m_pAnimation;
  // index of current frame
  DWORD m_dwFrame;
  // TRUE - the animation will loop
  BOOL m_bLoop;
protected:
  // reaction to time tick - moves to the next frame
  // if neccessary sends the repaint message
	void OnTimeTick(DWORD dwTime);
};

#define BEGIN_ANIMS() BEGIN_NOTIFIER(CAnimationInstance::DefaultNotID)

#define END_ANIMS() END_NOTIFIER()

#define ON_ANIMSREPAINT() \
  case CAnimationInstance::E_REPAINT: \
    OnAnimsRepaint((CAnimationInstance *)dwParam); \
    return FALSE;

#endif // !defined(AFX_ANIMATION_H__BE6D3E09_9CA0_11D2_ABB1_BFAA62284960__INCLUDED_)
