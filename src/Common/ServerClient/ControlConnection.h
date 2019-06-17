#ifndef SC_CONTROLCONNECTION_H_
#define SC_CONTROLCONNECTION_H_

enum EControlRequest{
  ControlRequest_Init = 1, // Initialization (followed by nothing)
};

enum EControlAnswer{
  ControlAnswer_Init = 1, // Initialization, followed by SControlAnswerInit
};

struct tagSControlAnswerInit
{
  DWORD m_dwCivilizationID; // Civilization ID of the connected client
  DWORD m_dwTimeslice; // Current timeslice number on the server
  DWORD m_dwTimesliceInterval; // Length of the timeslice in milliseconds
  DWORD m_dwStartPositionX;  // Start position of the viewport
  DWORD m_dwStartPositionY;
};
typedef tagSControlAnswerInit SControlAnswerInit;

#endif