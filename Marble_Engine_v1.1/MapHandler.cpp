#include "pch.h"
#include "MapHandler.h"

void MapHandler::dumpDataToFile()
{
	std::string tempStr;
	_bstr_t nameCStr;
	m_file.open("Maps\\" + m_mapFileName, std::ios::out | std::ofstream::trunc);

	if (m_file.is_open()) // Found
	{
		// Game Objects
		for (size_t i = 0; i < m_gameObjectData.size(); i++)
		{
			// Model
			m_file << MODEL_FILE_PREFIX << " " << m_gameObjectData[i].modelFile << "\n";
			m_file << SHADER_TYPE_PREFIX << " " << m_gameObjectData[i].shaderType << "\n";
			
			// Transform
			tempStr = f3ToString(m_gameObjectData[i].scale, " ");
			m_file << TRANSFORM_SCALE_PREFIX << " " << tempStr << "\n";
			tempStr = f3ToString(m_gameObjectData[i].rotation, " ");
			m_file << TRANSFORM_ROTATION_PREFIX << " " << tempStr << "\n";
			tempStr = f3ToString(m_gameObjectData[i].position, " ");
			m_file << TRANSFORM_POSITION_PREFIX << " " << tempStr << "\n";

			// Meshes
			m_file << NR_OF_MESHES_PREFIX << " " << m_gameObjectData[i].meshes.size() << "\n";

			for (size_t j = 0; j < m_gameObjectData[i].meshes.size(); j++)
			{
				// Name
				m_file << MESH_NAME_PREFIX << " " << m_gameObjectData[i].meshes[j].name << "\n";

				switch (m_gameObjectData[i].shaderType)
				{
				case PHONG:
				{
					// Texture Paths
					nameCStr = m_gameObjectData[i].meshes[j].matPhong.diffusePath.c_str();
					m_file << MAT_DIFF_PATH_PREFIX << " " << nameCStr << "\n";

					nameCStr = m_gameObjectData[i].meshes[j].matPhong.specularPath.c_str();
					m_file << PH_SPEC_PATH_PREFIX << " " << nameCStr << "\n";

					nameCStr = m_gameObjectData[i].meshes[j].matPhong.normalPath.c_str();
					m_file << MAT_NORM_PATH_PREFIX << " " << nameCStr << "\n";

					nameCStr = m_gameObjectData[i].meshes[j].matPhong.displacementPath.c_str();
					m_file << MAT_DISP_PATH_PREFIX << " " << nameCStr << "\n";

					// Colors
					tempStr = f4ToString(m_gameObjectData[i].meshes[j].matPhong.emissive, " ");
					m_file << PH_EMISSIVE_COL_PREFIX << " " << tempStr << "\n";

					tempStr = f4ToString(m_gameObjectData[i].meshes[j].matPhong.ambient, " ");
					m_file << PH_AMBIENT_COL_PREFIX << " " << tempStr << "\n";

					tempStr = f4ToString(m_gameObjectData[i].meshes[j].matPhong.diffuse, " ");
					m_file << MAT_DIFFUSE_COL_PREFIX << " " << tempStr << "\n";

					tempStr = f4ToString(m_gameObjectData[i].meshes[j].matPhong.specular, " ");
					m_file << PH_SPECULAR_COL_PREFIX << " " << tempStr << "\n";

					m_file << PH_SHININESS_PREFIX << " " << std::to_string(m_gameObjectData[i].meshes[j].matPhong.shininess) << "\n";

					// Texture Exists
					m_file << PH_DIFF_TEX_PREFIX << " " << m_gameObjectData[i].meshes[j].matPhong.diffTextureExists << "\n";
					m_file << PH_SPEC_TEX_PREFIX << " " << m_gameObjectData[i].meshes[j].matPhong.specTextureExists << "\n";
					m_file << PH_NORM_TEX_PREFIX << " " << m_gameObjectData[i].meshes[j].matPhong.normTextureExists << "\n";

					break;
				}
				case PBR:
				{
					// Texture Paths
					if (!m_gameObjectData[i].meshes[j].matPBR.albedoPath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.albedoPath.c_str();
						m_file << MAT_DIFF_PATH_PREFIX << " " << nameCStr << "\n";
					}

					if (!m_gameObjectData[i].meshes[j].matPBR.normalPath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.normalPath.c_str();
						m_file << MAT_NORM_PATH_PREFIX << " " << nameCStr << "\n";
					}

					if (!m_gameObjectData[i].meshes[j].matPBR.metallicPath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.metallicPath.c_str();
						m_file << PB_META_PATH_PREFIX << " " << nameCStr << "\n";
					}

					if (!m_gameObjectData[i].meshes[j].matPBR.roughnessPath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.roughnessPath.c_str();
						m_file << PB_ROUG_PATH_PREFIX << " " << nameCStr << "\n";
					}

					if (!m_gameObjectData[i].meshes[j].matPBR.emissivePath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.emissivePath.c_str();
						m_file << PB_EMIS_PATH_PREFIX << " " << nameCStr << "\n";
					}

					if (!m_gameObjectData[i].meshes[j].matPBR.ambientOcclusionPath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.ambientOcclusionPath.c_str();
						m_file << PB_AMOC_PATH_PREFIX << " " << nameCStr << "\n";
					}

					if (!m_gameObjectData[i].meshes[j].matPBR.displacementPath.empty())
					{
						nameCStr = m_gameObjectData[i].meshes[j].matPBR.displacementPath.c_str();
						m_file << MAT_DISP_PATH_PREFIX << " " << nameCStr << "\n";
					}

					// Material Values
					m_file << PB_METALLIC_PREFIX << " " << m_gameObjectData[i].meshes[j].matPBR.metallic << "\n";
					m_file << PB_ROUGHNESS_PREFIX << " " << m_gameObjectData[i].meshes[j].matPBR.roughness << "\n";
					m_file << PB_EMISS_STR_PREFIX << " " << m_gameObjectData[i].meshes[j].matPBR.emissiveStrength << "\n";

					// Texture Exists
					m_file << PB_TEXTURED_PREFIX << " " << m_gameObjectData[i].meshes[j].matPBR.materialTextured << "\n";
					m_file << PB_EMI_TEXT_PREFIX << " " << m_gameObjectData[i].meshes[j].matPBR.emissiveTextured << "\n";

					break;
				}
				default:
					break;
				}
			}

		}
		// Lights
		m_file << NR_OF_LIGHTS_PREFIX << " " << m_lightData.size() << "\n";

		for (size_t i = 0; i < m_lightData.size(); i++)
		{
			tempStr = f4ToString(m_lightData[i].first.position, " ", true);
			m_file << LIGHT_POSITION_PREFIX << " " << tempStr << "\n";

			tempStr = f3ToString(m_lightData[i].first.direction, " ");
			m_file << LIGHT_DIRECTION_PREFIX << " " << tempStr << "\n";

			m_file << LIGHT_INTENSITY_PREFIX << " " << m_lightData[i].first.intensity << "\n";

			tempStr = f3ToString(m_lightData[i].first.color, " ");
			m_file << LIGHT_COLOR_PREFIX << " " << tempStr << "\n";

			tempStr = f2ToString(m_lightData[i].second.spotAngles, " ");
			m_file << LIGHT_SPOTANGLES_PREFIX << " " << tempStr << "\n";

			m_file << LIGHT_RANGE_PREFIX << " " << m_lightData[i].first.range << "\n";
			m_file << LIGHT_TYPE_PREFIX << " " << m_lightData[i].first.type << "\n";
			m_file << LIGHT_ENABLED_PREFIX << " " << m_lightData[i].first.enabled << "\n";

			tempStr = f3ToString(m_lightData[i].second.rotationDeg, " ");
			m_file << LIGHT_ROTATION_PREFIX << " " << tempStr << "\n";
		}

		m_file.close();

		OutputDebugStringA("Game Object and Lights Saved to Map File: ");
		OutputDebugStringA(m_mapFileName.c_str());
		OutputDebugStringA("\n");
	}
	else
	{
		OutputDebugStringA("Error, could not find map file: ");
		OutputDebugStringA(m_mapFileName.c_str());
		OutputDebugStringA("\n");
	}
}

