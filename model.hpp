#pragma once
#include "opengl.hpp"
#include <vector>
#include <memory>
#include <string>

namespace model
{
	class CustomModel {
	public:
		unsigned int m_IndexCount{ 0 };
		std::vector<float> m_Vertices;
		std::vector<float> m_UVs;
		std::vector<float> m_Normals;
		CustomModel(std::string name)
		{
			LoadObj(name);
		}
	private:
		void LoadObj(std::string name, bool invertUVs = true);
	};
}