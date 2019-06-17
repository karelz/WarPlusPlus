// DSoundBuffer.h: interface for the CDSoundBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DSOUNDBUFFER_H__E65A6AB6_9E29_11D2_ABB3_DA1727655660__INCLUDED_)
#define AFX_DSOUNDBUFFER_H__E65A6AB6_9E29_11D2_ABB3_DA1727655660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DirectSound.h"

class CDSoundBuffer : public CObject  
{
  DECLARE_DYNAMIC(CDSoundBuffer);

public:
	// sets the volume for the buffer
  void SetVolume(LONG lVolume);
	// returns the volume of the buffer
  LONG GetVolume();
	// sets panning for the buffer
  void SetPan(LONG lPan);
	// returns the panning of the buffer
  LONG GetPan();
	// sets the frequency in samples per second for the buffer
  void SetFrequency(DWORD dwFrequency);
	// returns the frequency of the buffer
  DWORD GetFrequency();
	// stops playing the buffer
  void Stop();
  // starts playing the buffer
  void Play();
	// restores the buffer in memory
  virtual void Restore();
	// writes data to buffer at given position
  void WriteToBuffer(LPBYTE lpData, DWORD dwSize, DWORD dwOffset);
	// sets the length of the sample
  // must be called only before create
  void SetLength(DWORD dwLength);
  // sets the format of this buffer
  // it must be called before the create
  // after, it does nothing
	void SetFormat(DWORD dwChannels, DWORD dwBitsPerSample, DWORD dwSampleRate);
  // creates the buffer
  // it just calls the g_pDirectSound->CreateSoundBuffer
  BOOL Create();
  // returns pointer to IDirectSoundBuffer interface
  // !!!!! don't forget to call Release on it
  // after you finish using it
	LPDIRECTSOUNDBUFFER GetLP();
  // post create - used internally
	virtual BOOL PostCreate(LPDIRECTSOUNDBUFFER lpDSBuffer, BOOL bCallRelease = TRUE);
  // pre create - used internally
	virtual BOOL PreCreate(DSBUFFERDESC *pDSBD, PCMWAVEFORMAT *lppcmwf);
  // deletes the object
	virtual void Delete();
	// constructs the object
  CDSoundBuffer();
  // destructs the object
	virtual ~CDSoundBuffer();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // TRUE - the object is just the dummy one - no real data
  BOOL m_bDummy;
	// place to store results of operations
  HRESULT m_hResult;
  // pointer do IDirectSoundBuffer interface
	LPDIRECTSOUNDBUFFER m_lpDSoundBuffer;
  // length of the buffer in samples
	DWORD m_dwLength;
  // number of bits per one sample
	// (8, 16)
  DWORD m_dwBitsPerSample;
  // sample rate of the buffer
  // number of samples per second
	DWORD m_dwSampleRate;
  // number of channels in this buffer
  // (1 - mono, 2 - stereo)
	DWORD m_dwChannels;
private:
	// the DirectSound object pointer
  CDirectSound * m_pDirectSound;
	// next buffer in the list of buffers
  CDSoundBuffer * m_pNextSoundBuffer;
  // TRUE - remove the buffer from the list of buffers when deleting
  BOOL m_bRemoveFromList;
  // TRUE - call release when deleting the object
  BOOL m_bCallRelease;

  friend CDirectSound;
};

#endif // !defined(AFX_DSOUNDBUFFER_H__E65A6AB6_9E29_11D2_ABB3_DA1727655660__INCLUDED_)
