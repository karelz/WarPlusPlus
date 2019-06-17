// GameServerAppDoc.h : interface of the CGameServerAppDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMESERVERAPPDOC_H__8B3C8564_5C99_11D4_B527_00105ACA8325__INCLUDED_)
#define AFX_GAMESERVERAPPDOC_H__8B3C8564_5C99_11D4_B527_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WM_PROFILEUPDATED WM_USER + 20

class CGameServerAppDoc : public CDocument
{
protected: // create from serialization only
	CGameServerAppDoc();
	DECLARE_DYNCREATE(CGameServerAppDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGameServerAppDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	//}}AFX_VIRTUAL

  // returns TRUE if the doc is empty (no game is initialized)
  BOOL IsEmpty(){ return m_eGameStatus == Game_None; }

  // Refreshes the state of frame controls (status bar ...)
  void RefreshFrameState();
  // Post create called from InitInstance after all is done
  void PostCreate();

  // Creates the new game from TheMap file
  void CreateGame(CDataArchive TheMap);
  // Stops the current game (if any)
  void StopGame();

  // Loads the game from given archive
  void LoadGame(CDataArchive Archive);
  // Saves the game to given archive
  void SaveGame(CDataArchive Archive);

  // Unpause the game
  void UnpauseGame();
  // Pause game
  void PauseGame();

  // Fills the users tree ctrl
  void FillUsersTree(CTreeCtrl *pTree);

  // Show User properties dialog
  void OnUserProperties(DWORD dwUserID);
  // Show Civilization properties dialog
  void OnCivilizationProperties(CSCivilization *pCiv);

  // Adds new user to given civilization
  void OnAddUser(CSCivilization *pCiv);
  // Deletes user
  void OnDeleteUser(DWORD dwUserID);
  // Disables user
  void OnDisableUser(DWORD dwUserID);
  // Enables user
  void OnEnableUser(DWORD dwUserID);
  // returns TRUE if the user is enabled
  BOOL IsUserEnabled(DWORD dwUserID);
  // returns TRUE if the user is logged on
  BOOL IsUserLoggedOn(DWORD dwUserID);

  // register the window to recieve the WM_PROFILEUPDATE message
  // when the profiling counters had been updated
  void RegisterProfileObserver ( CWnd * pWindow );
  // unregisters the window from the profiling events
  void UnregisterProfileObserver ( CWnd * pWindow );

  // Send a message to user
  void OnUserSendMessage(DWORD dwUserID);
  // Send a message to all users in civilization
  void OnCivilizationSendMessage(CSCivilization *pCiv);

  // Initializes the game
  void InitGame();
  // Closes the game
  void CloseGame();

  // Writes all setting to the registry
  void WriteSettingToRegistry();
  // Reads all setting from the registry
  void ReadSettingsFromRegistry();

  // Resturns the falg if to show the profiling in the main view
  bool ShowProfilingInTheMainView () { return m_bShowProfiling; }

  enum EGameStatus{
    Game_None = 0, // No game initialized
    Game_Paused = 1, // The game is paused
    Game_Unpaused = 2, // The game is in progress
  };

  enum{
    Event_UserLoggedOn = 1,
    Event_UserLoggedOff = 2,
    Event_Timeslice = 3,
  };

  LRESULT OnMapObserverEvent(WPARAM, LPARAM);

  // Deletes all file and subdirectories in directory
  BOOL CleanDirectory ( LPCTSTR lpcszDirectory );

// Implementation
public:
	virtual ~CGameServerAppDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGameServerAppDoc)
	afx_msg void OnUpdateGamePause(CCmdUI* pCmdUI);
	afx_msg void OnGamePause();
	afx_msg void OnUpdateGameStop(CCmdUI* pCmdUI);
	afx_msg void OnGameStop();
	afx_msg void OnGameUnpause();
	afx_msg void OnUpdateGameUnpause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewOptions(CCmdUI* pCmdUI);
	afx_msg void OnViewOptions();
	afx_msg void OnFileSave();
	afx_msg void OnScriptSetMaker();
	afx_msg void OnUpdateScriptSetMaker(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGameSendMessage(CCmdUI* pCmdUI);
	afx_msg void OnGameSendMessage();
	afx_msg void OnCivilizationResetProfiling();
	afx_msg void OnUpdateCivilizationResetProfiling(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  static void CALLBACK ProfileClearTimerProc ( HWND, UINT, UINT_PTR, DWORD );

public:
  // The current game status
  EGameStatus m_eGameStatus;

private:

  struct SProfileObserverNode
  {
    CWnd * m_pWindow;
    struct SProfileObserverNode * m_pNext;
  };
  static SProfileObserverNode * m_pProfileObserverNodes;
  int m_nProfilingTimer;

  // The timeslice length
  DWORD m_dwTimesliceLength;

  // Paths
  // The temporary directory for the game (local script sets and so on...)
  CString m_strGameTempDir;
  // The data directory for the game (global script sets, namespaces...)
  CString m_strGameDataDir;
  // The temporary directory for the archives
  CString m_strArchiveTempDir;

  // Sets the status bar pane to match given game status
	void SetGameStatusPane(EGameStatus eStatus);
  // Sets the timeslice pane (if dwTimeslice == 0 - empty pane)
  void SetTimeslicePane(DWORD dwTimeslice);

  // Sets the status bar text
  void SetStatusText(UINT nID);

  // Show the profiling info in the main view
  bool m_bShowProfiling;
  // Show the tra notification messages
  bool m_bShowTrayInfos;

// Network
  // The network server object
  CNetworkServer m_NetworkServer;
  // The port to listen on
  WORD m_wServerPort;

  friend class CGameServerMapObserver;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GAMESERVERAPPDOC_H__8B3C8564_5C99_11D4_B527_00105ACA8325__INCLUDED_)
