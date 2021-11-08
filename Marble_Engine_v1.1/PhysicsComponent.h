#include "pch.h"
#ifndef PHYSICSCOMPONENT_H
#define PHYSICSCOMPONENT_H

#include "MovementComponent.h"

const float GRAVITY = 0.982f;

class PhysicsComponent
{
private:
	XMFLOAT3 m_velocity;
	XMFLOAT3 m_oldVelocity;
	XMFLOAT3 m_acceleration;
	float m_accelMultiplier = 1.f;
	float m_accelSpeedUpMultiplier = 1.f;
	XMFLOAT3 m_deceleration;
	float m_decelMultiplier = 1.f;
	float m_maxSpeed;
	float m_mass;

	// Jump
	bool m_isJumping;
	bool m_isFalling;

	BoundingBox* m_aabb;

	// Movement Component
	MovementComponent* m_moveComp;

public:
	PhysicsComponent()
	{
		m_velocity = XMFLOAT3(0.f, 0.f, 0.f);
		m_acceleration = XMFLOAT3(0.f, 0.f, 0.f);
		m_deceleration = XMFLOAT3(0.f, 0.f, 0.f);
		m_mass = 0.f;
		m_isJumping = false;
		m_isFalling = true;
		m_aabb = nullptr;
		m_moveComp = nullptr;
	}
	PhysicsComponent(const PhysicsComponent& otherPhysicsComponent)
	{
		m_velocity = otherPhysicsComponent.m_velocity;
		m_acceleration = otherPhysicsComponent.m_acceleration;
		m_accelMultiplier = otherPhysicsComponent.m_accelMultiplier;
		m_deceleration = otherPhysicsComponent.m_deceleration;
		m_decelMultiplier = otherPhysicsComponent.m_decelMultiplier;
		m_maxSpeed = otherPhysicsComponent.m_maxSpeed;
		m_mass = otherPhysicsComponent.m_mass;
		m_isJumping = otherPhysicsComponent.m_isJumping;
		m_isFalling = otherPhysicsComponent.m_isFalling;

		// AABB
		if (m_aabb)
		{
			delete m_aabb;
			m_aabb = nullptr;
		}
		if (otherPhysicsComponent.m_aabb != nullptr)
			m_aabb = new BoundingBox(*otherPhysicsComponent.m_aabb);

		// Movement Component
		m_moveComp = otherPhysicsComponent.m_moveComp;
	}
	~PhysicsComponent()
	{
		if (m_aabb)
		{
			delete m_aabb;
			m_aabb = nullptr;
		}
	}

	PhysicsComponent& operator=(const PhysicsComponent& otherPhysicsComponent)
	{
		if (this == &otherPhysicsComponent)
			return *this;

		m_velocity = otherPhysicsComponent.m_velocity;
		m_acceleration = otherPhysicsComponent.m_acceleration;
		m_accelMultiplier = otherPhysicsComponent.m_accelMultiplier;
		m_deceleration = otherPhysicsComponent.m_deceleration;
		m_decelMultiplier = otherPhysicsComponent.m_decelMultiplier;
		m_maxSpeed = otherPhysicsComponent.m_maxSpeed;
		m_mass = otherPhysicsComponent.m_mass;
		m_isJumping = otherPhysicsComponent.m_isJumping;
		m_isFalling = otherPhysicsComponent.m_isFalling;

		// AABB
		if (m_aabb)
		{
			delete m_aabb;
			m_aabb = nullptr;
		}
		if (otherPhysicsComponent.m_aabb != nullptr)
			m_aabb = new BoundingBox(*otherPhysicsComponent.m_aabb);

		// Movement Component
		m_moveComp = otherPhysicsComponent.m_moveComp;

		return *this;
	}

