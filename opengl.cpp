#include "opengl/gl.h"
#include "opengl.hpp"
#include "debug.hpp"
#include "draw.hpp"
#include <plog/Log.h>
#include <format>

namespace opengl
{
	bool showImGuiMenu{ false };
	
	bool vertexArrayEnabled{ false };
	bool textureCoordArrayEnabled{ false };
	bool normalArrayEnabled{ false };

	debug::fingerprint::VertexData latestVertexData;
	debug::fingerprint::UVData latestUVData;
	debug::fingerprint::NormalData latestNormalData;

	wglSwapBuffers originalWglSwapBuffers{ reinterpret_cast<wglSwapBuffers>(0) };
	BOOL __stdcall wglSwapBuffersHook(int* arg1) {
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (opengl::showImGuiMenu)
		{
			ImGui::ShowDemoWindow();
#ifdef _DEBUG
			debug::fingerprint::DrawFingerprintWindow();
#endif
		}

#ifdef _DEBUG
		debug::reset();
#endif

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		return originalWglSwapBuffers(arg1);
	}


	glDrawArrays originalGlDrawArrays{ reinterpret_cast<glDrawArrays>(0) };
	void __stdcall glDrawArraysHook(int mode, int first, int count)
	{
		PLOG_VERBOSE << std::format("Mode: {0}, First: {1}, Count: {2}",
									mode, first, count);
		bool drawOriginal{ true };
#ifdef _DEBUG
		if (count >= debug::fingerprint::DrawArraysFingerprint::CountLowerBound &&
			count <= debug::fingerprint::DrawArraysFingerprint::CountUpperBound &&
			mode == debug::fingerprint::DrawArraysFingerprint::Mode)
		{
			debug::fingerprint::DrawArraysFingerprint fingerprint;
			fingerprint.m_Mode = mode;
			fingerprint.m_First = first;
			fingerprint.m_Count = count;
			fingerprint.m_VertexData = latestVertexData;
			fingerprint.m_UVData = latestUVData;
			fingerprint.m_NormalData = latestNormalData;
			debug::fingerprint::drawArraysFingerprints.push_back(fingerprint);
			drawOriginal = false;
		}
#endif
		if (drawOriginal)
		{
			return originalGlDrawArrays(mode, first, count);
		}
	}

	glEnableClientState originalGlEnableClientState{ reinterpret_cast<glEnableClientState>(0) };
	void __stdcall glEnableClientStateHook(unsigned int array)
	{
		switch (array)
		{
			case GL_VERTEX_ARRAY:
			{
				PLOG_VERBOSE << "Enabling GL_VERTEX_ARRAY";
				vertexArrayEnabled = true;
				break;
			}
			case GL_TEXTURE_COORD_ARRAY:
			{
				PLOG_VERBOSE << "Enabling GL_TEXTURE_COORD_ARRAY";
				textureCoordArrayEnabled = true;
				break;
			}
			case GL_NORMAL_ARRAY:
			{
				PLOG_VERBOSE << "Enabling GL_NORMAL_ARRAY";
				normalArrayEnabled = true;
				break;
			}
		}
		return originalGlEnableClientState(array);
	}

	glDisableClientState originalGlDisableClientState{ reinterpret_cast<glDisableClientState>(0) };
	void __stdcall glDisableClientStateHook(unsigned int array)
	{
		switch (array)
		{
			case GL_VERTEX_ARRAY:
			{
				PLOG_VERBOSE << "Disabling GL_VERTEX_ARRAY";
				vertexArrayEnabled = false;
				break;
			}
			case GL_TEXTURE_COORD_ARRAY:
			{
				PLOG_VERBOSE << "Disabling GL_TEXTURE_COORD_ARRAY";
				textureCoordArrayEnabled = false;
				break;
			}
			case GL_NORMAL_ARRAY:
			{
				PLOG_VERBOSE << "Disabling GL_NORMAL_ARRAY";
				normalArrayEnabled = false;
				break;
			}
		}
		return originalGlDisableClientState(array);
	}

	glVertexPointer originalGlVertexPointer{ reinterpret_cast<glVertexPointer>(0) };
	void __stdcall glVertexPointerHook(unsigned int size, unsigned int type, unsigned int stride, void* data)
	{
		PLOG_VERBOSE << std::format("Size: {0}, Type: {1}, Stride: {2}, Data: {3}",
									size, type, stride, reinterpret_cast<unsigned int>(data));
		latestVertexData = debug::fingerprint::VertexData { vertexArrayEnabled,
															size, type, stride, data };
		return originalGlVertexPointer(size, type, stride, data);
	}

	glTexCoordPointer originalGlTexCoordPointer{ reinterpret_cast<glTexCoordPointer>(0) };
	void __stdcall glTexCoordPointerHook(unsigned int size, unsigned int type, unsigned int stride, void* data)
	{
		PLOG_VERBOSE << std::format("Size: {0}, Type: {1}, Stride: {2}, Data: {3}",
									size, type, stride, reinterpret_cast<unsigned int>(data));
		latestUVData = debug::fingerprint::UVData{ textureCoordArrayEnabled,
													size, type, stride, data };
		return originalGlTexCoordPointer(size, type, stride, data);
	}

	glNormalPointer originalGlNormalPointer{ reinterpret_cast<glNormalPointer>(0) };
	void __stdcall glNormalPointerHook(unsigned int type, unsigned int stride, void* data)
	{
		PLOG_VERBOSE << std::format("Type: {0}, Stride: {1}, Data: {2}",
									type, stride, reinterpret_cast<unsigned int>(data));
		latestNormalData = debug::fingerprint::NormalData{ normalArrayEnabled,
														type, stride, data };
		return originalGlNormalPointer(type, stride, data);
	}
}