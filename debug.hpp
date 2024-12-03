#pragma once

#include <memory>
#include <vector>
#include "imgui/imgui.h"

namespace debug
{
	namespace fingerprint
	{
		class VertexData
		{
		public:
			bool m_Enabled{ false };
			unsigned int m_Size{ 0 };
			unsigned int m_Type{ 0 };
			unsigned int m_Stride{ 0 };
			//std::unique_ptr<float[]> m_VertexBuffer;
			void* m_VertexBuffer{ nullptr };
		};

		class UVData
		{
		public:
			bool m_Enabled{ false };
			unsigned int m_Size{ 0 };
			unsigned int m_Type{ 0 };
			unsigned int m_Stride{ 0 };
			//std::unique_ptr<float[]> m_UVBuffer;
			void* m_UVBuffer{ nullptr };
		};

		class NormalData
		{
		public:
			bool m_Enabled{ false };
			unsigned int m_Type;
			unsigned int m_Stride;
			//std::unique_ptr<float[]> m_NormalBuffer;
			void* m_NormalBuffer{ nullptr };
		};

		class DrawArraysFingerprint
		{
		public:
			unsigned int m_Mode{ 0 };
			unsigned int m_First{ 0 };
			unsigned int m_Count{ 0 };
			VertexData m_VertexData;
			UVData m_UVData;
			NormalData m_NormalData;
			static unsigned int CountLowerBound;
			static unsigned int CountUpperBound;
			static unsigned int Mode;
		};

		extern std::vector<DrawArraysFingerprint> drawArraysFingerprints;

		void DrawFingerprintWindow(void);
	}

	void reset(void);
}