	// Initialization
	void initialize(MovementComponent* moveComp, float mass = 1.f, XMFLOAT3 acceleration = XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3 deceleration = XMFLOAT3(0.f, 0.f, 0.f), float maxSpeed = 20.f)
	{
		m_moveComp = moveComp;
		m_mass = mass;
		m_acceleration = acceleration;
		m_deceleration = deceleration;
		m_maxSpeed = maxSpeed;
		m_aabb = new BoundingBox();
	}

	// Getters
	XMFLOAT3 getVelocity() const
	{
		return m_velocity;
	}
	XMFLOAT3 getAcceleration() const
	{
		return m_acceleration;
	}
	float getAccelMultiplier() const
	{
		return m_accelMultiplier;
	}
	BoundingBox* getAABBPtr()
	{
		return m_aabb;
	}
	bool getIsJumping() const
	{
		return m_isJumping;
	}
	bool getIsFalling() const
	{
		return m_isFalling;
	}

	// Setters
	void setBoundingBox(XMFLOAT3 center, XMFLOAT3 extends)
	{
		m_aabb->Center = center;
		m_aabb->Extents = extends;
	}
	void setVelocity(XMFLOAT3 newVelocity)
	{
		m_velocity = newVelocity;
	}
	void setAcceleration(XMFLOAT3 newAcceleration)
	{
		m_acceleration = newAcceleration;
	}
	void setAccelMultiplier(float newMultiplier)
	{
		m_accelMultiplier = newMultiplier;
	}
	void setIsJumping(bool isJumping)
	{
		m_isJumping = isJumping;
	}
	void setIsFalling(bool isFalling)
	{
		m_isFalling = isFalling;
	}

	void addForce(XMFLOAT3 force, float dt)
	{
		m_velocity = XMFLOAT3(m_velocity.x + force.x, m_velocity.y + force.y, m_velocity.z + force.z);
	}
	void addForceDir(Direction dir, float dt, float multiplier = 1.f)
	{
		XMVECTOR finalForce = XMVectorZero();

		switch (dir)
		{
		case Direction::FORWARD:
			finalForce = XMVectorScale(m_moveComp->forward, m_acceleration.z * m_accelMultiplier * multiplier) * dt;
			break;
		case Direction::BACKWARD:
			finalForce = XMVectorScale(m_moveComp->backward, m_acceleration.z * m_accelMultiplier * multiplier) * dt;
			break;
		case Direction::LEFT:
			finalForce = XMVectorScale(m_moveComp->left, m_acceleration.x * m_accelMultiplier * multiplier) * dt;
			break;
		case Direction::RIGHT:
			finalForce = XMVectorScale(m_moveComp->right, m_acceleration.x * m_accelMultiplier * multiplier) * dt;
			break;
		case Direction::UP:
			finalForce = XMVectorScale(m_moveComp->up, m_acceleration.y * m_accelMultiplier * multiplier) * dt;
			break;
		case Direction::DOWN:
			finalForce = XMVectorScale(m_moveComp->down, m_acceleration.y * m_accelMultiplier * multiplier) * dt;
			break;
		default:
			assert(!"Error, no valid direction found!");
			break;
		}

		finalForce += XMLoadFloat3(&m_velocity);

		XMFLOAT3 finalForceF3 = XMFLOAT3(0.f, 0.f, 0.f);
		XMStoreFloat3(&finalForceF3, finalForce);

		//m_velocity = XMFLOAT3(std::clamp(finalForceF3.x, -m_maxSpeed, m_maxSpeed), std::clamp(finalForceF3.y, -m_maxSpeed, m_maxSpeed), std::clamp(finalForceF3.z, -m_maxSpeed, m_maxSpeed));
		m_velocity = finalForceF3;
	}
	void addForceDir(XMVECTOR dir, float dt, float multiplier = 1.f)
	{
		XMVECTOR finalForce = XMVectorZero();


		finalForce = XMVectorScale(dir, m_acceleration.x * m_accelMultiplier * multiplier);

		XMFLOAT3 finalForceF3 = XMFLOAT3(0.f, 0.f, 0.f);
		XMStoreFloat3(&finalForceF3, finalForce);

		m_velocity = XMFLOAT3(m_velocity.x + finalForceF3.x, m_velocity.y + finalForceF3.y, m_velocity.z + finalForceF3.z);
	}
	void addGravity(float dt)
	{
		m_velocity.y += m_mass * -GRAVITY * dt;
	}

