#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "GameObject.h"
#include "CameraObject.h"
#include "MapHandler.h"

class GameState
{
private:
	// Map Handler
	MapHandler m_mapHandler;
	
	// Game Objects
	std::vector<GameObject*> m_gameObjects;

	// Lights
	std::vector<Light*> m_lights;

	// Camera
	CameraObject m_camera;

	// ImGui
	bool m_windowMoveFlag;
	bool m_windowResizeFlag;
	bool m_enviormentPanelShow = true;

	float m_gameObjectSectionHeight = 500.f;
	float m_lightSectionHeight = 195.f;
	ID3D11Resource* m_defaultModelThumbnail;
	std::vector<std::pair<std::string, bool>> m_modelNames; // string = Name, bool = is Directory
	std::vector<bool> m_modelNameHoveringState;
	std::string m_currentDirectoryPath;
	std::string m_currentDirectoryName;
	const std::string m_rootModelDirectory = "Models\\";

	void loadModelList(std::string path = "Models\\");
	void setImGuiStyles();

	// Selection
	int m_selectedIndex;
	bool m_dragging;
	char m_draggingDimension;
	float m_origin;

	// Render Handler
	RenderHandler* m_renderHandler;

public:
	GameState();
	~GameState();

	// Inititialization
	void initialize(Settings settings);

	// Update
	void controls(float dt);
	void update(float dt);
};

#endif // !GAMESTATE_H