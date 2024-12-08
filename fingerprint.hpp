#pragma once

#include <memory>
#include <vector>
#include <utility>
#include <string>
#include <unordered_map>
#include "imgui/imgui.h"
#include "model.hpp"

namespace fingerprint
{
	class Vertex {
	public:
		float m_X{ 0 }, m_Y{ 0 }, m_Z{ 0 }, m_Unused{ 0 };
	};
	
	class UV {
	public:
		float m_X{ 0 }, m_Y{ 0 };
	};

	class VertexData
	{
	public:
		bool m_Enabled{ false };
		//unsigned int m_Size{ 0 };
		//unsigned int m_Type{ 0 };
		//unsigned int m_Stride{ 0 };
		//std::unique_ptr<float[]> m_VertexBuffer;
		void* m_VertexBuffer{ nullptr };
	};

	

	class UVData
	{
	public:
		bool m_Enabled{ false };
		//unsigned int m_Size{ 0 };
		//unsigned int m_Type{ 0 };
		//unsigned int m_Stride{ 0 };
		//std::unique_ptr<float[]> m_UVBuffer;
		void* m_UVBuffer{ nullptr };
	};



	//class NormalData
	//{
	//public:
	//	bool m_Enabled{ false };
	//	unsigned int m_Type;
	//	unsigned int m_Stride;
	//	//std::unique_ptr<float[]> m_NormalBuffer;
	//	void* m_NormalBuffer{ nullptr };
	//};

	extern VertexData latestVertexData;
	extern UVData latestUVData;

	namespace drawarrays
	{
		class Fingerprint
		{
		public:
			unsigned int m_Mode{ 0 };
			unsigned int m_First{ 0 };
			unsigned int m_Count{ 0 };
			VertexData m_VertexData;
			UVData m_UVData;
			std::shared_ptr<Vertex[]> m_VerticesPtr;
			std::shared_ptr<UV[]> m_UVsPtr;
			//NormalData m_NormalData;
			static unsigned int CountLowerBound;
			static unsigned int CountUpperBound;
			static unsigned int Mode;
		};

		class FingerprintIdentifier
		{
		public:
			std::string m_IdentifierName{};
			unsigned int m_Mode{ 0 };
			//unsigned int m_First{ 0 };
			unsigned int m_Count{ 0 };
			// if vertex array enabled
			std::vector<std::pair<unsigned int, Vertex>> m_IndexVertex;
			// if texture coord array enabled
			std::vector<std::pair<unsigned int, UV>> m_IndexUV;
			static float VertexValueDelta;
			static float UVValueDelta;
		};

		class FingerprintReplacement
		{
		public:
			FingerprintIdentifier m_FingerprintIdentifier;
			std::shared_ptr<model::CustomModel> m_CustomModel;
		};

		extern std::vector<Fingerprint> foundFingerprints;
		extern std::unordered_map<unsigned int, std::unordered_map<unsigned int, std::vector<FingerprintReplacement>>> modeToCountToFingerprintReplacements;

		//extern fingerprint::NormalData latestNormalData;

		void DrawWindow(void);
		void AddFingerprintReplacement(unsigned int mode, unsigned int count, FingerprintReplacement& fingerprintReplacement);
		void Initialise(void);
		std::shared_ptr<model::CustomModel> CheckFingerprintFound(unsigned int mode, unsigned int count);
		//void reset(void);
	}
}