MapHandler::MapHandler()
{
	m_nrOfDifference = 0;
}

MapHandler::~MapHandler()
{
	m_file.close();
}

void MapHandler::initialize(std::string mapFileName, int nrOfHardCodedGameObjects, bool createIfNotFound)
{
	m_mapFileName = mapFileName;
	m_nrOfDifference = nrOfHardCodedGameObjects;
	m_file.open("Maps\\" + m_mapFileName, std::ios::out | std::ios::in);

	if (m_file.is_open()) // Found
	{
		OutputDebugStringA("Map File opened: ");
		OutputDebugStringA(m_mapFileName.c_str());
		OutputDebugStringA("\n");

		std::stringstream sStream;
		std::string line = "";
		std::string prefix = "";
		std::string tempStr;
		_bstr_t tempCStr;
		int tempInt = 0;
		XMFLOAT3 tempF3 = XMFLOAT3(0.f, 0.f, 0.f);
		XMFLOAT4 tempF4 = XMFLOAT4(0.f, 0.f, 0.f, 1.f);

		while (getline(m_file, line))
		{
			sStream.clear();
			sStream.str(line);
			sStream >> prefix;

			if (prefix == MODEL_FILE_PREFIX)
			{
				sStream >> tempStr;
				m_gameObjectData.emplace_back();
				m_gameObjectData.back().modelFile = tempStr;
			}
			else if (prefix == SHADER_TYPE_PREFIX)
			{
				sStream >> tempInt;
				m_gameObjectData.back().shaderType = (ShaderStates)tempInt;
			}
			else if (prefix == TRANSFORM_SCALE_PREFIX)
			{
				sStream >> tempF3.x >> tempF3.y >> tempF3.z;
				m_gameObjectData.back().scale = tempF3;
			}
			else if (prefix == TRANSFORM_ROTATION_PREFIX)
			{
				sStream >> tempF3.x >> tempF3.y >> tempF3.z;
				m_gameObjectData.back().rotation = tempF3;
			}
			else if (prefix == TRANSFORM_POSITION_PREFIX)
			{
				sStream >> tempF3.x >> tempF3.y >> tempF3.z;
				m_gameObjectData.back().position = tempF3;
			}
			else if (prefix == NR_OF_MESHES_PREFIX)
			{
				sStream >> tempInt;
				m_gameObjectData.back().meshes.resize(tempInt);

				bool endOfMesh;

				for (size_t i = 0; i < m_gameObjectData.back().meshes.size(); i++)
				{
					endOfMesh = false;

					while (!endOfMesh && getline(m_file, line)) // should be remade, hard to read code
					{
						sStream.clear();
						sStream.str(line);
						sStream >> prefix;

						if (prefix == MESH_NAME_PREFIX)
						{
							sStream >> tempStr;
							if (tempStr.empty())
								tempStr = "mesh" + std::to_string(i);
							m_gameObjectData.back().meshes[i].name = tempStr;
							m_gameObjectData.back().meshes[i].matType = m_gameObjectData.back().shaderType;
						}
						else
						{
							switch (m_gameObjectData.back().shaderType)
							{
							case PHONG:
							{
								if (prefix == MAT_DIFF_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPhong.diffusePath = tempCStr;
								}
								else if (prefix == PH_SPEC_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPhong.specularPath = tempCStr;
								}
								else if (prefix == MAT_NORM_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPhong.normalPath = tempCStr;
								}
								else if (prefix == MAT_DISP_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPhong.normalPath = tempCStr;
								}
								else if (prefix == PH_EMISSIVE_COL_PREFIX)
								{
									sStream >> tempF4.x >> tempF4.y >> tempF4.z;
									m_gameObjectData.back().meshes[i].matPhong.emissive = tempF4;
								}
								else if (prefix == PH_AMBIENT_COL_PREFIX)
								{
									sStream >> tempF4.x >> tempF4.y >> tempF4.z;
									m_gameObjectData.back().meshes[i].matPhong.ambient = tempF4;
								}
								else if (prefix == MAT_DIFFUSE_COL_PREFIX)
								{
									sStream >> tempF4.x >> tempF4.y >> tempF4.z;
									m_gameObjectData.back().meshes[i].matPhong.diffuse = tempF4;
								}
								else if (prefix == PH_SPECULAR_COL_PREFIX)
								{
									sStream >> tempF4.x >> tempF4.y >> tempF4.z;
									m_gameObjectData.back().meshes[i].matPhong.specular = tempF4;
								}
								else if (prefix == PH_SHININESS_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPhong.shininess;
								}
								else if (prefix == PH_DIFF_TEX_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPhong.diffTextureExists;
								}
								else if (prefix == PH_SPEC_TEX_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPhong.specTextureExists;
								}
								else if (prefix == PH_NORM_TEX_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPhong.normTextureExists;
									endOfMesh = true;
								}

								break;
							}
							case PBR:
							{
								if (prefix == MAT_DIFF_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.albedoPath = tempCStr;
								}
								else if (prefix == MAT_NORM_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.normalPath = tempCStr;
								}
								else if (prefix == PB_META_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.metallicPath = tempCStr;
								}
								else if (prefix == PB_ROUG_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.roughnessPath = tempCStr;
								}
								else if (prefix == PB_EMIS_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.emissivePath = tempCStr;
								}
								else if (prefix == PB_AMOC_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.ambientOcclusionPath = tempCStr;
								}
								else if (prefix == MAT_DISP_PATH_PREFIX)
								{
									sStream >> tempStr;
									tempCStr = tempStr.c_str();
									m_gameObjectData.back().meshes[i].matPBR.displacementPath = tempCStr;
								}
								else if (prefix == MAT_DIFFUSE_COL_PREFIX)
								{
									sStream >> tempF3.x >> tempF3.y >> tempF3.z;
									m_gameObjectData.back().meshes[i].matPBR.albedo = tempF3;
								}
								else if (prefix == PB_METALLIC_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPBR.metallic;
								}
								else if (prefix == PB_ROUGHNESS_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPBR.roughness;
								}
								else if (prefix == PB_EMISS_STR_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPBR.emissiveStrength;
								}
								else if (prefix == PB_TEXTURED_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPBR.materialTextured;
								}
								else if (prefix == PB_EMI_TEXT_PREFIX)
								{
									sStream >> m_gameObjectData.back().meshes[i].matPBR.emissiveTextured;
									endOfMesh = true;
								}
								else
								{
									sStream >> tempStr;
									OutputDebugStringA("None: ");
									OutputDebugStringA(prefix.c_str());
									OutputDebugStringA(", ");
									OutputDebugStringA(tempStr.c_str());
									OutputDebugStringA("\n");
								}

								break;
							}
							default:
								break;
							}
						}
					}
				}
			}
			else if (prefix == NR_OF_LIGHTS_PREFIX)
			{
				sStream >> tempInt;
				m_lightData.resize(tempInt);

				bool endOfLight = false;
				OutputDebugStringA("Nr Of Lights: ");
				OutputDebugStringA(std::to_string(tempInt).c_str());
				OutputDebugStringA("\n");

				for (size_t i = 0; i < tempInt; i++)
				{
					endOfLight = false;

					while (!endOfLight && getline(m_file, line))
					{
						sStream.clear();
						sStream.str(line);
						sStream >> prefix;

						if (prefix == LIGHT_POSITION_PREFIX)
						{
							sStream >> tempF4.x >> tempF4.y >> tempF4.z >> tempF4.w;
							m_lightData[i].first.position = tempF4;
						}
						else if (prefix == LIGHT_DIRECTION_PREFIX)
						{
							sStream >> tempF3.x >> tempF3.y >> tempF3.z;
							m_lightData[i].first.direction = tempF3;
						}
						else if (prefix == LIGHT_INTENSITY_PREFIX)
						{
							sStream >> m_lightData[i].first.intensity;
						}
						else if (prefix == LIGHT_COLOR_PREFIX)
						{
							sStream >> tempF3.x >> tempF3.y >> tempF3.z;
							m_lightData[i].first.color = tempF3;
						}
						else if (prefix == LIGHT_SPOTANGLES_PREFIX)
						{
							sStream >> m_lightData[i].second.spotAngles.x >> m_lightData[i].second.spotAngles.y;
							m_lightData[i].first.spotAngles.x = 1.f / (cosf(m_lightData[i].second.spotAngles.x) - cosf(m_lightData[i].second.spotAngles.y));
							m_lightData[i].first.spotAngles.y = cosf(m_lightData[i].second.spotAngles.y);
						}
						else if (prefix == LIGHT_RANGE_PREFIX)
						{
							sStream >> m_lightData[i].first.range;
						}
						else if (prefix == LIGHT_TYPE_PREFIX)
						{
							sStream >> m_lightData[i].first.type;
						}
						else if (prefix == LIGHT_ENABLED_PREFIX)
						{
							sStream >> m_lightData[i].first.enabled;
						}
						else if (prefix == LIGHT_ISCASTINFSHADOW_PREFIX)
						{
							sStream >> tempStr;
						}
						else if (prefix == LIGHT_ROTATION_PREFIX)
						{
							sStream >> tempF3.x >> tempF3.y >> tempF3.z;
							m_lightData[i].second.rotationDeg = tempF3;
							endOfLight = true;
						}
						else
						{
							sStream >> tempStr;
							OutputDebugStringA("No Light: ");
							OutputDebugStringA(prefix.c_str());
							OutputDebugStringA(", ");
							OutputDebugStringA(tempStr.c_str());
							OutputDebugStringA("\n");
						}
					}
				}
			}
			else
			{
				sStream >> tempStr;
				OutputDebugStringA("None: ");
				OutputDebugStringA(prefix.c_str());
				OutputDebugStringA(", ");
				OutputDebugStringA(tempStr.c_str());
				OutputDebugStringA("\n");
			}
		}
		m_file.close();
	}
	else // Not found
	{
		if (createIfNotFound)
		{
			std::ofstream outFileStream("Maps\\" + m_mapFileName);
			outFileStream.close();
			m_file.open("Maps\\" + m_mapFileName, std::ios::out | std::ios::in);

			if (!m_file.is_open())
			{
				OutputDebugStringA("Error, could not create map file: ");
				OutputDebugStringA(m_mapFileName.c_str());
				OutputDebugStringA("\n");
			}
			else
				m_file.close();
		}
		else
		{
			OutputDebugStringA("Error, could not open map file: ");
			OutputDebugStringA(m_mapFileName.c_str());
			OutputDebugStringA("\n");
		}
	}
}

