#ifndef MOVEMENTCOMPONENT_H
#define MOVEMENTCOMPONENT_H

#include "pch.h"

enum class Direction { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class MovementComponent
{
public:
	// World Matrix Vectors
	XMVECTOR scale;
	XMVECTOR rotation;
	XMVECTOR localRotation;
	XMVECTOR position;

	// Speed
	float speed;

	// Direction Vectors
	XMVECTOR forward;
	XMVECTOR left;
	XMVECTOR right;
	XMVECTOR backward;
	XMVECTOR up;
	XMVECTOR down;

	// Consructor & Destructor
	MovementComponent()
	{
		scale = XMVectorSet(1.f, 1.f, 1.f, 0.f);
		rotation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		localRotation = XMVectorSet(0.f, 0.f, 0.f, 1.f);
		position = XMVectorSet(0.f, 0.f, 0.f, 1.f);

		speed = 0.01f;

		forward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
		left = XMVectorSet(-1.f, 0.f, 0.f, 0.f);
		right = XMVectorSet(1.f, 0.f, 0.f, 0.f);
		backward = XMVectorSet(0.f, 0.f, -1.f, 0.f);
		up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		down = XMVectorSet(0.f, -1.f, 0.f, 0.f);
	}

	// Getters
	XMFLOAT3 getPositionF3() const
	{
		XMFLOAT3 positionF3;
		XMStoreFloat3(&positionF3, position);
		return positionF3;
	}
	XMMATRIX getWorldMatrix() const
	{
		return DirectX::XMMATRIX(
			XMMatrixScalingFromVector(scale) *
			XMMatrixRotationRollPitchYawFromVector(localRotation) *
			XMMatrixRotationRollPitchYawFromVector(rotation) *
			XMMatrixTranslationFromVector(position));
	}

	// Update
	void move(XMVECTOR moveVector)
	{
		position = XMVectorAdd(moveVector, position);
	}
	void move(Direction direction)
	{
		if (direction == Direction::FORWARD)
			position = XMVectorAdd(forward * speed, position);
		else if (direction == Direction::BACKWARD)
			position = XMVectorAdd(backward * speed, position);
		else if (direction == Direction::LEFT)
			position = XMVectorAdd(left * speed, position);
		else if (direction == Direction::RIGHT)
			position = XMVectorAdd(right * speed, position);
	}
	void updateDirVectors()
	{
		XMMATRIX rotationMatrix = XMMatrixRotationY(XMVectorGetY(rotation));
		forward = XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 1.f, 0.f), rotationMatrix);
		left = XMVector3TransformCoord(XMVectorSet(-1.f, 0.f, 0.f, 0.f), rotationMatrix);
		right = XMVector3TransformCoord(XMVectorSet(1.f, 0.f, 0.f, 0.f), rotationMatrix);
		backward = XMVector3TransformCoord(XMVectorSet(0.f, 0.f, -1.f, 0.f), rotationMatrix);
	}
};

#endif // MOVEMENTCOMPONENT_H