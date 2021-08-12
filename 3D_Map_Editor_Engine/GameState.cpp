#include "pch.h"
#include "GameState.h"

void GameState::loadModelList()
{
	std::string path = "Models\\";
	std::string name = "";
	std::string fileExtension = "";
	struct dirent* entry;
	DIR* dir = opendir(path.c_str());

	m_modelNames.clear();
	m_modelNameHoveringState.clear();

	if (dir != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			name = entry->d_name;
			size_t i = name.rfind('.', name.length());
			if (i != std::string::npos) 
			{
				fileExtension = name.substr(i + 1, name.length() - i);
				if (fileExtension == "obj" || fileExtension == "FBX" || fileExtension == "fbx")
				{
					m_modelNames.push_back(entry->d_name);
					m_modelNameHoveringState.push_back(false);
				}
			}
		}

		closedir(dir);
	}
}

void GameState::setImGuiStyles()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.FrameBorderSize = 0.f;
	//style.WindowBorderSize = 0.f;
	style.ChildBorderSize = 2.f;
	style.TabBorderSize = 0.f;
	style.PopupBorderSize = 0.f;

	style.WindowPadding = ImVec2(5, 5);
	style.WindowRounding = 2.0f;
	style.FramePadding = ImVec2(5, 5);
	style.FrameRounding = 2.0f;
	style.ItemSpacing = ImVec2(0, 5);
	/*style.ItemInnerSpacing = ImVec2(8, 6);
	style.IndentSpacing = 25.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 3.0f;*/

	//style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	//style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.1f, 0.2f);
	//style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.1f, 0.15f, 0.2f, 1.f);
	//style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	//style.Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.f);
	style.Colors[ImGuiCol_FrameBg]			= ImVec4(0.1f, 0.15f, 0.2f, 1.f);
	style.Colors[ImGuiCol_FrameBgHovered]	= ImVec4(0.15f, 0.225f, 0.4f, 1.f);
	style.Colors[ImGuiCol_FrameBgActive]	= ImVec4(0.05f, 0.075f, 0.1f, 1.f);
	//style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	//style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	//style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.f, 0.f, 0.f, 1.f);
	//style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	//style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	//style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	//style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.9f, 0.9f, 0.8f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5f, 0.6f, 0.9f, 1.f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.f, 1.f / 255.f * 150.f, 1.f, 1.f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.f, 1.f / 255.f * 130.f, 1.f, 1.f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.f, 1.f / 255.f * 130.f, 1.f, 0.8f);
	style.Colors[ImGuiCol_Header]			= ImVec4(0.10f, 0.15f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered]	= ImVec4(0.15f, 0.225f, 0.30f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive]		= ImVec4(0.05f, 0.075f, 0.10f, 1.00f);
	//style.Colors[ImGuiCol_Separator] = ImVec4(1.f, 0.f, 0.f, 0.00f); // ImGuiCol_Column
	//style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	//style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	//style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);*/
	//style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	//style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style.Colors[ImGuiCol_closebutton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f); // ImGuiCol_CloseButton
	//style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	//style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.f);
	//style.Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	//style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	//style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	//style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	//style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	//style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}

GameState::GameState()
{
	m_windowResizeFlag = false;
	m_windowMoveFlag = false;
	m_defaultModelThumbnail = nullptr;
	m_selectedIndex = -1;
	m_dragging = false;
	m_draggingDimension = 'n';
	m_origin = 1.f;
	m_renderHandler = nullptr;
	srand(time(NULL));
}

GameState::~GameState()
{
	m_gameObjects.clear();
}

