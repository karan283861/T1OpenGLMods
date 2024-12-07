#include "opengl/gl.h"
#include "opengl.hpp"
#include "fingerprint.hpp"
#include <plog/Log.h>
#include <format>
#include "model.hpp"

namespace opengl
{
	bool showImGuiMenu{ false };

	bool vertexArrayEnabled{ false };
	bool textureCoordArrayEnabled{ false };
	bool normalArrayEnabled{ false };

	//fingerprint::NormalData latestNormalData;

	wglSwapBuffers originalWglSwapBuffers{ reinterpret_cast<wglSwapBuffers>(0) };
	BOOL __stdcall wglSwapBuffersHook(int* arg1) {
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (opengl::showImGuiMenu)
		{
			ImGui::ShowDemoWindow();
#ifdef _DEBUG
			fingerprint::drawarrays::DrawWindow();
#endif
		}

#ifdef _DEBUG
		/*fingerprint::reset();*/
		fingerprint::drawarrays::foundFingerprints.clear();
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
		if (count >= fingerprint::drawarrays::Fingerprint::CountLowerBound &&
			count <= fingerprint::drawarrays::Fingerprint::CountUpperBound &&
			mode == fingerprint::drawarrays::Fingerprint::Mode)
		{
			PLOG_DEBUG << std::format("Fingerprint found. Mode: {0}, First: {1}, Count: {2}",
									  mode, first, count);

			//latestVertexData.m_Enabled = vertexArrayEnabled || true;
			//latestUVData.m_Enabled = textureCoordArrayEnabled || true;
			//latestNormalData.m_Enabled = normalArrayEnabled;

			fingerprint::drawarrays::Fingerprint fingerprint;
			fingerprint.m_Mode = mode;
			fingerprint.m_First = first;
			fingerprint.m_Count = count;
			fingerprint.m_VertexData = fingerprint::latestVertexData;
			fingerprint.m_UVData = fingerprint::latestUVData;
			//fingerprint.m_NormalData = latestNormalData;
			fingerprint.m_VerticesPtr = std::make_shared<fingerprint::Vertex[]>(count);
			memcpy(fingerprint.m_VerticesPtr.get(), fingerprint::latestVertexData.m_VertexBuffer,
				   sizeof(fingerprint::Vertex) * count);
			fingerprint.m_UVsPtr = std::make_shared<fingerprint::UV[]>(4);
			memcpy(fingerprint.m_UVsPtr.get(), fingerprint::latestUVData.m_UVBuffer,
				   sizeof(fingerprint::UV) * 4);
			fingerprint::drawarrays::foundFingerprints.push_back(fingerprint);
			drawOriginal = false;
		}
#endif

		auto foundFingerprint{ fingerprint::drawarrays::CheckFingerprintFound(mode, count)};
		

		if (!foundFingerprint)
		{
			foundFingerprint = fingerprint::drawarrays::CheckFingerprintFound(mode, -1);
		}

		if (foundFingerprint)
		{
			static model::CustomModel customModel("rail.obj");
			static float vertices_array[66000];
			memcpy(vertices_array, customModel.m_Vertices.data(), sizeof(float) * customModel.m_Vertices.size());

			//drawOriginal = false;
			originalGlEnableClientState(GL_VERTEX_ARRAY);
			originalGlVertexPointer(VERTEX_POINTER_SIZE, VERTEX_POINTER_STRIDE, GL_FLOAT, customModel.m_Vertices.data());
			originalGlVertexPointer(3, GL_FLOAT, VERTEX_POINTER_STRIDE, vertices_array);
			
			//originalGlDisableClientState(GL_VERTEX_ARRAY);
			//memcpy(fingerprint::latestVertexData.m_VertexBuffer,
			//	   customModel.m_Vertices.data(), sizeof(float) * customModel.m_Vertices.size());

			originalGlEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if (customModel.m_UVs.size() > 0)
			{
				originalGlTexCoordPointer(2, GL_FLOAT, 0, customModel.m_UVs.data());
			}
			else {

			}

			return originalGlDrawArrays(mode, 0, customModel.m_IndexCount);
		}

		if (drawOriginal)
		{
			originalGlDrawArrays(mode, first, count);
		}
		
		originalGlEnableClientState(GL_VERTEX_ARRAY);
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
									size, type, stride, data);
		fingerprint::latestVertexData = fingerprint::VertexData{ false, /*size, type, stride,*/ data};
		return originalGlVertexPointer(size, type, stride, data);
	}

	glTexCoordPointer originalGlTexCoordPointer{ reinterpret_cast<glTexCoordPointer>(0) };
	void __stdcall glTexCoordPointerHook(unsigned int size, unsigned int type, unsigned int stride, void* data)
	{
		PLOG_VERBOSE << std::format("Size: {0}, Type: {1}, Stride: {2}, Data: {3}",
									size, type, stride, data);
		fingerprint::latestUVData = fingerprint::UVData{ false, /*size, type, stride,*/ data };
		return originalGlTexCoordPointer(size, type, stride, data);
	}

	glNormalPointer originalGlNormalPointer{ reinterpret_cast<glNormalPointer>(0) };
	void __stdcall glNormalPointerHook(unsigned int type, unsigned int stride, void* data)
	{
		PLOG_VERBOSE << std::format("Type: {0}, Stride: {1}, Data: {2}",
									type, stride, data);
		//latestNormalData = fingerprint::NormalData{ false, type, stride, data };
		return originalGlNormalPointer(type, stride, data);
	}
}