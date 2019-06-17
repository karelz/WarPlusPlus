// Keyboard.h: interface for the CKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYBOARD_H__8F2050C2_7C95_11D2_AB60_D89D541E9E61__INCLUDED_)
#define AFX_KEYBOARD_H__8F2050C2_7C95_11D2_AB60_D89D541E9E61__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "KeyboardWindow.h"
#include "KeyboardNotifier.h"

class CWindow;

class CKeyboard : public CNotifier  
{
  DECLARE_DYNAMIC(CKeyboard)
  DECLARE_OBSERVER_MAP(CKeyboard)

private:
  struct tagSKeyboardConnectionData{
    typedef enum ConnectionTypes{
      Con_Window = 0,
      Con_Global = 1
    } EConnectionTypes;

    EConnectionTypes m_eType; // type of the connection
  };
  typedef struct tagSKeyboardConnectionData SKeyboardConnectionData;

public:
  struct tagSKeyInfo{
    UINT nChar;
    DWORD dwFlags;
    BYTE nASCII;
  };
  typedef struct tagSKeyInfo SKeyInfo;

public:
	// returns TRUE if the Alt key is pressed
  BOOL AltPressed();
	// returns TRUE if the Ctrl key is pressed
	BOOL CtrlPressed();
	// returns TRUE if the Shift key is pressed
	BOOL ShiftPressed();
  // connects some window object to the keyboard
  // this connection takes care of focus
	void Connect(CWindow *pWindow, DWORD dwNotID = DefaultNotID);
  // connects some observer to the keyboard
  // doesn;t take care of focus
  // all messages will be recieved
  void ConnectGlobal(CObserver *pObserver, DWORD dwNotID = DefaultNotID);
  // deletes the object
	void Delete();
  // creates the object for givven main window
	BOOL Create(CKeyboardWindow *pKeyboardWindow);
  // static Close - deletes the global keyboard object
	static void Close();
  // static Init - creates the global keyboard object
  // parameter should be the main app window
	static BOOL Init(CKeyboardWindow *pKeyboardWindow);
  // constructor
	CKeyboard();
  // destructor
	virtual ~CKeyboard();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum { DefaultNotID = 0x0FFFF0003 };
  enum Events{
    E_KEYDOWN = 1,
    E_KEYUP = 2,
    E_KEYACCELERATOR = 3,
    E_CHAR = 4
  };

  enum AltKeys{
    LShiftDown = 0x00001,
    RShiftDown = 0x00002,
    ShiftDown  = 0x00003,
    LCtrlDown  = 0x00004,
    RCtrlDown  = 0x00008,
    CtrlDown   = 0x0000C,
    LAltDown   = 0x00010,
    RAltDown   = 0x00020,
    AltDown    = 0x00030
  };  
private:
	void SendKeyEvent(DWORD dwEvent, DWORD dwParam);
	// pointer to the main app window
  // this window will recieve the Windows keyboard messages
  CKeyboardWindow * m_pWindow;
  // Notifier object to send events from keyboard to keyboard
  // (we can resolve the destination of the event only at the time
  // it's beeing processed, not at time of its creation)
  CKeyboardNotifier m_KeyboardNotifier;

  enum { KeyboardNotifierID = 0x0EFFFFFFF };

  friend CKeyboardWindow;

protected:
  // reactions on the event -> redirect the event to the window with focus
  void OnChar(DWORD dwKey);
	void OnKeyUp(DWORD dwKey);
	void OnKeyDown(DWORD dwKey);
	// reaction on the abnormal program termination
  // deletes the object
  void OnAbort(DWORD dwExitCode);
  // removes some aditional information for the connection
	virtual void DeleteNode(SObserverNode *pNode);
};

// global pointer to the keyboard object
extern CKeyboard *g_pKeyboard;

#define BEGIN_KEYBOARD() BEGIN_NOTIFIER(CKeyboard::DefaultNotID)
#define END_KEYBOARD() END_NOTIFIER()
#define ON_KEYDOWN() \
    case CKeyboard::E_KEYDOWN: { \
      CKeyboard::SKeyInfo *pInfo = (CKeyboard::SKeyInfo *)dwParam; \
      if(!OnKeyDown(pInfo->nChar, pInfo->dwFlags)) return FALSE; \
      delete pInfo; \
      return TRUE; }
#define ON_KEYUP() \
    case CKeyboard::E_KEYUP: { \
      CKeyboard::SKeyInfo *pInfo = (CKeyboard::SKeyInfo *)dwParam; \
      if(!OnKeyUp(pInfo->nChar, pInfo->dwFlags)) return FALSE; \
      delete pInfo; \
      return TRUE; }
#define ON_KEYACCELERATOR() \
    case CKeyboard::E_KEYACCELERATOR: { \
      CKeyboard::SKeyInfo *pInfo = (CKeyboard::SKeyInfo *)dwParam; \
      if(!OnKeyAccelerator(pInfo->nChar, pInfo->dwFlags)) return FALSE; \
      delete pInfo; \
      return TRUE; }
#define ON_CHAR()\
    case CKeyboard::E_CHAR: { \
      CKeyboard::SKeyInfo *pInfo = (CKeyboard::SKeyInfo *)dwParam; \
      if(!OnChar(pInfo->nChar, pInfo->dwFlags)) return FALSE; \
      delete pInfo; \
      return TRUE; }

#endif // !defined(AFX_KEYBOARD_H__8F2050C2_7C95_11D2_AB60_D89D541E9E61__INCLUDED_)
