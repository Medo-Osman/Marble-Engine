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
	std::vector<Mesh<VertexPosNormTexTan>> m_meshes;

	std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t> strconverter;

	// Helper Functions
	Mesh<VertexPosNormTexTan> processMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<VertexPosNormTexTan> vertices;
		std::vector<UINT> indices;

		int indexOffset = m_vertices.size(); // Vertex offset for indices index
		m_vertices.reserve(m_vertices.size() + mesh->mNumVertices);
		m_indices.reserve(m_indices.size() + mesh->mNumFaces * 3);

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
		aiMaterial* aMaterial = scene->mMaterials[mesh->mMaterialIndex];
		aiColor3D color(0.f, 0.f, 0.f);
		aMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material.emissive = XMFLOAT4(color.r, color.g, color.b, 1.f);
		aMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material.ambient = XMFLOAT4(color.r, color.g, color.b, 1.f);
		aMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material.diffuse = XMFLOAT4(color.r, color.g, color.b, 1.f);
		if (material.ambient.x == material.diffuse.x && 
			material.ambient.y == material.diffuse.y &&
			material.ambient.z == material.diffuse.z)
		{
			material.ambient = XMFLOAT4(.1f, 0.f, 0.f, 1.f);
			material.diffuse = XMFLOAT4(1.f, 0.f, 0.f, 1.f);
		}
		aMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material.specular = XMFLOAT4(color.r, color.g, color.b, 1.f);
		aMaterial->Get(AI_MATKEY_SHININESS, material.shininess);

		if (material.shininess == 0.f)
			material.shininess = 30.f;
		else
			material.shininess /= 4.f; // Assimps scales by * 4, this reverses it

		// Material Textures
		TexturePaths texturePaths;
		texturePaths.diffusePath = L"";
		texturePaths.normalPath = L"";
		texturePaths.specularPath = L"";
		texturePaths.displacementPath = L"";

		aiString texturePath;

		if (aMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0 && aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
			texturePaths.diffusePath = strconverter.from_bytes(texturePath.C_Str()).c_str();

		if (aMaterial->GetTextureCount(aiTextureType_HEIGHT) > 0 && aMaterial->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS)
			texturePaths.normalPath = strconverter.from_bytes(texturePath.C_Str()).c_str();

		if (aMaterial->GetTextureCount(aiTextureType_SPECULAR) > 0 && aMaterial->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS)
			texturePaths.specularPath = strconverter.from_bytes(texturePath.C_Str()).c_str();
		
		return Mesh<VertexPosNormTexTan>(m_device, m_deviceContext, vertices, indices, material, texturePaths);
	}
	void processNodes(aiNode* node, const aiScene* scene)
	{
		for (UINT i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.push_back(processMesh(mesh, scene));

			std::string name = m_name + "_" + std::to_string(i);
			m_meshes.back().setName(name);
		}

		for (UINT i = 0; i < node->mNumChildren; i++)
			processNodes(node->mChildren[i], scene);
	}
	bool loadModel(std::string& modelName)
	{
		std::string modelPath = "Models\\" + modelName;
		Assimp::Importer importer;
		const aiScene* pScene = importer.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);

		if (!pScene) // if nullptr
			return false;

		processNodes(pScene->mRootNode, pScene);
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
	void initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int id, std::string modelName)
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
			m_meshes.push_back(Mesh<VertexPosNormTexTan>(m_device, m_deviceContext, vertices, indices, material, TexturePaths()));
			m_meshes.back().setName(id + "_Default");
		}
		else
			if (!loadModel(modelName))
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
	void setMaterial(PS_MATERIAL_BUFFER material)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i].setMaterial(material);
	}

	void setMaterialWithID(PS_MATERIAL_BUFFER material, int id)
	{
		m_meshes[id].setMaterial(material);
	}

	void setTexture(TexturePaths textures)
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i].setTextures(textures);
	}

	// Render
	void render()
	{
		for (size_t i = 0; i < m_meshes.size(); i++)
			m_meshes[i].render();
	}
};

#endif // !MODEL_H