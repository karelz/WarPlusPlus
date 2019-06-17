#include "stdafx.h"
#include "SmartPriorityQueue.h"

struct STest
{
  DWORD dw1, dw2;
};

// explicitni vytvoreni veskereho kodu
template<> class CSmartPriorityQueue<STest>;

// instance
// CSmartPriorityQueue<STest> qTest(100, 100);


