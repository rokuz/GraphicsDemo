/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "materialmanager.h"

namespace framework
{

MaterialManager& MaterialManager::instance()
{
	static MaterialManager manager;
	return manager;
}

MaterialManager::MaterialManager()
{
}

MaterialManager::~MaterialManager()
{
}

void MaterialManager::initializeMaterial( const std::shared_ptr<Geometry3D>& geometry )
{
	if (!geometry) return;
	auto meshes = geometry->getMeshes();

	int id = geometry->getID();
	if (m_materials.find(id) != m_materials.end()) return;

	std::vector<MaterialData> dat;
	dat.reserve(meshes.size());

	std::string dir = utils::Utils::getPath(geometry->getFilename());
	for (size_t i = 0; i < meshes.size(); i++)
	{
		MaterialData mat;

		if (!meshes[i].material.diffuseMapFilename.empty())
		{
			std::string path = dir + meshes[i].material.diffuseMapFilename + ".dds";
			if (utils::Utils::exists(path))
			{
				mat.textures[MAT_DIFFUSE_MAP] = createTexture(path);
			}
		}

		if (!meshes[i].material.normalMapFilename.empty())
		{
			std::string path = dir + meshes[i].material.normalMapFilename + ".dds";
			if (utils::Utils::exists(path))
			{
				mat.textures[MAT_NORMAL_MAP] = createTexture(path);
			}
		}

		if (!meshes[i].material.specularMapFilename.empty())
		{
			std::string path = dir + meshes[i].material.specularMapFilename + ".dds";
			if (utils::Utils::exists(path))
			{
				mat.textures[MAT_NORMAL_MAP] = createTexture(path);
			}
		}

		dat.push_back(mat);
	}

	m_materials.insert(std::make_pair(id, dat));
}

std::shared_ptr<Texture> MaterialManager::getTexture( const std::shared_ptr<Geometry3D>& geometry, int meshIndex, MaterialTexture textureType )
{
	return std::shared_ptr<Texture>();
}

std::weak_ptr<Texture> MaterialManager::createTexture( const std::string& path )
{
	auto it = m_textures.find(path);
	if (it != m_textures.end())
	{
		return it->second;
	}

	std::shared_ptr<Texture> tex(new Texture());
	if (!tex->initWithDDS(path))
	{
		return std::weak_ptr<Texture>();
	}
	m_textures.insert(std::make_pair(path, tex));

	return m_textures[path];
}

void MaterialManager::destroy()
{
	m_materials.clear();
	m_textures.clear();
}

}