#include "pch.h"
#include "GameObject.h"

GameObject::GameObject()
{
	m_id = 0;
	m_rotationOffset = XMFLOAT3(0.f, 0.f, 0.f);
	m_renderHandler = RenderHandler::getInstance();
	m_audioFileName = new char[10];
	for (size_t i = 0; i < 10; i++)
	{
		m_audioFileName[i] = '0';
	}
}

GameObject::~GameObject()
{
	if (m_renderKey.isValid())
		m_renderHandler->deleteRenderObject(m_renderKey);
}

void GameObject::initialize(std::string modelName, UINT id, ShaderStates shaderState, std::vector<MeshData>* meshData)
{
	m_id = id;
	m_modelName = modelName;
	m_movementComponent = std::make_unique<MovementComponent>();
	m_movementComponent->position = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	m_physicsComponent = std::make_unique<PhysicsComponent>();
	m_physicsComponent->initialize(m_movementComponent.get(), 10.f, XMFLOAT3(.1f, .1f, .1f), XMFLOAT3(.99f, .99f, .99f));

	m_shaderType = shaderState;
	m_renderKey = m_renderHandler->newRenderObject(modelName, shaderState, meshData);
}

void GameObject::setTextures(TexturePaths textures)
{
	m_renderHandler->setRenderObjectTextures(m_renderKey, textures);
}

void GameObject::setTextures(TexturePathsPBR textures)
{
	m_renderHandler->setRenderObjectTextures(m_renderKey, textures);
}

void GameObject::setMaterial(PS_MATERIAL_BUFFER material)
{
	m_renderHandler->setRenderObjectMaterial(m_renderKey, material);
}

void GameObject::setMaterial(PS_MATERIAL_PBR_BUFFER material)
{
	m_renderHandler->setRenderObjectMaterialPBR(m_renderKey, material);
}

void GameObject::fillMeshData(std::vector<MeshData>* meshes)
{
	m_renderHandler->fillMeshData(m_renderKey, meshes);
}

std::string GameObject::getModelName() const
{
	return m_modelName;
}

std::string GameObject::getModelNameAndId() const
{
	std::string label = m_modelName + ", " + std::to_string(m_id);
	if (m_modelName.empty())
		label = std::to_string(m_id);
	return label;
}

ShaderStates GameObject::getShaderType() const
{
	return (ShaderStates)m_shaderType;
}

XMVECTOR GameObject::getScale() const
{
	return m_movementComponent->scale;
}
XMFLOAT3 GameObject::getScaleF3() const
{
	XMFLOAT3 scale;
	XMStoreFloat3(&scale, m_movementComponent->scale);
	return scale;
}

XMVECTOR GameObject::getRotation() const
{
	return m_movementComponent->rotation;
}
XMFLOAT3 GameObject::getRotationF3() const
{
	XMFLOAT3 rotation;
	XMStoreFloat3(&rotation, m_movementComponent->rotation);
	return rotation;
}

XMVECTOR GameObject::getPosition() const
{
	return m_movementComponent->position;
}
XMFLOAT3 GameObject::getPositionF3() const
{
	XMFLOAT3 position;
	XMStoreFloat3(&position, m_movementComponent->position);
	return position;
}

RenderObjectKey GameObject::getKey() const
{
	return m_renderKey;
}

void GameObject::setScale(XMVECTOR newScale)
{
	m_movementComponent->scale = newScale;
}
void GameObject::setScale(XMFLOAT3 newScale)
{
	m_movementComponent->scale = XMLoadFloat3(&newScale);
}

void GameObject::setRotation(XMVECTOR newRotation)
{
	m_movementComponent->rotation = newRotation;
}
void GameObject::setRotation(XMFLOAT3 newRotation)
{
	m_movementComponent->rotation = XMLoadFloat3(&newRotation);
}

void GameObject::setPosition(XMVECTOR newPosition)
{
	m_movementComponent->position = newPosition;
}
void GameObject::setPosition(XMFLOAT3 newPosition)
{
	m_movementComponent->position = XMLoadFloat3(&newPosition);
}

void GameObject::update(double dt)
{
	// ImGui
	//ImGui::Text(getModelNameAndId().c_str());
	ImGui::PushID(m_id);
	if (ImGui::CollapsingHeader("Movement"))
	{
		ImGui::DragFloat3("Scale", &m_movementComponent->scale.m128_f32[0], 0.1f);
		ImGui::DragFloat3("Rotation", &m_movementComponent->rotation.m128_f32[0], 0.1f);
		ImGui::DragFloat3("Position", &m_movementComponent->position.m128_f32[0], 0.1f);
	}
	//if (ImGui::CollapsingHeader("Other Component"))
	//{
	//	ImGui::Checkbox("Audio Component", &m_audioComponent);
	//	ImGui::Checkbox("Looping", &m_loopingAudio);
	//	ImGui::InputText("Name", m_audioFileName, 5);
	//	//ImGui::SameLine();
	//	ImGui::DragFloat("Volume", &m_volumeAudio, 0.1f);

	//	ImGui::Text("____________________________");

	//	ImGui::Checkbox("Audio Component", &m_audioComponent);
	//	ImGui::Checkbox("Looping", &m_loopingAudio);
	//	ImGui::InputText("Name", m_audioFileName, 5);
	//	//ImGui::SameLine();
	//	ImGui::DragFloat("Volume", &m_volumeAudio, 0.1f);
	//}
	if (ImGui::BeginCombo("Shader State", ShaderStatesNames[m_shaderType]))
	{
		for (int n = 0; n < (int)ShaderStates::NUM; n++)
		{
			bool is_selected = (ShaderStatesNames[m_shaderType] == ShaderStatesNames[n]);
			if (ImGui::Selectable(ShaderStatesNames[n], is_selected))
			{
				m_shaderType = n;
				m_renderKey = m_renderHandler->setShaderState(m_renderKey, (ShaderStates)m_shaderType);
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	m_renderHandler->modelTextureUIUpdate(m_renderKey);

	ImGui::PopID();

	// Movement
	m_physicsComponent->updatePosition(dt);
	m_renderHandler->updateRenderObjectWorld(m_renderKey, m_movementComponent->getWorldMatrix());
}