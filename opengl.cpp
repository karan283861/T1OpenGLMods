#include <format>
#include <plog/Log.h>
#include "opengl/gl.h"
#include "opengl.hpp"

namespace opengl
{
	bool showImGuiMenu{ false };

	wglMakeCurrent originalWglMakeCurrent{ reinterpret_cast<wglMakeCurrent>(0) };
	void __stdcall wglMakeCurrentHook(void* unnamedParam1, void* unnamedParam2) {
		if (!unnamedParam2)
		{
			ImGui_ImplOpenGL2_Shutdown();
		}
		else
		{
			ImGui_ImplOpenGL2_Init();
		}
		
		originalWglMakeCurrent(unnamedParam1, unnamedParam2);
	}

	wglSwapBuffers originalWglSwapBuffers{ reinterpret_cast<wglSwapBuffers>(0) };
	BOOL __stdcall wglSwapBuffersHook(int* arg1) {
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (opengl::showImGuiMenu)
		{
            ImGui::ShowDemoWindow();
		}

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		return originalWglSwapBuffers(arg1);
	}
}