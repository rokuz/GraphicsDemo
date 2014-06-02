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

#ifndef __FBX_LOADER_H__
#define __FBX_LOADER_H__

namespace geom
{

class FbxLoader : public GeometryLoader
{
public:
	FbxLoader();
	virtual ~FbxLoader();
	
	virtual Data load(const std::string& filename);

private:
	FbxManager* m_manager;

	struct GroupData
	{
		std::list<size_t> polygonIndices;
		std::map<size_t, size_t> reindexer;
	};

	void processFbxNode(DataWriter& dataWriter, FbxNode* node, std::list<std::pair<FbxMesh*, Data::Material> >& meshes);
	size_t getAdditionalUVsCount(const std::list<std::pair<FbxMesh*, Data::Material> >& meshes);
    bool checkVertexContent(DataWriter& dataWriter, FbxMesh* mesh);
	void getSmoothingGroup(FbxMesh* mesh, std::vector<int>& group);
	void reindexing(FbxMesh* mesh, std::vector<GroupData>& groups, size_t& reindexerIndex);
	std::string getTextureName(FbxSurfaceMaterial* material, const std::string& textureType);

	template <typename ElementType, typename VectorType> 
	void fbxVectorToBuffer(const VectorType& fbxVec, float* ptr, size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            ptr[i] = (float)fbxVec[(int)i];
        }
    }

    template <typename ElementType, typename VectorType>
    void importComponent(ElementType* element, float* vbuf, FbxMesh* mesh, int polygonIndex, size_t componentSize)
    {
        if (element)
        {
            int elementIndex = 0;
            if (element->GetReferenceMode() == FbxGeometryElement::eDirect )
                elementIndex = polygonIndex;
                        
            if (element->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
                elementIndex = element->GetIndexArray().GetAt(polygonIndex);
                        
            VectorType val = element->GetDirectArray().GetAt(elementIndex);
                    
            fbxVectorToBuffer<ElementType, VectorType>(val, vbuf, componentSize);
        }
    }

	template <>
	void fbxVectorToBuffer<FbxGeometryElementUV, FbxVector2>(const FbxVector2& fbxVec, float* ptr, size_t count)
    {
        ptr[0] = (float)fbxVec[0];
		
		float v = (float)fbxVec[1];
		int iv = (int)v;
		float fv = v - (float)iv;
		ptr[1] = float(iv + 1) - fv;
    }

	void loadMaterial(const std::string& filename, DataWriter& dataWriter);
};

}

#endif