void MapHandler::importGameObjects(std::vector<GameObject*>& gameObjects, std::vector<std::pair<Light, LightHelper>>& lights)
{
	size_t sizeBefore = gameObjects.size();
	size_t sizeAfter = sizeBefore + m_gameObjectData.size();
	gameObjects.resize(sizeAfter);
	std::vector<MeshData>* meshData = nullptr;
	for (size_t i = sizeBefore; i < sizeAfter; i++)
	{
		gameObjects[i] = new GameObject();
		if (m_gameObjectData[i - sizeBefore].meshes.empty())
			meshData = nullptr;
		else
			meshData = &m_gameObjectData[i - sizeBefore].meshes;

		gameObjects[i]->initialize(m_gameObjectData[i - sizeBefore].modelFile, (UINT)i + 1, m_gameObjectData[i - sizeBefore].shaderType, meshData);
		gameObjects[i]->setScale(m_gameObjectData[i - sizeBefore].scale);
		gameObjects[i]->setRotation(m_gameObjectData[i - sizeBefore].rotation);
		gameObjects[i]->setPosition(m_gameObjectData[i - sizeBefore].position);
	}
	lights = m_lightData;
}

void MapHandler::addGameObjectToFile(GameObject* gameObject)
{
	GameObjectData data;
	data.modelFile = gameObject->getModelName();
	data.shaderType = gameObject->getShaderType();
	data.scale = gameObject->getScaleF3();
	data.rotation = gameObject->getRotationF3();
	data.position = gameObject->getPositionF3();
	gameObject->fillMeshData(&data.meshes);

	m_gameObjectData.push_back(data);
	std::string tempStr;
	_bstr_t tempCStr;

	m_file.open("Maps\\" + m_mapFileName, std::ios::out | std::ios::app);

	if (m_file.is_open()) // Found
	{
		m_file << "\n";
		m_file << MODEL_FILE_PREFIX << " " << data.modelFile << "\n";
		m_file << SHADER_TYPE_PREFIX << " " << data.shaderType << "\n";
		tempStr = f3ToString(data.scale, " ");
		m_file << TRANSFORM_SCALE_PREFIX << " " << tempStr << "\n";
		tempStr = f3ToString(data.rotation, " ");
		m_file << TRANSFORM_ROTATION_PREFIX << " " << tempStr << "\n";
		tempStr = f3ToString(data.position, " ");
		m_file << TRANSFORM_POSITION_PREFIX << " " << tempStr;

		m_file << NR_OF_MESHES_PREFIX << " " << data.meshes.size() << "\n";
		for (size_t i = 0; i < data.meshes.size(); i++)
		{
			// Name
			m_file << MESH_NAME_PREFIX << " " << data.meshes[i].name << "\n";
			data.meshes[i].matType = data.shaderType;

			// Material
			switch (data.shaderType)
			{
			case PHONG:
			{
				// Texture Paths
				if (!data.meshes[i].matPhong.diffusePath.empty())
				{
					tempCStr = data.meshes[i].matPhong.diffusePath.c_str();
					m_file << MAT_DIFF_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPhong.specularPath.empty())
				{
					tempCStr = data.meshes[i].matPhong.specularPath.c_str();
					m_file << PH_SPEC_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPhong.normalPath.empty())
				{
					tempCStr = data.meshes[i].matPhong.normalPath.c_str();
					m_file << MAT_NORM_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPhong.displacementPath.empty())
				{
					tempCStr = data.meshes[i].matPhong.displacementPath.c_str();
					m_file << MAT_DISP_PATH_PREFIX << " " << tempCStr << "\n";
				}

				// Colors
				tempStr = f4ToString(data.meshes[i].matPhong.emissive, " ", true);
				m_file << PH_EMISSIVE_COL_PREFIX << " " << tempStr << "\n";
				tempStr = f4ToString(data.meshes[i].matPhong.ambient, " ", true);
				m_file << PH_AMBIENT_COL_PREFIX << " " << tempStr << "\n";
				tempStr = f4ToString(data.meshes[i].matPhong.diffuse, " ", true);
				m_file << MAT_DIFFUSE_COL_PREFIX << " " << tempStr << "\n";
				tempStr = f4ToString(data.meshes[i].matPhong.specular, " ", true);
				m_file << PH_SPECULAR_COL_PREFIX << " " << tempStr << "\n";

				m_file << PH_SHININESS_PREFIX << " " << data.meshes[i].matPhong.shininess;

				// Texture Exists
				m_file << PH_DIFF_TEX_PREFIX << " " << data.meshes[i].matPhong.diffTextureExists << "\n";
				m_file << PH_SPEC_TEX_PREFIX << " " << data.meshes[i].matPhong.specTextureExists << "\n";
				m_file << PH_NORM_TEX_PREFIX << " " << data.meshes[i].matPhong.normTextureExists << "\n";
			}
			case PBR:
			{
				// Texture Paths
				if (!data.meshes[i].matPBR.albedoPath.empty())
				{
					tempCStr = data.meshes[i].matPBR.albedoPath.c_str();
					m_file << MAT_DIFF_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPBR.normalPath.empty())
				{
					tempCStr = data.meshes[i].matPBR.normalPath.c_str();
					m_file << MAT_NORM_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPBR.metallicPath.empty())
				{
					tempCStr = data.meshes[i].matPBR.metallicPath.c_str();
					m_file << PB_META_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPBR.roughnessPath.empty())
				{
					tempCStr = data.meshes[i].matPBR.roughnessPath.c_str();
					m_file << PB_ROUG_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPBR.emissivePath.empty())
				{
					tempCStr = data.meshes[i].matPBR.emissivePath.c_str();
					m_file << PB_EMIS_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPBR.ambientOcclusionPath.empty())
				{
					tempCStr = data.meshes[i].matPBR.ambientOcclusionPath.c_str();
					m_file << PB_AMOC_PATH_PREFIX << " " << tempCStr << "\n";
				}
				if (!data.meshes[i].matPBR.displacementPath.empty())
				{
					tempCStr = data.meshes[i].matPBR.displacementPath.c_str();
					m_file << MAT_DISP_PATH_PREFIX << " " << tempCStr << "\n";
				}

				tempStr = f3ToString(data.meshes[i].matPBR.albedo, " ");
				m_file << MAT_DIFFUSE_COL_PREFIX << " " << tempStr << "\n";
				m_file << PB_METALLIC_PREFIX << " " << data.meshes[i].matPBR.metallic << "\n";
				m_file << PB_ROUGHNESS_PREFIX << " " << data.meshes[i].matPBR.roughness << "\n";
				m_file << PB_EMISS_STR_PREFIX << " " << data.meshes[i].matPBR.emissiveStrength << "\n";
				m_file << PB_TEXTURED_PREFIX << " " << data.meshes[i].matPBR.materialTextured << "\n";
				m_file << PB_EMI_TEXT_PREFIX << " " << data.meshes[i].matPBR.emissiveTextured << "\n";
			}
			default:
			{ break; }
			}
		}

		m_file.close();

		OutputDebugStringA("Game Object Added to Map File: ");
		OutputDebugStringA(m_mapFileName.c_str());
		OutputDebugStringA("\n");
	}
	else
	{
		OutputDebugStringA("Error, could not find map file: ");
		OutputDebugStringA(m_mapFileName.c_str());
		OutputDebugStringA("\n");
	}
}

