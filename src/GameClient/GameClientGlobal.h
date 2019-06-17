#ifndef GAMECLIENT_GLOBAL_H_
#define GAMECLIENT_GLOBAL_H_

// This means that animations (and unit moves) estimates, that
// timeslice is 100 milliseconds long.
// It mean sthat animation speed are taken as 100 speed units per
// timeslice
// These units we'll call animseconds
#define GAMECLIENT_TIMESLICE_LENGTH 100

class CCCivilization;
class CCMap;

// The only one map on the client
extern CCMap *g_pMap;

// Civilization the client is logged on
extern CCCivilization *g_pCivilization;

#endif