	void addYDecel(float dt)
	{
		m_velocity.y *= m_deceleration.y * dt;
	}
	float decelerate(float source, float target, float smoothing, double dt)
	{
		return lerpF(source, target, (float)(1.0 - std::pow(smoothing, dt)));
	}

	void jump(float accelerationMultipler, float dt)
	{
		if (!m_isJumping)
		{
			m_isJumping = true;
			m_isFalling = false;
			m_velocity.y += m_acceleration.y * accelerationMultipler;
		}
	}

	// Update
	void handleCollision(std::vector<BoundingBox*> boundingBoxes, float dt, std::vector<BoundingOrientedBox*> orientedBoundingBoxes)
	{
		BoundingBox AABBNextFrame = *(m_aabb);
		AABBNextFrame.Center = XMFLOAT3(m_aabb->Center.x + m_velocity.x * dt, m_aabb->Center.y + m_velocity.y * dt, m_aabb->Center.z + m_velocity.z * dt);

		BoundingBox xAABB = *(m_aabb);
		xAABB.Center = XMFLOAT3(m_aabb->Center.x + m_velocity.x * dt, m_aabb->Center.y, m_aabb->Center.z);

		BoundingBox yAABB = *(m_aabb);
		yAABB.Center = XMFLOAT3(m_aabb->Center.x, m_aabb->Center.y + m_velocity.y * dt, m_aabb->Center.z);

		BoundingBox zAABB = *(m_aabb);
		zAABB.Center = XMFLOAT3(m_aabb->Center.x, m_aabb->Center.y, m_aabb->Center.z + m_velocity.z * dt);

		for (size_t i = 0; i < boundingBoxes.size(); i++)
		{
			if (AABBNextFrame.Intersects(*boundingBoxes[i]))
			{
				if (xAABB.Intersects(*boundingBoxes[i]))
				{
					if (m_velocity.x > 0)
						m_moveComp->position = XMVectorSetX(m_moveComp->position, boundingBoxes[i]->Center.x - boundingBoxes[i]->Extents.x - AABBNextFrame.Extents.x - 0.0001f);
					else
						m_moveComp->position = XMVectorSetX(m_moveComp->position, boundingBoxes[i]->Center.x + boundingBoxes[i]->Extents.x + AABBNextFrame.Extents.x + 0.0001f);

					m_velocity.x = 0;
				}

				if (yAABB.Intersects(*boundingBoxes[i]))
				{
					if (m_velocity.y > 0)
						m_moveComp->position = XMVectorSetY(m_moveComp->position, boundingBoxes[i]->Center.y - boundingBoxes[i]->Extents.y - AABBNextFrame.Extents.y - 0.0001f);
					else
					{
						m_moveComp->position = XMVectorSetY(m_moveComp->position, boundingBoxes[i]->Center.y + boundingBoxes[i]->Extents.y + AABBNextFrame.Extents.y + 0.0001f);
						m_isJumping = false;
						m_isFalling = false;
					}

					m_velocity.y = 0;
				}
				else
				{
					m_isFalling = true;
				}

				if (zAABB.Intersects(*boundingBoxes[i]))
				{
					if (m_velocity.z > 0)
						m_moveComp->position = XMVectorSetZ(m_moveComp->position, boundingBoxes[i]->Center.z - boundingBoxes[i]->Extents.z - AABBNextFrame.Extents.z - 0.0001f);
					else
						m_moveComp->position = XMVectorSetZ(m_moveComp->position, boundingBoxes[i]->Center.z + boundingBoxes[i]->Extents.z + AABBNextFrame.Extents.z + 0.0001f);

					m_velocity.z = 0;
				}
			}
		}
	}

