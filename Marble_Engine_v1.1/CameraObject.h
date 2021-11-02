#ifndef CAMERAOBJECT_H
#define CAMERAOBJECT_H

#include "RenderHandler.h"
#include "InputHandler.h"
#include "MovementComponent.h"
#include "PhysicsComponent.h"

class CameraObject
{
private:
	// Camera Sensetivity
	float m_mouseSensitivity;

	// Components
	std::unique_ptr< MovementComponent > m_movementComponent;
	std::unique_ptr< PhysicsComponent > m_physicsComponent;

	// Renderer
	RenderHandler* m_renderHandler;

public:
	CameraObject();

	// Initialize
	void initialize(float mouseSensitivity, XMVECTOR startPosition = XMVectorSet(0.f, 5.f, -1.f, 1.f));

	// Getters
	XMFLOAT3 getPositionF3() const;
	float getAccelMultiplier() const;

	// Controlls
	void rotate(int mouseX, int mouseY);
	void resetPosAndRot();
	void setAccelMultiplier(float accel);
	void addForce(Direction direction, float dt, float multiplier = 1.f);

	// Update
	void update(float dt);
	void updateUI();
};

#endif // !CAMERAOBJECT_H