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

	auto scale = model::Scale{ 0.1, 0.1, 0.1 };
	auto translate = model::Translate{ 0, 3.485,0 };
	auto rotate = model::Rotate{ 81.818, 1, 0, 0};

	//fingerprint::NormalData latestNormalData;

	static bool reloadTextures{ true };

	wglMakeCurrent originalWglMakeCurrent{ reinterpret_cast<wglMakeCurrent>(0) };
	void __stdcall wglMakeCurrentHook(void* unnamedParam1, void* unnamedParam2) {
		if (!unnamedParam2)
		{
			ImGui_ImplOpenGL2_Shutdown();
		}
		else
		{
			ImGui_ImplOpenGL2_Init();
			reloadTextures = true;
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
#ifdef _DEBUG
			//fingerprint::drawarrays::DrawWindow();
#endif
			{
				ImGui::Begin("Fingerprint replacements");
				static char* customModelNames[256];
				for (int i = 0; i < model::customModels.size(); i++)
				{
					const auto& customModel = model::customModels[i];
					customModelNames[i] = (char*)customModel->m_FileName.c_str();
				}

				for (const auto& fingerprintReplacement : fingerprint::drawarrays::fingerprintReplacements)
				{
					int customModelIndex = -1;

					//ImGui::CollapsingHeader(fingerprintReplacement->m_FingerprintIdentifier.m_IdentifierName.c_str());
					if (ImGui::CollapsingHeader(fingerprintReplacement->m_FingerprintIdentifier.m_IdentifierName.c_str()))
					{
						if (ImGui::Combo(std::string{ "CustomModel" }.
										 append("##").append(fingerprintReplacement->m_FingerprintIdentifier.m_IdentifierName)
										 .c_str(),
										 &customModelIndex, customModelNames, model::customModels.size()))
						{
							fingerprintReplacement->m_CustomModel = model::customModels[customModelIndex];
						}
					}
				}
				ImGui::End();
			}

			{
				ImGui::Begin("Custom models");
				for (const auto& customModel : model::customModels)
				{
					if (ImGui::CollapsingHeader(customModel->m_FileName.c_str()))
					{
						ImGui::SliderFloat3("Scale", &customModel->m_Scale.x, -10, 10);
						ImGui::SliderFloat3("Translate", &customModel->m_Translate.x, -10, 10);
						ImGui::SliderFloat3("Rotate", &customModel->m_Rotate.a, -360, 360);
					}
				}
				ImGui::End();
			}
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
			auto customModel = foundFingerprint->m_CustomModel.get();
			if (!customModel)
			{
				return;
			}
			else
			{
				if (customModel->m_SucessfullyLoadedModel)
				{
					originalGlEnableClientState(GL_VERTEX_ARRAY);
					originalGlVertexPointer(3, GL_FLOAT, VERTEX_POINTER_STRIDE, customModel->m_Vertices->data());
					originalGlEnableClientState(GL_TEXTURE_COORD_ARRAY);
					originalGlTexCoordPointer(2, GL_FLOAT, 0, customModel->m_UVs->data());
					originalGlEnableClientState(GL_NORMAL_ARRAY);
					originalGlNormalPointer(GL_FLOAT, 0, customModel->m_Normals->data());
					/*originalGlDisableClientState(GL_NORMAL_ARRAY);
					originalGlNormalPointer(GL_FLOAT, 0, nullptr);*/

					originalGlScalef(customModel->m_Scale.x, customModel->m_Scale.y, customModel->m_Scale.z);
					originalGlTranslatef(customModel->m_Translate.x, customModel->m_Translate.y,
										 customModel->m_Translate.z);
					originalGlRotatef(customModel->m_Rotate.a, customModel->m_Rotate.b,
									  customModel->m_Rotate.c, customModel->m_Rotate.d);

					originalGlEnable(GL_TEXTURE_2D);
					originalGlTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

					originalGlBindTexture(GL_TEXTURE_2D, customModel->m_Texture.m_TextureName);
					originalGlTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					originalGlTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					originalGlDrawArrays(mode, 0, customModel->m_IndexCount);

					originalGlBindTexture(GL_TEXTURE_2D, 0);
					originalGlTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

					return;
				}
			}

			
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

	glRotatef originalGlRotatef{ reinterpret_cast<glRotatef>(0) };
	glTranslatef originalGlTranslatef{ reinterpret_cast<glTranslatef>(0) };
	glScalef originalGlScalef{ reinterpret_cast<glScalef>(0) };

	glGenTextures originalGlGenTextures{ reinterpret_cast<glGenTextures>(0) };
	glBindTexture originalGlBindTexture{ reinterpret_cast<glBindTexture>(0) };
	glTexImage2D originalGlTexImage2D{ reinterpret_cast<glTexImage2D>(0) };

	void __stdcall glBindTextureHook(int target, int texture)
	{
		if (reloadTextures)
		{
			reloadTextures = false;
			for (auto& customModel : model::customModels)
			{
				customModel.get()->m_Texture.LoadTexture(customModel.get()->m_FileName);
			}
		}
		return originalGlBindTexture(target, texture);
	}

	glTexParameteri originalGlTexParameteri{ reinterpret_cast<glTexParameteri>(0) };
	glTexEnvi originalGlTexEnvi{ reinterpret_cast<glTexEnvi>(0) };
	glEnable originalGlEnable{ reinterpret_cast<glEnable>(0) };

}