#pragma once

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
}