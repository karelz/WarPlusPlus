// DSoundBuffer.cpp: implementation of the CDSoundBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSoundBuffer.h"

#include "DirectSoundException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDSoundBuffer, CObject);

CDSoundBuffer::CDSoundBuffer()
{
  m_lpDSoundBuffer = NULL;
  m_bCallRelease = FALSE;
  m_bRemoveFromList = FALSE;
  m_pNextSoundBuffer = NULL;
  m_pDirectSound = NULL;

  m_dwLength = 0;
  m_dwChannels = 2;
  m_dwSampleRate = 44100;
  m_dwBitsPerSample = 16;

  m_bDummy = FALSE;
}

CDSoundBuffer::~CDSoundBuffer()
{
  Delete();
}

#ifdef _DEBUG

void CDSoundBuffer::AssertValid() const
{
  CObject::AssertValid();

  if(m_bDummy) return;
  ASSERT(m_lpDSoundBuffer != NULL);
}

void CDSoundBuffer::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);

  if(m_bDummy){
    dc << "Dummy DirectSoundBuffer object.\n";
  }
  else{
    dc << "IDirectSoundBuffer pointer : " << m_lpDSoundBuffer << "\n";
  }
}

#endif


void CDSoundBuffer::Delete()
{
  if(m_bDummy){
    m_bDummy = FALSE;
    return;
  }

  if(m_lpDSoundBuffer){
    if(m_bCallRelease)
      m_lpDSoundBuffer->Release();
    m_bCallRelease = FALSE;

    m_lpDSoundBuffer = NULL;

    if(m_bRemoveFromList){
      ASSERT(m_pDirectSound != NULL);
      if(m_pDirectSound) m_pDirectSound->DeleteSoundBuffer(this);
    }
  }
}

BOOL CDSoundBuffer::PreCreate(DSBUFFERDESC *pDSBD, PCMWAVEFORMAT *lppcmwf)
{
  if(m_bDummy) return TRUE;

  lppcmwf->wf.nChannels = (WORD)m_dwChannels;
  lppcmwf->wf.nSamplesPerSec = m_dwSampleRate;
  lppcmwf->wf.nBlockAlign = (WORD)(m_dwChannels * (m_dwBitsPerSample / 8));
  lppcmwf->wf.nAvgBytesPerSec = 
    lppcmwf->wf.nSamplesPerSec * lppcmwf->wf.nBlockAlign;
  lppcmwf->wBitsPerSample = (WORD)m_dwBitsPerSample;
  
  pDSBD->dwFlags = DSBCAPS_CTRLDEFAULT;
  pDSBD->dwBufferBytes = m_dwLength * lppcmwf->wf.nBlockAlign;
  
  return TRUE;
}

BOOL CDSoundBuffer::PostCreate(LPDIRECTSOUNDBUFFER lpDSBuffer, BOOL bCallRelease)
{
  if(m_bDummy) return TRUE;

  m_lpDSoundBuffer = lpDSBuffer;
  m_bCallRelease = bCallRelease;

  return TRUE;
}

LPDIRECTSOUNDBUFFER CDSoundBuffer::GetLP()
{
  if(m_bDummy) return NULL;
  if(m_lpDSoundBuffer == NULL) return NULL;

  m_lpDSoundBuffer->AddRef();
  return m_lpDSoundBuffer;
}

BOOL CDSoundBuffer::Create()
{
  ASSERT(g_pDirectSound != NULL);
  ASSERT_VALID(g_pDirectSound);

  if(!g_pDirectSound->CreateSoundBuffer(this)) return FALSE;

  return TRUE;
}

void CDSoundBuffer::SetFormat(DWORD dwChannels, DWORD dwBitsPerSample, DWORD dwSampleRate)
{
  if(m_lpDSoundBuffer != NULL) return;

  m_dwChannels = dwChannels;
  m_dwBitsPerSample = dwBitsPerSample;
  m_dwSampleRate = dwSampleRate;
}

void CDSoundBuffer::SetLength(DWORD dwLength)
{
  if(m_lpDSoundBuffer != NULL) return;

  m_dwLength = dwLength;
}

void CDSoundBuffer::WriteToBuffer(LPBYTE lpData, DWORD dwSize, DWORD dwOffset)
{
  LPVOID lpVoid1, lpVoid2;
  DWORD dwSize1, dwSize2;

  if(m_bDummy) return;

  m_hResult = m_lpDSoundBuffer->Lock(dwOffset, dwSize, &lpVoid1, &dwSize1, 
    &lpVoid2, &dwSize2, 0);
  if(m_hResult == DSERR_BUFFERLOST){
    Restore();
    m_hResult = m_lpDSoundBuffer->Lock(dwOffset, dwSize, &lpVoid1, &dwSize1, 
      &lpVoid2, &dwSize2, 0);
  }
  DIRECTSOUND_ERROR(m_hResult);

  CopyMemory(lpVoid1, lpData, dwSize1);
  if(lpVoid2 != NULL) CopyMemory(lpVoid2, lpData + dwSize1, dwSize2);

  m_hResult = m_lpDSoundBuffer->Unlock(lpVoid1, dwSize1, lpVoid2, dwSize2);
  DIRECTSOUND_ERROR(m_hResult);
}

void CDSoundBuffer::Restore()
{
  if(m_bDummy) return;
  // just restore the buffer in memory
  // some inherited classes will also restore the data in the memory
  // for exanple they will reload the sample from the disk
  m_lpDSoundBuffer->Restore();
}

void CDSoundBuffer::Play()
{
  if(m_bDummy) return;

  m_hResult = m_lpDSoundBuffer->Play(0, 0, 0);
  if(m_hResult == DSERR_BUFFERLOST){
    Restore();
    m_hResult = m_lpDSoundBuffer->Play(0, 0, 0);
  }
  DIRECTSOUND_ERROR(m_hResult);
}

void CDSoundBuffer::Stop()
{
  if(m_bDummy) return;

  m_hResult = m_lpDSoundBuffer->Stop();
  DIRECTSOUND_ERROR(m_hResult);
}


DWORD CDSoundBuffer::GetFrequency()
{
  DWORD dwFrequency;

  if(m_bDummy) return 0;

  m_hResult = m_lpDSoundBuffer->GetFrequency(&dwFrequency);
  DIRECTSOUND_ERROR(m_hResult);

  return dwFrequency;
}

void CDSoundBuffer::SetFrequency(DWORD dwFrequency)
{
  if(m_bDummy) return;

  m_hResult = m_lpDSoundBuffer->SetFrequency(dwFrequency);
  DIRECTSOUND_ERROR(m_hResult);
}

LONG CDSoundBuffer::GetPan()
{
  LONG lPan;

  if(m_bDummy) return 0;

  m_hResult = m_lpDSoundBuffer->GetPan(&lPan);
  DIRECTSOUND_ERROR(m_hResult);

  return lPan;
}

void CDSoundBuffer::SetPan(LONG lPan)
{
  if(m_bDummy) return;

  m_hResult = m_lpDSoundBuffer->SetPan(lPan);
  DIRECTSOUND_ERROR(m_hResult);
}


LONG CDSoundBuffer::GetVolume()
{
  LONG lVolume;

  if(m_bDummy) return 0;

  m_hResult = m_lpDSoundBuffer->GetVolume(&lVolume);
  DIRECTSOUND_ERROR(m_hResult);

  return lVolume;
}

void CDSoundBuffer::SetVolume(LONG lVolume)
{
  if(m_bDummy) return;

  m_hResult = m_lpDSoundBuffer->SetVolume(lVolume);
  DIRECTSOUND_ERROR(m_hResult);
}
