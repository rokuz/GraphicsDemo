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
#include "fbxloader.h"

namespace geom
{

FbxLoader::FbxLoader(): 
	m_manager(0)
{
	m_manager = FbxManager::Create();
	FbxIOSettings* pIOsettings = FbxIOSettings::Create(m_manager, IOSROOT);
	m_manager->SetIOSettings(pIOsettings);
}

FbxLoader::~FbxLoader()
{
    if (m_manager != 0)
    {
        m_manager->Destroy();
        m_manager = 0;
    }
}

Data FbxLoader::load(const std::string& filename)
{
	Data data;
	DataWriter writer(&data);

    FbxImporter* importer = FbxImporter::Create(m_manager, "");
    FbxScene* fbxScene = FbxScene::Create(m_manager, "importedScene");
        
    bool success = importer->Initialize(filename.c_str(), -1, m_manager->GetIOSettings());
    if (!success)
    {  
        if (importer->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
        {
            writer.getLastErrorRef() = "Version of fbx-file is not supported";
        }
		else
		{
			FbxString error = importer->GetStatus().GetErrorString();
			writer.getLastErrorRef() = std::string(error.Buffer());
		}
            
        fbxScene->Destroy();
        importer->Destroy();
        return data;
    }
        
    success = importer->Import(fbxScene);
    if (!success) 
	{
		writer.getLastErrorRef() = "Importing from fbx scene failed";
		return data;
	}
        
    importer->Destroy();
        
    std::list<std::pair<FbxMesh*, Data::Material> > meshes;
    FbxNode* pFbxRootNode = fbxScene->GetRootNode();
    if (pFbxRootNode != 0)
    {
        ProcessFbxNode(writer, pFbxRootNode, meshes);
        for(int i = 0; i < pFbxRootNode->GetChildCount(); i++)
        {
            FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);
            ProcessFbxNode(writer, pFbxChildNode, meshes);
        }
    }
	if (!data.isCorrect())
	{
		fbxScene->Destroy();
		return data;
	}
        
    if (meshes.size() == 0)
    {
		writer.getLastErrorRef() = "There is not any mesh in the file";

        fbxScene->Destroy();
        return data;
    }
        
    writer.getMeshesRef().resize(meshes.size());
        
	std::vector< std::vector<GroupData> > meshesData;
	meshesData.resize(meshes.size());
	size_t i = 0;
	size_t reindexerIndex = 0;
	size_t verticesCount = 0;
    size_t indicesCount = 0;
	for (auto it = meshes.cbegin(); it != meshes.cend(); ++it, ++i)
    {
		Reindexing(it->first, meshesData[i], reindexerIndex);
		for (size_t j = 0; j < meshesData[i].size(); j++)
		{
			verticesCount += meshesData[i][j].reindexer.size();
			indicesCount += meshesData[i][j].polygonIndices.size() * 3;
		}
	}
		
	writer.getVerticesCountRef() = verticesCount;
    writer.getAdditionalUVsCountRef() = GetAdditionalUVsCount(meshes);
        
    size_t vertexSize = data.getVertexSize();
        
	writer.getVertexBufferRef().resize(vertexSize * verticesCount);
    memset(writer.getVertexBufferRef().data(), 0, writer.getVertexBufferRef().size()); // clear

    size_t meshesCounter = 0;
	writer.getBoundingBoxRef().begin_extend();

    for (auto it = meshes.cbegin(); it != meshes.cend(); ++it, ++meshesCounter)
    {
		size_t vertexCounter = 0;
		for (size_t j = 0; j < meshesData[meshesCounter].size(); j++)
		{
			for (std::list<size_t>::iterator it2 = meshesData[meshesCounter][j].polygonIndices.begin(); it2 != meshesData[meshesCounter][j].polygonIndices.end(); ++it2)
			{
				for (size_t k = 0; k < 3; k++)
				{
					int index_k = it->first->GetPolygonVertex(*it2, k);
					size_t new_index = meshesData[meshesCounter][j].reindexer[(size_t)index_k];

					for (size_t component = 0; component < data.getVertexComponentsCount(); component++)
					{
						size_t offset = data.getVertexComponentOffset(component);
						size_t componentSize = data.getVertexComponentSize(component);
            
						// fill vertex buffer
						if (component == 0) // position
						{
							float* ptr = (float*)(writer.getVertexBufferRef().data() + new_index * vertexSize + offset);
							FbxVector4 vertex = it->first->GetControlPointAt(index_k);
							ptr[0] = (float)vertex[0];
							ptr[1] = (float)vertex[1];
							ptr[2] = (float)vertex[2];
							writer.getBoundingBoxRef().extend(ptr[0], ptr[1], ptr[2]);
						}
						else if (component == 1) // normal
						{
							FbxGeometryElementNormal* normalElement = it->first->GetElementNormal();
							float* ptr = (float*)(writer.getVertexBufferRef().data() + new_index * vertexSize + offset);
							ImportComponent<FbxGeometryElementNormal, FbxVector4>(normalElement, ptr, it->first, (*it2) * 3 + k, componentSize / sizeof(float));
						}
						else if (component == 2) // uv0
						{
							FbxGeometryElementUV* uvElement = it->first->GetElementUV(0);
							float* ptr = (float*)(writer.getVertexBufferRef().data() + new_index * vertexSize + offset);
							ImportComponent<FbxGeometryElementUV, FbxVector2>(uvElement, ptr, it->first, (*it2) * 3 + k, componentSize / sizeof(float));
						}
						else if (component == 3) // tangent
						{
							FbxGeometryElementTangent* tangentElement = it->first->GetElementTangent();
							float* ptr = (float*)(writer.getVertexBufferRef().data() + new_index * vertexSize + offset);
							ImportComponent<FbxGeometryElementTangent, FbxVector4>(tangentElement, ptr, it->first, (*it2) * 3 + k, componentSize / sizeof(float));
						}
						else if (component == 4) // binormal
						{
							FbxGeometryElementBinormal* binormalElement = it->first->GetElementBinormal();
							float* ptr = (float*)(writer.getVertexBufferRef().data() + new_index * vertexSize + offset);
							ImportComponent<FbxGeometryElementBinormal, FbxVector4>(binormalElement, ptr, it->first, (*it2) * 3 + k, componentSize / sizeof(float));
						}
						else // additional uvs
						{
							int uvIndex = (int)component - 4;
							FbxGeometryElementUV* uvElement = it->first->GetElementUV((int)uvIndex);
							float* ptr = (float*)(writer.getVertexBufferRef().data() + new_index * vertexSize + offset);
							ImportComponent<FbxGeometryElementUV, FbxVector2>(uvElement, ptr, it->first, (*it2) * 3 + k, componentSize / sizeof(float));
						}
					}
				}
			}
			vertexCounter += (meshesData[meshesCounter][j].reindexer.size());
		}
    }

	writer.getBoundingBoxRef().end_extend();

	// index buffer
	writer.getIndexBufferRef().resize(indicesCount);
  
    meshesCounter = 0;
	size_t meshOffsetInIB = 0;
    for (auto it = meshes.cbegin(); it != meshes.cend(); ++it, ++meshesCounter)
    {
		size_t indicesInMesh = 0;
		for (size_t j = 0; j < meshesData[meshesCounter].size(); j++)
		{
			for (std::list<size_t>::iterator it2 = meshesData[meshesCounter][j].polygonIndices.begin(); it2 != meshesData[meshesCounter][j].polygonIndices.end(); ++it2)
			{
				for (size_t k = 0; k < 3; k++)
				{
					int index_k = it->first->GetPolygonVertex(*it2, k);
					size_t new_index = meshesData[meshesCounter][j].reindexer[(size_t)index_k];
					writer.getIndexBufferRef().data()[meshOffsetInIB + indicesInMesh + k] = (unsigned int)new_index;
				}
				indicesInMesh += 3;
			}
		}

		writer.getMeshesRef()[meshesCounter].indicesCount = indicesInMesh;
		writer.getMeshesRef()[meshesCounter].offsetInIB = meshOffsetInIB;
		writer.getMeshesRef()[meshesCounter].material = it->second;
        meshOffsetInIB += indicesInMesh;
    }
		    
    fbxScene->Destroy();	
	
	return data;
}

void FbxLoader::ProcessFbxNode(DataWriter& dataWriter, FbxNode* node, std::list<std::pair<FbxMesh*, Data::Material> >& meshes)
{
    if (node->GetNodeAttribute() == 0)
        return;
        
    FbxNodeAttribute::EType AttributeType = node->GetNodeAttribute()->GetAttributeType();
        
    if (AttributeType != FbxNodeAttribute::eMesh)
        return;
        
	FbxMesh* mesh = (FbxMesh*)node->GetNodeAttribute();
	for(int lPolygonIndex = 0; lPolygonIndex < mesh->GetPolygonCount(); lPolygonIndex++)
    {
        int lPolygonSize = mesh->GetPolygonSize(lPolygonIndex);
        if (lPolygonSize != 3)
		{
			dataWriter.getLastErrorRef() = "Mesh contains non-triangular polygons";
			return;
		}
    }

	FbxGeometryElementSmoothing* lSmoothingElement = mesh->GetElementSmoothing();
    if (!lSmoothingElement || lSmoothingElement->GetMappingMode() != FbxGeometryElement::eByPolygon)
	{
		dataWriter.getLastErrorRef() = "Mesh has to contain at least one smoothing group in by-polygon mode";
		return;
	}

	if (!CheckVertexContent(dataWriter, mesh))
	{
		return;
	}

	FbxGeometryElementNormal* normalElement = mesh->GetElementNormal();
	if (normalElement && normalElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex)
	{
		dataWriter.getLastErrorRef() = "Mesh has to contain normals in by-polygon mode";
		return;
	}

	FbxGeometryElementTangent* tangentElement = mesh->GetElementTangent();
	if (tangentElement && tangentElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex)
	{
		dataWriter.getLastErrorRef() = "Mesh has to contain tangents in by-polygon mode";
		return;
	}

	FbxGeometryElementBinormal* binormalElement = mesh->GetElementBinormal();
	if (binormalElement && binormalElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex)
	{
		dataWriter.getLastErrorRef() = "Mesh has to contain binormals in by-polygon mode";
		return;
	}
		
	size_t uvcount = mesh->GetUVLayerCount();
	for (size_t i = 0; i < uvcount; i++)
	{
		FbxGeometryElementUV* uvElement = mesh->GetElementUV(i);
		if (uvElement && uvElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex)
		{
			dataWriter.getLastErrorRef() = "Mesh has to contain texture coordinates in by-polygon mode";
			return;
		}
	}

	Data::Material mat;
	int materialsCount = mesh->GetElementMaterialCount();
	if (materialsCount == 1)
	{
		FbxGeometryElementMaterial* materialElement = mesh->GetElementMaterial(0);
		if (materialElement != 0 && materialElement->GetMappingMode() != FbxGeometryElement::eByPolygon)
		{
			FbxSurfaceMaterial* material = mesh->GetNode()->GetMaterial(materialElement->GetIndexArray().GetAt(0));
			int matId = materialElement->GetIndexArray().GetAt(0);
			if (material != 0 && matId >= 0)
			{
				mat.diffuseMapFilename = GetTextureName(material, FbxSurfaceMaterial::sDiffuse);
				mat.normalMapFilename = GetTextureName(material, FbxSurfaceMaterial::sNormalMap);
				mat.specularMapFilename = GetTextureName(material, FbxSurfaceMaterial::sSpecular);
			}
		}
	}

	meshes.push_back(std::make_pair(mesh, mat));
}

std::string FbxLoader::GetTextureName(FbxSurfaceMaterial* material, const std::string& textureType)
{
	FbxProperty diffTexProperty = material->FindProperty(textureType.c_str());
	if (diffTexProperty.GetSrcObjectCount<FbxFileTexture>() > 0)
	{
		FbxFileTexture* texture = diffTexProperty.GetSrcObject<FbxFileTexture>(0);
		if (texture != 0)
		{
			std::string textureName = texture->GetFileName();
			if (textureName.empty()) textureName = texture->GetRelativeFileName();
			textureName = utils::Utils::getFilename(textureName);
			textureName = utils::Utils::trimExtention(textureName);
			return textureName;
		}
	}
	return "";
}
    
size_t FbxLoader::GetAdditionalUVsCount(const std::list<std::pair<FbxMesh*, Data::Material> >& meshes)
{
    size_t additional = 0;
    for (auto it = meshes.cbegin(); it != meshes.cend(); ++it)
    {
        size_t uvcount = it->first->GetUVLayerCount();
        if (uvcount > 1)
        {
            size_t a = uvcount - 1;
            if (a > additional) additional = a;
        }
    }
    return additional;
}
    
bool FbxLoader::CheckVertexContent(DataWriter& dataWriter, FbxMesh* mesh)
{
    // normals
    FbxLayer* normalsLayer = mesh->GetLayer(0, FbxLayerElement::EType::eNormal);
    if (!normalsLayer)
    {
        bool result = mesh->GenerateNormals(true, true);
		if (!result) dataWriter.getLastErrorRef() = "Normals have not been generated";
		return false;
    }
    else
    {
        FbxLayerElementNormal* normals = normalsLayer->GetNormals();
        if (normals == 0) 
		{
			bool result = mesh->GenerateNormals();
			if (!result) dataWriter.getLastErrorRef() = "Normals have not been generated";
			return false;
		}
    }
            
    // tangents
    FbxLayer* tangentsLayer = mesh->GetLayer(0, FbxLayerElement::EType::eTangent);
    if (!tangentsLayer)
    {
        bool result = mesh->GenerateTangentsData(0, true);
        if (!result) dataWriter.getLastErrorRef() = "Tangents have not been generated";
		return false;
    }
    else
    {
        FbxLayerElementTangent* tangents = tangentsLayer->GetTangents();
        if (tangents == 0) 
		{
			bool result = mesh->GenerateTangentsData(0, true);
			if (!result) dataWriter.getLastErrorRef() = "Tangents have not been generated";
			return false;
		}
    }

	return true;
}
    
void FbxLoader::GetSmoothingGroup(FbxMesh* mesh, std::vector<int>& group)
{
	FbxGeometryElementSmoothing* lSmoothingElement = mesh->GetElementSmoothing();
	group.resize(mesh->GetPolygonCount());
    for(int lPolygonIndex = 0; lPolygonIndex < mesh->GetPolygonCount(); lPolygonIndex++)
    {
        int lSmoothingIndex = 0;
        if( lSmoothingElement->GetReferenceMode() == FbxGeometryElement::eDirect )
            lSmoothingIndex = lPolygonIndex;
        if(lSmoothingElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
            lSmoothingIndex = lSmoothingElement->GetIndexArray().GetAt(lPolygonIndex);

        group[lPolygonIndex] = lSmoothingElement->GetDirectArray().GetAt(lSmoothingIndex);
    }
}

void FbxLoader::Reindexing(FbxMesh* mesh, std::vector<GroupData>& groups, size_t& reindexerIndex)
{
	std::vector<int> smoothingGroupsPerPolygon;
	GetSmoothingGroup(mesh, smoothingGroupsPerPolygon);
		
	std::vector<int> smoothingGroups = smoothingGroupsPerPolygon;
	std::vector<int>::iterator it = std::unique(smoothingGroups.begin(), smoothingGroups.end());
	smoothingGroups.resize(std::distance(smoothingGroups.begin(), it));

	groups.resize(smoothingGroups.size());

	for (size_t group = 0; group < smoothingGroups.size(); group++)
	{
		for (size_t polygonIndex = 0; polygonIndex < smoothingGroupsPerPolygon.size(); polygonIndex++)
		{
			if (smoothingGroups[group] == smoothingGroupsPerPolygon[polygonIndex])
			{
				groups[group].polygonIndices.push_back(polygonIndex);

				int lPolygonSize = mesh->GetPolygonSize(polygonIndex);
				for(int i = 0; i < lPolygonSize; i++)
				{
					int index = mesh->GetPolygonVertex(polygonIndex, i);
					int newIndex;
					if (groups[group].reindexer.find(index) == groups[group].reindexer.end())
					{
						newIndex = reindexerIndex;
						groups[group].reindexer.insert(std::make_pair((size_t)index, newIndex));
						reindexerIndex++;
					}
				}
			}
		}
	}
}

}