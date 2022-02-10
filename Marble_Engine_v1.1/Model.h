#include "pch.h"
#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"

class Model
{
private:
	// Device
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

	// Name
	std::string m_name;

	// Vertices
	std::vector<XMFLOAT3> m_vertices;
	std::vector<UINT> m_indices;

	// Meshes
	std::vector<Mesh<VertexPosNormTexTan>*> m_meshes;

	// Helper Functions
	Mesh<VertexPosNormTexTan>* processMesh(aiMesh* mesh, const aiScene* scene, int meshIndex = -1, std::vector<MeshData>* meshData = nullptr)
	{
		std::vector<VertexPosNormTexTan> vertices;
		std::vector<UINT> indices;

		int indexOffset = (int)m_vertices.size(); // Vertex offset for indices index
		m_vertices.reserve(m_vertices.size() + mesh->mNumVertices);
		m_indices.reserve(m_indices.size() + mesh->mNumFaces * (unsigned int)3);

		// Vertices
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			VertexPosNormTexTan vertex;

			vertex.position = { mesh->mVertices[i].x,
								mesh->mVertices[i].y,
								mesh->mVertices[i].z };

			if (mesh->HasNormals())
			{
				vertex.normal = {	mesh->mNormals[i].x,
									mesh->mNormals[i].y,
									mesh->mNormals[i].z };
			}
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent = {	mesh->mTangents[i].x,
									mesh->mTangents[i].y,
									mesh->mTangents[i].z };

				vertex.bitangent = {mesh->mBitangents[i].x,
									mesh->mBitangents[i].y,
									mesh->mBitangents[i].z };
			}
			if (mesh->mTextureCoords[0])
				vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };

			vertices.push_back(vertex);
			m_vertices.push_back(vertex.position);
		}

		// Indices
		for (UINT i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (UINT i = 0; i < face.mNumIndices; i++)
			{
				indices.push_back(face.mIndices[i]);
				m_indices.push_back(indexOffset + face.mIndices[i]);
			}
		}

		// Material Values
		PS_MATERIAL_BUFFER material;
		PS_MATERIAL_PBR_BUFFER materialPBR;
		aiMaterial* aMaterial = scene->mMaterials[mesh->mMaterialIndex];
		aiColor3D color(0.f, 0.f, 0.f);

		TexturePaths texturePaths;
		TexturePathsPBR texturePathsPBR;


		if (meshData) // not nullptr
		{
			switch (meshData->at(meshIndex).matType)
			{
			case PHONG:
				texturePaths.diffusePath = meshData->at(meshIndex).matPhong.diffusePath;
				texturePathsPBR.albedoPath = texturePaths.diffusePath;

				texturePaths.normalPath = meshData->at(meshIndex).matPhong.normalPath;
				texturePathsPBR.normalPath = texturePaths.normalPath;

				texturePaths.specularPath = meshData->at(meshIndex).matPhong.specularPath;

				texturePaths.displacementPath = meshData->at(meshIndex).matPhong.displacementPath;
				texturePathsPBR.displacementPath = texturePaths.displacementPath;

				material.emissive = meshData->at(meshIndex).matPhong.emissive;
				material.ambient = meshData->at(meshIndex).matPhong.ambient;
				material.diffuse = meshData->at(meshIndex).matPhong.diffuse;
				material.specular = meshData->at(meshIndex).matPhong.specular;
				material.shininess = meshData->at(meshIndex).matPhong.shininess;

				material.diffTextureExists = meshData->at(meshIndex).matPhong.diffTextureExists;
				material.specTextureExists = meshData->at(meshIndex).matPhong.specTextureExists;
				material.normTextureExists = meshData->at(meshIndex).matPhong.normTextureExists;
				break;
			case PBR:
				texturePathsPBR.albedoPath = meshData->at(meshIndex).matPBR.albedoPath;
				texturePaths.diffusePath = texturePathsPBR.albedoPath;

				texturePathsPBR.normalPath = meshData->at(meshIndex).matPBR.normalPath;
				texturePaths.normalPath = texturePathsPBR.normalPath;

				texturePathsPBR.metallicPath = meshData->at(meshIndex).matPBR.metallicPath;
				texturePathsPBR.roughnessPath = meshData->at(meshIndex).matPBR.roughnessPath;
				texturePathsPBR.emissivePath = meshData->at(meshIndex).matPBR.emissivePath;
				texturePathsPBR.ambientOcclusionPath = meshData->at(meshIndex).matPBR.ambientOcclusionPath;

				texturePathsPBR.displacementPath = meshData->at(meshIndex).matPBR.displacementPath;
				texturePaths.displacementPath = texturePathsPBR.displacementPath;

				materialPBR.albedo = meshData->at(meshIndex).matPBR.albedo;
				materialPBR.metallic = meshData->at(meshIndex).matPBR.metallic;
				materialPBR.roughness = meshData->at(meshIndex).matPBR.roughness;
				materialPBR.emissiveStrength = meshData->at(meshIndex).matPBR.emissiveStrength;
				materialPBR.materialTextured = meshData->at(meshIndex).matPBR.materialTextured;
				materialPBR.emissiveTextured = meshData->at(meshIndex).matPBR.emissiveTextured;

				break;
			default:
				break;
			}
		}
		else
		{
			aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			material.emissive = XMFLOAT4(color.r, color.g, color.b, 1.f);
			XMVECTOR emVector = XMLoadFloat4(&material.emissive);
			materialPBR.emissiveStrength = DirectX::XMVector3Length(emVector).m128_f32[0];

			aMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
			material.ambient = XMFLOAT4(color.r, color.g, color.b, 1.f);

			aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			material.diffuse = XMFLOAT4(color.r, color.g, color.b, 1.f);
			materialPBR.albedo = XMFLOAT3(color.r, color.g, color.b);

			if (material.ambient.x == material.diffuse.x &&
				material.ambient.y == material.diffuse.y &&
				material.ambient.z == material.diffuse.z)
			{
				material.ambient = XMFLOAT4(.1f, .1f, .1f, 1.f);
				material.diffuse = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
				materialPBR.albedo = XMFLOAT3(1.f, 1.f, 1.f);
			}
			aMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
			material.specular = XMFLOAT4(color.r, color.g, color.b, 1.f);
			XMVECTOR specVector = XMLoadFloat4(&material.specular);
			materialPBR.metallic = DirectX::XMVector3Length(specVector).m128_f32[0];

			aMaterial->Get(AI_MATKEY_SHININESS, material.shininess);
			if (material.shininess == 0.f)
				material.shininess = 30.f;
			else
				material.shininess /= 4.f; // Assimps scales by * 4, this reverses it
			materialPBR.roughness = std::pow(1 - material.shininess, 2.f);


			// Material Textures
			aiString texturePath;

			if (aMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 && aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePaths.diffusePath = texturePathsPBR.albedoPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_NORMALS) > 0 && aMaterial->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePaths.normalPath = texturePathsPBR.normalPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0 && aMaterial->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePaths.specularPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_METALNESS) > 0 && aMaterial->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePathsPBR.metallicPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_SHININESS) > 0 && aMaterial->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePathsPBR.roughnessPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_AMBIENT) > 0 && aMaterial->GetTexture(aiTextureType_AMBIENT, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePathsPBR.ambientOcclusionPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_EMISSIVE) > 0 && aMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePathsPBR.emissivePath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}

			if (aMaterial->GetTextureCount(aiTextureType_DISPLACEMENT) > 0 && aMaterial->GetTexture(aiTextureType_DISPLACEMENT, 0, &texturePath) == AI_SUCCESS)
			{
				std::string strTexturePath(texturePath.C_Str());
				size_t pos = strTexturePath.find("Textures\\");
				strTexturePath.erase(0, pos);

				texturePaths.displacementPath = texturePathsPBR.displacementPath = extractFileName(charToWchar(strTexturePath.c_str()).c_str());
			}


			aiString fileBaseColor, fileMetallicRoughness;
			aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_BASE_COLOR_TEXTURE, &fileBaseColor);
			aMaterial->GetTexture(AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE, &fileMetallicRoughness);

			if (fileMetallicRoughness.length > 0)
			{
				texturePathsPBR.metallicPath = extractFileName(charToWchar(fileMetallicRoughness.C_Str()).c_str());
				texturePathsPBR.roughnessPath = extractFileName(charToWchar(fileMetallicRoughness.C_Str()).c_str());
			}
		}

		Mesh<VertexPosNormTexTan>* finalMesh = new Mesh<VertexPosNormTexTan>(m_device, m_deviceContext, vertices, indices, material, texturePaths, mesh->mName.C_Str());
		if (meshData && meshData->at(meshIndex).matType == PBR)
			finalMesh->setMaterial(materialPBR);
		finalMesh->setTextures(texturePathsPBR);

		return finalMesh;
	}
	void processNodes(aiNode* node, const aiScene* scene, std::vector<MeshData>* meshData = nullptr)
	{
		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.push_back(processMesh(mesh, scene, (int)m_meshes.size(), meshData));
		}

		for (UINT i = 0; i < node->mNumChildren; i++)
			processNodes(node->mChildren[i], scene, meshData);
	}
	bool loadModel(std::string& modelName, std::vector<MeshData>* meshData = nullptr)
	{
		std::string modelPath = "Models\\" + modelName;
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);
		// Assimp tries to load gltf2 files with gltf1 importer first for some reason and throws a exception, 
		// just ignore it as it will import with version 2 of the importer right after
		
		if (!pScene) // if nullptr
			return false;

		processNodes(pScene->mRootNode, pScene, meshData);

		OutputDebugStringA("Model loaded: ");
		OutputDebugStringA(modelName.c_str());
		OutputDebugStringA("\n");
		return true;
	}