void GameState::initialize(Settings settings)
{
	// Render Handler
	m_renderHandler = RenderHandler::getInstance();

	// ImGui
	// - Styles
	setImGuiStyles();

	// - Model List
	loadModelList();

	// Games Objects
	// - Ground quad
	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("", m_gameObjects.size());
	m_gameObjects.back()->setScale(XMVectorSet(1000.f, 1000.f, 1000.f, 1.f));

	// Map Handler
	m_mapHandler.initialize("map1.txt", m_gameObjects.size(), true);

	// - Game Objects from Map file
	m_mapHandler.importGameObjects(m_gameObjects);

	// - Nanosuit
	/*m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("nanosuit.obj", m_gameObjects.size());

	// - Alter
	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("Platform.FBX", m_gameObjects.size());
	m_gameObjects.back()->setRotation(XMVectorSet(XM_PI / 2, 0.f, 0.f, 0.f));
	m_gameObjects.back()->setPosition(XMVectorSet(-80.f, 4.4f, 0.f, 1.f));
	TexturePaths terrainTextures;
	terrainTextures.diffusePath = L"alter_model\\Platform_diffuse.png";
	terrainTextures.normalPath = L"alter_model\\Platform_normal.png";
	terrainTextures.specularPath = L"alter_model\\Platform_specular.png";
	m_gameObjects.back()->setTextures(terrainTextures);

	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("Altar.FBX", m_gameObjects.size());
	m_gameObjects.back()->setRotation(XMVectorSet(XM_PI / 2, 0.f, 0.f, 0.f));
	m_gameObjects.back()->setPosition(XMVectorSet(-79.f, 0.2f, 2.3f, 1.f));
	terrainTextures.diffusePath = L"alter_model\\Altar_diffuse.png";
	terrainTextures.normalPath = L"alter_model\\Altar_normal.png";
	terrainTextures.specularPath = L"alter_model\\Altar_specular.png";
	m_gameObjects.back()->setTextures(terrainTextures);

	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("Arches.FBX", m_gameObjects.size());
	m_gameObjects.back()->setRotation(XMVectorSet(XM_PI / 2, 0.f, 0.f, 0.f));
	m_gameObjects.back()->setPosition(XMVectorSet(-80.f, 0.f, 7.f, 1.f));
	terrainTextures.diffusePath = L"alter_model\\Arches_diffuse.png";
	terrainTextures.normalPath = L"alter_model\\Arches_normal.png";
	terrainTextures.specularPath = L"alter_model\\Arches_specular.png";
	m_gameObjects.back()->setTextures(terrainTextures);

	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("Flag.FBX", m_gameObjects.size());
	m_gameObjects.back()->setRotation(XMVectorSet(XM_PI / 2, 0.f, 0.f, 0.f));
	m_gameObjects.back()->setPosition(XMVectorSet(-87.7f, 28.f, 18.3f, 1.f));
	terrainTextures.diffusePath = L"alter_model\\Flag_diffuse.png";
	terrainTextures.normalPath = L"alter_model\\Flag_normal.png";
	terrainTextures.specularPath = L"alter_model\\Flag_specular.png";
	m_gameObjects.back()->setTextures(terrainTextures);

	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("Tree.FBX", m_gameObjects.size());
	m_gameObjects.back()->setRotation(XMVectorSet(XM_PI / 2, 0.f, 0.f, 0.f));
	m_gameObjects.back()->setPosition(XMVectorSet(-97.f, 5.5f, 46.f, 1.f));
	terrainTextures.diffusePath = L"alter_model\\Tree_diffuse.png";
	terrainTextures.normalPath = L"alter_model\\Tree_normal.png";
	terrainTextures.specularPath = L"alter_model\\Tree_specular.png";
	m_gameObjects.back()->setTextures(terrainTextures);*/
	// Alter end
	
	/*m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("ddh.obj", m_gameObjects.size());
	m_gameObjects.back()->setPosition(XMVectorSet(-10.f, 10.f, 0.f, 1.f));

	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("FinalHook.obj", m_gameObjects.size());
	m_gameObjects.back()->setPosition(XMVectorSet(0.f, 10.f, 20.f, 1.f));
	TexturePaths textures;
	textures.diffusePath = L"hook.png";
	m_gameObjects.back()->setTextures(textures);*/
	
	m_camera.initialize(settings.mouseSensitivity);
}

