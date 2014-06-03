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

#pragma once

namespace framework
{

class Geometry3D;
class Texture;

enum MaterialTexture
{
	MAT_DIFFUSE_MAP = 0,
	MAT_NORMAL_MAP,
	MAT_SPECULAR_MAP,

	MAT_TEXTURE_COUNT
};

class MaterialManager
{
public:
	static MaterialManager& instance();

	void initializeMaterial(const std::shared_ptr<Geometry3D>& geometry);
	void initializeMaterial(const std::shared_ptr<Geometry3D>& geometry, 
							const std::string& diffuseMap,
							const std::string& normalMap,
							const std::string& specularMap);
	std::shared_ptr<Texture> getTexture(const std::shared_ptr<Geometry3D>& geometry, int meshIndex, MaterialTexture textureType);

	void destroy();

private:
	MaterialManager();
	~MaterialManager();

	struct MaterialData
	{
		std::weak_ptr<Texture> textures[MAT_TEXTURE_COUNT];
	};
	std::map<int, std::vector<MaterialData> > m_materials;
	std::map<std::string, std::shared_ptr<Texture> > m_textures;

	std::string findTextureName(const std::string& dir, const std::string& name);
	std::weak_ptr<Texture> createTexture(const std::string& path); 
};

}