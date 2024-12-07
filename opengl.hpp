#pragma once

#define VERTEX_POINTER_SIZE 4
#define VERTEX_POINTER_STRIDE 16
#define TEXCOORD_POINTER_SIZE 2
#define TEXCOORD_POINTER_STRIDE 0

#include "opengl/gl.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl2.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "fingerprint.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace opengl
{
	extern bool showImGuiMenu;

	typedef BOOL(__stdcall* wglSwapBuffers)(int*);
	extern wglSwapBuffers originalWglSwapBuffers;
	BOOL __stdcall wglSwapBuffersHook(int* arg1);

	typedef void(__stdcall* glDrawArrays)(int, int, int);
	extern glDrawArrays originalGlDrawArrays;
	void __stdcall glDrawArraysHook(int mode, int first, int count);

	typedef void(__stdcall* glEnableClientState)(unsigned int);
	extern glEnableClientState originalGlEnableClientState;
	void __stdcall glEnableClientStateHook(unsigned int array);
	typedef void(__stdcall* glDisableClientState)(unsigned int);
	extern glDisableClientState originalGlDisableClientState;
	void __stdcall glDisableClientStateHook(unsigned int array);
	extern bool vertexArrayEnabled;
	extern bool textureCoordArrayEnabled;
	extern bool normalArrayEnabled;

	typedef void(__stdcall* glVertexPointer)(unsigned int, unsigned int, unsigned int, void*);

	extern glVertexPointer originalGlVertexPointer;
	void __stdcall glVertexPointerHook(unsigned int size, unsigned int type, unsigned int stride, void* data);
	typedef void(__stdcall* glTexCoordPointer)(unsigned int, unsigned int, unsigned int, void*);

	extern glTexCoordPointer originalGlTexCoordPointer;
	void __stdcall glTexCoordPointerHook(unsigned int size, unsigned int type, unsigned int stride, void* data);

	typedef void(__stdcall* glNormalPointer)(int, int, void*);
	extern glNormalPointer originalGlNormalPointer;
	void __stdcall glNormalPointerHook(unsigned int type, unsigned int stride, void* data);
}