void GameState::controlls(float dt)
{
	if (!InputHandler::getInstance().keyBufferIsEmpty())
	{
		//Rotate Camera
		while (!InputHandler::getInstance().mouseBufferIsEmpty())
		{
			MouseEvent mouseEvent = InputHandler::getInstance().readMouseEvent();
			if (mouseEvent.type == MouseEventType::RawMove)
			{
				if (InputHandler::getInstance().keyIsPressed(KeyCodes::F))
				{
					// Hide Cursor
					while (::ShowCursor(FALSE) >= 0);
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;

					// Camera Rotation
					m_camera.rotate(mouseEvent.point.x, mouseEvent.point.y);
				}
				else
				{
					while (::ShowCursor(TRUE) < 0);
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
				}
			}
			else if (mouseEvent.type == MouseEventType::LPress)
			{
				if (m_selectedIndex > -1)
				{
					if (!m_dragging)
					{
						float arrowHit = m_renderHandler->selectionArrowPicking((float)mouseEvent.point.x, (float)mouseEvent.point.y, 'x');
						if (arrowHit)
						{
							m_dragging = true;
							m_origin = (-arrowHit / 2.f) - 1.f;
							m_draggingDimension = 'x';
						}

						arrowHit = m_renderHandler->selectionArrowPicking((float)mouseEvent.point.x, (float)mouseEvent.point.y, 'y');
						if (arrowHit)
						{
							m_dragging = true;
							m_origin = (-arrowHit / 2.f) - 1.f;
							m_draggingDimension = 'y';
						}

						arrowHit = m_renderHandler->selectionArrowPicking((float)mouseEvent.point.x, (float)mouseEvent.point.y, 'z');
						if (arrowHit)
						{
							m_dragging = true;
							m_origin = (-arrowHit / 2.f) - 1.f;
							m_draggingDimension = 'z';
						}
					}
				}
			}
			else if (mouseEvent.type == MouseEventType::LRelease)
			{
				if (m_dragging)
				{
					m_dragging = false;
					m_draggingDimension = 'n';
				}
			}
			else if (mouseEvent.type == MouseEventType::Move)
			{
				if (m_selectedIndex > -1 && m_dragging)
				{
					// Dragging Logic
					XMFLOAT3 objectPosition = m_gameObjects[m_selectedIndex]->getPositionF3();
					XMFLOAT3 rayDirection = m_renderHandler->getRayWorldDirection(mouseEvent.point.x, mouseEvent.point.y);
					XMFLOAT3 planeNormal;
					switch (m_draggingDimension)
					{
					case 'x':
						planeNormal = XMFLOAT3(0.f, -rayDirection.y, -rayDirection.z);
						break;

					case 'y':
						planeNormal = XMFLOAT3(-rayDirection.x, 0.f, -rayDirection.z);
						break;

					case 'z':
						planeNormal = XMFLOAT3(-rayDirection.x, -rayDirection.y, 0.f);
						break;

					default:
						assert(!"Error, invalid dragging dimension!");
						break;
					}
					SimpleMath::Plane plane(objectPosition, planeNormal);
					SimpleMath::Ray ray(m_camera.getPositionF3(), rayDirection);

					float distance = -1.f;
					ray.Intersects(plane, distance);
					SimpleMath::Vector3 rayDirectionNorm = ray.direction;
					rayDirectionNorm.Normalize();

					XMFLOAT3 endPosition = (rayDirectionNorm * distance) + ray.position;

					switch (m_draggingDimension)
					{
					case 'x':
						m_gameObjects[m_selectedIndex]->setPosition(XMVectorSet((m_origin + endPosition.x), objectPosition.y, objectPosition.z, 1.f));
						break;

					case 'y':
						m_gameObjects[m_selectedIndex]->setPosition(XMVectorSet(objectPosition.x, (m_origin + endPosition.y), objectPosition.z, 1.f));
						break;

					case 'z':
						m_gameObjects[m_selectedIndex]->setPosition(XMVectorSet(objectPosition.x, objectPosition.y, (m_origin + endPosition.z), 1.f));
						break;

					default:
						assert(!"Error, invalid dragging dimension!");
						break;
					}
					m_renderHandler->updateSelectedObject(m_gameObjects[m_selectedIndex]->getKey(), m_gameObjects[m_selectedIndex]->getPositionF3());
				}
			}
		}

		// Camera Movement
		if (InputHandler::getInstance().keyIsPressed(KeyCodes::Home))
			m_camera.resetPosAndRot();

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::W))
			m_camera.addForce(Direction::FORWARD, dt);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::S))
			m_camera.addForce(Direction::BACKWARD, dt);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::A))
			m_camera.addForce(Direction::LEFT, dt);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::D))
			m_camera.addForce(Direction::RIGHT, dt);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::Space))
			m_camera.addForce(Direction::UP, dt);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::LeftShift))
			m_camera.addForce(Direction::DOWN, dt);
	}
}

