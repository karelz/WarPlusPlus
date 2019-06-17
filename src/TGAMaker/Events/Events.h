// Events.h main header file for the Events.dll
//

#if !defined(EVENTS_H_)
#define EVENTS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Events\Events\EventManager.h"   // the Event manager
#include "..\Events\Events\AbortNotifier.h"  // notifier for the app abort
#include "..\Events\Events\EventWindow.h"    // frame window for the events
#include "..\Events\Events\Notifier.h"       // notifier object
#include "..\Events\Events\Observer.h"       // observer object
#include "..\Events\Events\MultithreadNotifier.h" // multithread notifier object

#endif // !defined(EVENTS_H_)
