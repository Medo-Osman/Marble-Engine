#include "pch.h"
#include "GameState.h"
#include "Application.h"

void GameState::loadModelList(std::string path)
{
	std::string name = "";
	std::string fileExtension = "";
	struct dirent* entry;
	DIR* dir = opendir(path.c_str());

	m_modelNames.clear();
	m_modelNameHoveringState.clear();
	std::vector<std::string> modelNames;
	if (dir != NULL)
	{
		while ((entry = readdir(dir)) != NULL)
		{
			name = entry->d_name;
			if (entry->d_type == DT_DIR && name != "." &&
				((path == m_rootModelDirectory && name != "..") ||
				(path != m_rootModelDirectory)))
			{
				std::pair<std::string, bool> directory(name, true);
				m_modelNames.push_back(directory);
				m_modelNameHoveringState.push_back(false);
			}
			else
			{
				size_t i = name.rfind('.', name.length());
				if (i != std::string::npos) 
				{
					fileExtension = name.substr(i + 1, name.length() - i);
					if (fileExtension == "obj" || fileExtension == "FBX" || fileExtension == "fbx" || fileExtension == "glb" || fileExtension == "gltf")
					{
						modelNames.push_back(name);
					}
				}
			}
		}

		for (size_t i = 0; i < modelNames.size(); i++)
		{
			std::pair<std::string, bool> modelFile(modelNames[i], false);
			m_modelNames.push_back(modelFile);
			m_modelNameHoveringState.push_back(false);
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
	style.ItemSpacing = ImVec2(5, 5);
	style.ItemInnerSpacing = ImVec2(5, 5);
	/*style.IndentSpacing = 25.0f;
	style.ScrollbarSize = 15.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 5.0f;
	style.GrabRounding = 3.0f;*/

	//style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	//style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.1f, 0.5f);
	//style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	//style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.1f, 0.3f);
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
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.f, 0.f, 0.1f, 0.2f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5f, 0.6f, 0.9f, 1.f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.f, 1.f / 255.f * 150.f, 1.f, 1.f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.f, 1.f / 255.f * 130.f, 1.f, 1.f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.f, 1.f / 255.f * 130.f, 1.f, 0.8f);
	style.Colors[ImGuiCol_Header]			= ImVec4(0.10f, 0.15f, 0.20f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered]	= ImVec4(0.15f, 0.225f, 0.30f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive]		= ImVec4(0.05f, 0.075f, 0.10f, 1.00f);
	style.Colors[ImGuiCol_Separator] = ImVec4(.8f, .8f, .8f, 1.f); // ImGuiCol_Column
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

void DrawSplitter(bool split_vertically, float thickness, float* size0, float* size1, float min_size0, float min_size1, float size = -1.f, float buttonPadding = 0.f)
{
	ImVec2 backup_pos = ImGui::GetCursorPos();
	if (split_vertically)
		ImGui::SetCursorPosY(backup_pos.y + *size0 + buttonPadding);
	else
		ImGui::SetCursorPosX(backup_pos.x + *size0 + buttonPadding);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.29f, 0.478f, 0.8f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0)); // We don't draw while active/pressed because as we move the panes the splitter button will be 1 frame late
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.8f, 0.60f));
	ImGui::Button("##Splitter", ImVec2(!split_vertically ? thickness : size, split_vertically ? thickness : size));
	ImGui::PopStyleColor(3);

	ImGui::SetItemAllowOverlap(); // This is to allow having other buttons OVER our splitter. 

	if (ImGui::IsItemActive())
	{
		float mouse_delta = split_vertically ? ImGui::GetIO().MouseDelta.y : ImGui::GetIO().MouseDelta.x;

		// Minimum pane size
		if (mouse_delta < min_size0 - *size0)
			mouse_delta = min_size0 - *size0;
		if (mouse_delta > *size1 - min_size1)
			mouse_delta = *size1 - min_size1;

		// Apply resize
		*size0 += mouse_delta;
		*size1 -= mouse_delta;
	}
	ImGui::SetCursorPos(backup_pos);
}

