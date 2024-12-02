#include "shared.hpp"
#include "opengl.hpp"

namespace opengl
{
	bool showImGuiMenu{ false };
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