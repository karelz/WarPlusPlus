// DirectSound.h: interface for the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIRECTSOUND_H__E65A6AB4_9E29_11D2_ABB3_DA1727655660__INCLUDED_)
#define AFX_DIRECTSOUND_H__E65A6AB4_9E29_11D2_ABB3_DA1727655660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmreg.h>
#include <dsound.h>
#include "DirectSoundException.h"
#include "..\..\Events\Events.h"

class CDSoundBuffer;

class CDirectSound : public CObserver
{
  DECLARE_DYNAMIC(CDirectSound);
  DECLARE_OBSERVER_MAP(CDirectSound);

public:
  // creates this object with dummy data
  // it means it will behave as normal, but no sound will be produced
  // USE it if the normal Create did not succeeded
  // All other calls can be the same as if the Create succeeded
	void CreateDummy();
	// deletes the sound buffer
  void DeleteSoundBuffer(CDSoundBuffer *pDSoundBuffer);
  // creates the sound buffer
  // some sound buffers need the pDSBD parameter to be NULL
	BOOL CreateSoundBuffer(CDSoundBuffer *pDSoundBuffer, DSBUFFERDESC *pDSBD = NULL);
  // returns pointer to IDirectSound interface
  // !!!!  don't forget to call Release on it
  // after you finish using it
	LPDIRECTSOUND GetLP();
  // deletes the object
  // deinitialize DirectSound
	virtual void Delete();
	// creates the DirectSound object
  // it initialize the DirectSound
  // as pWindow give it the main window of your app
  BOOL Create(CWnd *pWindow, DWORD dwFlags = DSSCL_NORMAL);
	// creates the object
  CDirectSound();
  // destructs the object
	virtual ~CDirectSound();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
private:
	BOOL m_bDummy;
	CDSoundBuffer * m_pBuffers;
  // place to store the results of operations
	HRESULT m_hResult;
  // pointer fo IDirectSound interface
	LPDIRECTSOUND m_lpDirectSound;
protected:
	void OnAbort(DWORD dwExitCode);
};

extern CDirectSound *g_pDirectSound;

#endif // !defined(AFX_DIRECTSOUND_H__E65A6AB4_9E29_11D2_ABB3_DA1727655660__INCLUDED_)
