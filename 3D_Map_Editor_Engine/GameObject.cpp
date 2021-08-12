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

void GameObject::initialize(std::string modelName, UINT id)
{
	m_id = id;
	m_modelName = modelName;
	m_movementComponent = std::make_unique<MovementComponent>();
	m_movementComponent->position = XMVectorSet(0.f, 0.f, 0.f, 1.f);

	m_physicsComponent = std::make_unique<PhysicsComponent>();
	m_physicsComponent->initialize(m_movementComponent.get(), 10.f, XMFLOAT3(.1f, .1f, .1f), XMFLOAT3(.99f, .99f, .99f));

	m_renderKey = m_renderHandler->newRenderObject(modelName);
}

void GameObject::setTextures(TexturePaths textures)
{
	m_renderHandler->setRenderObjectTextures(m_renderKey, textures);
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

void GameObject::update(float dt)
{
	// ImGui
	//ImGui::Text(getModelNameAndId().c_str());
	ImGui::PushID(m_id);
	if (ImGui::CollapsingHeader("Movement Component"))
	{
		ImGui::DragFloat3("Scale", &m_movementComponent->scale.m128_f32[0], 0.1f);
		ImGui::DragFloat3("Rotation", &m_movementComponent->rotation.m128_f32[0], 0.1f);
		ImGui::DragFloat3("Position", &m_movementComponent->position.m128_f32[0], 0.1f);
	}
	if (ImGui::CollapsingHeader("Other Component"))
	{
		ImGui::Checkbox("Audio Component", &m_audioComponent);
		ImGui::Checkbox("Looping", &m_loopingAudio);
		ImGui::InputText("Name", m_audioFileName, 5);
		//ImGui::SameLine();
		ImGui::DragFloat("Volume", &m_volumeAudio, 0.1f);

		ImGui::Text("____________________________");

		ImGui::Checkbox("Audio Component", &m_audioComponent);
		ImGui::Checkbox("Looping", &m_loopingAudio);
		ImGui::InputText("Name", m_audioFileName, 5);
		//ImGui::SameLine();
		ImGui::DragFloat("Volume", &m_volumeAudio, 0.1f);
	}
	ImGui::PopID();
	ImGui::NewLine();

	// Movement
	m_physicsComponent->updatePosition(dt);
	m_renderHandler->updateRenderObjectWorld(m_renderKey, m_movementComponent->getWorldMatrix());
}