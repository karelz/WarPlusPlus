#ifndef _GAMECLIENT_PROFILINGFRAME_H_
#define _GAMECLIENT_PROFILINGFRAME_H_

class CProfilingFrame : public CCaptionWindow
{
  DECLARE_DYNAMIC ( CProfilingFrame );
  DECLARE_OBSERVER_MAP ( CProfilingFrame );

public:
// Constructor & destructor
  // Constructor
  CProfilingFrame ();
  // Destructor
  virtual ~CProfilingFrame ();

// Debug functions
#ifdef _DEBUG
  // Asserts object validity
  virtual void AssertValid () const;
  // Dumps object's data
  virtual void Dump ( CDumpContext &dc ) const;
#endif

// Creation
  // Creates the window
  void Create ( CRect &rcBount, CWindow *pParent, CCaptionWindowLayout *pLayout,
    CNotifier * pUpdateSender );
  // Deletes the window
  virtual void Delete ();

  // Sets the profiling information
  void SetProfilingInformations ( CString strInfo );
  void SetInstructionCounters ( __int64 nLocal, __int64 nGlobal );

  // Events
  enum
  {
    E_UpdateInfo = 1, // This event is received from the script editor
                      // to update the profiling info
                      // the param is pointer to the info text

    E_UpdateCounters = 2, // This event is recieved from the script editor
                          // to update the instraction counters info
                          // in param is an array of __in64 [2], first is the local
                          // second is the global counter (must delete the array)

    E_StopUpdating = 1, // This event is sent when we don't want to recieve no more
                        // updates
  };

protected:
// Event reactions
  // The close button was pressed
  void OnClose ();

  // The window has changed the size
  virtual void OnSize ( CSize size );

  // Reaction on the update info event
  void OnUpdateInfo ( char *pInfo );
  void OnUpdateCounters ( __int64 *pCounters );

private:
// Data members
  // The list containing the profling informations
  CListControl m_wndList;

  // The close button
  CImageButton m_wndCloseButton;

  // The profiling information
  CString m_strProfiling;

  // Notifier which sends us the update events
  CNotifier * m_pUpdateSender;

  // The Control IDs
  enum
  {
    IDC_Close = 0x100,
    IDC_ScriptEditor = 0x101,
  };

// Helper functions
  // Returns the rect for the list control
  void GetListRect ( CRect * pRect );
  // Fills the list with the profiling data
  void FillList ();
  // Inserts one profiling line to the list
  void InsertProfileLine ( CString &strLine, int &nItem );
};

#endif // _GAMECLIENT_PROFILINGFRAME_H_