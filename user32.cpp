#include "shared.hpp"
#include "user32.hpp"
#include "opengl.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"

namespace user32
{
	HWND gameHWND{ nullptr };
	DWORD mouseLLHookFunctionAddress{ 0 };
	DWORD mouseLLHookOriginalByteCodeBuffer[3]{ 0 };
	BYTE mouseLLHookModifiedByteCodeBuffer[3]{ 0xC2, 0x0C, 0x00 };

	WNDPROC originalWNDPROCCallBack{ nullptr };

	SetWindowLongW originalSetWindowLongW{ reinterpret_cast<SetWindowLongW>(0x75A15940) };
	LRESULT __stdcall SetWindowLongWHook(HWND hWnd, int arg1, long arg2)
	{
		LRESULT res;
		return originalSetWindowLongW(hWnd, arg1, arg2);
	}

	CallWindowProcW originalCallWindowProcW = { reinterpret_cast<CallWindowProcW>(0) };
	LRESULT __stdcall CallWindowProcWHook(WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{
		return originalCallWindowProcW(lpPrevWndFunc, hWnd, Msg, wParam, lParam);
	}

	LRESULT WINAPI CustomWindowProcCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		PLOG_VERBOSE << "CustomWindowProcCallback called";
		if (msg == WM_KEYDOWN)
		{
			PLOG_DEBUG << "Key down: " << wParam;
			if (wParam == VK_INSERT)
			{
				PLOG_INFO << "Toggling ImGui menu";
				opengl::showImGuiMenu = !opengl::showImGuiMenu;
				if (opengl::showImGuiMenu)
				{
					memcpy(reinterpret_cast<void*>(mouseLLHookFunctionAddress), &mouseLLHookModifiedByteCodeBuffer,
						   sizeof(mouseLLHookModifiedByteCodeBuffer));
				}
				else
				{
					memcpy(reinterpret_cast<void*>(mouseLLHookFunctionAddress), &mouseLLHookOriginalByteCodeBuffer,
						   sizeof(mouseLLHookModifiedByteCodeBuffer));
				}
			}
		}
		else if (msg == WM_KEYUP)
		{

		}

		ImGuiIO& io = ImGui::GetIO();
		if (opengl::showImGuiMenu)
		{
			io.MouseDrawCursor = true;
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		}
		else
		{
			io.MouseDrawCursor = false;
		}
		return originalCallWindowProcW(originalWNDPROCCallBack, hWnd, msg, wParam, lParam);
	}

	SetWindowsHookExW originalSetWindowsHookExW = { reinterpret_cast<SetWindowsHookExW>(0) };
	HHOOK __stdcall SetWindowsHookExWHook(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId) {
		PLOG_DEBUG << "SetWindowsHookExWHook called";
		auto hookFunctionAddress = reinterpret_cast<DWORD>(lpfn);

		static DWORD protection{ 0 };
		
		if (idHook == WH_MOUSE_LL && hookFunctionAddress != mouseLLHookFunctionAddress) {
			PLOG_INFO << "New WH_MOUSE_LL hook function found";
			mouseLLHookFunctionAddress = hookFunctionAddress;
			VirtualProtect((void*)mouseLLHookFunctionAddress, sizeof(float), PAGE_EXECUTE_READWRITE, &protection);
			memcpy(&mouseLLHookOriginalByteCodeBuffer, reinterpret_cast<void*>(mouseLLHookFunctionAddress),
				   sizeof(mouseLLHookOriginalByteCodeBuffer));
		}

		HHOOK hhook = originalSetWindowsHookExW(idHook, lpfn, hmod, dwThreadId);
		return hhook;
	}
}