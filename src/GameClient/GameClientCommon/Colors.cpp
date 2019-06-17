#include "stdafx.h"
#include "Colors.h"

void Color_RGB2HSV(int R, int G, int B, double &H, double &S, double &V)
{
  double min, max, delta;
  double r, g, b;
  r = (double)R / 255;
  g = (double)G / 255;
  b = (double)B / 255;

  min = __min(__min(r, g), b);
  max = __max(__max(r, g), b);
  V = max;
  delta = max - min;
  if(max != 0){
    S = delta / max;
  }
  else{
    S = 0;
  }
  if(delta != 0){ // chromatic case
    if(r == max){
      H = (g - b)/delta;
    }
    else if(g == max){
      H = 2 + (b - r)/delta;
    }
    else{
      H = 4 + (r - g)/delta;
    }

    H *= 60; // to degrees
    if(H < 0) H += 360;
  }
  else{ // achromatic case
    H = 0;
  }
}

void Color_HSV2RGB(double H, double S, double V, int &R, int &G, int &B)
{
  double r, g, b;
  double f /* , p, q, t */;
  int i;

  // Roman - urychleni. Misto, abychom nasobili r,g,b 255-ti na konci
  // vynasobime si V 255-ti uz ted. Usetrime dve nasobeni
  V*=255;
  
  if(S == 0){ // achromatic case
    r = V; g = V; b = V;
  }
  else{  // chromatic case
    if(H == 360) H = 0;
    H /= 60;
    i = (int)H;
    f = H - (double)i;

	/* Puvodne jsme si predpocitavali p, q, t.
	   Nyni to delame jenom pokud to je opravdu treba.. 
	   usetrime nejmene dve nasobeni a dve odcitani
	p = V * (1 - S);
    q = V * (1 - S * f);
    t = V * (1 - S * (1 - f));
	*/

    switch(i){
    case 0:
      r = V; b = /*p*/ V * (1 - S); g = /*t*/ b + V * S * f; break;
    case 1:
      r = /*q*/ V * (1 - S * f); g = V; b = /*p*/ V * (1 - S); break;
    case 2:
      r = /*p*/ V * (1 - S); g = V; b = /*t*/ r + V * S * f; break;
    case 3:
      r = /*p*/ V * (1 - S); g = /*q*/ V * (1 - S * f); b = V; break;
    case 4:
      g = /*p*/ V * (1 - S); r = /*t*/ g + V * S * f; b = V; break;
    case 5:
      r = V; g = /*p*/ V * (1 - S); b = /*q*/ V * (1 - S * f); break;
    }
  }

  R = (int)r;
  G = (int)g;
  B = (int)b;
}

