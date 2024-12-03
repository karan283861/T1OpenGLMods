#include "debug.hpp"
#include <memory>
#include <string>
namespace debug
{
	namespace fingerprint
	{
		decltype(DrawArraysFingerprint::CountLowerBound) DrawArraysFingerprint::CountLowerBound{ 0 };
		decltype(DrawArraysFingerprint::CountUpperBound) DrawArraysFingerprint::CountUpperBound{ 0 };
		decltype(DrawArraysFingerprint::Mode) DrawArraysFingerprint::Mode{ 4 };

		std::vector<DrawArraysFingerprint> drawArraysFingerprints;

		void DrawFingerprintWindow(void)
		{
			ImGui::Begin("Fingerprint");
			ImGui::SliderInt("Count (Lower Bound)", (int*)&DrawArraysFingerprint::CountLowerBound, 0, 1000);
			ImGui::SliderInt("Count (Upper Bound)", (int*)&DrawArraysFingerprint::CountUpperBound, 0, 1000);
			for (int i = 0; i < drawArraysFingerprints.size(); i++)
			{
				auto& fingerprint = drawArraysFingerprints[i];
				auto indexString = std::to_string(i);
				if (ImGui::CollapsingHeader(indexString.c_str()))
				{
					ImGui::Text(std::string{ "Mode: " }.append(std::to_string(fingerprint.m_Mode)).c_str());
					ImGui::Text(std::string{ "First: " }.append(std::to_string(fingerprint.m_First)).c_str());
					ImGui::Text(std::string{ "Count: " }.append(std::to_string(fingerprint.m_Count)).c_str());
					ImGui::Separator();
					if (fingerprint.m_VertexData.m_Enabled)
					{
						auto vertexDataHeaderTitle = std::string{ "Vertex Data##" }.append(std::to_string(i));
						if (ImGui::CollapsingHeader(vertexDataHeaderTitle.c_str()))
						{
							for (int j = 0; j < fingerprint.m_Count; j++)
							{
								auto& vertexData = fingerprint.m_VertexData;
								auto vertexDataTitle = std::to_string(j).append("##").append(std::to_string(i));
								if (ImGui::CollapsingHeader(vertexDataTitle.c_str()))
								{

									ImGui::Text(std::string{ "X: " }.append(std::to_string(((float*)(vertexData.m_VertexBuffer))[j * 4 + 0])).c_str());
									ImGui::Text(std::string{ "Y: " }.append(std::to_string(((float*)(vertexData.m_VertexBuffer))[j * 4 + 1])).c_str());
									ImGui::Text(std::string{ "Z: " }.append(std::to_string(((float*)(vertexData.m_VertexBuffer))[j * 4 + 2])).c_str());
								}
							}
						}
					}
				}
			}
			ImGui::End();
		}
	}

	void reset(void)
	{
		fingerprint::drawArraysFingerprints.clear();
	}
}