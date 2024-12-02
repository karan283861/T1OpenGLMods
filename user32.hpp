#pragma once

#include <Windows.h>

namespace user32
{
	extern HWND gameHWND;
	
	LRESULT WINAPI CustomWindowProcCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	extern WNDPROC originalWNDPROCCallBack;
	
	typedef LRESULT(__stdcall* SetWindowLongW)(HWND, int, long);
	extern SetWindowLongW originalSetWindowLongW;
	LRESULT __stdcall SetWindowLongWHook(HWND hWnd, int arg1, long arg2);

	typedef LRESULT(__stdcall* CallWindowProcW)(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
	extern CallWindowProcW originalCallWindowProcW;
	LRESULT __stdcall CallWindowProcWHook(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	typedef HHOOK(__stdcall* SetWindowsHookExW)(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId);
	extern SetWindowsHookExW originalSetWindowsHookExW;
	HHOOK __stdcall SetWindowsHookExWHook(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId);
}