// dllmain.cpp : Defines the entry point for the DLL application.

#include <Windows.h>
#include <cstdio>

#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>
#ifndef _DEBUG
#define PLOG_DISABLE_LOGGING
#endif

#include "Detours/include/detours.h"
//#include "stb/stb_image.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl2.h"
#include "imgui/backends/imgui_impl_win32.h"

#include "user32.hpp"
#include "opengl.hpp"

void OnDLLProcessAttach(void)
{
	auto base_address = reinterpret_cast<unsigned int>(GetModuleHandle(0));
	user32::gameHWND = FindWindowA(NULL, "Tribes");

#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(plog::debug, &consoleAppender);
#endif
	PLOG_INFO << "Successfully Injected DLL.";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;  // disable ini loading

	ImGui_ImplWin32_Init(user32::gameHWND);
	ImGui_ImplOpenGL2_Init();

	user32::originalWNDPROCCallBack = (WNDPROC)SetWindowLongW(user32::gameHWND, GWL_WNDPROC, (LONG_PTR)user32::CustomWindowProcCallback);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	auto hModule = GetModuleHandle(L"User32.dll");

	user32::originalCallWindowProcW = reinterpret_cast<user32::CallWindowProcW>(GetProcAddress(hModule, "CallWindowProcW"));
	DetourAttach(&(PVOID&)user32::originalCallWindowProcW, user32::CallWindowProcWHook);

	user32::originalSetWindowsHookExW = reinterpret_cast<user32::SetWindowsHookExW>(GetProcAddress(hModule, "SetWindowsHookExW"));
	DetourAttach(&(PVOID&)user32::originalSetWindowsHookExW, user32::SetWindowsHookExWHook);


	hModule = GetModuleHandle(L"opengl32.dll");

	opengl::originalWglSwapBuffers = reinterpret_cast<opengl::wglSwapBuffers>(GetProcAddress(hModule, "wglSwapBuffers"));
	DetourAttach(&(PVOID&)opengl::originalWglSwapBuffers, opengl::wglSwapBuffersHook);

	DetourTransactionCommit();

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnDLLProcessAttach, NULL, NULL, NULL);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

