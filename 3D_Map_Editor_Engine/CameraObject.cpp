#include "pch.h"
#include "CameraObject.h"

CameraObject::CameraObject()
{
	m_mouseSensitivity = 0.1f;
	m_movementComponent = nullptr;
}

void CameraObject::initialize(float mouseSensitivity, XMVECTOR startPosition)
{
	m_mouseSensitivity = mouseSensitivity;

	m_movementComponent = std::make_unique<MovementComponent>();
	m_movementComponent->position = startPosition;

	m_physicsComponent = std::make_unique<PhysicsComponent>();
	m_physicsComponent->initialize(m_movementComponent.get(), 60.f, XMFLOAT3(.5f, .5f, .5f), XMFLOAT3(40.f, 40.f, 40.f), 20.f);
	
	m_renderHandler = RenderHandler::getInstance();
	m_renderHandler->updateCamera(m_movementComponent->position, m_movementComponent->rotation);
}

XMFLOAT3 CameraObject::getPositionF3() const
{
	return m_movementComponent->getPositionF3();
}

void CameraObject::rotate(int mouseX, int mouseY)
{
	// Rotate Camera
	XMFLOAT2 mouseDelta = XMFLOAT2((float)mouseX, (float)mouseY);

	// Set Pitch
	XMFLOAT3 rotationF3;
	XMStoreFloat3(&rotationF3, m_movementComponent->rotation);
	rotationF3.x += mouseDelta.y * m_mouseSensitivity;

	// Limit pitch to straight up or straight down with a little fudge-factor to avoid gimbal lock
	float limit = XM_PI / 2.0f - 0.01f;
	rotationF3.x = std::max(-limit, rotationF3.x);
	rotationF3.x = std::min(limit, rotationF3.x);

	// Set Yaw
	rotationF3.y += mouseDelta.x * m_mouseSensitivity;

	// Keep longitude in sane range by wrapping
	if (rotationF3.x > XM_PI)
		rotationF3.y -= XM_PI * 2.0f;
	else if (rotationF3.x < -XM_PI)
		rotationF3.y += XM_PI * 2.0f;

	// Update Movement Component
	m_movementComponent->rotation = XMLoadFloat3(&rotationF3);
	m_movementComponent->updateDirVectors();
}

void CameraObject::resetPosAndRot()
{
	m_movementComponent->position = XMVectorSet(0.f, 5.f, -1.f, 1.f);
	m_movementComponent->rotation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
	m_movementComponent->updateDirVectors();
	m_physicsComponent->setVelocity({ 0.f, 0.f, 0.f });
}

void CameraObject::addForce(Direction direction, float dt)
{
	m_physicsComponent->addForceDir(direction, dt);
}

void CameraObject::update(float dt)
{
	m_physicsComponent->updatePosition(dt);
	m_renderHandler->updateCamera(m_movementComponent->position, m_movementComponent->rotation);
}

void CameraObject::updateUI()
{
	ImGui::PushID("camera");
	m_physicsComponent->updateUI();
	ImGui::PopID();
}
