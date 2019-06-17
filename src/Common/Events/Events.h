// Events.h main header file for the Events.dll
//

#if !defined(EVENTS_H_)
#define EVENTS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common\MemoryPool\TypedMemoryPool.h"

#include "Common\Events\Events\EventManager.h"   // the Event manager
#include "Common\Events\Events\AbortNotifier.h"  // notifier for the app abort
#include "Common\Events\Events\EventWindow.h"    // frame window for the events
#include "Common\Events\Events\Notifier.h"       // notifier object
#include "Common\Events\Events\Observer.h"       // observer object
#include "Common\Events\Events\MultithreadNotifier.h" // multithread notifier object

#endif // !defined(EVENTS_H_)
