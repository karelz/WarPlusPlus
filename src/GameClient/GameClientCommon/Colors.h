#if !defined(COLORS_H_)
#define COLORS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void Color_RGB2HSV(int R, int G, int B, double &H, double &S, double &V);
void Color_HSV2RGB(double H, double S, double V, int &R, int &G, int &B);

// R, G, B are 0..255. H is 0..360, S is 0..255, V is 0..255
inline void Color_RGB2HSV_int(int R, int G, int B, int &H, int &S, int &V);
inline void Color_HSV2RGB_int(int H, int S, int V, int &R, int &G, int &B);

void Color_RGB2HSV_int(int R, int G, int B, int &H, int &S, int &V)
{
  int min, max, delta;
  int r, g, b;
  r = R << 8;
  g = G << 8;
  b = B << 8;

  min = __min(__min(r, g), b);
  max = __max(__max(r, g), b);
  V = max>>8;
  delta = max - min;
  if(max != 0){
    S = (delta * 255) / max;
  }
  else{
    S = 0;
  }
  if(delta != 0){ // chromatic case
    if(r == max){
      H = ((g - b)<<8)/delta;
    }
    else if(g == max){
      H = 0x200 + ((b - r)<<8)/delta;
    }
    else{
      H = 0x400 + ((r - g)<<8)/delta;
    }

    H *= 60; // to degrees
    if(H < 0) H += 360<<8;
	H>>=8;
  }
  else{ // achromatic case
    H = 0;
  }
}

void Color_HSV2RGB_int(int H, int S, int V, int &R, int &G, int &B)
{
  int f;
//  int p, q, t;
  int i;
  
  if(S == 0){ // achromatic case
    R = V; G = V; B = V;
  }
  else{  // chromatic case
    if(H == 360) H = 0;
    i = H/60;
    f = H%60;

//	p = (V * (255 - S))/255;
//  q = (V * (255*59 - S * f))/(255*59);
//  t = (V * (255*59 - S * (59 - f)))/(255*59);

    switch(i){
    case 0:
      R = V; G = /*t*/ (V * (255*59 - S * (59 - f)))/(255*59); B = /*p*/ (V * (255 - S))/255;break;
    case 1:
      R = /*q*/ (V * (255*59 - S * f))/(255*59); G = V; B = /*p*/ (V * (255 - S))/255; break;
    case 2:
      R = /*p*/ (V * (255 - S))/255; G = V; B = /*t*/ (V * (255*59 - S * (59 - f)))/(255*59); break;
    case 3:
      R = /*p*/ (V * (255 - S))/255; G = /*q*/ (V * (255*59 - S * f))/(255*59); B = V; break;
    case 4:
      R = /*t*/ (V * (255*59 - S * (59 - f)))/(255*59); G = /*p*/ (V * (255 - S))/255; B = V; break;
    case 5:
      R = V; G = /*p*/ (V * (255 - S))/255; B = /*q*/ (V * (255*59 - S * f))/(255*59); break;
    }
  }
}

#endif // !defined(COLORS_H_)