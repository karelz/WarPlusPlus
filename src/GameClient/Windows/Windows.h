// WINDOWS.h main header file for the SystemNotifiers.dll
//

#if !defined(WINDOWS_H_)
#define WINDOWS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// dependencies
#include "Archive\Archive\Archive.h"
#include "Common\Events\Events.h"
#include "Common\DirectX\DirectX.h"
#include "Common\ConfigFile\ConfigFile\ConfigFile.h"

// our includes
#include "Animation.h"
#include "Controls.h"
#include "Fonts.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Timer.h"
#include "Window.h"

#endif // !defined(WINDOWS_H_)