public:
	Model()
	{
		m_device = nullptr;
		m_deviceContext = nullptr;
	}
	Model(const Model& otherModel)
	{
		m_deviceContext = otherModel.m_deviceContext;
		m_deviceContext = otherModel.m_deviceContext;
		m_name = otherModel.m_name;
		m_meshes = otherModel.m_meshes;
	}

	// Initialization
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int id, std::string modelName, std::vector<MeshData>* meshData = nullptr)
	{
		m_device = device;
		m_deviceContext = deviceContext;
		m_name = std::to_string(id) + "_" + modelName;

		if (modelName.empty())
		{
			std::vector<VertexPosNormTexTan> vertices =
			{
				{
					XMFLOAT3(-0.5f, 0.f, -0.5f),
					XMFLOAT3(0.f, 1.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT2(1.f, 0.f)
				},
				{
					XMFLOAT3(-0.5f, 0.f, 0.5f),
					XMFLOAT3(0.f, 1.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT2(0.f, 1.f)
				},
				{
					XMFLOAT3(0.5f, 0.f, 0.5f),
					XMFLOAT3(0.f, 1.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT2(0.f, 1.f)
				},
				{
					XMFLOAT3(0.5f, 0.f, -0.5f),
					XMFLOAT3(0.f, 1.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT3(0.f, 0.f, 0.f),
					XMFLOAT2(1.f, 1.f)
				}
			};
			std::vector<UINT> indices({ 0, 1, 2, 0, 2, 3 });
			PS_MATERIAL_BUFFER material;
			//material.diffuse = XMFLOAT4(.2f, .2f, 0.2f, 1.f); // Grey
			material.diffuse = XMFLOAT4(0.13f, .25f, 0.004f, 1.f); // Forest Green
			material.specular = XMFLOAT4(.1f, .1f, 0.1f, 1.f);
			material.shininess = 32.f;
			auto* finalMesh = new Mesh<VertexPosNormTexTan>(m_device, m_deviceContext, vertices, indices, material, TexturePaths(), "Plane");
			m_meshes.push_back(finalMesh);
			m_meshes.back()->setName(id + "_Default");
		}
		else
			if (!loadModel(modelName, meshData))
				assert(!"Error, failed to load Model!");
	}

	// Picking
	float pick(XMVECTOR rayOrigin, XMVECTOR rayDirection, char dimension = 'n')
	{
		UINT i0;
		UINT i1;
		UINT i2;
		float distance = 0.f;
		SimpleMath::Ray ray(rayOrigin, rayDirection);
		size_t nrOfFaces = m_indices.size() / 3;
		for (size_t i = 0; i < nrOfFaces; i++)
		{
			i0 = m_indices[i * 3 + 0];
			i1 = m_indices[i * 3 + 1];
			i2 = m_indices[i * 3 + 2];

			if (ray.Intersects(m_vertices[i0], m_vertices[i1], m_vertices[i2], distance))
			{
				SimpleMath::Vector3 rayDirectionNorm = ray.direction;
				rayDirectionNorm.Normalize();

				XMFLOAT3 hitPosition = (rayDirectionNorm * distance) + ray.position;

				switch (dimension)
				{
				case 'x':
					return hitPosition.y;
					break;

				case 'y':
					return hitPosition.y;
					break;

				case 'z':
					return hitPosition.y;
					break;

				default:
					return 1.f; // Normal Hit without position in specific dimension
					break;
				}
				return true;
			}
		}
		return 0.f; // Not Hit
	}

	// Setters
	void setShaderState(ShaderStates shaderState)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->setShaderState(shaderState);
	}
	// - PHONG
	void setMaterial(PS_MATERIAL_BUFFER material)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->setMaterial(material);
	}

	void setMaterialWithID(PS_MATERIAL_BUFFER material, int id)
	{
		m_meshes[id]->setMaterial(material);
	}

	void setTexture(TexturePaths textures)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->setTextures(textures);
	}
	// - PBR
	void setMaterial(PS_MATERIAL_PBR_BUFFER material)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->setMaterial(material);
	}

	void setMaterialWithID(PS_MATERIAL_PBR_BUFFER material, int id)
	{
		m_meshes[id]->setMaterial(material);
	}

	void setTexture(TexturePathsPBR textures)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->setTextures(textures);
	}

	// UI Update
	void updateUI()
	{
		if (ImGui::CollapsingHeader("Mesh List"))
		{
			for (size_t i = 0; i < m_meshes.size(); i++)
			{
				ImGui::PushID(std::string(std::to_string(i) + m_name).c_str());
				m_meshes[i]->updateUI();
				ImGui::PopID();
			}
		}
	}

	// Save Mesh Data
	void fillMeshData(std::vector<MeshData>* meshes)
	{
		meshes->resize(m_meshes.size());
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->fillMeshData(&meshes->at(i));
	}

	// Render
	void render()
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i]->render();
	}
};

#endif // !MODEL_H