void GameState::generateMaxRandomLights()
{
	XMFLOAT3 posScale = XMFLOAT3(21, 15, 53);
	XMFLOAT3 posBias = XMFLOAT3(-10.5f, 3, -26);

	srand(12645);
	auto randUint = []() -> uint32_t
	{
		return rand();
	};
	auto randFloat = [randUint]() -> float
	{
		return randUint() * (1.0f / RAND_MAX); // convert [0, RAND_MAX] to [0, 1]
	};
	auto randVecUniform = [randFloat]() -> XMFLOAT3
	{
		return XMFLOAT3(randFloat(), randFloat(), randFloat());
	};
	auto randGaussian = [randFloat]() -> float
	{
		// polar box-muller
		static bool gaussianPair = true;
		static float y2;

		if (gaussianPair)
		{
			gaussianPair = false;

			float x1, x2, w;
			do
			{
				x1 = 2 * randFloat() - 1;
				x2 = 2 * randFloat() - 1;
				w = x1 * x1 + x2 * x2;
			} while (w >= 1);

			w = sqrtf(-2 * logf(w) / w);
			y2 = x2 * w;
			return x1 * w;
		}
		else
		{
			gaussianPair = true;
			return y2;
		}
	};
	auto randVecGaussian = [randGaussian]() -> XMFLOAT3
	{
		XMFLOAT3 newF3;
		XMStoreFloat3(&newF3, XMVector3Normalize(XMVectorSet(randGaussian() * 360.f, randGaussian() * 360.f, randGaussian() * 360.f, 1.f)));
		return newF3;
	};

	Light newLight;
	LightHelper newLightHelper;
	uint32_t type;
	const float pi = 3.14159265359f;
	for (uint32_t n = 0; n < LIGHT_CAP - 1; n++) // leave initialize() directional light as default
	{
		XMFLOAT3 newRandF3 = randVecUniform();
		XMFLOAT3 pos = XMFLOAT3(newRandF3.x * posScale.x + posBias.x, newRandF3.y * posScale.y + posBias.y, newRandF3.z * posScale.z + posBias.z);
		float lightRadius = randFloat() * 800.0f + 200.0f;

		XMFLOAT3 color = randVecUniform();
		float colorScale = randFloat() * .3f + .3f;
		color = XMFLOAT3(color.x * colorScale, color.y * colorScale, color.z * colorScale);

		// force types to match 32-bit boundaries for the BIT_MASK_SORTED case
		if (n < LIGHT_CAP / 2)
			type = 0;
		else
			type = 1;

		newLightHelper.rotationDeg = randVecGaussian();
		XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(
			XMConvertToRadians(newLightHelper.rotationDeg.x),
			XMConvertToRadians(newLightHelper.rotationDeg.y),
			XMConvertToRadians(newLightHelper.rotationDeg.z));
		XMFLOAT3 coneDir;
		XMStoreFloat3(&coneDir, rotQuat * XMVectorSet(0, 0, 1.f, 1.f));
		float coneInner = (randFloat() * .2f + .025f) * pi;
		float coneOuter = coneInner + randFloat() * .1f * pi;

		newLight.intensity = 20.f;
		if (type == SPOT_LIGHT)
			newLight.intensity = newLight.intensity * 5.f;

		newLight.position = XMFLOAT4(pos.x, pos.y, pos.z, 1.f);
		newLight.direction = XMFLOAT4(coneDir.x, coneDir.y, coneDir.z, 1.f);
		newLight.color = color;
		newLight.spotAngle = XMConvertToRadians(45.0f);
		newLight.attenuation = XMFLOAT3(1.f, 0.6f, 1.f);
		newLight.range = lightRadius;
		newLight.type = type;
		newLight.enabled = true;
		newLight.isCastingShadow = false;
		/*newLight.coneAngles[0] = 1.0f / (cosf(coneInner) - cosf(coneOuter));
		newLight.coneAngles[1] = cosf(coneOuter);*/

		m_lights.push_back(std::make_pair(new Light(newLight), newLightHelper));
		m_renderHandler->addLight(
			newLight, 
			XMFLOAT3(
				XMConvertToRadians(newLightHelper.rotationDeg.x), 
				XMConvertToRadians(newLightHelper.rotationDeg.y), 
				XMConvertToRadians(newLightHelper.rotationDeg.z)), 
			newLight.isCastingShadow);
	}
}