	void updatePosition(double dt, bool isCamera = false)
	{
		m_moveComp->position = XMVectorAdd(m_moveComp->position, XMLoadFloat3(&m_velocity));
		m_aabb->Center = m_moveComp->getPositionF3();

		m_moveComp->updateDirVectors();

		m_velocity.x = decelerate(m_velocity.x, 0, m_deceleration.x * m_decelMultiplier, dt);
		m_velocity.y = decelerate(m_velocity.y, 0, m_deceleration.y * m_decelMultiplier, dt);
		m_velocity.z = decelerate(m_velocity.z, 0, m_deceleration.z * m_decelMultiplier, dt);

		//m_oldVelocity = m_velocity;

		// X
		//if (m_velocity.x != 0.f)
		//{
		//	if (m_velocity.x > 0.f)
		//		m_velocity.x -= m_deceleration.x * m_decelMultiplier * dt;
		//	else
		//		m_velocity.x += m_deceleration.x * m_decelMultiplier * dt;

		//	// Avoid Direction Reversal from decelerating to much
		//	if (m_oldVelocity.x > 0 && m_velocity.x < 0)
		//		m_velocity.x = 0;
		//	else if (m_oldVelocity.x < 0 && m_velocity.x > 0)
		//		m_velocity.x = 0;
		//}
		//// Y
		//if (m_velocity.y != 0.f)
		//{
		//	if (m_velocity.y > 0.f)
		//		m_velocity.y -= m_deceleration.y * m_decelMultiplier * dt;
		//	else
		//		m_velocity.y += m_deceleration.y * m_decelMultiplier * dt;

		//	// Avoid Direction Reversal from decelerating to much
		//	if (m_oldVelocity.y > 0 && m_velocity.y < 0)
		//		m_velocity.y = 0;
		//	else if (m_oldVelocity.y < 0 && m_velocity.y > 0)
		//		m_velocity.y = 0;
		//}
		//// Z
		//if (m_velocity.z != 0.f)
		//{
		//	if (m_velocity.z > 0.f)
		//		m_velocity.z -= m_deceleration.z * m_decelMultiplier * dt;
		//	else
		//		m_velocity.z += m_deceleration.z * m_decelMultiplier * dt;

		//	// Avoid Direction Reversal from decelerating to much
		//	if (m_oldVelocity.z > 0 && m_velocity.z < 0)
		//		m_velocity.z = 0;
		//	else if (m_oldVelocity.z < 0 && m_velocity.z > 0)
		//		m_velocity.z = 0;
		//}

		/*m_velocity.x = lerpF(m_velocity.x, 0.f, m_deceleration.x * dt);
		m_velocity.y = lerpF(m_velocity.y, 0.f, m_deceleration.y * dt);
		m_velocity.z = lerpF(m_velocity.z, 0.f, m_deceleration.z * dt);*/
	}
	void updatePositionNoDecel(float dt, bool isCamera = false)
	{
		m_moveComp->position = XMVectorAdd(
			m_moveComp->position,
			XMVectorSet(
				m_velocity.x * dt,
				m_velocity.y * dt,
				m_velocity.z * dt, 1.f)
		);
		m_aabb->Center = m_moveComp->getPositionF3();

		m_moveComp->updateDirVectors();
	}

	void updateUI()
	{
		// Acceleration
		ImGui::PushItemWidth(90);
		ImGui::DragFloat("Acceleration", &m_accelMultiplier, 0.1f);
		// Deceleration
		ImGui::DragFloat("Deceleration", &m_decelMultiplier, 0.1f);
		// Max Speed
		//ImGui::DragFloat("Max Speed", &m_maxSpeed, 0.1f);
		ImGui::PopItemWidth();
	}
};

#endif // !PHYSICSCOMPONENT_H