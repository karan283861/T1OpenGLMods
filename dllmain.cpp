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

#include "fingerprint.hpp"
#include "model.hpp"

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

	auto moduleUser32 = GetModuleHandle("User32.dll");
	if (moduleUser32)
	{
		PLOG_INFO << "Successfully found User32.dll module";

		user32::originalCallWindowProcW = reinterpret_cast<user32::CallWindowProcW>(GetProcAddress(moduleUser32, "CallWindowProcW"));
		DetourAttach(&(PVOID&)user32::originalCallWindowProcW, user32::CallWindowProcWHook);

		user32::originalSetWindowsHookExW = reinterpret_cast<user32::SetWindowsHookExW>(GetProcAddress(moduleUser32, "SetWindowsHookExW"));
		DetourAttach(&(PVOID&)user32::originalSetWindowsHookExW, user32::SetWindowsHookExWHook);
	}
	else
	{
		PLOG_ERROR << "Failed to find User32.dll module";
	}

	auto moduleOpenGL32 = GetModuleHandle("opengl32.dll");

	if (moduleOpenGL32)
	{
		PLOG_INFO << "Successfully found OpenGL32.dll module";

		opengl::originalWglSwapBuffers = reinterpret_cast<opengl::wglSwapBuffers>(GetProcAddress(moduleOpenGL32, "wglSwapBuffers"));
		DetourAttach(&(PVOID&)opengl::originalWglSwapBuffers, opengl::wglSwapBuffersHook);

		opengl::originalWglMakeCurrent = reinterpret_cast<opengl::wglMakeCurrent>(GetProcAddress(moduleOpenGL32, "wglMakeCurrent"));
		DetourAttach(&(PVOID&)opengl::originalWglMakeCurrent, opengl::wglMakeCurrentHook);

		opengl::originalGlDrawArrays = reinterpret_cast<opengl::glDrawArrays>(GetProcAddress(moduleOpenGL32, "glDrawArrays"));
		DetourAttach(&(PVOID&)opengl::originalGlDrawArrays, opengl::glDrawArraysHook);

		opengl::originalGlEnableClientState = reinterpret_cast<opengl::glEnableClientState>(GetProcAddress(moduleOpenGL32, "glEnableClientState"));
		DetourAttach(&(PVOID&)opengl::originalGlEnableClientState, opengl::glEnableClientStateHook);

		opengl::originalGlDisableClientState = reinterpret_cast<opengl::glDisableClientState>(GetProcAddress(moduleOpenGL32, "glDisableClientState"));
		DetourAttach(&(PVOID&)opengl::originalGlDisableClientState, opengl::glDisableClientStateHook);

		opengl::originalGlVertexPointer = reinterpret_cast<opengl::glVertexPointer>(GetProcAddress(moduleOpenGL32, "glVertexPointer"));
		DetourAttach(&(PVOID&)opengl::originalGlVertexPointer, opengl::glVertexPointerHook);

		opengl::originalGlTexCoordPointer = reinterpret_cast<opengl::glTexCoordPointer>(GetProcAddress(moduleOpenGL32, "glTexCoordPointer"));
		DetourAttach(&(PVOID&)opengl::originalGlTexCoordPointer, opengl::glTexCoordPointerHook);

		opengl::originalGlNormalPointer = reinterpret_cast<opengl::glNormalPointer>(GetProcAddress(moduleOpenGL32, "glNormalPointer"));
		DetourAttach(&(PVOID&)opengl::originalGlNormalPointer, opengl::glNormalPointerHook);

		opengl::originalGlRotatef = reinterpret_cast<opengl::glRotatef>(GetProcAddress(moduleOpenGL32, "glRotatef"));
		opengl::originalGlTranslatef = reinterpret_cast<opengl::glTranslatef>(GetProcAddress(moduleOpenGL32, "glTranslatef"));
		opengl::originalGlScalef = reinterpret_cast<opengl::glScalef>(GetProcAddress(moduleOpenGL32, "glScalef"));

		opengl::originalGlGenTextures = reinterpret_cast<opengl::glGenTextures>(GetProcAddress(moduleOpenGL32, "glGenTextures"));
		opengl::originalGlBindTexture = reinterpret_cast<opengl::glBindTexture>(GetProcAddress(moduleOpenGL32, "glBindTexture"));
		DetourAttach(&(PVOID&)opengl::originalGlBindTexture, opengl::glBindTextureHook);
		opengl::originalGlTexImage2D = reinterpret_cast<opengl::glTexImage2D>(GetProcAddress(moduleOpenGL32, "glTexImage2D"));

		opengl::originalGlTexParameteri = reinterpret_cast<opengl::glTexParameteri>(GetProcAddress(moduleOpenGL32, "glTexParameteri"));
		opengl::originalGlTexEnvi = reinterpret_cast<opengl::glTexEnvi>(GetProcAddress(moduleOpenGL32, "glTexEnvi"));
		opengl::originalGlEnable = reinterpret_cast<opengl::glEnable>(GetProcAddress(moduleOpenGL32, "glEnable"));
	}
	else
	{
		PLOG_ERROR << "Failed to find OpenGL32.dll module";
	}

	DetourTransactionCommit();

	fingerprint::drawarrays::Initialise();
	model::Initialise();
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

