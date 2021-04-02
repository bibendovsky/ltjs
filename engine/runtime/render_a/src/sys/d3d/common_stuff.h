
// Defines most of the console variables and some helper stuff.

#ifndef __COMMON_STUFF_H__
#define __COMMON_STUFF_H__


#ifndef __D3D_CONVAR_H__
#	include "d3d_convar.h"
#endif

#if LTJS_SDL_BACKEND
#include "ltjs_main_window_descriptor.h"
#endif // LTJS_SDL_BACKEND

struct RenderStruct;

// Both renderers use this for the render contexts.
struct RenderContext 
{
	uint16		m_CurFrameCode; 
};

/****************** CONSOLE VARIABLES ******************/
// D3D Device Variable...

extern bool		g_bRunWindowed;

extern RenderStruct* g_pStruct;
extern int32 g_ScreenWidth;
extern int32 g_ScreenHeight;

#if LTJS_SDL_BACKEND
extern const ltjs::MainWindowDescriptor* g_hWnd;
#else
extern HWND		g_hWnd;
#endif // LTJS_SDL_BACKEND

void*	dalloc(uint32 size);
void*	dalloc_z(uint32 size);
void	dfree(void *ptr);
void	AddDebugMessage(uint32 debugLevel, const char *pMsg, ...);
void	d3d_CreateConsoleVariables();
void	d3d_ReadConsoleVariables();
 
#endif  // __COMMON_STUFF_H__




