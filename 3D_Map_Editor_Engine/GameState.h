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

	// Camera
	CameraObject m_camera;

	// IMGUI
	bool m_windowMoveFlag;
	bool m_windowResizeFlag;
	ID3D11Resource* m_defaultModelThumbnail;
	std::vector<std::string> m_modelNames;
	std::vector<bool> m_modelNameHoveringState;
	void loadModelList();
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
	void controlls(float dt);
	void update(float dt);
};

#endif // !GAMESTATE_H