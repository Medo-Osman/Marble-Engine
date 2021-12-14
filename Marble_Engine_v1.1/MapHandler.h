#ifndef MAPHANDLER_H
#define MAPHANDLER_H

#include "GameObject.h"

class MapHandler
{
private:
	std::string m_mapFileName;
	std::fstream m_file;
	std::vector<GameObjectData> m_gameObjectData;
	std::vector<std::pair<Light, LightHelper>> m_lightData;
	int m_nrOfDifference;

	void dumpDataToFile();

public:
	MapHandler();
	~MapHandler();

	// Initialize
	void initialize(std::string mapFileName, int nrOfHardCodedGameObjects, bool createIfNotFound = false);

	// Getters
	size_t getNrOfGameObjects() const { return m_gameObjectData.size(); }

	// Update
	void importGameObjects(std::vector<GameObject*>& gameObjects, std::vector<std::pair<Light, LightHelper>>& lights);
	void addGameObjectToFile(GameObject* gameObject);
	void removeGameObjectFromFile(int removedIndex, int gameObjectSize);
	void updateDataList(std::vector<GameObject*>& gameObjects, std::vector<std::pair<Light, LightHelper>>& lights);
};

#endif // !MAPHANDLER_H