GameState::GameState()
{
	m_windowResizeFlag = false;
	m_windowMoveFlag = false;
	m_defaultModelThumbnail = nullptr;
	m_currentDirectoryPath = m_rootModelDirectory;
	m_currentDirectoryName = m_rootModelDirectory;
	m_selectedIndex = -1;
	m_dragging = false;
	m_draggingDimension = 'n';
	m_origin = 1.f;
	m_renderHandler = nullptr;
	srand((unsigned int)time(NULL));
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

	// - Game objects and Lighting window
	float windowContentHeight = m_renderHandler->getClientHeight() - 70.f; // ImGui window header and border heights
	m_gameObjectSectionHeight = windowContentHeight * 0.6f ;
	m_lightSectionHeight = windowContentHeight - m_gameObjectSectionHeight - m_sectionSeperatorHeight - m_splitterButtonPadding;

	// - Model List
	loadModelList();

	// Games Objects
	// - Ground quad
	m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("", (UINT)m_gameObjects.size(), ShaderStates::PBR);
	m_gameObjects.back()->setScale(XMVectorSet(1000.f, 1000.f, 1000.f, 1.f));
	PS_MATERIAL_PBR_BUFFER groundMat;
	groundMat.albedo = XMFLOAT3(1.f, 1.f, 1.f);
	groundMat.metallic = 0.5f;
	groundMat.roughness = 1.f;
	groundMat.materialTextured = false;
	m_gameObjects.back()->setMaterial(groundMat);

	// PBR Test
	/*m_gameObjects.push_back(new GameObject());
	m_gameObjects.back()->initialize("Cerberus_by_Andrew_Maximov\\Cerberus_LP.FBX", (UINT)m_gameObjects.size(), ShaderStates::PBR);
	m_gameObjects.back()->setScale(XMVectorSet(0.1f, 0.1f, 0.1f, 1.f));
	m_gameObjects.back()->setRotation(XMVectorSet(XM_PIDIV2, 0.f, 0.f, 1.f));
	m_gameObjects.back()->setPosition(XMVectorSet(-3.1f, 5.9f, 0.f, 1.f));
	TexturePathsPBR pbrTextures;
	pbrTextures.albedoPath = L"Cerberus_A.tga";
	pbrTextures.normalPath = L"Cerberus_N.tga";
	pbrTextures.metallicPath = L"Cerberus_M.tga";
	pbrTextures.roughnessPath = L"Cerberus_R.tga";
	pbrTextures.ambientOcclusionPath = L"Cerberus_AO.tga";
	m_gameObjects.back()->setTextures(pbrTextures);*/

	// Map Handler
	m_mapHandler.initialize("map1.txt", (UINT)m_gameObjects.size(), true);

	// - Game Objects from Map file
	m_mapHandler.importGameObjects(m_gameObjects);

	// Lights
	Light light;
	LightHelper lightHelper;
	// Point Light 0
	//light.position = XMFLOAT4(-10.f, 5.f, 0.f, 1.f);
	//light.color = XMFLOAT4(1.f, 0.f, 0.f, 0.f);
	//light.attenuation = XMFLOAT3(1.f, 0.4f, 1.f);
	//light.range = 30.f;
	//light.type = POINT_LIGHT;
	//light.enabled = true;
	//light.isCastingShadow = false;
	//m_lights.push_back(new Light(light));
	//m_renderHandler->addLight(light);

	// Point Light 1
	/*light.position = XMFLOAT4(0.f, 10.f, -10.f, 1.f);
	light.color = XMFLOAT3(1.f, 1.f, 1.f);
	light.attenuation = XMFLOAT3(1.f, 0.4f, 1.f);
	light.range = 30.f;
	light.type = POINT_LIGHT;
	light.enabled = true;
	light.isCastingShadow = false;
	m_lights.push_back(new Light(light));
	m_renderHandler->addLight(light);*/

	// Point Light 2
	/*light.position = XMFLOAT4(0.f, 10.f, 10.f, 1.f);
	light.color = XMFLOAT4(0.f, 1.f, 0.f, 1.f);
	light.attenuation = XMFLOAT3(1.f, 0.4f, 1.f);
	light.range = 40.f;
	light.type = POINT_LIGHT;
	light.enabled = true;
	light.isCastingShadow = false;
	m_lights.push_back(new Light(light));
	m_renderHandler->addLight(light);*/

	// Directional Light
	lightHelper.rotationDeg = XMFLOAT3(-90.f, 0.f, 0.f);

	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(
		XMConvertToRadians(lightHelper.rotationDeg.x),
		XMConvertToRadians(lightHelper.rotationDeg.y),
		XMConvertToRadians(lightHelper.rotationDeg.z));

	XMVECTOR rotQuatInverse = XMQuaternionInverse(rotQuat);
	XMVECTOR lightDir = XMQuaternionMultiply(rotQuat, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
	lightDir = XMQuaternionMultiply(lightDir, rotQuatInverse);
	XMStoreFloat4(&light.direction, lightDir);

	light.color = XMFLOAT3(1.f, 0.7f, 0.5f);
	light.intensity = 1.0f;
	light.type = DIRECTIONAL_LIGHT;
	light.enabled = true;
	light.isCastingShadow = true;
	m_lights.push_back(std::make_pair(new Light(light), lightHelper));
	m_renderHandler->addLight(
		light,
		XMFLOAT3(
			XMConvertToRadians(lightHelper.rotationDeg.x),
			XMConvertToRadians(lightHelper.rotationDeg.y),
			XMConvertToRadians(lightHelper.rotationDeg.z)),
		light.isCastingShadow);

	//generateMaxRandomLights();

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

void GameState::controls(double dt)
{
	if (!InputHandler::getInstance().keyBufferIsEmpty())
	{
		//Rotate Camera
		while (!InputHandler::getInstance().mouseBufferIsEmpty())
		{
			MouseEvent mouseEvent = InputHandler::getInstance().readMouseEvent();
			if (mouseEvent.type == MouseEventType::RawMove)
			{
				if (InputHandler::getInstance().keyIsPressed(KeyCodes::F) || InputHandler::getInstance().isMouseRightDown())
				{
					// Hide Cursor
					while (::ShowCursor(FALSE) >= 0);
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
					// Clip To Window
					Application::getInstance().setClipCursor(true);

					// Camera Rotation
					m_camera.rotate(mouseEvent.point.x, mouseEvent.point.y);
				}
				else
				{
					while (::ShowCursor(TRUE) < 0);
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
					Application::getInstance().setClipCursor(false);
				}
			}
			else if (mouseEvent.type == MouseEventType::LPress)
			{
				if (m_selectedIndex > -1)
				{
					if (!m_dragging)
					{
						float arrowHit = m_renderHandler->selectionArrowPicking((UINT)mouseEvent.point.x, (UINT)mouseEvent.point.y, 'x');
						if (arrowHit)
						{
							m_dragging = true;
							m_origin = (-arrowHit / 2.f) - 1.f;
							m_draggingDimension = 'x';
						}

						arrowHit = m_renderHandler->selectionArrowPicking((UINT)mouseEvent.point.x, (UINT)mouseEvent.point.y, 'y');
						if (arrowHit)
						{
							m_dragging = true;
							m_origin = (-arrowHit / 2.f) - 1.f;
							m_draggingDimension = 'y';
						}

						arrowHit = m_renderHandler->selectionArrowPicking((UINT)mouseEvent.point.x, (UINT)mouseEvent.point.y, 'z');
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
		float speedUpMultiplier = ((float)InputHandler::getInstance().keyIsPressed(KeyCodes::LeftShift) * 3.f) + 1.f;
		if (InputHandler::getInstance().keyIsPressed(KeyCodes::Home))
			m_camera.resetPosAndRot();

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::W))
			m_camera.addForce(Direction::FORWARD, dt, speedUpMultiplier);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::S))
			m_camera.addForce(Direction::BACKWARD, dt, speedUpMultiplier);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::A))
			m_camera.addForce(Direction::LEFT, dt, speedUpMultiplier);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::D))
			m_camera.addForce(Direction::RIGHT, dt, speedUpMultiplier);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::E))
			m_camera.addForce(Direction::UP, dt, speedUpMultiplier);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::Q))
			m_camera.addForce(Direction::DOWN, dt, speedUpMultiplier);

		if (InputHandler::getInstance().keyIsPressed(KeyCodes::R)) // Update PBR Shaders
		{
			m_renderHandler->updateShaderState(ShaderStates::PHONG);
			m_renderHandler->updateShaderState(ShaderStates::PBR);
		}
			//m_renderHandler->updatePassShaders();
		if (InputHandler::getInstance().keyIsPressed(KeyCodes::T)) // Update light Shaders
			m_renderHandler->updatePassShaders();
		
		if (InputHandler::getInstance().keyIsPressed(KeyCodes::C))
			m_renderHandler->resetParticles();
	}
}

