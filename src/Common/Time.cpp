// Time functions

#include "stdafx.h"

__int64 GetPreciseTimeOfThread ()
{
  FILETIME t1, t2, t3, t4;
  GetThreadTimes ( GetCurrentThread () , &t1, &t2, &t3, &t4 );

  return (*(reinterpret_cast < __int64 * > ( &t3 ) ) + * ( reinterpret_cast < __int64 * > ( &t4 ) ));
}