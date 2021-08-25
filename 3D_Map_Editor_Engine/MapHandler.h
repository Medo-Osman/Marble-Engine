#include "pch.h"
#ifndef MAPHANDLER_H
#define MAPHANDLER_H

struct GameObjectData
{
	std::string modelFile = "";
	ShaderStates shaderType = ShaderStates::PHONG;
	XMFLOAT3 scale				= { 1.f,1.f,1.f };
	XMFLOAT3 rotation			= { 0.f,0.f,0.f };
	XMFLOAT3 position			= { 0.f,0.f,0.f };
};

const std::string MODEL_FILE_PREFIX = "mf";
const std::string SHADER_TYPE_PREFIX = "st";
const std::string TRANSFORM_SCALE_PREFIX = "ts";
const std::string TRANSFORM_ROTATION_PREFIX = "tr";
const std::string TRANSFORM_POSITION_PREFIX = "tp";

class MapHandler
{
private:
	std::string m_mapFileName;
	std::fstream m_file;
	std::vector<GameObjectData> m_gameObjectData;
	int m_nrOfDifference;

	void dumpDataToFile()
	{
		std::string tempStr;
		m_file.open("Maps\\" + m_mapFileName, std::ios::out | std::ofstream::trunc);

		if (m_file.is_open()) // Found
		{
			for (size_t i = 0; i < m_gameObjectData.size(); i++)
			{
				m_file << MODEL_FILE_PREFIX << " " << m_gameObjectData[i].modelFile << "\n";
				m_file << SHADER_TYPE_PREFIX << " " << m_gameObjectData[i].shaderType << "\n";
				tempStr = f3ToString(m_gameObjectData[i].scale, " ");
				m_file << TRANSFORM_SCALE_PREFIX << " " << tempStr << "\n";
				tempStr = f3ToString(m_gameObjectData[i].rotation, " ");
				m_file << TRANSFORM_ROTATION_PREFIX << " " << tempStr << "\n";
				tempStr = f3ToString(m_gameObjectData[i].position, " ");
				m_file << TRANSFORM_POSITION_PREFIX << " " << tempStr;
				m_file << "\n";
			}

			m_file.close();

			OutputDebugStringA("Game Object Saved to Map File: ");
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

public:
	MapHandler()
	{
		m_nrOfDifference = 0;
	}
	~MapHandler()
	{
		m_file.close();
	}

	void initialize(std::string mapFileName, int nrOfHardCodedGameObjects, bool createIfNotFound = false)
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
			int tempShaderType;
			XMFLOAT3 tempF3 = XMFLOAT3(0.f, 0.f, 0.f);

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
				if (prefix == SHADER_TYPE_PREFIX)
				{
					sStream >> tempShaderType;
					m_gameObjectData.back().shaderType = (ShaderStates)tempShaderType;
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
				else
				{
					sStream >> tempStr;
					OutputDebugStringA("None: ");
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

	size_t getNrOfGameObjects() const { return m_gameObjectData.size(); }

	void importGameObjects(std::vector<GameObject*>& gameObjects)
	{
		size_t sizeBefore = gameObjects.size();
		size_t sizeAfter = sizeBefore + m_gameObjectData.size();
		gameObjects.resize(sizeAfter);
		for (size_t i = sizeBefore; i < sizeAfter; i++)
		{
			gameObjects[i] = new GameObject();
			gameObjects[i]->initialize(		m_gameObjectData[i - sizeBefore].modelFile, i + 1, m_gameObjectData[i - sizeBefore].shaderType);
			gameObjects[i]->setScale(		m_gameObjectData[i - sizeBefore].scale);
			gameObjects[i]->setRotation(	m_gameObjectData[i - sizeBefore].rotation);
			gameObjects[i]->setPosition(	m_gameObjectData[i - sizeBefore].position);
		}
	}

	void addGameObjectToFile(GameObject* gameObject)
	{
		GameObjectData data;
		data.modelFile	= gameObject->getModelName();
		data.shaderType	= gameObject->getShaderType();
		data.scale		= gameObject->getScaleF3();
		data.rotation	= gameObject->getRotationF3();
		data.position	= gameObject->getPositionF3();
		m_gameObjectData.push_back(data);
		std::string tempStr;

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

	void removeGameObjectFromFile(int removedIndex, int gameObjectSize)
	{
		int sizeDifference = gameObjectSize - m_gameObjectData.size();
		m_gameObjectData.erase(m_gameObjectData.begin() + removedIndex - sizeDifference);

		dumpDataToFile();
	}

	void updateDataList(std::vector<GameObject*>& gameObjects)
	{
		size_t totalSize = gameObjects.size();
		m_gameObjectData.clear();
		m_gameObjectData.resize(totalSize - m_nrOfDifference);
		for (size_t i = m_nrOfDifference; i <= m_gameObjectData.size(); i++)
		{
			m_gameObjectData[i - m_nrOfDifference].modelFile	= gameObjects[i]->getModelName();
			m_gameObjectData[i - m_nrOfDifference].shaderType	= gameObjects[i]->getShaderType();
			m_gameObjectData[i - m_nrOfDifference].scale		= gameObjects[i]->getScaleF3();
			m_gameObjectData[i - m_nrOfDifference].rotation		= gameObjects[i]->getRotationF3();
			m_gameObjectData[i - m_nrOfDifference].position		= gameObjects[i]->getPositionF3();
		}

		dumpDataToFile();
	}
};

#endif // !MAPHANDLER_H