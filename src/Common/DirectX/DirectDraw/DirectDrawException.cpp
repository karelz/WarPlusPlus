// DirectDrawException.cpp: implementation of the CDirectDrawException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ddraw.h>
#include "DirectDrawException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectDrawException::CDirectDrawException(HRESULT hResult)
{
  m_hResult = hResult;

#ifdef _DEBUG
  char txt[256];

  GetErrorMessage(txt, 255, 0);
  TRACE("DirectDraw exception : %s \n", txt);
#endif
}

CDirectDrawException::CDirectDrawException(CDirectDrawException &source)
{
  m_hResult = source.m_hResult;
}

CDirectDrawException::~CDirectDrawException()
{

}

BOOL CDirectDrawException::GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
  CString txt, msg;

  // Translate the error code to the string
  switch(m_hResult){
  case DD_OK: txt = "No error."; break;
  case DDERR_ALREADYINITIALIZED: txt = "The object has already been initialized."; break;
  case DDERR_BLTFASTCANTCLIP: txt = "A DirectDrawClipper object is attached to a source surface that has passed into a call to the IDirectDrawSurface2::BltFast method."; break;
  case DDERR_CANNOTATTACHSURFACE: txt = "A surface cannot be attached to another requested surface."; break;
  case DDERR_CANNOTDETACHSURFACE: txt = "A surface cannot be detached from another requested surface."; break;
  case DDERR_CANTCREATEDC: txt = "Windows cannot create any more device contexts (DCs)."; break;
  case DDERR_CANTDUPLICATE: txt = "Primary and 3D surfaces, or surfaces that are implicitly created, cannot be duplicated."; break;
  case DDERR_CANTLOCKSURFACE: txt = "Access to this surface is refused because an attempt was made to lock the primary surface without DCI support."; break;
  case DDERR_CANTPAGELOCK: txt = "An attempt to page lock a surface failed. Page lock will not work on a display-memory surface or an emulated primary surface."; break;
  case DDERR_CANTPAGEUNLOCK: txt = "An attempt to page unlock a surface failed. Page unlock will not work on a display-memory surface or an emulated primary surface."; break;
  case DDERR_CLIPPERISUSINGHWND: txt = "An attempt was made to set a clip list for a DirectDrawClipper object that is already monitoring a window handle."; break;
  case DDERR_COLORKEYNOTSET: txt = "No source color key is specified for this operation."; break;
  case DDERR_CURRENTLYNOTAVAIL: txt = "No support is currently available."; break;
  case DDERR_DCALREADYCREATED: txt = "A device context (DC) has already been returned for this surface. Only one DC can be retrieved for each surface."; break;
  case DDERR_DIRECTDRAWALREADYCREATED: txt = "A DirectDraw object representing this driver has already been created for this process."; break;
  case DDERR_EXCEPTION: txt = "An exception was encountered while performing the requested operation."; break;
  case DDERR_EXCLUSIVEMODEALREADYSET: txt = "An attempt was made to set the cooperative level when it was already set to exclusive."; break;
  case DDERR_GENERIC: txt = "There is an undefined error condition."; break;
  case DDERR_HEIGHTALIGN: txt = "The height of the provided rectangle is not a multiple of the required alignment."; break;
  case DDERR_HWNDALREADYSET: txt = "The DirectDraw cooperative level window handle has already been set. It cannot be reset while the process has surfaces or palettes created."; break;
  case DDERR_HWNDSUBCLASSED: txt = "DirectDraw is prevented from restoring state because the DirectDraw cooperative level window handle has been subclassed."; break;
  case DDERR_IMPLICITLYCREATED: txt = "The surface cannot be restored because it is an implicitly created surface."; break;
  case DDERR_INCOMPATIBLEPRIMARY: txt = "The primary surface creation request does not match with the existing primary surface."; break;
  case DDERR_INVALIDCAPS: txt = "One or more of the capability bits passed to the callback function are incorrect."; break;
  case DDERR_INVALIDCLIPLIST: txt = "DirectDraw does not support the provided clip list."; break;
  case DDERR_INVALIDDIRECTDRAWGUID: txt = "The globally unique identifier (GUID) passed to the DirectDrawCreate function is not a valid DirectDraw driver identifier."; break;
  case DDERR_INVALIDMODE: txt = "DirectDraw does not support the requested mode."; break;
  case DDERR_INVALIDOBJECT: txt = "DirectDraw received a pointer that was an invalid DirectDraw object."; break;
  case DDERR_INVALIDPARAMS: txt = "One or more of the parameters passed to the method are incorrect."; break;
  case DDERR_INVALIDPIXELFORMAT: txt = "The pixel format was invalid as specified."; break;
  case DDERR_INVALIDPOSITION: txt = "The position of the overlay on the destination is no longer legal."; break;
  case DDERR_INVALIDRECT: txt = "The provided rectangle was invalid."; break;
  case DDERR_INVALIDSURFACETYPE: txt = "The requested operation could not be performed because the surface was of the wrong type."; break;
  case DDERR_LOCKEDSURFACES: txt = "One or more surfaces are locked, causing the failure of the requested operation."; break;
  case DDERR_NO3D: txt = "No 3D hardware or emulation is present."; break;
  case DDERR_NOALPHAHW: txt = "No alpha acceleration hardware is present or available, causing the failure of the requested operation."; break;
  case DDERR_NOBLTHW: txt = "No blitter hardware is present."; break;
  case DDERR_NOCLIPLIST: txt = "No clip list is available."; break;
  case DDERR_NOCLIPPERATTACHED: txt = "No DirectDrawClipper object is attached to the surface object."; break;
  case DDERR_NOCOLORCONVHW: txt = "The operation cannot be carried out because no color-conversion hardware is present or available."; break;
  case DDERR_NOCOLORKEY: txt = "The surface does not currently have a color key."; break;
  case DDERR_NOCOLORKEYHW: txt = "The operation cannot be carried out because there is no hardware support for the destination color key."; break;
  case DDERR_NOCOOPERATIVELEVELSET: txt = "A create function is called without the IDirectDraw2::SetCooperativeLevel method being called."; break;
  case DDERR_NODC: txt = "No DC has ever been created for this surface."; break;
  case DDERR_NODDROPSHW: txt = "No DirectDraw raster operation (ROP) hardware is available."; break;
  case DDERR_NODIRECTDRAWHW: txt = "Hardware-only DirectDraw object creation is not possible; the driver does not support any hardware."; break;
  case DDERR_NODIRECTDRAWSUPPORT: txt = "DirectDraw support is not possible with the current display driver."; break;
  case DDERR_NOEMULATION: txt = "Software emulation is not available."; break;
  case DDERR_NOEXCLUSIVEMODE: txt = "The operation requires the application to have exclusive mode, but the application does not have exclusive mode."; break;
  case DDERR_NOFLIPHW: txt = "Flipping visible surfaces is not supported."; break;
  case DDERR_NOGDI: txt = "No GDI is present."; break;
  case DDERR_NOHWND: txt = "Clipper notification requires a window handle, or no window handle has been previously set as the cooperative level window handle."; break;
  case DDERR_NOMIPMAPHW: txt = "The operation cannot be carried out because no mipmap texture mapping hardware is present or available."; break;
  case DDERR_NOMIRRORHW: txt = "The operation cannot be carried out because no mirroring hardware is present or available."; break;
  case DDERR_NOOVERLAYDEST: txt = "The IDirectDrawSurface2::GetOverlayPosition method is called on an overlay that the IDirectDrawSurface2::UpdateOverlay method has not been called on to establish a destination."; break;
  case DDERR_NOOVERLAYHW: txt = "The operation cannot be carried out because no overlay hardware is present or available."; break;
  case DDERR_NOPALETTEATTACHED: txt = "No palette object is attached to this surface."; break;
  case DDERR_NOPALETTEHW: txt = "There is no hardware support for 16- or 256-color palettes."; break;
  case DDERR_NORASTEROPHW: txt = "The operation cannot be carried out because no appropriate raster operation hardware is present or available."; break;
  case DDERR_NOROTATIONHW: txt = "The operation cannot be carried out because no rotation hardware is present or available."; break;
  case DDERR_NOSTRETCHHW: txt = "The operation cannot be carried out because there is no hardware support for stretching."; break;
  case DDERR_NOT4BITCOLOR: txt = "The DirectDrawSurface object is not using a 4-bit color palette and the requested operation requires a 4-bit color palette."; break;
  case DDERR_NOT4BITCOLORINDEX: txt = "The DirectDrawSurface object is not using a 4-bit color index palette and the requested operation requires a 4-bit color index palette."; break;
  case DDERR_NOT8BITCOLOR: txt = "The DirectDrawSurface object is not using an 8-bit color palette and the requested operation requires an 8-bit color palette."; break;
  case DDERR_NOTAOVERLAYSURFACE: txt = "An overlay component is called for a non-overlay surface."; break;
  case DDERR_NOTEXTUREHW: txt = "The operation cannot be carried out because no texture-mapping hardware is present or available."; break;
  case DDERR_NOTFLIPPABLE: txt = "An attempt has been made to flip a surface that cannot be flipped."; break;
  case DDERR_NOTFOUND: txt = "The requested item was not found."; break;
  case DDERR_NOTINITIALIZED: txt = "An attempt was made to call an interface method of a DirectDraw object created by CoCreateInstance before the object was initialized."; break;
  case DDERR_NOTLOCKED: txt = "An attempt is made to unlock a surface that was not locked."; break;
  case DDERR_NOTPAGELOCKED: txt = "An attempt is made to page unlock a surface with no outstanding page locks."; break;
  case DDERR_NOTPALETTIZED: txt = "The surface being used is not a palette-based surface."; break;
  case DDERR_NOVSYNCHW: txt = "The operation cannot be carried out because there is no hardware support for vertical blank synchronized operations."; break;
  case DDERR_NOZBUFFERHW: txt = "The operation to create a z-buffer in display memory or to perform a blit using a z-buffer cannot be carried out because there is no hardware support for z-buffers."; break;
  case DDERR_NOZOVERLAYHW: txt = "The overlay surfaces cannot be z-layered based on the z-order because the hardware does not support z-ordering of overlays."; break;
  case DDERR_OUTOFCAPS: txt = "The hardware needed for the requested operation has already been allocated."; break;
  case DDERR_OUTOFMEMORY: txt = "DirectDraw does not have enough memory to perform the operation."; break;
  case DDERR_OUTOFVIDEOMEMORY: txt = "DirectDraw does not have enough display memory to perform the operation."; break;
  case DDERR_OVERLAYCANTCLIP: txt = "The hardware does not support clipped overlays."; break;
  case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: txt = "An attempt was made to have more than one color key active on an overlay."; break;
  case DDERR_OVERLAYNOTVISIBLE: txt = "The IDirectDrawSurface2::GetOverlayPosition method is called on a hidden overlay."; break;
  case DDERR_PALETTEBUSY: txt = "Access to this palette is refused because the palette is locked by another thread."; break;
  case DDERR_PRIMARYSURFACEALREADYEXISTS: txt = "This process has already created a primary surface."; break;
  case DDERR_REGIONTOOSMALL: txt = "The region passed to the IDirectDrawClipper::GetClipList method is too small."; break;
  case DDERR_SURFACEALREADYATTACHED: txt = "An attempt was made to attach a surface to another surface to which it is already attached."; break;
  case DDERR_SURFACEALREADYDEPENDENT: txt = "An attempt was made to make a surface a dependency of another surface to which it is already dependent."; break;
  case DDERR_SURFACEBUSY: txt = "Access to the surface is refused because the surface is locked by another thread."; break;
  case DDERR_SURFACEISOBSCURED: txt = "Access to the surface is refused because the surface is obscured."; break;
  case DDERR_SURFACELOST: txt = "Access to the surface is refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have the IDirectDrawSurface2::Restore method called on it."; break;
  case DDERR_SURFACENOTATTACHED: txt = "The requested surface is not attached."; break;
  case DDERR_TOOBIGHEIGHT: txt = "The height requested by DirectDraw is too large."; break;
  case DDERR_TOOBIGSIZE: txt = "The size requested by DirectDraw is too large. However, the individual height and width are OK."; break;
  case DDERR_TOOBIGWIDTH: txt = "The width requested by DirectDraw is too large."; break;
  case DDERR_UNSUPPORTED: txt = "The operation is not supported."; break;
  case DDERR_UNSUPPORTEDFORMAT: txt = "The FourCC format requested is not supported by DirectDraw."; break;
  case DDERR_UNSUPPORTEDMASK: txt = "The bitmask in the pixel format requested is not supported by DirectDraw."; break;
  case DDERR_UNSUPPORTEDMODE: txt = "The display is currently in an unsupported mode."; break;
  case DDERR_VERTICALBLANKINPROGRESS: txt = "A vertical blank is in progress."; break;
  case DDERR_WASSTILLDRAWING: txt = "The previous blit operation that is transferring information to or from this surface is incomplete."; break;
  case DDERR_WRONGMODE: txt = "This surface cannot be restored because it was created in a different mode."; break;
  case DDERR_XALIGN: txt = "The provided rectangle was not horizontally aligned on a required boundary."; break;
  default: return FALSE;
  }
  // Make it formated - just looks better
  msg.Format("DirectDraw error ocured:\r\n     %s",txt);
  TRACE("DirectDraw exception : %s\n  Error code : %X\n", msg, m_hResult);
  
  strncpy(lpszError, msg, nMaxError);
  return TRUE;
}