void GameState::update(double dt)
{
	// ImGUI
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGuiStyle& imguiStyle = ImGui::GetStyle();

	// Settings Window
	ImGuiWindowFlags windowFlags = 0;
	if (!m_windowResizeFlag)
		windowFlags |= ImGuiWindowFlags_NoResize;
	if (!m_windowMoveFlag)
		windowFlags |= ImGuiWindowFlags_NoMove;
	{
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
	
		ImGui::SameLine(ImGui::GetWindowWidth() - 56);
		if (ImGui::ImageButton(ResourceHandler::getInstance().getTexture(L"outline_refresh_white_18dp.png"), ImVec2(20, 20)))
		{
			size_t nrOf = m_gameObjects.size();
			for (size_t i = 1; i < nrOf; i++) // Dont remove plane
			{
				delete m_gameObjects[m_gameObjects.size() - 1];
				m_gameObjects.erase(m_gameObjects.end() - 1);
			}
			m_mapHandler.importGameObjects(m_gameObjects);
		}
		ImGui::PopStyleVar();
		// Save to Map File End

		if (ImGui::CollapsingHeader("Settings"))
		{
			ImGui::Checkbox(" Wireframe Mode", RenderHandler::getInstance()->getWireframeModePtr());
			RenderHandler::getInstance()->UIssaoSettings();
			RenderHandler::getInstance()->UIadaptiveExposureSettings();
			RenderHandler::getInstance()->UIVolumetricSunSettings();
			RenderHandler::getInstance()->UIbloomSettings();
			ImGui::PushItemWidth(-1);
			ImGui::PopItemWidth();
			ImGui::Checkbox(" Window Resize", &m_windowResizeFlag);
			ImGui::Checkbox(" Window Move", &m_windowMoveFlag);
			if (ImGui::CollapsingHeader("Camera"))
				m_camera.updateUI();
		}
		ImGui::End();
		ImGui::PopStyleColor();
	}

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
			// - Thumbnail
			ImGui::BeginChildFrame((ImGuiID)i + 1, ImVec2(110, 130));
			if (m_modelNames[i].second)
			{
				if (m_modelNames[i].first == "..")
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"previous_folder_icon.png"), ImVec2(100, 100));
				else
					ImGui::Image(ResourceHandler::getInstance().getTexture(L"folder_icon.png"), ImVec2(100, 100));
			}
			else
				ImGui::Image(ResourceHandler::getInstance().getTexture(L"model_icon.png"), ImVec2(100, 100));

			// - Name
			if (m_modelNames[i].first == "..")
				ImGui::Text("%s", "Back");
			else
				ImGui::Text("%s", m_modelNames[i].first.c_str());

			ImGui::EndChildFrame();
			// Model card End

			// Hover style Pop
			ImGui::PopStyleColor();
			ImGui::PopStyleVar();

			// Mouse Behavior
			if (ImGui::IsItemClicked())
			{
				if (!m_modelNames[i].second)
				{
					TexturePaths terrainTextures;

					m_gameObjects.push_back(new GameObject());

					size_t position = m_currentDirectoryPath.find_first_of("\\/") + 1;
					std::string modelPath = m_currentDirectoryPath + m_modelNames[i].first;
					modelPath.erase(0, position);
					m_gameObjects.back()->initialize(modelPath, (UINT)m_gameObjects.size(), ShaderStates::PBR);

					//m_mapHandler.addGameObjectToFile(m_gameObjects.back());
				}
				else
				{
					if (m_modelNames[i].first == "..")
					{
						m_currentDirectoryPath.erase(m_currentDirectoryPath.length() - m_currentDirectoryName.length() - 1, m_currentDirectoryName.length() + 2);
						size_t position = m_currentDirectoryPath.find_last_of("\\/");
						m_currentDirectoryName = m_currentDirectoryPath.substr(0, position);
					}
					else
					{
						m_currentDirectoryName = m_modelNames[i].first;
						m_currentDirectoryPath += m_currentDirectoryName + "\\";
					}
					loadModelList(m_currentDirectoryPath);
					break;
				}
			}
			m_modelNameHoveringState[i] = ImGui::IsItemHovered();
			// Mouse Behavior End
			ImGui::NextColumn();
		}
		/*ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Border] = ImVec4(1.1f, 0.15f, 1.0f, 1.f);*/
		ImGui::Columns(1);
		ImGui::End();
	}

	// GameObject Menu
	ImGui::Begin("Objects", NULL, windowFlags);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.29f, 0.478f, 0.8f));
	ImGui::BeginChildFrame(994999, ImVec2(ImGui::GetWindowSize().x - 10.f, 25.f));
	ImGui::Text("Game Objects");
	ImGui::EndChildFrame();
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Border, imguiStyle.Colors[ImGuiCol_ChildBg]);
	DrawSplitter(true, m_sectionSeperatorHeight, &m_gameObjectSectionHeight, &m_lightSectionHeight, 61.f, 61.f, ImGui::GetWindowSize().x - 10.f, m_splitterButtonPadding);
	ImGui::BeginChild("Game Objects", ImVec2(ImGui::GetWindowSize().x - (imguiStyle.ScrollbarSize / 1.5f), m_gameObjectSectionHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
	for (size_t i = 0; i < m_gameObjects.size(); i++)
	{
		ImGui::Text(m_gameObjects[i]->getModelNameAndId().c_str());
		ImGui::PushID("gameobject_update_" + i);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::SameLine(ImGui::GetWindowWidth() - 48);
		if (ImGui::ImageButton(ResourceHandler::getInstance().getTexture(L"baseline_delete_white_18dp.png"), ImVec2(20, 20)))
		{
			if (m_selectedIndex = (int)i)
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
			ImGui::SameLine(ImGui::GetWindowWidth() - 48 - 56);
			if (ImGui::Button("Select"))
			{
				m_selectedIndex = (int)i;
				m_renderHandler->updateSelectedObject(m_gameObjects[i]->getKey(), m_gameObjects[i]->getPositionF3());
			}
			m_gameObjects[i]->update(dt);
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	// Lighting Menu
	// - Label
	ImGui::Dummy(ImVec2(0.0f, m_splitterButtonPadding / 2.f));
	ImGui::Indent(5.0f);
	ImGui::Text("Lighting");
	ImGui::Unindent(5.0f);
	ImGui::Dummy(ImVec2(0.0f, m_splitterButtonPadding / 2.f));

	// - Group
	ImGui::BeginChild("Lights", ImVec2(ImGui::GetWindowSize().x - (imguiStyle.ScrollbarSize / 1.5f), m_lightSectionHeight), true);

	// - - Enviorment
	RenderHandler::getInstance()->UIEnviormentPanel();
	// - - Lights
	if (ImGui::CollapsingHeader("Lights Panel", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text((std::to_string(m_lights.size()) + " / " + std::to_string(LIGHT_CAP)).c_str());
		ImGui::SameLine(ImGui::GetWindowWidth() - 123.f);
		ImGui::PushItemWidth(100.f);

		if (ImGui::BeginCombo("##Add Light", "Add Light"))
		{
			for (int n = 0; n < 3; n++)
			{
				if (ImGui::Selectable(LightTypeNames[n], false))
				{
					Light newLight;
					LightHelper newLightHelper;
					newLight.position = XMFLOAT4(0.f, 5.f, 0.f, 1.f);
					newLight.direction = XMFLOAT4(0.1f, -0.4f, 0.f, 0.f);
					newLight.color = XMFLOAT3(1.f, 1.f, 1.f);
					newLight.intensity = 1.f;
					newLight.spotAngle = 1.f;
					newLight.attenuation = XMFLOAT3(1.f, 0.4f, 1.f);
					newLight.range = 1.f;
					newLight.type = (LightType)n;
					newLight.enabled = true;
					newLight.isCastingShadow = false;

					if (RenderHandler::getInstance()->addLight(newLight) != -1)
						m_lights.push_back(std::make_pair(new Light(newLight), newLightHelper));
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		ImGui::Separator();

		ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));
		ImGui::Columns(2, "lightsSettings", false);
		static float colWidth = 105.f;
		ImGui::SetColumnWidth(0, colWidth);

		for (size_t i = 0; i < m_lights.size(); i++)
		{
			ImGui::Dummy(ImVec2(0.0f, 3.f));
			ImGui::Text((std::to_string(i) + ", " + LightTypeNames[m_lights[i].first->type]).c_str());
			ImGui::PushID("light" + i);

			ImGui::NextColumn();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::SameLine(ImGui::GetWindowWidth() - colWidth - 48);
			if (ImGui::ImageButton(ResourceHandler::getInstance().getTexture(L"baseline_delete_white_18dp.png"), ImVec2(20, 20)))
			{
				if (m_lights[i].first->isCastingShadow) // Disable Shadow Mapping if light is casting Shadows
				{
					m_renderHandler->changeShadowMappingLight(
						m_lights[i].first, 
						XMFLOAT3(
							XMConvertToRadians(m_lights[i].second.rotationDeg.x),
							XMConvertToRadians(m_lights[i].second.rotationDeg.y),
							XMConvertToRadians(m_lights[i].second.rotationDeg.z)),
						true);

				}

				delete m_lights[i].first;
				m_lights.erase(m_lights.begin() + i);
				m_renderHandler->removeLight((int)i);
				ImGui::PopStyleVar();
			}
			else
			{
				ImGui::PopStyleVar();

				ImGui::Separator();

				ImGui::NextColumn();
				ImGui::Text("Color");
				ImGui::NextColumn();

				ImGui::PushItemWidth(0.f);
				if (ImGui::ColorEdit3(std::string("##Color" + std::to_string(i)).c_str(), &m_lights[i].first->color.x, ImGuiColorEditFlags_Float))
					m_renderHandler->updateLight(m_lights[i].first, (int)i);
				ImGui::PopItemWidth();
				ImGui::NextColumn();

				ImGui::Text("Intensity");
				ImGui::NextColumn();
				ImGui::PushItemWidth(0.f);
				if (ImGui::DragFloat(std::string("##Intensity" + std::to_string(i)).c_str(), &m_lights[i].first->intensity, 0.01f, 0.f, 200.f))
					m_renderHandler->updateLight(m_lights[i].first, (int)i);
				ImGui::PopItemWidth();
				if (m_lights[i].first->type != DIRECTIONAL_LIGHT)
				{
					ImGui::NextColumn();
					ImGui::Text("Position");
					ImGui::NextColumn();
					ImGui::PushItemWidth(0.f);
					if (ImGui::DragFloat3(std::string("##Position" + std::to_string(i)).c_str(), &m_lights[i].first->position.x, 0.1f))
						m_renderHandler->updateLight(m_lights[i].first, (int)i);
					ImGui::PopItemWidth();

					ImGui::NextColumn();
					ImGui::Text("Range");
					ImGui::NextColumn();
					ImGui::PushItemWidth(0.f);
					if (ImGui::DragFloat(std::string("##Range" + std::to_string(i)).c_str(), &m_lights[i].first->range, 0.01f, 0.f, 100.f))
						m_renderHandler->updateLight(m_lights[i].first, (int)i);
					ImGui::PopItemWidth();
				}

				if (m_lights[i].first->type == SPOT_LIGHT)
				{
					ImGui::NextColumn();
					ImGui::Text("Direction");
					ImGui::NextColumn();
					ImGui::PushItemWidth(0.f);
					if (ImGui::DragFloat3(std::string("##Direction" + std::to_string(i)).c_str(), &m_lights[i].second.rotationDeg.x, 1.f, -180.f, 180.f, "%.fdeg"))
					{
						XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(
							XMConvertToRadians(m_lights[i].second.rotationDeg.x),
							XMConvertToRadians(m_lights[i].second.rotationDeg.y),
							XMConvertToRadians(m_lights[i].second.rotationDeg.z));

						XMVECTOR rotQuatInverse = XMQuaternionInverse(rotQuat);
						XMVECTOR lightDir = XMQuaternionMultiply(rotQuat, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
						XMStoreFloat4(&m_lights[i].first->direction, XMQuaternionMultiply(lightDir, rotQuatInverse));
						
						m_renderHandler->updateLight(m_lights[i].first, (int)i);
					}
					ImGui::PopItemWidth();
				}

				if (m_lights[i].first->type == DIRECTIONAL_LIGHT)
				{
					ImGui::NextColumn();
					ImGui::Text("Direction");
					ImGui::NextColumn();
					ImGui::PushItemWidth(0.f);
					if (ImGui::DragFloat3(std::string("##Direction" + std::to_string(i)).c_str(), &m_lights[i].second.rotationDeg.x, 1.f, -180.f, 180.f, "%.fdeg"))
					{
						XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(
							XMConvertToRadians(m_lights[i].second.rotationDeg.x),
							XMConvertToRadians(m_lights[i].second.rotationDeg.y),
							XMConvertToRadians(m_lights[i].second.rotationDeg.z));

						XMVECTOR rotQuatInverse = XMQuaternionInverse(rotQuat);
						XMVECTOR lightDir = XMQuaternionMultiply(rotQuat, XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
						XMStoreFloat4(&m_lights[i].first->direction, XMQuaternionMultiply(lightDir, rotQuatInverse));

						//if (!(m_lights[i].first->direction.x == 0.f && m_lights[i].first->direction.y == 0.f && m_lights[i].first->direction.z == 0.f))
						{
							if (m_lights[i].first->isCastingShadow)
							{
								m_renderHandler->changeShadowMappingLight(
									m_lights[i].first,
									XMFLOAT3(
										XMConvertToRadians(m_lights[i].second.rotationDeg.x),
										XMConvertToRadians(m_lights[i].second.rotationDeg.y),
										XMConvertToRadians(m_lights[i].second.rotationDeg.z)),
									false);
							}
							m_renderHandler->updateLight(m_lights[i].first, (int)i);
						}
					}
					ImGui::PopItemWidth();

					ImGui::NextColumn();
					ImGui::Text("Casts Shadow");
					ImGui::NextColumn();
					if (ImGui::Checkbox(std::string("##Casts Shadow" + std::to_string(i)).c_str(), (bool*)(&m_lights[i].first->isCastingShadow)))
					{
						for (size_t j = 0; j < m_lights.size(); j++)
						{
							if (m_lights[j].first->type == DIRECTIONAL_LIGHT && j != i) // Not Current Directional Light
								m_lights[j].first->isCastingShadow = false;
						}
						m_renderHandler->changeShadowMappingLight(
							m_lights[i].first,
							XMFLOAT3(
								XMConvertToRadians(m_lights[i].second.rotationDeg.x),
								XMConvertToRadians(m_lights[i].second.rotationDeg.y),
								XMConvertToRadians(m_lights[i].second.rotationDeg.z)),
							!m_lights[i].first->isCastingShadow);
					}
				}
				ImGui::NextColumn();
			}
			ImGui::PopID();

			ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(.8f, .8f, .8f, 1.f));
			ImGui::Separator();
			ImGui::PopStyleColor();
		}
		ImGui::Columns(1);
		ImGui::PopStyleColor();
	}
	ImGui::EndChild(); // Whole Section
	ImGui::PopStyleColor();
	
	ImGui::End();

	// Render Texture Window, for Debug
	m_renderHandler->UIRenderShadowMap();
	m_renderHandler->UITonemappingWindow();
	m_renderHandler->UIRenderPipelineTexturesWindow();

	ImGui::ShowDemoWindow(); // For debugging
	ImGui::Render(); // Render ImGui(Runs at the end of RenderHandler render funtion!)

	// Controls
	controls(dt);

	// Camera
	m_camera.update(dt);
}