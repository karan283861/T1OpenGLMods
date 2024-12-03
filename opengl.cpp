#include "opengl/gl.h"
#include "opengl.hpp"
#include "fingerprint.hpp"
#include <plog/Log.h>
#include <format>

namespace opengl
{
	bool showImGuiMenu{ false };

	bool vertexArrayEnabled{ false };
	bool textureCoordArrayEnabled{ false };
	bool normalArrayEnabled{ false };

	fingerprint::VertexData latestVertexData;
	fingerprint::UVData latestUVData;
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

			latestVertexData.m_Enabled = vertexArrayEnabled;
			latestUVData.m_Enabled = textureCoordArrayEnabled;
			//latestNormalData.m_Enabled = normalArrayEnabled;

			fingerprint::drawarrays::Fingerprint fingerprint;
			fingerprint.m_Mode = mode;
			fingerprint.m_First = first;
			fingerprint.m_Count = count;
			fingerprint.m_VertexData = latestVertexData;
			fingerprint.m_UVData = latestUVData;
			//fingerprint.m_NormalData = latestNormalData;
			fingerprint.m_VerticesPtr = std::make_shared<fingerprint::Vertex[]>(count);
			memcpy(fingerprint.m_VerticesPtr.get(), latestVertexData.m_VertexBuffer,
				   sizeof(fingerprint::Vertex) * count);
			fingerprint::drawarrays::foundFingerprints.push_back(fingerprint);
			drawOriginal = false;
		}
#endif
		auto countToFingerprintReplacements = fingerprint::drawarrays::modeToCountToFingerprintReplacements.find(mode);
		if (countToFingerprintReplacements != fingerprint::drawarrays::modeToCountToFingerprintReplacements.end())
		{
			auto fingerprintReplacements = countToFingerprintReplacements->second.find(count);
			if (fingerprintReplacements != countToFingerprintReplacements->second.end())
			{
				auto& replacements = fingerprintReplacements->second;
				auto replacementFound{ false };
				for (auto& replacement : replacements)
				{
					auto& fingerprintIdentifier = replacement.m_FingerprintIdentifier;
					if (fingerprintIdentifier.m_Mode == mode && fingerprintIdentifier.m_Count == count)
					{
						auto foundVertex{ fingerprintIdentifier.m_IndexVertex.size() == 0 };
						if (vertexArrayEnabled)
						{
							foundVertex = false;
							auto foundAllVertex{ true };
							for (auto& indexVertex : fingerprintIdentifier.m_IndexVertex)
							{
								auto index = indexVertex.first;
								// get rid of magic numbers
								auto vertexBufferX{ ((float*)(latestVertexData.m_VertexBuffer))[index * VERTEX_POINTER_SIZE + 0]};
								auto vertexBufferY{ ((float*)(latestVertexData.m_VertexBuffer))[index * VERTEX_POINTER_SIZE + 1] };
								auto vertexBufferZ{ ((float*)(latestVertexData.m_VertexBuffer))[index * VERTEX_POINTER_SIZE + 2] };

								if (abs(indexVertex.second.m_X - vertexBufferX) < fingerprint::drawarrays::FingerprintIdentifier::VertexValueDelta &&
									abs(indexVertex.second.m_Y - vertexBufferY) < fingerprint::drawarrays::FingerprintIdentifier::VertexValueDelta &&
									abs(indexVertex.second.m_Z - vertexBufferZ) < fingerprint::drawarrays::FingerprintIdentifier::VertexValueDelta)
								{
									PLOG_DEBUG << std::format("Found Vertex. Index: {0}, X: {1}, Y: {2}, Z: {3}",
															  indexVertex.first, indexVertex.second.m_X, indexVertex.second.m_Y, indexVertex.second.m_Z);
								}
								else
								{
									PLOG_DEBUG << std::format("Did NOT find Vertex. Index: {0}, X: {1}, Y: {2}, Z: {3}",
															  indexVertex.first, indexVertex.second.m_X, indexVertex.second.m_Y, indexVertex.second.m_Z);
									foundAllVertex = false;
									break;
								}
							}
							foundVertex = foundAllVertex;
						}

						auto foundUV{ fingerprintIdentifier.m_IndexUV.size() == 0 };
						if (textureCoordArrayEnabled)
						{
							foundUV = false;
							auto foundAllUV{ true };
							for (auto& indexUV : fingerprintIdentifier.m_IndexUV)
							{
								auto index = indexUV.first;
								auto uvBufferX{ ((float*)(latestUVData.m_UVBuffer))[index * TEXCOORD_POINTER_SIZE + 0] };
								auto uvBufferY{ ((float*)(latestUVData.m_UVBuffer))[index * TEXCOORD_POINTER_SIZE + 1] };

								if (abs(indexUV.second.m_X - uvBufferX) < fingerprint::drawarrays::FingerprintIdentifier::UVValueDelta &&
									abs(indexUV.second.m_Y - uvBufferY) < fingerprint::drawarrays::FingerprintIdentifier::UVValueDelta)
								{
									PLOG_DEBUG << std::format("Found UV. Index: {0}, X: {1}, Y: {2}",
															  indexUV.first, indexUV.second.m_X, indexUV.second.m_Y);
								}
								else
								{
									PLOG_DEBUG << std::format("Did not find UV. Index: {0}, X: {1}, Y: {2}",
															  indexUV.first, indexUV.second.m_X, indexUV.second.m_Y);
									foundAllUV = false;
									break;
								}
							}
							foundUV = foundAllUV;
						}

						if (foundVertex && foundUV)
						{
							drawOriginal = false;
						}
					}
				}
			}
		}
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
									size, type, stride, data);
		latestVertexData = fingerprint::VertexData{ false, /*size, type, stride,*/ data};
		return originalGlVertexPointer(size, type, stride, data);
	}

	glTexCoordPointer originalGlTexCoordPointer{ reinterpret_cast<glTexCoordPointer>(0) };
	void __stdcall glTexCoordPointerHook(unsigned int size, unsigned int type, unsigned int stride, void* data)
	{
		PLOG_VERBOSE << std::format("Size: {0}, Type: {1}, Stride: {2}, Data: {3}",
									size, type, stride, data);
		latestUVData = fingerprint::UVData{ false, /*size, type, stride,*/ data };
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