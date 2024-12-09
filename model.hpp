#pragma once
#include <Windows.h>
#include "opengl.hpp"
#include <vector>
#include <memory>
#include <string>
#include <filesystem>

namespace model
{
	struct Translate
	{
		float x, y, z;
	};

	struct Scale
	{
		float x{ 0.5 }, y{ 0.5 }, z{ 0.5 };
	};

	struct Rotate
	{
		float a, b, c, d;
	};

	class Texture
	{
	public:

		int m_X{ 0 };
		int m_Y{ 0 };
		int m_Channels{ 0 };
		unsigned int m_TextureName{ 0 };
		//std::shared_ptr<void> m_ImageData;
		void* m_ImageData{ nullptr };

		void LoadTexture(std::string name);
	};

	class CustomModel
	{
	public:
		unsigned int m_IndexCount{ 0 };
		std::shared_ptr<std::vector<float>> m_Vertices;
		std::shared_ptr<std::vector<float>> m_UVs;
		std::shared_ptr<std::vector<float>> m_Normals;
		Scale m_Scale{ 0.418, 0.418, 0.418 };
		Rotate m_Rotate{ 71.25, 47.917, 2.083, -2.083 };
		Translate m_Translate = { 2.7, -1.146, -4.271 };
		bool m_SucessfullyLoadedModel{ false };
		Texture m_Texture;
		std::string m_FileName;

		CustomModel(std::string name);
	private:
		void LoadObj(std::string name, bool invertUVs = true);
	};

	extern std::vector<std::shared_ptr<CustomModel>> customModels;
}