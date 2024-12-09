#include "fingerprint.hpp"
#include <memory>
#include <string>
#include "opengl.hpp"
#include <plog/Log.h>
#include <format>

using std::string;
using std::to_string;

namespace fingerprint
{
	fingerprint::VertexData latestVertexData;
	fingerprint::UVData latestUVData;

	namespace drawarrays
	{
		decltype(Fingerprint::CountLowerBound) Fingerprint::CountLowerBound{ 0 };
		decltype(Fingerprint::CountUpperBound) Fingerprint::CountUpperBound{ 0 };
		decltype(Fingerprint::Mode) Fingerprint::Mode{ 4 };

		decltype(FingerprintIdentifier::VertexValueDelta) FingerprintIdentifier::VertexValueDelta{ 0.001 };
		decltype(FingerprintIdentifier::UVValueDelta) FingerprintIdentifier::UVValueDelta{ 0.001 };

		std::vector<Fingerprint> foundFingerprints;
		std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::vector<std::shared_ptr<FingerprintReplacement>>>> modeToCountToFingerprintReplacements;
		std::vector<std::shared_ptr<FingerprintReplacement>> fingerprintReplacements;

		FingerprintReplacement::FingerprintReplacement(void)
		{
			fingerprintReplacements.push_back(std::shared_ptr<FingerprintReplacement>(this));
		}

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
					auto indexString = to_string(i);
					ImGui::Indent();
					if (ImGui::CollapsingHeader(indexString.c_str()))
					{
						ImGui::Text(string{ "Mode: " }.append(to_string(fingerprint.m_Mode)).c_str());
						ImGui::Text(string{ "First: " }.append(to_string(fingerprint.m_First)).c_str());
						ImGui::Text(string{ "Count: " }.append(to_string(fingerprint.m_Count)).c_str());

						if (fingerprint.m_VertexData.m_Enabled || true)
						{
							ImGui::Separator();
							ImGui::Indent();

							auto vertexDataHeaderTitle = string{ "Vertex Data##" }.append(to_string(i));
							if (ImGui::CollapsingHeader(vertexDataHeaderTitle.c_str()))
							{
								for (int j = 0; j < fingerprint.m_Count; j++)
								{
									ImGui::Indent();
									//auto& vertexData = fingerprint.m_VertexData;
									auto vertexDataTitle = to_string(j).append("##").append(to_string(i));
									if (ImGui::CollapsingHeader(vertexDataTitle.c_str()))
									{
										ImGui::SliderFloat(string{ "X##V" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_VerticesPtr.get()))[j * VERTEX_POINTER_SIZE + 0]), -1E5, 1E5, "%.6f");
										ImGui::SliderFloat(string{ "Y##V" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_VerticesPtr.get()))[j * VERTEX_POINTER_SIZE + 1]), -1E5, 1E5, "%.6f");
										ImGui::SliderFloat(string{ "Z##V" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_VerticesPtr.get()))[j * VERTEX_POINTER_SIZE + 2]), -1E5, 1E5, "%.6f");
									}
									ImGui::Unindent();
								}
							}

							ImGui::Unindent();
						}

						if (fingerprint.m_UVData.m_Enabled || true)
						{
							ImGui::Separator();
							ImGui::Indent();

							auto headerTitle = string{ "UV Data##" }.append(to_string(i));
							if (ImGui::CollapsingHeader(headerTitle.c_str()))
							{
								for (int j = 0; j < 5; j++)
								{
									ImGui::Indent();
									auto dataTitle = to_string(j).append("##").append(to_string(i));
									if (ImGui::CollapsingHeader(dataTitle.c_str()))
									{
										ImGui::SliderFloat(string{ "X##U" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_UVsPtr.get()))[j * TEXCOORD_POINTER_SIZE + 0]), -1, 1, "%.6f");
										ImGui::SliderFloat(string{ "Y##U" }.append(to_string(i)).append("##").append(to_string(j)).c_str(),
														   &(((float*)(fingerprint.m_UVsPtr.get()))[j * TEXCOORD_POINTER_SIZE + 1]), -1, 1, "%.6f");
									}
									ImGui::Unindent();
								}
							}
							ImGui::Unindent();
						}
					}
					ImGui::Unindent();
				}

			}
			ImGui::End();

			ImGui::Begin("Fingerprint Replacement");
			{
				ImGui::Indent();
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
					/*static std::pair<unsigned int, UV> indexUV;
					if (ImGui::CollapsingHeader("UV Data"))
					{
						ImGui::SliderInt("Index", (int*)&indexVertex.first, 0, fingerprintIdentifier.m_Count);
						ImGui::SliderFloat2("UV", (float*)&indexUV.second, 0, 1, "%.6f");
						if (ImGui::Button("Add UV"))
						{
							fingerprintIdentifier.m_IndexUV.push_back(indexUV);
							indexUV = std::pair<unsigned int, UV>{};
						}
					}*/

					ImGui::Separator();
					if (ImGui::Button("Add Fingerprint"))
					{
						/*AddFingerprintReplacement(fingerprintReplacement.m_FingerprintIdentifier.m_Mode,
												  fingerprintReplacement.m_FingerprintIdentifier.m_Count,
												  fingerprintReplacement);*/
						fingerprintReplacement = FingerprintReplacement{};
					}
				}
				if (ImGui::CollapsingHeader("Existing"))
				{
					for (auto& [mode, countToFingerprintReplacements] : modeToCountToFingerprintReplacements)
					{
						if (ImGui::CollapsingHeader(string{ "Mode " }.append(to_string(mode)).c_str()))
						{
							for (auto& [count, fingerprintReplacements] : countToFingerprintReplacements)
							{
								if (ImGui::CollapsingHeader(string{ "Count " }.
															append(to_string(count)).
															append("##").append(to_string(mode)).c_str()))
								{
									for (int i = 0; i < fingerprintReplacements.size(); i++)
									{
										auto& fingerprintReplacement = fingerprintReplacements[i];
										auto& fingerprintIdentifier = fingerprintReplacement->m_FingerprintIdentifier;
										if (ImGui::CollapsingHeader(string{ "Fingerprint Replacement" }.
																	append("##").append(to_string(i)).c_str()))
										{
											for (auto& vertexData : fingerprintIdentifier.m_IndexVertex)
											{
												ImGui::Text(string{ "Index: " }.append(to_string(vertexData.first)).c_str());
												ImGui::Text(string{ "Vertex X: " }.append(to_string(vertexData.second.m_X)).c_str());
												ImGui::Text(string{ "Vertex Y: " }.append(to_string(vertexData.second.m_Y)).c_str());
												ImGui::Text(string{ "Vertex Z: " }.append(to_string(vertexData.second.m_Z)).c_str());
												ImGui::Separator();
											}
										}
									}
								}
							}
						}
					}
				}
				ImGui::Unindent();
			}
			ImGui::End();
		}

		//void AddFingerprintReplacement(unsigned int mode, unsigned int count, FingerprintReplacement& fingerprintReplacement)
		//{
		//	//modeToCountToFingerprintReplacements[mode][count].push_back(fingerprintReplacement);
		//}

		//void AddFingerprintReplacement(FingerprintReplacement& fingerprintReplacement)
		//{
		//	//modeToCountToFingerprintReplacements[fingerprintReplacement.m_FingerprintIdentifier.m_Mode][fingerprintReplacement.m_FingerprintIdentifier.m_Count].push_back(fingerprintReplacement);
		//}

		void AddFingerprintReplacement(std::shared_ptr<FingerprintReplacement> fingerprintReplacement)
		{
			modeToCountToFingerprintReplacements[fingerprintReplacement->m_FingerprintIdentifier.m_Mode][fingerprintReplacement->m_FingerprintIdentifier.m_Count].push_back(fingerprintReplacement);
		}

		void Initialise(void)
		{
			auto spinfusorMainFingerprintReplacement = std::make_shared<FingerprintReplacement>();
			auto& spinfusorMainFingerprintIdentifer = spinfusorMainFingerprintReplacement->m_FingerprintIdentifier;
			spinfusorMainFingerprintIdentifer.m_IdentifierName = string{ "Main spinfusor body" };
			spinfusorMainFingerprintIdentifer.m_Mode = 4;
			spinfusorMainFingerprintIdentifer.m_Count = -1;
			spinfusorMainFingerprintIdentifer.m_IndexUV = { {0, {0.946972, 0.325699}} };
			//spinfusorMainFingerprintReplacement->m_CustomModel = std::make_shared<model::CustomModel>("mp5.obj");
			AddFingerprintReplacement(spinfusorMainFingerprintReplacement);

			//model::customModels.push_back(spinfusorMainFingerprintReplacement.m_CustomModel);
			//auto spinfusorDiscFingerprint1Replacement = FingerprintReplacement{};
			//auto& spinfusorDiscFingerprint1Identifer = spinfusorDiscFingerprint1Replacement.m_FingerprintIdentifier;
			//spinfusorDiscFingerprint1Identifer.m_IdentifierName = string{ "Spinfusir disc body 1" };
			//spinfusorDiscFingerprint1Identifer.m_Mode = 4;
			//spinfusorDiscFingerprint1Identifer.m_Count = 90;
			//spinfusorDiscFingerprint1Identifer.m_IndexVertex = { {0, {-0.000559, -0.000559, 0.081636, 0}} };
			//AddFingerprintReplacement(spinfusorDiscFingerprint1Identifer.m_Mode,
			//						  spinfusorDiscFingerprint1Identifer.m_Count,
			//						  spinfusorDiscFingerprint1Replacement);

			auto spinfusorDiscFingerprint2Replacement = std::make_shared<FingerprintReplacement>();
			auto& spinfusorDiscFingerprint2Identifer = spinfusorDiscFingerprint2Replacement->m_FingerprintIdentifier;
			spinfusorDiscFingerprint2Identifer.m_IdentifierName = string{ "Spinfusor disc" };
			spinfusorDiscFingerprint2Identifer.m_Mode = 4;
			spinfusorDiscFingerprint2Identifer.m_Count = -1;
			spinfusorDiscFingerprint2Identifer.m_IndexUV = { {0, {0.219318, 0.846034}} };
			//spinfusorDiscFingerprint2Replacement.m_CustomModel = std::make_shared<model::CustomModel>("cube.obj");
			AddFingerprintReplacement(spinfusorDiscFingerprint2Replacement);

			//auto chaingunFingerprintReplacement = FingerprintReplacement{};
			//auto& chaingunFingerprintReplacementIdentifer = spinfusorDiscFingerprint2Replacement.m_FingerprintIdentifier;
			//chaingunFingerprintReplacementIdentifer.m_IdentifierName = string{ "Chaingun" };
			//chaingunFingerprintReplacementIdentifer.m_Mode = 4;
			//chaingunFingerprintReplacementIdentifer.m_Count = -1;
			//chaingunFingerprintReplacementIdentifer.m_IndexUV = { {0, {0.549891, 0.375469}} };
			////spinfusorDiscFingerprint2Replacement.m_CustomModel = std::make_shared<model::CustomModel>("cube.obj");
			//AddFingerprintReplacement(spinfusorDiscFingerprint2Identifer.m_Mode,
			//						  spinfusorDiscFingerprint2Identifer.m_Count,
			//						  spinfusorDiscFingerprint2Replacement);
		
			
			auto chaingunFingerprintPiece1Replacement = std::make_shared<FingerprintReplacement>();
																			/*FingerprintReplacement{ {"Chaingun piece 1",
																			4,
																			18 * 0 - 1,
																			{},
																			{{0, {0.579140, 0.787832}}}},
																			nullptr};*/
			chaingunFingerprintPiece1Replacement->m_FingerprintIdentifier = FingerprintIdentifier{ "Chaingun piece 1",
																			4,
																			18 * 0 - 1,
																			{},
																			{{0, {0.579140, 0.787832} }} };
			AddFingerprintReplacement(chaingunFingerprintPiece1Replacement);

			auto chaingunFingerprintPiece2Replacement = std::make_shared<FingerprintReplacement>();
			chaingunFingerprintPiece2Replacement->m_FingerprintIdentifier = FingerprintIdentifier{ "Chaingun piece 2",
																			4,
																			42 * 0 - 1,
																			{},
																			{{0, {0.831307, 0.565450}}} };
			AddFingerprintReplacement(chaingunFingerprintPiece2Replacement);

			auto chaingunFingerprintBarrelReplacement = std::make_shared<FingerprintReplacement>();
			chaingunFingerprintBarrelReplacement->m_FingerprintIdentifier = FingerprintIdentifier{ "Chaingun Barrel",
																			4,
																			84 * 0 - 1,
																			{},
																			{{0, {0.140749, 0.985437}}} };
			AddFingerprintReplacement(chaingunFingerprintBarrelReplacement);

			auto chaingunFingerprintPiece3Replacement = std::make_shared<FingerprintReplacement>();
			chaingunFingerprintPiece3Replacement->m_FingerprintIdentifier = FingerprintIdentifier{ "Chaingun piece 3",
																			4,
																			147 * 0 - 1,
																			{},
																			{{0, {0.549891, 0.375469}}} };
			//chaingunFingerprintPiece3Replacement->m_CustomModel = std::make_shared<model::CustomModel>("mp5.obj");
			AddFingerprintReplacement(chaingunFingerprintPiece3Replacement);


		}

		/*void reset(void)
		{
			foundFingerprints.clear();
		}*/

		std::shared_ptr<FingerprintReplacement> CheckFingerprintFound(unsigned int mode, unsigned int count)
		{
			auto countToFingerprintReplacements = modeToCountToFingerprintReplacements.find(mode);
			if (countToFingerprintReplacements != modeToCountToFingerprintReplacements.end())
			{
				auto fingerprintReplacements = countToFingerprintReplacements->second.find(count);
				if (fingerprintReplacements != countToFingerprintReplacements->second.end())
				{
					auto& replacements = fingerprintReplacements->second;
					auto replacementFound{ false };
					for (auto& replacement : replacements)
					{
						auto& fingerprintIdentifier = replacement->m_FingerprintIdentifier;
						if (fingerprintIdentifier.m_Mode == mode && (fingerprintIdentifier.m_Count == count ||
																	 count == -1))
						{
							auto foundVertex{ fingerprintIdentifier.m_IndexVertex.size() == 0 };
							if (true)
							{
								foundVertex = false;
								auto foundAllVertex{ true };
								//for (auto& indexVertex : fingerprintIdentifier.m_IndexVertex)
								//{
								//	auto index = indexVertex.first;
								//	// get rid of magic numbers
								//	auto vertexBufferX{ ((float*)(latestVertexData.m_VertexBuffer))[index * VERTEX_POINTER_SIZE + 0] };
								//	auto vertexBufferY{ ((float*)(latestVertexData.m_VertexBuffer))[index * VERTEX_POINTER_SIZE + 1] };
								//	auto vertexBufferZ{ ((float*)(latestVertexData.m_VertexBuffer))[index * VERTEX_POINTER_SIZE + 2] };

								//	if (abs(indexVertex.second.m_X - vertexBufferX) < fingerprint::drawarrays::FingerprintIdentifier::VertexValueDelta &&
								//		abs(indexVertex.second.m_Y - vertexBufferY) < fingerprint::drawarrays::FingerprintIdentifier::VertexValueDelta &&
								//		abs(indexVertex.second.m_Z - vertexBufferZ) < fingerprint::drawarrays::FingerprintIdentifier::VertexValueDelta)
								//	{
								//		PLOG_DEBUG << std::format("Found Vertex. Index: {0}, X: {1}, Y: {2}, Z: {3}",
								//								  indexVertex.first, indexVertex.second.m_X, indexVertex.second.m_Y, indexVertex.second.m_Z);
								//	}
								//	else
								//	{
								//		PLOG_DEBUG << std::format("Did NOT find Vertex. Index: {0}, X: {1}, Y: {2}, Z: {3}",
								//								  indexVertex.first, indexVertex.second.m_X, indexVertex.second.m_Y, indexVertex.second.m_Z);
								//		foundAllVertex = false;
								//		return nullptr;
								//		break;
								//	}
								//}
								foundVertex = foundAllVertex;
							}

							auto foundUV{ fingerprintIdentifier.m_IndexUV.size() == 0 };

							if (true)
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
										//return nullptr;
										break;
									}
								}
								foundUV = foundAllUV;
							}

							if (foundVertex && foundUV)
							{
								PLOG_DEBUG << "Fingerprint found: " << std::format("Mode: {0}, Count: {1}", mode, count);
								return replacement;
							}
						}
					}
				}
			}
			return nullptr;
		}
	}
}