void GameState::update(float dt)
{
	// ImGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Settings Window
	ImGuiWindowFlags windowFlags = 0;
	if (!m_windowResizeFlag)
		windowFlags |= ImGuiWindowFlags_NoResize;
	if (!m_windowMoveFlag)
		windowFlags |= ImGuiWindowFlags_NoMove;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.05f, 0.1f, 1.f));
	ImGui::SetNextWindowSize(ImVec2(200.f, 0));
	ImGui::Begin("Main", NULL, windowFlags | ImGuiWindowFlags_NoTitleBar);
	ImGui::Text("%.0f FPS", ImGui::GetIO().Framerate);
	//ImGui::Text("DisplaySize = %f, %f", ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);
	
	// Save to Map File Start
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::SameLine(ImGui::GetWindowWidth() - 28);
	if (ImGui::ImageButton(ResourceHandler::getInstance().getTexture(L"baseline_save_white_18dp.png"), ImVec2(20, 20)))
		m_mapHandler.updateDataList(m_gameObjects);
	ImGui::PopStyleVar();
	// Save to Map File End

	if (ImGui::CollapsingHeader("Settings"))
	{
		ImGui::Checkbox(" Wireframe Mode", RenderHandler::getInstance()->getWireframeModePtr());
		ImGui::Checkbox(" Window Resize", &m_windowResizeFlag);
		ImGui::Checkbox(" Window Move", &m_windowMoveFlag);
	}
	ImGui::End();
	ImGui::PopStyleColor();
	
	// Model Card Window
	if (m_modelNames.size())
	{
		ImGui::Begin("Models", NULL, windowFlags);
		ImGui::Columns(3, 0, false);
		for (size_t i = 0; i < m_modelNames.size(); i++)
		{
			// Hover style Push
			if (m_modelNameHoveringState[i])
			{
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.7f, 0.9f, 1.f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.f);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.15f, 0.2f, 1.f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.f);
			}

			// Model card Begin
			ImGui::BeginChildFrame(i + 1, ImVec2(110, 130));
			ImGui::Image(ResourceHandler::getInstance().getTexture(L"circle_pattern.png"), ImVec2(100, 100));
			ImGui::Text("%s", m_modelNames[i].c_str());
			ImGui::EndChildFrame();
			// Model card End

			// Hover style Pop
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

			// Mouse Behavior
			if (ImGui::IsItemClicked())
			{
				TexturePaths terrainTextures;
				m_gameObjects.push_back(new GameObject());
				m_gameObjects.back()->initialize(m_modelNames[i], m_gameObjects.size());
				//m_mapHandler.addGameObjectToFile(m_gameObjects.back());
			}
			m_modelNameHoveringState[i] = ImGui::IsItemHovered();
			// Mouse Behavior End
			ImGui::NextColumn();
		}
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Border] = ImVec4(0.1f, 0.15f, 0.2f, 1.f);
		ImGui::Columns(1);
		ImGui::End();
	}

	// GameObject Menu
	ImGui::Begin("Game Objects", NULL, windowFlags);
	for (size_t i = 0; i < m_gameObjects.size(); i++)
	{
		ImGui::Text(m_gameObjects[i]->getModelNameAndId().c_str());
		ImGui::PushID("gameobject_update_" + i);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::SameLine(ImGui::GetWindowWidth() - 28);
		if (ImGui::ImageButton(ResourceHandler::getInstance().getTexture(L"baseline_delete_white_18dp.png"), ImVec2(20, 20)))
		{
			if (m_selectedIndex = i)
			{
				m_selectedIndex = -1;
				m_renderHandler->deselectObject();
			}
			//m_mapHandler.removeGameObjectFromFile(i, m_gameObjects.size());
			delete m_gameObjects[i];
			m_gameObjects.erase(m_gameObjects.begin() + i);
			ImGui::PopStyleVar();
		}
		else
		{
			ImGui::PopStyleVar();
			ImGui::SameLine(ImGui::GetWindowWidth() - 28 - 56);
			if (ImGui::Button("Select"))
			{
				m_selectedIndex = i;
				m_renderHandler->updateSelectedObject(m_gameObjects[i]->getKey(), m_gameObjects[i]->getPositionF3());
			}
			m_gameObjects[i]->update(dt);
		}
		ImGui::PopID();
	}
	ImGui::End();

	ImGui::ShowDemoWindow(); // For debugging
	// Render ImGui(Runs at the end of RenderHandler render funtion!)
	ImGui::Render();

	// Controlls
	controlls(dt);

	// Camera
	m_camera.update(dt);
}