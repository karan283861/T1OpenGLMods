#pragma once
#include "opengl.hpp"
#include <vector>
#include <memory>
#include <string>

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

	class CustomModel
	{
	public:
		unsigned int m_IndexCount{ 0 };
		std::shared_ptr<std::vector<float>> m_Vertices;
		std::shared_ptr<std::vector<float>> m_UVs;
		std::shared_ptr<std::vector<float>> m_Normals;
		Scale m_Scale;
		Rotate m_Rotate;
		Translate m_Translate;

		CustomModel(std::string name)
		{
			m_Vertices = std::make_shared<std::vector<float>>();
			m_UVs = std::make_shared<std::vector<float>>();
			m_Normals = std::make_shared<std::vector<float>>();
			LoadObj(name);
		}
	private:
		void LoadObj(std::string name, bool invertUVs = true);
	};
}