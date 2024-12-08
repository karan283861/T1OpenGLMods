#include "model.hpp"
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj/fast_obj.h"

namespace model
{
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

					m_Vertices->push_back(mesh->positions[(mesh->indices[j].p - 1) * 3 + 0]);
					m_Vertices->push_back(mesh->positions[(mesh->indices[j].p - 1) * 3 + 1]);
					m_Vertices->push_back(mesh->positions[(mesh->indices[j].p - 1) * 3 + 2]);
					m_Vertices->push_back(0.0);

					if (mesh->texcoords && mesh->texcoord_count > 0) {
						if (mesh->indices[j].t > 0) {
							m_UVs->push_back(mesh->texcoords[(mesh->indices[j].t - 1) * 2 + 0]);
							if (!invertUVs)
								m_UVs->push_back(mesh->texcoords[(mesh->indices[j].t - 1) * 2 + 1]);
							else
								m_UVs->push_back(1 - mesh->texcoords[(mesh->indices[j].t - 1) * 2 + 1]);
						}
					}

					if (mesh->normals && mesh->normal_count > 0) {
						m_Normals->push_back(mesh->normals[(mesh->indices[j].n - 1) * 3 + 0]);
						m_Normals->push_back(mesh->normals[(mesh->indices[j].n - 1) * 3 + 1]);
						m_Normals->push_back(mesh->normals[(mesh->indices[j].n - 1) * 3 + 2]);
					}
					m_IndexCount++;
				}
			}
			break;
		}
	}
}