#include "model.hpp"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj/fast_obj.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace model
{

	std::vector<std::shared_ptr<CustomModel>> customModels;

	void Texture::LoadTexture(std::string name)
	{
		opengl::originalGlGenTextures(1, &m_TextureName);
		m_ImageData = stbi_load(name.c_str(), &m_X, &m_Y, &m_Channels, 0);
		opengl::originalGlBindTexture(GL_TEXTURE_2D, m_TextureName);
		if (m_Channels == 4)
		{
			opengl::originalGlTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_X, m_Y, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_ImageData);
		}
		else
		{
			opengl::originalGlTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_X, m_Y, 0, GL_RGB, GL_UNSIGNED_BYTE, m_ImageData);
		}

	}

	CustomModel::CustomModel(std::string name)
	{
		m_Vertices = std::make_shared<std::vector<float>>();
		m_UVs = std::make_shared<std::vector<float>>();
		m_Normals = std::make_shared<std::vector<float>>();

		static char executeablePath_[256];
		GetModuleFileNameA(NULL, executeablePath_, 255);
		auto executeablePath = std::filesystem::path(executeablePath_);
		auto folderPath = executeablePath.parent_path();

		auto objPath = folderPath / "replacements" / name;

		if (objPath.has_extension() && objPath.extension() == ".obj" && std::filesystem::exists(objPath))
		{
			auto name = objPath;
			m_FileName = name.filename().string();
			LoadObj(std::filesystem::absolute(objPath).string());

			name = name.replace_extension(".png");
			auto pngPath = folderPath / "replacements" / name;
			m_TextureFileName = pngPath.string();

			//m_Texture.LoadTexture(pngPath.string());
			m_SucessfullyLoadedModel = true;

			//customModels.push_back(std::shared_ptr<CustomModel>(customModel));
		}
	}

	void CustomModel::LoadObj(std::string name, bool invertUVs)
	{
		fastObjMesh* mesh = fast_obj_read(name.c_str());
		for (int o = mesh->object_count - 1; o < mesh->object_count; o++) {
			m_IndexCount = 0;
			for (int i = 0; i < mesh->objects[o].face_count; i++) {

				int face_offset = mesh->objects[o].face_offset;
				int face_verticies = mesh->face_vertices[face_offset + i];
				for (int k = 0; k < face_verticies; k++) {
					int j = mesh->objects[o].index_offset + m_IndexCount;

					m_Vertices->push_back(mesh->positions[(mesh->indices[j].p) * 3 + 0]);
					m_Vertices->push_back(mesh->positions[(mesh->indices[j].p) * 3 + 1]);
					m_Vertices->push_back(mesh->positions[(mesh->indices[j].p) * 3 + 2]);
					m_Vertices->push_back(0.0);

					if (mesh->texcoords && mesh->texcoord_count > 0) {
						if (mesh->indices[j].t > 0) {
							m_UVs->push_back(mesh->texcoords[(mesh->indices[j].t) * 2 + 0]);
							if (!invertUVs)
								m_UVs->push_back(mesh->texcoords[(mesh->indices[j].t) * 2 + 1]);
							else
								m_UVs->push_back(1 - mesh->texcoords[(mesh->indices[j].t) * 2 + 1]);
						}
					}

					if (mesh->normals && mesh->normal_count > 0) {
						m_Normals->push_back(mesh->normals[(mesh->indices[j].n) * 3 + 0]);
						m_Normals->push_back(mesh->normals[(mesh->indices[j].n) * 3 + 1]);
						m_Normals->push_back(mesh->normals[(mesh->indices[j].n) * 3 + 2]);
					}
					m_IndexCount++;
				}
			}
			break;
		}
	}

	void Initialise(void)
	{
		static char executeablePath_[256];
		GetModuleFileNameA(NULL, executeablePath_, 255);
		auto executeablePath = std::filesystem::path(executeablePath_);
		auto folderPath = executeablePath.parent_path();
		auto replacementPath = folderPath / "replacements";
		for (const auto& iter : std::filesystem::directory_iterator(replacementPath))
		{
			if (iter.path().extension() == ".obj")
			{
				auto objName = iter.path().filename().string();
				//auto customModel = std::make_shared<CustomModel>(objName);
				customModels.push_back(std::make_shared<CustomModel>(objName));
			}
		}
	}
}