void MapHandler::removeGameObjectFromFile(int removedIndex, int gameObjectSize)
{
	int sizeDifference = gameObjectSize - (int)m_gameObjectData.size();
	m_gameObjectData.erase(m_gameObjectData.begin() + removedIndex - sizeDifference);

	dumpDataToFile();
}

void MapHandler::updateDataList(std::vector<GameObject*>& gameObjects, std::vector<std::pair<Light, LightHelper>>& lights)
{
	size_t totalSize = gameObjects.size();
	m_gameObjectData.clear();
	m_gameObjectData.resize(totalSize - m_nrOfDifference);
	for (size_t i = m_nrOfDifference; i <= m_gameObjectData.size(); i++)
	{
		m_gameObjectData[i - m_nrOfDifference].modelFile = gameObjects[i]->getModelName();
		m_gameObjectData[i - m_nrOfDifference].shaderType = gameObjects[i]->getShaderType();
		m_gameObjectData[i - m_nrOfDifference].scale = gameObjects[i]->getScaleF3();
		m_gameObjectData[i - m_nrOfDifference].rotation = gameObjects[i]->getRotationF3();
		m_gameObjectData[i - m_nrOfDifference].position = gameObjects[i]->getPositionF3();
		gameObjects[i]->fillMeshData(&m_gameObjectData[i - m_nrOfDifference].meshes);
	}
	m_lightData = lights;

	dumpDataToFile();
}
