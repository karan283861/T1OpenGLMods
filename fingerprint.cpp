#include "fingerprint.hpp"
#include <memory>
#include <string>
#include "opengl.hpp"

using std::string;
using std::to_string;

namespace fingerprint
{
	namespace drawarrays
	{
		decltype(Fingerprint::CountLowerBound) Fingerprint::CountLowerBound{ 0 };
		decltype(Fingerprint::CountUpperBound) Fingerprint::CountUpperBound{ 0 };
		decltype(Fingerprint::Mode) Fingerprint::Mode{ 4 };

		decltype(FingerprintIdentifier::VertexValueDelta) FingerprintIdentifier::VertexValueDelta{ 0.001 };
		decltype(FingerprintIdentifier::UVValueDelta) FingerprintIdentifier::UVValueDelta{ 0.001 };

		std::vector<Fingerprint> foundFingerprints;
		std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::vector<FingerprintReplacement>>> modeToCountToFingerprintReplacements;

		void DrawWindow(void)
		{
			static const char* modeLabels[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
			static std::vector<unsigned int> modeValues = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

			ImGui::Begin("Fingerprint");
			{
				ImGui::Combo("Mode", (int*)&Fingerprint::Mode, modeLabels, modeValues.size());

				ImGui::SliderInt("Count (Lower Bound)", (int*)&Fingerprint::CountLowerBound, 0, 1000);
				ImGui::SliderInt("Count (Upper Bound)", (int*)&Fingerprint::CountUpperBound, 0, 1000);

				for (int i = 0; i < foundFingerprints.size(); i++)
				{
					auto& fingerprint = foundFingerprints[i];
					auto indexString = std::to_string(i);
					if (ImGui::CollapsingHeader(indexString.c_str()))
					{
						ImGui::Text(std::string{ "Mode: " }.append(std::to_string(fingerprint.m_Mode)).c_str());
						ImGui::Text(std::string{ "First: " }.append(std::to_string(fingerprint.m_First)).c_str());
						ImGui::Text(std::string{ "Count: " }.append(std::to_string(fingerprint.m_Count)).c_str());

						if (fingerprint.m_VertexData.m_Enabled)
						{
							ImGui::Separator();

							auto vertexDataHeaderTitle = std::string{ "Vertex Data##" }.append(std::to_string(i));
							if (ImGui::CollapsingHeader(vertexDataHeaderTitle.c_str()))
							{
								for (int j = 0; j < fingerprint.m_Count; j++)
								{
									//auto& vertexData = fingerprint.m_VertexData;
									auto vertexDataTitle = std::to_string(j).append("##").append(std::to_string(i));
									if (ImGui::CollapsingHeader(vertexDataTitle.c_str()))
									{
										ImGui::SliderFloat(string{ "X##" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_VerticesPtr.get()))[j * VERTEX_POINTER_SIZE + 0]), -1E5, 1E5, "%.6f");
										ImGui::SliderFloat(string{ "Y##" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_VerticesPtr.get()))[j * VERTEX_POINTER_SIZE + 1]), -1E5, 1E5, "%.6f");
										ImGui::SliderFloat(string{ "Z##" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_VerticesPtr.get()))[j * VERTEX_POINTER_SIZE + 2]), -1E5, 1E5, "%.6f");
									}
								}
							}
						}

