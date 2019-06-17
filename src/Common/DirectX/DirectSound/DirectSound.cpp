// DirectSound.cpp: implementation of the CDirectSound class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirectSound.h"
#include "DirectSoundException.h"
#include "DSoundBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CDirectSound, CObserver);

BEGIN_OBSERVER_MAP(CDirectSound, CObserver)
  BEGIN_ABORT()
    ON_ABORT()
  END_ABORT()
END_OBSERVER_MAP(CDirectSound, CObserver)


CDirectSound::CDirectSound()
{
  m_lpDirectSound = NULL;
  m_pBuffers = NULL;
  m_bDummy = FALSE;
}

CDirectSound::~CDirectSound()
{
  Delete();
}

CDirectSound *g_pDirectSound = NULL;

#ifdef _DEBUG

void CDirectSound::AssertValid() const
{
  CObserver::AssertValid();

  if(m_bDummy) return;
  ASSERT(m_lpDirectSound != NULL);
}

void CDirectSound::Dump(CDumpContext &dc) const
{
  CObserver::Dump(dc);

  if(m_bDummy){
    dc << "Dummy DirectSound object.\n";
  }
  else{
    dc << "IDirectSound pointer : " << m_lpDirectSound << "\n";
  }
}

#endif

BOOL CDirectSound::Create(CWnd *pWindow, DWORD dwFlags)
{
  ASSERT(g_pDirectSound == NULL);
  
  m_hResult = DirectSoundCreate(NULL, &m_lpDirectSound, NULL);
  DIRECTSOUND_ERROR(m_hResult);

  m_hResult = m_lpDirectSound->SetCooperativeLevel(pWindow->GetSafeHwnd(), dwFlags);
  DIRECTSOUND_ERROR(m_hResult);

  g_pDirectSound = this;

  // we will react to the abort by destroying our object
  g_AbortNotifier.Connect(this);

  return TRUE;
}

void CDirectSound::Delete()
{
  if(m_bDummy){
    m_bDummy = FALSE;
    return;
  }
  
  if(m_lpDirectSound){
    // no more abort notifications needed
    g_AbortNotifier.Disconnect(this);

    ASSERT(g_pDirectSound == this);
    g_pDirectSound = NULL;
    m_lpDirectSound->Release();
    m_lpDirectSound = NULL;
  }
}

LPDIRECTSOUND CDirectSound::GetLP()
{
  if(m_bDummy) return NULL;

  m_lpDirectSound->AddRef();
  return m_lpDirectSound;
}

BOOL CDirectSound::CreateSoundBuffer(CDSoundBuffer *pDSoundBuffer, DSBUFFERDESC *pDSBD)
{
  PCMWAVEFORMAT pcmwf;
  DSBUFFERDESC dsbdesc;
  LPDIRECTSOUNDBUFFER lpDSBuffer;

  if(!m_bDummy){
    if(pDSBD == NULL){
      memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
      pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
      
      memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
      dsbdesc.dwSize = sizeof(DSBUFFERDESC);
      dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
      
      if(!pDSoundBuffer->PreCreate(&dsbdesc, &pcmwf)) return FALSE;
      
      pDSBD = &dsbdesc;
    }
    
    m_hResult = m_lpDirectSound->CreateSoundBuffer(pDSBD, &lpDSBuffer, NULL);
    DIRECTSOUND_ERROR(m_hResult);
    
    if(!pDSoundBuffer->PostCreate(lpDSBuffer)) return FALSE;
  }

  if(m_bDummy){
    pDSoundBuffer->m_bDummy = TRUE;
  }

  // Add the buffer to the list of all surfaces
  pDSoundBuffer->m_pDirectSound = this;
  pDSoundBuffer->m_pNextSoundBuffer = m_pBuffers;
  m_pBuffers = pDSoundBuffer;

  pDSoundBuffer->m_bRemoveFromList = TRUE;

  return TRUE;
}

void CDirectSound::DeleteSoundBuffer(CDSoundBuffer *pDSoundBuffer)
{
  CDSoundBuffer *pCurrent;

  if(m_pBuffers == pDSoundBuffer){
    m_pBuffers = pDSoundBuffer->m_pNextSoundBuffer;
    return;
  }

  pCurrent = m_pBuffers;
  while(pCurrent->m_pNextSoundBuffer != pDSoundBuffer){
    if(pCurrent->m_pNextSoundBuffer == NULL){
      ASSERT(FALSE);  // Given buffer was not created by us
      return; // We failed to find it
    }
    pCurrent = pCurrent->m_pNextSoundBuffer;
  }
  // remove it from list
  pCurrent->m_pNextSoundBuffer = pDSoundBuffer->m_pNextSoundBuffer;

}


// on abort we will destroy our object
void CDirectSound::OnAbort(DWORD dwExitCode)
{
  Delete();
}

// creates this object with dummy data
// it means it will behave as normal, but no sound will be produced
void CDirectSound::CreateDummy()
{
  m_bDummy = TRUE;

  ASSERT(g_pDirectSound == NULL);
  g_pDirectSound = this;
}
