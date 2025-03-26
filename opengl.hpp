#pragma once

#define VERTEX_POINTER_SIZE 4
#define VERTEX_POINTER_STRIDE 16
#define TEXCOORD_POINTER_SIZE 2
#define TEXCOORD_POINTER_STRIDE 0

#include "opengl/gl.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl2.h"
#include "imgui/backends/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace opengl
{
	extern bool showImGuiMenu;

	typedef BOOL(__stdcall* wglSwapBuffers)(int*);
	extern wglSwapBuffers originalWglSwapBuffers;
	BOOL __stdcall wglSwapBuffersHook(int* arg1);

	typedef void(__stdcall* wglMakeCurrent)(void* unnamedParam1, void* unnamedParam2);
	extern wglMakeCurrent originalWglMakeCurrent;
	void __stdcall wglMakeCurrentHook(void* unnamedParam1, void* unnamedParam2);
}