						if (fingerprint.m_UVData.m_Enabled)
						{
							ImGui::Separator();

							auto uvDataHeaderTitle = std::string{ "UV Data##" }.append(std::to_string(i));
							if (ImGui::CollapsingHeader(uvDataHeaderTitle.c_str()))
							{
								for (int j = 0; j < 10; j++) // TODO: Remove magic number
								{
									auto& uvData = fingerprint.m_UVData;
									auto uvDataTitle = std::to_string(j).append("##").append(std::to_string(i));
									if (ImGui::CollapsingHeader(uvDataTitle.c_str()))
									{
										ImGui::SliderFloat(string{ "X##" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(uvData.m_UVBuffer))[j * TEXCOORD_POINTER_SIZE + 0]), 0, 1, "%.6f");
										ImGui::SliderFloat(string{ "Y##" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(uvData.m_UVBuffer))[j * TEXCOORD_POINTER_SIZE + 1]), 0, 1, "%.6f");
									}
								}
							}
						}
					}
				}

			}
			ImGui::End();

			ImGui::Begin("Fingerprint Replacement");
			{
				if (ImGui::CollapsingHeader("New"))
				{
					static FingerprintReplacement fingerprintReplacement{};
					auto& fingerprintIdentifier = fingerprintReplacement.m_FingerprintIdentifier;
					ImGui::Combo("Mode", (int*)&fingerprintIdentifier.m_Mode, modeLabels, modeValues.size());
					ImGui::SliderInt("Count", (int*)&fingerprintIdentifier.m_Count, 0, 1000);
					ImGui::Separator();
					static std::pair<unsigned int, Vertex> indexVertex;
					if (ImGui::CollapsingHeader("Vertex Data"))
					{
						ImGui::SliderInt("Index", (int*)&indexVertex.first, 0, fingerprintIdentifier.m_Count);
						ImGui::SliderFloat3("Vertex", (float*)&indexVertex.second, -10000, 10000, "%.6f");
						if (ImGui::Button("Add Vertex"))
						{
							fingerprintIdentifier.m_IndexVertex.push_back(indexVertex);
							indexVertex = std::pair<unsigned int, Vertex>{};
						}
					}
					static std::pair<unsigned int, UV> indexUV;
					if (ImGui::CollapsingHeader("UV Data"))
					{
						ImGui::SliderInt("Index", (int*)&indexVertex.first, 0, fingerprintIdentifier.m_Count);
						ImGui::SliderFloat2("UV", (float*)&indexUV.second, 0, 1, "%.6f");
						if (ImGui::Button("Add UV"))
						{
							fingerprintIdentifier.m_IndexUV.push_back(indexUV);
							indexUV = std::pair<unsigned int, UV>{};
						}
					}

					ImGui::Separator();
					if (ImGui::Button("Add Fingerprint"))
					{
						AddFingerprintReplacement(fingerprintReplacement.m_FingerprintIdentifier.m_Mode,
												  fingerprintReplacement.m_FingerprintIdentifier.m_Count,
												  fingerprintReplacement);
						fingerprintReplacement = FingerprintReplacement{};
					}
				}
				if (ImGui::CollapsingHeader("Existing"))
				{
					for (auto& [mode, countToFingerprintReplacements] : modeToCountToFingerprintReplacements)
					{
						if (ImGui::CollapsingHeader(std::string{ "Mode " }.append(std::to_string(mode)).c_str()))
						{
							for (auto& [count, fingerprintReplacements] : countToFingerprintReplacements)
							{
								if (ImGui::CollapsingHeader(std::string{ "Count " }.
															append(std::to_string(count)).
															append("##").append(std::to_string(mode)).c_str()))
								{
									for (int i = 0; i < fingerprintReplacements.size(); i++)
									{
										auto& fingerprintReplacement = fingerprintReplacements[i];
										auto& fingerprintIdentifier = fingerprintReplacement.m_FingerprintIdentifier;
										if (ImGui::CollapsingHeader(std::string{ "Fingerprint Replacement" }.
																	append("##").append(std::to_string(i)).c_str()))
										{
											for (auto& vertexData : fingerprintIdentifier.m_IndexVertex)
											{
												ImGui::Text(std::string{ "Index: " }.append(std::to_string(vertexData.first)).c_str());
												ImGui::Text(std::string{ "Vertex X: " }.append(std::to_string(vertexData.second.m_X)).c_str());
												ImGui::Text(std::string{ "Vertex Y: " }.append(std::to_string(vertexData.second.m_Y)).c_str());
												ImGui::Text(std::string{ "Vertex Z: " }.append(std::to_string(vertexData.second.m_Z)).c_str());
												ImGui::Separator();
											}
										}
									}
								}
							}
						}
					}
				}
			}
			ImGui::End();
		}

		void AddFingerprintReplacement(unsigned int mode, unsigned int count, FingerprintReplacement& fingerprintReplacement)
		{
			modeToCountToFingerprintReplacements[mode][count].push_back(fingerprintReplacement);
		}

		void Initialise(void)
		{
			auto spinfusorMainFingerprintReplacement = FingerprintReplacement{};
			auto& spinfusorMainFingerprintIdentifer = spinfusorMainFingerprintReplacement.m_FingerprintIdentifier;
			spinfusorMainFingerprintIdentifer.m_IdentifierName = std::string{ "Main spinfusor body" };
			spinfusorMainFingerprintIdentifer.m_Mode = 4;
			spinfusorMainFingerprintIdentifer.m_Count = 108;
			spinfusorMainFingerprintIdentifer.m_IndexVertex = { {0, {0.049055, -0.029239, -0.008155, 0}} };
			//spinfusorMainFingerprintIdentifer.m_IndexUV = { {0, {0, 0}} };
			
			AddFingerprintReplacement(4, 108, spinfusorMainFingerprintReplacement);
		}

		/*void reset(void)
		{
			foundFingerprints.clear();
		}*/
	}
}